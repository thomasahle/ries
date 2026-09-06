// src/workers/ries.worker.js
//
// Runs RIES inside a Web Worker. The WASM binary is compiled once and every
// calculation gets a fresh Emscripten module instance that is discarded when
// the calculation ends: RIES's main() allocates and never frees (about 12 MB
// per run), so reusing one instance leaks until the tab runs out of memory.
// Requests are processed strictly one at a time; callMain() is synchronous and
// blocks this thread, so the main thread owns the only timeout.

// Get the base URL from the worker's location
const getBasePath = () => {
  // For GitHub Pages deployment at /ries/
  if (self.location.pathname.includes('/ries/')) {
    return '/ries/';
  }
  // Local development
  return '/';
};

const basePath = getBasePath();

// Load the RIES initializer (createRIESModule) into the worker.
importScripts(`${basePath}ries.js`);

// ries.c exits with EXIT_NO_ERROR when a search ends early on success
// (e.g. an exact match was found); it is not a failure.
const RIES_EXIT_NO_ERROR = -9998;

// Cached promise for the compiled WASM module, shared by all instances.
let wasmModulePromise = null;

// Requests waiting to run, in arrival order.
const pendingRequests = [];
let processing = false;

/**
 * Fetch and compile ries.wasm once.
 * @returns {Promise<WebAssembly.Module>} The compiled (not instantiated) module
 */
function getWasmModule() {
  if (!wasmModulePromise) {
    wasmModulePromise = fetch(`${basePath}ries.wasm`, { credentials: 'same-origin' })
      .then((response) => {
        if (!response.ok) {
          throw new Error(`Failed to load ries.wasm (HTTP ${response.status})`);
        }
        return response.arrayBuffer();
      })
      .then((bytes) => WebAssembly.compile(bytes));
    // Let the next request try again if loading failed (e.g. a network hiccup)
    wasmModulePromise.catch(() => {
      wasmModulePromise = null;
    });
  }
  return wasmModulePromise;
}

/**
 * Create a fresh RIES module instance backed by the shared compiled module.
 * @param {WebAssembly.Module} wasmModule - The compiled RIES module
 * @param {{stdout: string, stderr: string}} output - Receives everything RIES prints
 * @returns {Promise<Object>} The initialized Emscripten module instance
 */
function createInstance(wasmModule, output) {
  let rejectInstantiation;
  const instantiationFailed = new Promise((_, reject) => {
    rejectInstantiation = reject;
  });

  const ready = createRIESModule({
    thisProgram: 'ries', // argv[0], which RIES prefixes to its own error messages
    instantiateWasm: (imports, onInstantiated) => {
      WebAssembly.instantiate(wasmModule, imports)
        .then((instance) => onInstantiated(instance, wasmModule), rejectInstantiation);
      return {};
    },
    print: (text) => {
      output.stdout += text + '\n';
    },
    printErr: (text) => {
      output.stderr += text + '\n';
    },
  });

  // Emscripten never settles its ready promise when instantiateWasm fails,
  // so race it against our own rejection.
  return Promise.race([ready, instantiationFailed]);
}

/**
 * Run one RIES calculation in a fresh instance.
 * @param {string} targetValue - The value to find equations for
 * @param {string[]} args - RIES command line flags (without the target value)
 * @returns {Promise<{rawOutput: string, stderr: string, exitCode: number}>}
 */
async function runCalculation(targetValue, args) {
  const wasmModule = await getWasmModule();
  const output = { stdout: '', stderr: '' };
  const instance = await createInstance(wasmModule, output);

  // Use provided args if available, or fall back to default formatting
  const commandArgs = args.length > 0 ? [...args, targetValue] : ['-F3', targetValue];

  let exitCode;
  try {
    // Synchronous: blocks this thread until RIES finishes. With EXIT_RUNTIME
    // off callMain returns the exit code; other Emscripten configurations
    // throw an ExitStatus instead, so handle both.
    exitCode = instance.callMain(commandArgs);
  } catch (err) {
    if (err && err.name === 'ExitStatus') {
      exitCode = err.status;
    } else {
      throw err;
    }
  }
  if (exitCode === RIES_EXIT_NO_ERROR) {
    exitCode = 0;
  }

  return { rawOutput: output.stdout, stderr: output.stderr, exitCode };
}

/**
 * Run queued requests one at a time so two calculations can never overlap.
 */
async function processQueue() {
  processing = true;
  try {
    while (pendingRequests.length > 0) {
      const { id, targetValue, args = [] } = pendingRequests.shift();
      try {
        const result = await runCalculation(targetValue, args);
        postMessage({ id, result });
      } catch (err) {
        postMessage({ id, error: (err && err.message) || String(err) });
      }
    }
  } finally {
    processing = false;
  }
}

/**
 * Handle messages from the main thread
 */
onmessage = (e) => {
  pendingRequests.push(e.data);
  if (!processing) {
    processQueue();
  }
};
