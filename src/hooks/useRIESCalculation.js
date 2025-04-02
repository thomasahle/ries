import { useQuery } from 'react-query';
import { sendRiesRequest } from '../workers/riesWorkerClient';
import { parseRIESOutput } from '../utils/riesParser.js';

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

// parseRIESOutput is now imported from utils/riesParser.js

export function useRIESCalculation(targetValue, riesOptions, onRawOutputUpdate) {
  const { data, isLoading, isError, error, isFetching, refetch } = useQuery(
    ['ries', targetValue, riesOptions],
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
      const result = await sendRiesRequest(targetValue, riesOptions);
      const equations = parseRIESOutput(result.rawOutput);
      
      // Update raw output via callback if provided
      if (onRawOutputUpdate) {
        onRawOutputUpdate(result.rawOutput);
      }
      
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

// parseRIESOutput is now imported from ../utils/riesParser.js
