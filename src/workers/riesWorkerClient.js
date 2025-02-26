// src/workers/riesWorkerClient.js

// Singleton worker instance
let worker = null;
const callbacks = new Map();
let msgId = 0;

/**
 * Initialize the worker once and reuse it
 */
function initWorker() {
  if (worker) return;
  
  // Create the worker
  worker = new Worker(new URL('./ries.worker.js', import.meta.url));
  
  // Handle messages from the worker
  worker.onmessage = (event) => {
    const { id, result, error } = event.data;
    const cb = callbacks.get(id);
    
    if (cb) {
      if (error) {
        cb.reject(new Error(error));
      } else {
        cb.resolve(result);
      }
      callbacks.delete(id);
    }
  };
  
  // Handle worker errors
  worker.onerror = (err) => {
    // In case of a worker-level error, reject all pending promises
    callbacks.forEach(cb => cb.reject(err));
    callbacks.clear();
  };
}

/**
 * Send a calculation request to the RIES worker
 * @param {string} targetValue - The value to find equations for
 * @returns {Promise} - Promise that resolves with the calculation results
 */
export function sendRiesRequest(targetValue) {
  initWorker();
  
  return new Promise((resolve, reject) => {
    const id = msgId++;
    callbacks.set(id, { resolve, reject });
    worker.postMessage({ id, targetValue });
  });
}
