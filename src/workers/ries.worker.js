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

// Constants
const TIMEOUT_DURATION = 60000; // 60-second timeout for calculations
const HEARTBEAT_INTERVAL = 1000; // Send heartbeat every second

// Memory-related error patterns
const MEMORY_ERROR_PATTERNS = [
  "out of memory", 
  "memory allocation",
  "allocation failed",
  "memory access",
  "access out of bounds",
  "memory out of bounds",
  "heap limit",
  "heap maximum size reached"
];

// Cache the RIES module instance so it isn't reinitialized on every call.
let riesModuleInstance = null;
let capturedOutput = "";

// Track execution state with timeouts and heartbeats
let calculationRunning = false;
let lastHeartbeat = 0;
let heartbeatInterval = null;
let timeoutTimer = null;

/**
 * Log memory statistics if available
 * @param {string} label - Label for the log message
 */
function logMemoryStats(label) {
  if (self.performance && self.performance.memory) {
    const memUsed = Math.round(self.performance.memory.usedJSHeapSize / (1024 * 1024));
    const memTotal = Math.round(self.performance.memory.totalJSHeapSize / (1024 * 1024));
    const usagePercent = Math.round(memUsed/memTotal*100);
    console.debug(`📊 Memory ${label}: ${memUsed}MB/${memTotal}MB (${usagePercent}%)`);
    return { used: memUsed, total: memTotal, percent: usagePercent };
  }
  return null;
}

/**
 * Initialize the RIES module or reinitialize if necessary
 * @param {boolean} force - Force reinitialization even if instance exists
 * @returns {Object} The RIES module instance
 */
async function initRiesModule(force = false) {
  console.debug(`🧠 RIES module initialization - force=${force}`);
  
  if (riesModuleInstance && !force) {
    console.debug('✅ Reusing existing RIES module');
    return riesModuleInstance;
  }
  
  // If we're forcing reinitialization, clean up the old instance
  if (riesModuleInstance) {
    try {
      console.debug('🗑️ Cleaning up old RIES module instance');
      logMemoryStats('before cleanup');
      
      // Attempt to clean up old instance resources if possible
      riesModuleInstance = null;
      
      // Manual garbage collection to help free memory
      if (global && global.gc) {
        try {
          console.debug('🧹 Running garbage collection');
          global.gc();
        } catch (gcErr) {
          console.debug('⚠️ Unable to run garbage collection');
        }
      }
      
      logMemoryStats('after cleanup');
    } catch (e) {
      console.warn("⚠️ Failed to clean up old RIES module instance:", e);
    }
  }
  
  console.debug('🔧 Creating new RIES module instance');
  
  try {
    riesModuleInstance = await createRIESModule({
      locateFile: (path) => path.endsWith('.wasm') ? `${basePath}ries.wasm` : path,
      print: (text) => { 
        capturedOutput += text + "\n"; 
        // Log important output for debugging
        if (text.includes("memory") || text.includes("heap") || text.includes("alloc")) {
          console.debug(`📝 RIES output: ${text}`);
        }
      },
      printErr: (text) => { 
        capturedOutput += "Error: " + text + "\n"; 
        console.error("❌ RIES error:", text);
      }
    });
    
    console.debug('✅ RIES module created successfully');
    logMemoryStats('after initialization');
    
    return riesModuleInstance;
  } catch (err) {
    console.error('❌ Failed to create RIES module:', err);
    throw err;
  }
}

/**
 * Start sending heartbeats while calculation is running
 */
function startHeartbeat() {
  lastHeartbeat = Date.now();
  calculationRunning = true;
  
  // Clear any existing intervals/timeouts
  if (heartbeatInterval) clearInterval(heartbeatInterval);
  if (timeoutTimer) clearTimeout(timeoutTimer);
  
  // Send heartbeats periodically
  heartbeatInterval = setInterval(() => {
    if (calculationRunning) {
      postMessage({ type: 'heartbeat', timestamp: Date.now() });
      lastHeartbeat = Date.now();
    } else {
      clearInterval(heartbeatInterval);
    }
  }, HEARTBEAT_INTERVAL);
}

/**
 * Stop sending heartbeats and clear timers
 */
function stopHeartbeat() {
  calculationRunning = false;
  if (heartbeatInterval) {
    clearInterval(heartbeatInterval);
    heartbeatInterval = null;
  }
  if (timeoutTimer) {
    clearTimeout(timeoutTimer);
    timeoutTimer = null;
  }
}

/**
 * Check if an error is related to memory issues
 * @param {Error} error - The error to check
 * @returns {boolean} True if this is a memory-related error
 */
function isMemoryError(error) {
  if (!error || !error.message) return false;
  
  return MEMORY_ERROR_PATTERNS.some(pattern => 
    error.message.includes(pattern)
  );
}

/**
 * Handle messages from the main thread
 */
onmessage = async (e) => {
  const { id, targetValue, retry = false, command } = e.data;
  
  // Handle special commands
  if (command === 'restart') {
    // Force restart the module
    try {
      stopHeartbeat();
      await initRiesModule(true);
      postMessage({ id, result: { status: 'restarted' } });
    } catch (error) {
      postMessage({ id, error: 'Failed to restart module: ' + error.message });
    }
    return;
  }
  
  try {
    // Get or initialize the module, force reinitialize if this is a retry
    const moduleInstance = await initRiesModule(retry);
    
    // Reset output capture
    capturedOutput = "";
    
    // Set up tracking
    startHeartbeat();
    
    // Set an execution timeout as a fallback
    timeoutTimer = setTimeout(() => {
      if (calculationRunning) {
        // If we're still running after the timeout, assume we're stuck/OOM
        calculationRunning = false;
        postMessage({ id, memoryError: true, message: "Calculation timed out, likely due to memory limitations" });
      }
    }, TIMEOUT_DURATION);
    
    // Add a check for memory usage using performance API if available
    let memoryUsage = null;
    if (self.performance && self.performance.memory) {
      memoryUsage = {
        before: {
          usedJSHeapSize: self.performance.memory.usedJSHeapSize,
          totalJSHeapSize: self.performance.memory.totalJSHeapSize
        }
      };
    }
    
    // Track calculation start time
    const startTime = Date.now();
    console.debug(`🧮 Starting RIES calculation for target value: ${targetValue}`);
    logMemoryStats('before calculation');
    
    // Run RIES with the target value
    moduleInstance.callMain(["-F3", targetValue]);
    
    // Calculate elapsed time
    const elapsedTime = Date.now() - startTime;
    console.debug(`⏱️ RIES calculation completed in ${elapsedTime}ms`);
    
    // Check memory after calculation if available
    if (memoryUsage && self.performance && self.performance.memory) {
      memoryUsage.after = {
        usedJSHeapSize: self.performance.memory.usedJSHeapSize,
        totalJSHeapSize: self.performance.memory.totalJSHeapSize
      };
      
      // If we're using more than 80% of available heap, consider it a warning
      const heapUsageRatio = memoryUsage.after.usedJSHeapSize / memoryUsage.after.totalJSHeapSize;
      if (heapUsageRatio > 0.8) {
        console.warn(`⚠️ High memory usage (${Math.round(heapUsageRatio * 100)}%) during calculation`);
      }
    }
    
    // Calculation complete, stop monitoring
    stopHeartbeat();
    
    // Send the captured output back to the main thread
    postMessage({ 
      id, 
      result: { 
        rawOutput: capturedOutput,
        memoryStats: memoryUsage
      } 
    });
  } catch (error) {
    // Stop monitoring on error
    stopHeartbeat();
    
    // Check if this is a memory-related error
    if (isMemoryError(error)) {
      // If we haven't already retried, reinitialize the module and try again
      if (!retry) {
        console.debug(`🔄 Memory error detected, signaling for retry: ${error.message}`);
        postMessage({ id, memoryError: true });
      } else {
        // If we've already retried, give up and report the error
        console.error(`❌ Failed after retry with memory error: ${error.message}`);
        postMessage({ id, error: "Failed to complete calculation due to memory constraints: " + error.message });
      }
    } else {
      // For other errors, just report them
      console.error(`❌ Non-memory error during calculation: ${error.message}`);
      postMessage({ id, error: error.message });
    }
  }
};
