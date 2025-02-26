// src/workers/riesWorkerClient.js

let worker = null;
const callbacks = new Map();
let msgId = 0;

function initWorker() {
  if (!worker) {
    worker = new Worker(new URL('./ries.worker.js', import.meta.url));
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
    worker.onerror = (err) => {
      // In case of a worker-level error, reject all pending promises.
      callbacks.forEach(cb => cb.reject(err));
      callbacks.clear();
    };
  }
}

export function sendRiesRequest(targetValue) {
  initWorker();
  return new Promise((resolve, reject) => {
    const id = msgId++;
    callbacks.set(id, { resolve, reject });
    worker.postMessage({ id, targetValue });
  });
}
