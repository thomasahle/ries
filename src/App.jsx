import React, { useState, useEffect, useRef } from 'react';
import { QueryClient, QueryClientProvider } from 'react-query';
import InputForm from './components/InputForm';
import EquationDisplay from './components/EquationDisplay';
import DebugPanel from './components/DebugPanel';
import { MathJaxProvider } from './utils/MathJaxContext';
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

// A plain decimal number, optionally signed and with an exponent (e.g. "-2.5", ".5", "1e-7")
const NUMBER_PATTERN = /^[+-]?(\d+\.?\d*|\.\d+)([eE][+-]?\d+)?$/;

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
    onlyUseSymbols: ''
  });

  // True while the user is editing the input. The first change of an edit
  // pushes a history entry and later keystrokes replace it, so history grows
  // by one entry per committed value and Back returns to the previous one.
  const editingRef = useRef(false);

  // Write the value and solve flag to the URL (and history state)
  const syncUrl = (value, solve, { replace }) => {
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
    const state = { T: value, solve };
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

    // Handle solve parameter
    if (solveParam !== null) {
      setRiesOptions(prev => ({ ...prev, solveForX: solve }));
    }

    // Generate a random value between 0 and 10 if no valid T value is provided
    const value = tParam ?? (Math.random() * 10).toFixed(6);
    setInputValue(value);
    // Record the initial value without adding a history entry
    syncUrl(value, solve, { replace: true });
  }, []);

  // Apply a new input value. Typing keeps replacing the entry pushed by the
  // first keystroke; a committed value (dice button) ends the edit session.
  const updateInputValue = (value, { commit = false } = {}) => {
    setInputValue(value);
    syncUrl(value, riesOptions.solveForX, { replace: editingRef.current && !commit });
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
      setRiesOptions(prev => ({ ...prev, [name]: value }));
      return;
    }
    // Changing solveForX commits the current value and gets its own history entry
    if ('solveForX' in newOptions && newOptions.solveForX !== riesOptions.solveForX) {
      syncUrl(inputValue, newOptions.solveForX, { replace: false });
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

      // The restored value is a committed one; the next keystroke starts a new entry
      editingRef.current = false;
      setInputValue(t);
      setRiesOptions(prev => ({ ...prev, solveForX: solve }));
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
              solveForX={riesOptions.solveForX}
            />
            <DebugPanel
              targetValue={inputValue}
              rawOutput={rawOutput}
              onRandomValue={handleInputChange}
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
