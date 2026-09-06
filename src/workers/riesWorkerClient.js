// src/workers/riesWorkerClient.js
//
// Main-thread client for the RIES worker. Exactly one request is in flight at
// a time and at most one more is queued: the worker cannot interrupt a running
// calculation, so a newer request supersedes the queued one instead of waiting
// behind it. The only safety net is a per-request timeout that terminates and
// recreates the worker.

import { searchLevelArg } from '../utils/searchLevels';

// Constants
const REQUEST_TIMEOUT = 60000; // 60-second timeout per calculation

// Result resolved for a request that was replaced by a newer one before it ran
const SUPERSEDED_RESULT = Object.freeze({ superseded: true, rawOutput: '', stderr: '', exitCode: null });

// Singleton worker instance
let worker = null;
let msgId = 0;
let inFlight = null; // { id, targetValue, args, resolve, reject, timer }
let queued = null; // { targetValue, args, resolve, reject }

/**
 * Create the worker and wire up its handlers
 * @returns {Worker} The new worker
 */
function createWorker() {
  const created = new Worker(new URL('./ries.worker.js', import.meta.url));

  // Handle messages from the worker
  created.onmessage = (event) => {
    const { id, result, error } = event.data;
    if (!inFlight || inFlight.id !== id) return;

    const request = inFlight;
    inFlight = null;
    clearTimeout(request.timer);

    if (error) {
      request.reject(new Error(error));
    } else {
      request.resolve(result);
    }
    dispatchNext();
  };

  // The worker script itself failed (e.g. ries.js could not be loaded). This
  // cannot happen during a calculation because callMain blocks the worker.
  created.onerror = (event) => {
    const message = event && event.message
      ? `RIES worker failed: ${event.message}`
      : 'RIES worker failed to start';
    console.error(message);
    failInFlight(new Error(message));
  };

  return created;
}

/**
 * Discard the current worker and reject the in-flight request, if any
 * @param {Error} error - The error the in-flight request is rejected with
 */
function failInFlight(error) {
  const request = inFlight;
  inFlight = null;

  if (worker) {
    worker.terminate();
    worker = null;
  }

  if (request) {
    clearTimeout(request.timer);
    request.reject(error);
  }
  dispatchNext();
}

/**
 * Send the queued request to the worker if nothing is in flight
 */
function dispatchNext() {
  if (inFlight || !queued) return;

  const request = queued;
  queued = null;

  if (!worker) {
    worker = createWorker();
  }

  const id = msgId++;
  const timer = setTimeout(() => {
    if (!inFlight || inFlight.id !== id) return;
    failInFlight(new Error(
      `RIES timed out after ${REQUEST_TIMEOUT / 1000} seconds computing ${request.targetValue}`
    ));
  }, REQUEST_TIMEOUT);

  inFlight = { ...request, id, timer };
  worker.postMessage({ id, targetValue: request.targetValue, args: request.args });
}

/**
 * Send a calculation request to the RIES worker
 * @param {string} targetValue - The value to find equations for
 * @param {Object} options - Options to pass to RIES
 * @returns {Promise<{rawOutput: string, stderr: string, exitCode: number|null, superseded?: boolean}>}
 *   rawOutput is RIES's stdout; superseded is set when a newer request replaced this one before it ran
 */
export function sendRiesRequest(targetValue, options = {}) {
  // Build RIES command arguments
  const args = [];

  // Add arguments based on options
  if (options) {
    // Add never use symbols (-N)
    if (options.neverUseSymbols) {
      args.push(`-N${options.neverUseSymbols}`);
    }

    // Add only use once symbols (-O)
    if (options.onlyOneSymbols) {
      args.push(`-O${options.onlyOneSymbols}`);
    }

    // Add only use these symbols (-S)
    if (options.onlyUseSymbols) {
      args.push(`-S${options.onlyUseSymbols}`);
    }

    // Add solution type constraint if specified
    if (options.solutionType) {
      args.push(`-${options.solutionType}`);
    }

    // Add the search depth (-l); the default level needs no flag
    const levelArg = searchLevelArg(options.searchLevel);
    if (levelArg) {
      args.push(levelArg);
    }

    // Add solve-for-x flag if enabled
    if (options.solveForX) {
      args.push('-s');
    }
  }

  // Always include the default formatting option
  if (!args.some(arg => arg.startsWith('-F'))) {
    args.push('-F3');
  }

  return new Promise((resolve, reject) => {
    // Latest request wins: a request still waiting for the worker is dropped
    if (queued) {
      queued.resolve(SUPERSEDED_RESULT);
    }
    queued = { targetValue, args, resolve, reject };
    dispatchNext();
  });
}
