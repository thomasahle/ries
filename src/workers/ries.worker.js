// src/workers/ries.worker.js

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

// Cache the RIES module instance so it isn't reinitialized on every call.
let riesModuleInstance = null;
let capturedOutput = "";

// Initialize the RIES module once and reuse it
async function initRiesModule() {
  if (riesModuleInstance) return riesModuleInstance;
  
  riesModuleInstance = await createRIESModule({
    locateFile: (path) => path.endsWith('.wasm') ? `${basePath}ries.wasm` : path,
    print: (text) => { capturedOutput += text + "\n"; },
    printErr: (text) => console.error("RIES error:", text)
  });
  
  return riesModuleInstance;
}

onmessage = async (e) => {
  const { id, targetValue } = e.data;
  try {
    // Get or initialize the module
    const moduleInstance = await initRiesModule();
    
    // Reset output capture
    capturedOutput = "";
    
    // Run RIES with the target value
    moduleInstance.callMain(["-F3", targetValue]);
    
    // Send the captured output back to the main thread
    postMessage({ id, result: { rawOutput: capturedOutput } });
  } catch (error) {
    postMessage({ id, error: error.message });
  }
};
