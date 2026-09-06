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
 * @param {{text: string}} output - Receives everything RIES prints
 * @returns {Promise<Object>} The initialized Emscripten module instance
 */
function createInstance(wasmModule, output) {
  let rejectInstantiation;
  const instantiationFailed = new Promise((_, reject) => {
    rejectInstantiation = reject;
  });

  const ready = createRIESModule({
    instantiateWasm: (imports, onInstantiated) => {
      WebAssembly.instantiate(wasmModule, imports)
        .then((instance) => onInstantiated(instance, wasmModule), rejectInstantiation);
      return {};
    },
    print: (text) => {
      output.text += text + '\n';
    },
    printErr: (text) => {
      output.text += 'Error: ' + text + '\n';
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
 * @returns {Promise<{rawOutput: string}>}
 */
async function runCalculation(targetValue, args) {
  const wasmModule = await getWasmModule();
  const output = { text: '' };
  const instance = await createInstance(wasmModule, output);

  // Use provided args if available, or fall back to default formatting
  const commandArgs = args.length > 0 ? [...args, targetValue] : ['-F3', targetValue];

  // Synchronous: blocks this thread until RIES finishes.
  instance.callMain(commandArgs);

  return { rawOutput: output.text };
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
