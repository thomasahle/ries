import { useEffect, useRef, useState } from 'react';
import { useQuery, useQueryClient, CancelledError, keepPreviousData } from '@tanstack/react-query';
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

  // While the inputs wait out the debounce the query is still keyed on the old
  // values and reports neither loading nor fetching. That window is pending
  // too; otherwise a fresh load briefly falls through to "No equations found".
  const isDebouncing = queryTarget !== targetValue || queryOptions !== riesOptions;

  const queryClient = useQueryClient();
  const queryKey = ['ries', queryTarget, queryOptions];

  const { data, isLoading, isError, error, isFetching, refetch } = useQuery({
    queryKey,
    queryFn: async () => {
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
        // A newer value replaced this one before it ran. Cancel the query so
        // it reverts silently: nothing is cached and no error is reported.
        await queryClient.cancelQueries({ queryKey, exact: true });
        throw new CancelledError({ revert: true });
      }
      const equations = parseRIESOutput(result.rawOutput);

      return {
        equations,
        rawOutput: result.rawOutput,
        stderr: result.stderr || '',
        exitCode: result.exitCode ?? null,
        computedTarget: queryTarget,
      };
    },
    retry: false,
    refetchOnWindowFocus: false,
    // Keep showing the previous result (blurred) while the next one computes
    placeholderData: keepPreviousData,
    enabled: !isBlank(queryTarget),
    staleTime: 5 * 60 * 1000, // 5 minutes
  });

  // Raw output shown to the user: cleared when the input is blank, otherwise
  // whatever result is currently displayed (cached, zero case or fresh).
  const rawOutput = isBlank(targetValue) ? '' : (data?.rawOutput || '');

  // Keep the debug pane in sync with the displayed result. The callback is
  // read through a ref so a new function identity per render doesn't refire it.
  const onRawOutputUpdateRef = useRef(onRawOutputUpdate);
  onRawOutputUpdateRef.current = onRawOutputUpdate;
  useEffect(() => {
    if (onRawOutputUpdateRef.current) {
      onRawOutputUpdateRef.current(rawOutput);
    }
  }, [rawOutput]);

  return {
    equations: data?.equations || [],
    rawOutput,
    stderr: data?.stderr || "",
    exitCode: data?.exitCode ?? null,
    computedTarget: data?.computedTarget || targetValue,
    isLoading: isLoading || (isDebouncing && data === undefined),
    isError,
    error,
    isFetching: isFetching || isDebouncing,
    refetch
  };
}

// parseRIESOutput is now imported from ../utils/riesParser.js
