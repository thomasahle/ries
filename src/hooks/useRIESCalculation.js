import { useEffect, useState } from 'react';
import { useQuery, CancelledError } from 'react-query';
import { sendRiesRequest } from '../workers/riesWorkerClient';
import { parseRIESOutput } from '../utils/riesParser.js';

// How long the inputs must be stable before a calculation starts. The input
// field and URL update instantly; only the (uninterruptible) calculation waits.
const DEBOUNCE_MS = 200;

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

/**
 * Check whether a target value is blank.
 * @param {string} targetValue - The raw input value
 * @returns {boolean} True if there is nothing to calculate
 */
function isBlank(targetValue) {
  return !targetValue || targetValue.trim() === '';
}

/**
 * Return the inputs as they were the last time they stayed unchanged for DEBOUNCE_MS.
 * @param {string} targetValue - The value to find equations for
 * @param {Object} riesOptions - Options to pass to RIES
 * @returns {{targetValue: string, riesOptions: Object}} The debounced inputs
 */
function useDebouncedInputs(targetValue, riesOptions) {
  const [debounced, setDebounced] = useState({ targetValue, riesOptions });

  useEffect(() => {
    if (debounced.targetValue === targetValue && debounced.riesOptions === riesOptions) {
      return undefined;
    }
    const timer = setTimeout(() => setDebounced({ targetValue, riesOptions }), DEBOUNCE_MS);
    return () => clearTimeout(timer);
  }, [targetValue, riesOptions]);

  return debounced;
}

export function useRIESCalculation(targetValue, riesOptions, onRawOutputUpdate) {
  const { targetValue: queryTarget, riesOptions: queryOptions } =
    useDebouncedInputs(targetValue, riesOptions);

  const { data, isLoading, isError, error, isFetching, refetch } = useQuery(
    ['ries', queryTarget, queryOptions],
    async () => {
      if (isBlank(queryTarget)) {
        return { equations: [], rawOutput: '', computedTarget: queryTarget };
      }
      if (queryTarget === "0" || parseFloat(queryTarget) === 0) {
        return {
          equations: ZERO_CASE_EQUATIONS,
          rawOutput: "Zero case - special equations",
          computedTarget: queryTarget,
        };
      }

      // Validate input is a number.
      const num = parseFloat(queryTarget);
      if (isNaN(num)) {
        throw new Error("Please enter a valid number");
      }

      // Use the worker to run the calculation.
      const result = await sendRiesRequest(queryTarget, queryOptions);
      if (result.superseded) {
        // A newer value replaced this one before it ran. Revert the query
        // silently so react-query neither caches it nor reports an error.
        throw new CancelledError({ revert: true });
      }
      const equations = parseRIESOutput(result.rawOutput);

      // Update raw output via callback if provided
      if (onRawOutputUpdate) {
        onRawOutputUpdate(result.rawOutput);
      }

      return {
        equations,
        rawOutput: result.rawOutput,
        stderr: result.stderr || '',
        exitCode: result.exitCode ?? null,
        computedTarget: queryTarget,
      };
    },
    {
      retry: false,
      refetchOnWindowFocus: false,
      keepPreviousData: true,
      enabled: !isBlank(queryTarget),
      staleTime: 5 * 60 * 1000, // 5 minutes
    }
  );

  return {
    equations: data?.equations || [],
    rawOutput: data?.rawOutput || "",
    stderr: data?.stderr || "",
    exitCode: data?.exitCode ?? null,
    computedTarget: data?.computedTarget || targetValue,
    isLoading,
    isError,
    error,
    isFetching,
    refetch
  };
}

// parseRIESOutput is now imported from ../utils/riesParser.js
