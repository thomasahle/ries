// src/workers/ries.worker.js

// Load the RIES initializer (createRIESModule) into the worker.
importScripts('/ries.js');

// Cache the RIES module instance so it isn't reinitialized on every call.
let riesModuleInstance = null;
let capturedOutput = "";

async function initRiesModule() {
  if (riesModuleInstance) return riesModuleInstance;
  riesModuleInstance = await createRIESModule({
    locateFile: (path, prefix) => {
      // When the WASM file is requested, force the correct URL
      if (path.endsWith('.wasm')) {
        return '/ries.wasm';
      }
      return prefix + path;
    },
    print: (text) => { capturedOutput += text + "\n"; },
    printErr: (text) => console.error("RIES error:", text)
  });
  return riesModuleInstance;
}

onmessage = async (e) => {
  const { id, targetValue } = e.data;
  try {
    const moduleInstance = await initRiesModule();
    capturedOutput = "";
    const originalPrint = moduleInstance.print;
    moduleInstance.print = (text) => { capturedOutput += text + "\n"; };

    moduleInstance.callMain(["-F3", targetValue]);
    moduleInstance.print = originalPrint;
    
    postMessage({ id, result: { rawOutput: capturedOutput } });
  } catch (error) {
    postMessage({ id, error: error.message });
  }
};
