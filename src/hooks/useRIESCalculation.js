import { useQuery } from 'react-query';
import { sendRiesRequest } from '../workers/riesWorkerClient';
import { convertForthToLatex } from '../utils/forthParser.js';

// Constants for zero-case
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

export function useRIESCalculation(targetValue) {
  const { data, isLoading, isError, error, isFetching, refetch } = useQuery(
    ['ries', targetValue],
    async () => {
      if (!targetValue || targetValue.trim() === '') {
        return { equations: [], rawOutput: '', computedTarget: targetValue };
      }
      if (targetValue === "0" || parseFloat(targetValue) === 0) {
        return {
          equations: ZERO_CASE_EQUATIONS,
          rawOutput: "Zero case - special equations",
          computedTarget: targetValue,
        };
      }
      
      // Validate input is a number.
      const num = parseFloat(targetValue);
      if (isNaN(num)) {
        throw new Error("Please enter a valid number");
      }
      
      // Use the worker to run the calculation.
      const result = await sendRiesRequest(targetValue);
      const equations = parseRIESOutput(result.rawOutput);
      return { equations, rawOutput: result.rawOutput, computedTarget: targetValue };
    },
    {
      retry: false,
      refetchOnWindowFocus: false,
      keepPreviousData: true,
      enabled: !!targetValue && targetValue.trim() !== '',
      staleTime: 5 * 60 * 1000, // 5 minutes
    }
  );
  
  return {
    equations: data?.equations || [],
    rawOutput: data?.rawOutput || "",
    computedTarget: data?.computedTarget || targetValue,
    isLoading,
    isError,
    error,
    isFetching,
    refetch
  };
}

/**
 * Parse the RIES output to extract equations in FORTH format.
 * @param {string} text - The raw output from RIES.
 * @returns {Array} Parsed equations with LaTeX formatting.
 */
function parseRIESOutput(text) {
  if (!text || text.length === 0) {
    return [];
  }
  
  const lines = text.split("\n");
  const equationLines = lines.filter(line => 
    line.includes("=") && 
    line.includes("for x = T") && 
    !line.includes("Your target value")
  );
  
  if (equationLines.length === 0) {
    if (text.includes("Your target value:") || text.includes("mrob.com/ries")) {
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
  
  const results = [];
  for (const line of lines) {
    const match = line.match(EQUATION_REGEX);
    if (match) {
      const lhsRaw = match[1].trim();
      const rhsRaw = match[2].trim();
      const offsetRaw = match[3].trim();
      
      try {
        results.push({
          lhs: convertForthToLatex(lhsRaw),
          rhs: convertForthToLatex(rhsRaw),
          offset: offsetRaw
        });
      } catch (error) {
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
