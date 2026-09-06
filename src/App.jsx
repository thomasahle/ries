import React, { useState, useEffect, useRef } from 'react';
import { QueryClient, QueryClientProvider } from '@tanstack/react-query';
import InputForm from './components/InputForm';
import EquationDisplay from './components/EquationDisplay';
import DebugPanel from './components/DebugPanel';
import { MathJaxProvider } from './utils/MathJaxContext';
import { DEFAULT_SEARCH_LEVEL, parseSearchLevel } from './utils/searchLevels';
import './App.css';

// List of possible header prompts; one will be selected at random per session
const promptOptions = [
  'Type any number to discover equations:',
  'Input a value—we’ll hunt down equations:',
  'Give a number and show symbolic identities:',
  'Enter a numeric input to see formulas:',
  'Provide a value to uncover equations:',
  'Feed in a number to find expressions:',
  'Drop a number and explore its algebraic twins:',
  'Key in a value to view matching symbolic forms:'
];

// A plain decimal number as the number input accepts it (HTML's "valid
// floating-point number"): an optional minus sign, digits with an optional
// fraction, an optional exponent (e.g. "-2.5", ".5", "1e-7"). Anything the
// input would blank out, such as "+5" or "5.", must not reach RIES either.
const NUMBER_PATTERN = /^-?(\d+(\.\d+)?|\.\d+)([eE][+-]?\d+)?$/;

/**
 * Read the T query parameter, accepting only strings that are a finite number.
 * @param {URLSearchParams} urlParams - The parsed query string.
 * @returns {string|null} The validated value, or null if absent or malformed.
 */
function getTargetParam(urlParams) {
  const raw = urlParams.get('T');
  if (raw === null) return null;
  const value = raw.trim();
  return NUMBER_PATTERN.test(value) && Number.isFinite(Number(value)) ? value : null;
}

function App() {
  // Create the query client once for the lifetime of the app
  const [queryClient] = useState(() => new QueryClient());
  // Random header prompt selected once per session
  const [promptText] = useState(() =>
    promptOptions[Math.floor(Math.random() * promptOptions.length)]
  );
  
  const [inputValue, setInputValue] = useState('');
  const [rawOutput, setRawOutput] = useState('');
  const [riesOptions, setRiesOptions] = useState({
    solutionType: '',
    solveForX: false,
    neverUseSymbols: '',
    onlyOneSymbols: '',
    onlyUseSymbols: '',
    searchLevel: DEFAULT_SEARCH_LEVEL
  });

  // True while the user is editing the input. The first change of an edit
  // pushes a history entry and later keystrokes replace it, so history grows
  // by one entry per committed value and Back returns to the previous one.
  const editingRef = useRef(false);

  // Write the value, solve flag and search depth to the URL (and history state)
  const syncUrl = (value, solve, level, { replace }) => {
    const url = new URL(window.location);
    if (value) {
      url.searchParams.set('T', value);
    } else {
      url.searchParams.delete('T');
    }
    if (solve) {
      url.searchParams.set('solve', 'true');
    } else {
      url.searchParams.delete('solve');
    }
    // The default level is left out, so URLs without it keep their meaning
    if (level !== DEFAULT_SEARCH_LEVEL) {
      url.searchParams.set('l', String(level));
    } else {
      url.searchParams.delete('l');
    }
    const state = { T: value, solve, l: level };
    if (replace) {
      window.history.replaceState(state, '', url);
    } else {
      window.history.pushState(state, '', url);
    }
  };

  // Check URL parameters on initial load
  useEffect(() => {
    const urlParams = new URLSearchParams(window.location.search);
    const tParam = getTargetParam(urlParams);
    const solveParam = urlParams.get('solve');
    const solve = solveParam === 'true';
    // An unknown level falls back to the default, as a malformed T does
    const level = parseSearchLevel(urlParams.get('l'));

    // Handle solve and search depth parameters
    if (solveParam !== null || urlParams.has('l')) {
      setRiesOptions(prev => ({ ...prev, solveForX: solve, searchLevel: level }));
    }

    // Generate a random value between 0 and 10 if no valid T value is provided
    const value = tParam ?? (Math.random() * 10).toFixed(6);
    setInputValue(value);
    // Record the initial value without adding a history entry
    syncUrl(value, solve, level, { replace: true });
  }, []);

  // Apply a new input value. Typing keeps replacing the entry pushed by the
  // first keystroke; a committed value (dice button) ends the edit session.
  const updateInputValue = (value, { commit = false } = {}) => {
    setInputValue(value);
    syncUrl(value, riesOptions.solveForX, riesOptions.searchLevel, { replace: editingRef.current && !commit });
    editingRef.current = !commit;
  };

  // Handle input change from the form (typing)
  const handleInputChange = (value) => {
    updateInputValue(value);
  };

  // The typed value is committed (Enter or blur); the next change starts a new entry
  const handleInputCommit = () => {
    editingRef.current = false;
  };

  // Generate a new random value; it is committed immediately
  const handleRandom = () => {
    const randomValue = (Math.random() * 10).toFixed(6);
    updateInputValue(randomValue, { commit: true });
  };

  // Handle RIES options changes
  const handleRIESOptionChange = (newOptions) => {
    // If the argument is an event (from the solution type selector)
    if (newOptions.target) {
      const { name, value } = newOptions.target;
      newOptions = { [name]: value };
    }
    // Changing solveForX or the search depth commits the current value and
    // gets its own history entry
    const solve = 'solveForX' in newOptions ? newOptions.solveForX : riesOptions.solveForX;
    const level = 'searchLevel' in newOptions ? newOptions.searchLevel : riesOptions.searchLevel;
    if (solve !== riesOptions.solveForX || level !== riesOptions.searchLevel) {
      syncUrl(inputValue, solve, level, { replace: false });
      editingRef.current = false;
    }
    setRiesOptions(prev => ({ ...prev, ...newOptions }));
  };

  // Update raw output when it's available from calculations
  const handleRawOutputUpdate = (output) => {
    setRawOutput(output);
  };

  // Listen for Back/Forward navigation and sync app state
  useEffect(() => {
    const handlePop = (event) => {
      const urlParams = new URLSearchParams(window.location.search);
      // Use history state if available (for popstate), else fall back to URL
      const t = event.state?.T ?? getTargetParam(urlParams) ?? '';
      const solve = event.state?.solve ?? (urlParams.get('solve') === 'true');
      const level = parseSearchLevel(event.state?.l ?? urlParams.get('l'));

      // The restored value is a committed one; the next keystroke starts a new entry
      editingRef.current = false;
      setInputValue(t);
      setRiesOptions(prev => ({ ...prev, solveForX: solve, searchLevel: level }));
    };
    window.addEventListener('popstate', handlePop);
    return () => window.removeEventListener('popstate', handlePop);
  }, []);

  return (
    <QueryClientProvider client={queryClient}>
      <MathJaxProvider>
        <div className="container">
          <header>
            <h1>Inverse Symbolic Calculator</h1>
            <p>{promptText}</p>
          </header>

          <main>
            <InputForm
              value={inputValue}
              onChange={handleInputChange}
              onCommit={handleInputCommit}
              onRandom={handleRandom}
            />
            <EquationDisplay
              targetValue={inputValue}
              riesOptions={riesOptions}
              onRawOutputUpdate={handleRawOutputUpdate}
            />
            <DebugPanel
              targetValue={inputValue}
              rawOutput={rawOutput}
              riesOptions={riesOptions}
              onRIESOptionChange={handleRIESOptionChange}
            />
          </main>

        </div>
      </MathJaxProvider>
    </QueryClientProvider>
  );
}

export default App;
