// src/workers/riesWorkerClient.js

// Constants
const MAX_HEARTBEAT_SILENCE = 5000; // 5 seconds without heartbeat is considered failure
const MEMORY_READINGS_MAX = 5; // Keep last 5 readings
const MEMORY_GROWTH_THRESHOLD = 20; // 20% growth between readings is suspicious
const HEARTBEAT_CHECK_INTERVAL = 1000; // Check heartbeat every second
const RETRY_DELAY = 100; // Regular retry delay
const CRITICAL_RETRY_DELAY = 500; // Longer delay for critical errors

// Memory error patterns to detect in global errors
const MEMORY_ERROR_PATTERNS = [
  'memory access out of bounds',
  'out of memory',
  'heap',
  'allocation'
];

// Singleton worker instance
let worker = null;
const callbacks = new Map();
let msgId = 0;
let lastHeartbeatTime = 0;
let heartbeatWatchdog = null;
let memoryReadings = [];

/**
 * Log memory statistics if available
 * @param {string} label - Label for the log message
 * @returns {object|null} Memory stats object or null if not available
 */
function logMemoryStats(label) {
  if (window.performance && window.performance.memory) {
    const memUsed = Math.round(window.performance.memory.usedJSHeapSize / (1024 * 1024));
    const memTotal = Math.round(window.performance.memory.totalJSHeapSize / (1024 * 1024));
    const memLimit = Math.round(window.performance.memory.jsHeapSizeLimit / (1024 * 1024));
    const usagePercent = Math.round(memUsed/memTotal*100);
    
    console.debug(`📊 Memory ${label}: ${memUsed}MB/${memTotal}MB/${memLimit}MB (${usagePercent}% usage)`);
    return { used: memUsed, total: memTotal, limit: memLimit, percent: usagePercent };
  }
  return null;
}

/**
 * Reset and recreate the worker
 */
function resetWorker() {
  console.warn('🔄 Resetting RIES worker due to potential memory issues');
  logMemoryStats('before reset');
  
  // Clear the watchdog timer
  if (heartbeatWatchdog) {
    clearInterval(heartbeatWatchdog);
    heartbeatWatchdog = null;
    console.debug('🛑 Heartbeat watchdog cleared');
  }
  
  // Terminate the existing worker if it exists
  if (worker) {
    try {
      console.debug('🔫 Terminating current worker');
      worker.terminate();
      console.debug('✅ Worker terminated successfully');
    } catch (e) {
      console.error('❌ Failed to terminate worker:', e);
    }
    worker = null;
  }
  
  // Force garbage collection if the browser supports it
  if (window.gc) {
    try {
      console.debug('🧹 Attempting to force garbage collection');
      window.gc();
    } catch (e) {
      console.debug('⚠️ Garbage collection not available');
    }
  }
  
  console.debug('⏳ Creating new worker instance');
  
  // Create a new worker
  worker = new Worker(new URL('./ries.worker.js', import.meta.url));
  setupWorkerHandlers();
  console.debug('✅ New worker created and handlers set up');
}

/**
 * Retry a calculation request
 * @param {number} id - The request ID
 * @param {object} cb - The callback object
 * @param {number} delay - Delay before retry in ms
 * @param {boolean} isCritical - Whether this is a critical error retry
 */
function retryRequest(id, cb, delay = RETRY_DELAY, isCritical = false) {
  if (!cb.hasRetried) {
    console.log(`🔄 ${isCritical ? 'Critical recovery' : 'Retrying'} for request ${id}`);
    callbacks.set(id, {...cb, hasRetried: true});
    
    setTimeout(() => {
      // Include original args if they exist when retrying 
      worker.postMessage({ 
        id, 
        targetValue: cb.targetValue,
        args: cb.options ? [
          // Reconstruct args from options
          ...(cb.options.neverUseSymbols ? [`-N${cb.options.neverUseSymbols}`] : []),
          ...(cb.options.onlyOneSymbols ? [`-O${cb.options.onlyOneSymbols}`] : []),
          ...(cb.options.onlyUseSymbols ? [`-S${cb.options.onlyUseSymbols}`] : []),
          ...(cb.options.solutionType ? [`-${cb.options.solutionType}`] : []),
          ...(cb.options.solveForX ? ['-s'] : []),
          '-F3' // Always use default formatting
        ] : ["-F3"],
        retry: true 
      });
    }, delay);
  } else {
    console.warn(`❌ Already retried request ${id}, giving up`);
    const errorMsg = isCritical ? 
      "Critical memory error occurred after retry" : 
      "Calculation failed after retry, likely due to memory limitations";
    cb.reject(new Error(errorMsg));
    callbacks.delete(id);
  }
}

/**
 * Process pending callbacks after a worker reset
 * @param {Map} pendingCallbacks - Map of pending callbacks
 * @param {boolean} isCritical - Whether this is a critical error situation
 */
function processPendingCallbacks(pendingCallbacks, isCritical = false) {
  pendingCallbacks.forEach((cb, id) => {
    retryRequest(id, cb, isCritical ? CRITICAL_RETRY_DELAY : RETRY_DELAY, isCritical);
  });
}

/**
 * Set up event handlers for the worker
 */
function setupWorkerHandlers() {
  // Handle messages from the worker
  worker.onmessage = (event) => {
    const { id, result, error, memoryError, type, timestamp } = event.data;
    
    // Handle heartbeat messages
    if (type === 'heartbeat') {
      lastHeartbeatTime = timestamp;
      return;
    }
    
    const cb = callbacks.get(id);
    if (!cb) return;
    
    if (memoryError) {
      console.log("📉 RIES calculation failed due to memory constraints, restarting worker...");
      
      // If we get a memory error, restart the worker completely and retry
      resetWorker();
      
      // After reset, retry the request
      retryRequest(id, cb);
    } else if (error) {
      cb.reject(new Error(error));
      callbacks.delete(id);
    } else {
      cb.resolve(result);
      callbacks.delete(id);
    }
  };
  
  // Handle worker errors
  worker.onerror = (err) => {
    console.error("🔥 Worker error detected:", err);
    
    // Reset the worker and get callbacks before reset
    const pendingCallbacks = new Map(callbacks);
    resetWorker();
    
    // Process all pending callbacks
    processPendingCallbacks(pendingCallbacks);
  };
  
  // Handle worker uncaught errors
  if (window.addEventListener) {
    window.addEventListener('error', function(event) {
      // Check if this is a memory error from the worker
      if (event && event.message && 
          MEMORY_ERROR_PATTERNS.some(pattern => event.message.includes(pattern)) && 
          event.filename && event.filename.includes('riesWorkerClient.js')) {
        
        console.error(`🚨 Critical memory error detected: ${event.message}`);
        
        // Get all pending callbacks before reset
        const pendingCallbacks = new Map(callbacks);
        
        // Force a complete reset of the worker
        resetWorker();
        
        // Process callbacks with critical error handling
        processPendingCallbacks(pendingCallbacks, true);
        
        return true; // Prevent default error handling
      }
      return false; // Let other errors propagate normally
    }, true);
  }
}

/**
 * Add a memory reading and check for concerning patterns
 * @returns {boolean} true if there's a concerning memory pattern
 */
function trackMemoryUsage() {
  if (!(window.performance && window.performance.memory)) {
    return false; // Can't track without the API
  }

  const currentMemory = {
    timestamp: Date.now(),
    used: window.performance.memory.usedJSHeapSize,
    total: window.performance.memory.totalJSHeapSize,
    limit: window.performance.memory.jsHeapSizeLimit
  };
  
  // Add to readings and keep only the most recent ones
  memoryReadings.push(currentMemory);
  if (memoryReadings.length > MEMORY_READINGS_MAX) {
    memoryReadings.shift();
  }
  
  // Need at least 2 readings to detect trends
  if (memoryReadings.length < 2) {
    return false;
  }
  
  // Calculate memory growth rate
  const oldestReading = memoryReadings[0];
  const newestReading = memoryReadings[memoryReadings.length - 1];
  
  // Calculate percentage growth
  const memoryGrowthPercent = ((newestReading.used - oldestReading.used) / oldestReading.used) * 100;
  const usagePercent = (newestReading.used / newestReading.total) * 100;
  
  // Only log when there's significant memory activity
  if (Math.abs(memoryGrowthPercent) > 5 || usagePercent > 60) {
    console.debug(`📈 Memory trend: ${memoryGrowthPercent.toFixed(1)}% growth, current usage: ${usagePercent.toFixed(1)}%`);
  }
  
  // Check if we're seeing rapid memory growth or very high usage
  if (memoryGrowthPercent > MEMORY_GROWTH_THRESHOLD && usagePercent > 70) {
    console.warn(`⚠️ Concerning memory pattern detected: ${memoryGrowthPercent.toFixed(1)}% growth with ${usagePercent.toFixed(1)}% usage`);
    return true;
  }
  
  // Check if we're near heap limit
  const percentOfLimit = (newestReading.used / newestReading.limit) * 100;
  if (percentOfLimit > 80) {
    console.warn(`⚠️ Approaching memory limit: ${percentOfLimit.toFixed(1)}% of maximum heap used`);
    return true;
  }
  
  return false;
}

/**
 * Start watching for heartbeats from the worker
 */
function startHeartbeatWatchdog() {
  console.debug('🐕 Starting heartbeat watchdog');
  
  // Clear any existing watchdog
  if (heartbeatWatchdog) {
    clearInterval(heartbeatWatchdog);
  }
  
  lastHeartbeatTime = Date.now();
  memoryReadings = []; // Reset memory readings
  
  // Set up the watchdog timer
  heartbeatWatchdog = setInterval(() => {
    const timeSinceLastHeartbeat = Date.now() - lastHeartbeatTime;
    
    // Only log if it's been a while since the last heartbeat
    if (timeSinceLastHeartbeat > 2000) {
      console.debug(`❗ Heartbeat check: ${timeSinceLastHeartbeat}ms since last heartbeat`);
    }
    
    // Check memory usage patterns
    const memoryConcern = trackMemoryUsage();
    
    // If we haven't received a heartbeat in too long or memory pattern is concerning, assume issues
    if (timeSinceLastHeartbeat > MAX_HEARTBEAT_SILENCE || memoryConcern) {
      if (timeSinceLastHeartbeat > MAX_HEARTBEAT_SILENCE) {
        console.warn(`⚠️ Worker hasn't sent a heartbeat in ${timeSinceLastHeartbeat}ms, resetting...`);
      } else if (memoryConcern) {
        console.warn(`⚠️ Resetting worker due to concerning memory usage pattern`);
      }
      
      // Get all pending callbacks before resetting
      const pendingCallbacks = new Map(callbacks);
      
      // Reset the worker
      resetWorker();
      
      // Process all pending callbacks
      processPendingCallbacks(pendingCallbacks);
    }
  }, HEARTBEAT_CHECK_INTERVAL);
}

/**
 * Initialize the worker once and reuse it
 */
function initWorker() {
  if (worker) return;
  
  // Create the worker
  worker = new Worker(new URL('./ries.worker.js', import.meta.url));
  setupWorkerHandlers();
  startHeartbeatWatchdog();
}

/**
 * Manually restart the RIES module
 * @returns {Promise} Promise that resolves when the module has been restarted
 */
export function restartRiesModule() {
  initWorker();
  
  return new Promise((resolve, reject) => {
    const id = msgId++;
    callbacks.set(id, { resolve, reject });
    worker.postMessage({ id, command: 'restart' });
  });
}

/**
 * Send a calculation request to the RIES worker
 * @param {string} targetValue - The value to find equations for
 * @param {Object} options - Options to pass to RIES
 * @returns {Promise} - Promise that resolves with the calculation results
 */
export function sendRiesRequest(targetValue, options = {}) {
  initWorker();
  
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
    const id = msgId++;
    callbacks.set(id, { 
      resolve, 
      reject,
      targetValue, // Store the target value so we can retry if needed
      hasRetried: false, // Track if we've already retried this request
      options // Store options for potential retry
    });
    
    // Pass both the target value and arguments to the worker
    worker.postMessage({ 
      id, 
      targetValue,
      args 
    });
  });
}
