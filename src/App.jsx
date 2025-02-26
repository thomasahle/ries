import React, { useState, useEffect } from 'react';
import { QueryClient, QueryClientProvider } from 'react-query';
import InputForm from './components/InputForm';
import EquationDisplay from './components/EquationDisplay';
import DebugPanel from './components/DebugPanel';
import { MathJaxProvider } from './utils/MathJaxContext';
import './App.css';

// Create a client for React Query
const queryClient = new QueryClient({
  defaultOptions: {
    queries: {
      retry: false,
      refetchOnWindowFocus: false,
    }
  }
});

function App() {
  const [inputValue, setInputValue] = useState('');
  

  // Read URL parameters on initial load
  useEffect(() => {
    const params = new URLSearchParams(window.location.search);
    const Tparam = params.get("T");
    if (Tparam) {
      // Set value from URL and calculations will run automatically
      setInputValue(Tparam);
    }
  }, []);
  
  // Handle input change and update URL
  const handleInputChange = (value) => {
    setInputValue(value);
    
    if (value && value.trim() !== '') {
      const url = new URL(window.location);
      url.searchParams.set("T", value);
      window.history.pushState({ T: value }, "", url);
    }
  };
  
  // Handle browser back/forward navigation
  useEffect(() => {
    const handlePopState = (event) => {
      if (event.state && typeof event.state.T !== "undefined") {
        setInputValue(event.state.T);
      }
    };
    
    window.addEventListener("popstate", handlePopState);
    return () => window.removeEventListener("popstate", handlePopState);
  }, []);
  
  return (
    <QueryClientProvider client={queryClient}>
      <MathJaxProvider>
        <div className="container">
          <h1>Inverse Symbolic Calculator</h1>
          <p>Enter a numerical value to find matching equations:</p>
          <InputForm value={inputValue} onChange={handleInputChange} />
          <EquationDisplay targetValue={inputValue} />
          {inputValue && <DebugPanel targetValue={inputValue} onRandomValue={handleInputChange} />}
        </div>
      </MathJaxProvider>
    </QueryClientProvider>
  );
}

export default App;