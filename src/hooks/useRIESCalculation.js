import { useEffect, useRef } from 'react';
import { useQuery } from 'react-query';
import { convertForthToLatex } from '../utils/forthParser.js';

// Constants
const ZERO_CASE_EQUATIONS = [
  { lhs: "x", rhs: "0", offset: "0" },
  { lhs: "x", rhs: "\\sin(0)", offset: "0" },
  { lhs: "\\sin(x)", rhs: "0", offset: "0" },
  { lhs: "\\tan(x)", rhs: "0", offset: "0" },
  { lhs: "\\cos(x)-1", rhs: "0", offset: "0" },
  { lhs: "e^x-1", rhs: "0", offset: "0" },
  { lhs: "\\ln(1+x)", rhs: "0", offset: "0" },
  { lhs: "x^2", rhs: "0", offset: "0" },
];

// Regular expression for FORTH format output
const EQUATION_REGEX = /^\s*(.*?)\s*=\s*(.*?)\s+for\s+x\s*=\s*T\s*([+\-]\s*[\d\.e\-]+)/;

/**
 * Custom hook to perform RIES calculations
 * @param {string} targetValue - The numeric value to find equations for
 * @returns {Object} Calculation results and state
 */
export function useRIESCalculation(targetValue) {
  const abortControllerRef = useRef(null);
  
  // Use React Query for data fetching and caching
  const { data, isLoading, isError, error, refetch } = useQuery(
    ['ries', targetValue],
    async () => {
      // Skip empty values
      if (!targetValue || targetValue.trim() === '') {
        return { equations: [], rawOutput: '' };
      }
      
      // Handle special case for zero
      if (targetValue === "0" || parseFloat(targetValue) === 0) {
        return {
          equations: ZERO_CASE_EQUATIONS,
          rawOutput: "Zero case - special equations"
        };
      }
      
      // Validate input is a number
      const num = parseFloat(targetValue);
      if (isNaN(num)) {
        throw new Error("Please enter a valid number");
      }
      
      // Create/abort controller for cancellation
      if (abortControllerRef.current) {
        abortControllerRef.current.abort();
      }
      abortControllerRef.current = new AbortController();
      
      return new Promise((resolve, reject) => {
        // Check if module is available
        if (typeof createRIESModule !== 'function') {
          return reject(new Error("RIES WASM module not loaded"));
        }
        
        // Clear global output buffer
        window.capturedRIESOutput = "";
        
        // Initialize or use existing module
        if (!window.riesModuleInstance) {
          initializeModule()
            .then(module => runCalculation(module, resolve, reject, targetValue))
            .catch(err => reject(new Error("Failed to initialize RIES module: " + err.message)));
        } else {
          runCalculation(window.riesModuleInstance, resolve, reject, targetValue);
        }
      });
    },
    {
      retry: false,
      refetchOnWindowFocus: false,
      keepPreviousData: true,
      enabled: !!targetValue && targetValue.trim() !== '',
      staleTime: 5 * 60 * 1000, // 5 minutes
    }
  );
  
  // Cleanup function to abort calculation when component unmounts
  useEffect(() => {
    return () => {
      if (abortControllerRef.current) {
        abortControllerRef.current.abort();
      }
    };
  }, []);
  
  // Expose both equations and raw output, along with loading/error state
  return {
    equations: data?.equations || [],
    rawOutput: data?.rawOutput || "",
    isLoading,
    isError,
    error,
    refetch
  };
}

/**
 * Initialize the RIES WASM module
 * @returns {Promise} Module instance
 */
async function initializeModule() {
  const module = await createRIESModule({
    print: function(text) {
      window.capturedRIESOutput += text + "\n";
    },
    printErr: function(text) {
      console.error("RIES ERROR:", text);
    }
  });
  
  window.riesModuleInstance = module;
  window.RIES_MODULE_LOADED = true;
  
  return module;
}

/**
 * Run a calculation with the RIES module
 * @param {Object} moduleInstance - The RIES module instance
 * @param {Function} resolve - Promise resolve function
 * @param {Function} reject - Promise reject function
 */
function runCalculation(moduleInstance, resolve, reject, targetValue) {
  // Clear the global output buffer
  window.capturedRIESOutput = "";
  
  // Store original print function
  const originalPrint = moduleInstance.print;
  
  // Replace with our capture function
  moduleInstance.print = function(text) {
    window.capturedRIESOutput += text + "\n";
  };
  
  try {
    // Run the calculation
    moduleInstance.callMain(["-F3", targetValue]);
    
    // Get the captured output from our global variable
    const outputBuffer = window.capturedRIESOutput;
    
    // Restore original print function
    moduleInstance.print = originalPrint;
    
    // Parse the output and resolve
    const equations = parseRIESOutput(outputBuffer);
    resolve({
      equations,
      rawOutput: outputBuffer
    });
  } catch (err) {
    // Restore original print function
    moduleInstance.print = originalPrint;
    
    // Handle specific errors
    if (err.name === 'RuntimeError' && err.message.includes('memory access out of bounds')) {
      reject(new Error("This calculation requires too much memory. Try a different number."));
    } else {
      reject(new Error("Calculation failed: " + err.message));
    }
  }
}

/**
 * Parse the RIES output to extract equations in FORTH format
 * @param {string} text - The raw output from RIES
 * @returns {Array} Parsed equations with LaTeX formatting
 */
function parseRIESOutput(text) {
  if (!text || text.length === 0) {
    return [];
  }
  
  const lines = text.split("\n");
  
  // First check if we have any actual RIES output with equations
  const equationLines = lines.filter(line => 
    line.includes("=") && 
    line.includes("for x = T") && 
    !line.includes("Your target value")
  );
  
  if (equationLines.length === 0) {
    // Check if this is RIES output at all (contains expected header)
    if (text.includes("Your target value:") || text.includes("mrob.com/ries")) {
      // Return a helpful equation
      const matchedValue = text.match(/T = ([\d\.]+)/);
      const valueDisplay = matchedValue ? matchedValue[1] : "value";
      
      return [{
        lhs: "x",
        rhs: `\\text{No equations found for } ${valueDisplay}`,
        offset: "0"
      }];
    }
    
    return [];
  }
  
  // Extract and convert equations
  const results = [];
  
  for (const line of lines) {
    const match = line.match(EQUATION_REGEX);
    if (match) {
      const lhsRaw = match[1].trim();
      const rhsRaw = match[2].trim();
      const offsetRaw = match[3].trim();
      
      try {
        // Convert to LaTeX using the forthParser
        results.push({
          lhs: convertForthToLatex(lhsRaw),
          rhs: convertForthToLatex(rhsRaw),
          offset: offsetRaw
        });
      } catch (error) {
        // Add a fallback with the raw strings
        results.push({
          lhs: lhsRaw,
          rhs: rhsRaw,
          offset: offsetRaw || "0"
        });
      }
    }
  }
  
  return results;
}