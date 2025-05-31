import React, { useState, useEffect } from 'react';
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

function App() {
  // Create a client
  const queryClient = new QueryClient();
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

  // Check URL parameters on initial load
  useEffect(() => {
    const urlParams = new URLSearchParams(window.location.search);
    const tParam = urlParams.get('T');
    
    if (tParam) {
      setInputValue(tParam);
    } else {
      // Generate a random value between 0 and 10 if no T value is provided
      const randomValue = (Math.random() * 10).toFixed(6);
      setInputValue(randomValue);
      
      // Update URL with the random value
      const url = new URL(window.location);
      url.searchParams.set('T', randomValue);
      window.history.pushState({ T: randomValue }, '', url);
    }
  }, []);

  // Handle input change from the form or random generator
  const handleInputChange = (value) => {
    setInputValue(value);
    // Update URL with the new value
    const url = new URL(window.location);
    if (value) {
      url.searchParams.set('T', value);
    } else {
      url.searchParams.delete('T');
    }
    window.history.pushState({ T: value }, '', url);
  };
  
  // Generate a new random value and feed it through the same handler
  const handleRandom = () => {
    const randomValue = (Math.random() * 10).toFixed(6);
    handleInputChange(randomValue);
  };

  // Handle RIES options changes
  const handleRIESOptionChange = (newOptions) => {
    setRiesOptions(prev => {
      // If the argument is an event (from the solution type selector)
      if (newOptions.target) {
        const { name, value } = newOptions.target;
        return { ...prev, [name]: value };
      }
      // If it's a complete options object
      return { ...prev, ...newOptions };
    });
  };

  // Update raw output when it's available from calculations
  const handleRawOutputUpdate = (output) => {
    setRawOutput(output);
  };

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
