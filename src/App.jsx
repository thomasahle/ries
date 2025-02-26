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
  // Single state for input value - this is also the target value for calculations
  const [inputValue, setInputValue] = useState('');
  
  // Load RIES module when the app mounts
  useEffect(() => {
    console.log("App mounted. Checking for RIES module...");
    
    // Function to initialize the RIES module
    const initRIESModule = async () => {
      if (typeof createRIESModule !== 'function') {
        console.error("createRIESModule is not available");
        return;
      }
      
      if (!window.riesModuleInstance) {
        try {
          console.log("Creating RIES module instance");
          
          // Initialize the RIES module
          window.riesModuleInstance = await createRIESModule({
            print: text => {
              console.log("RIES output:", text);
              // Store output for debugging
              if (!window.lastRIESOutput) window.lastRIESOutput = "";
              window.lastRIESOutput += text + "\n";
            },
            printErr: text => console.error("RIES error:", text)
          });
          
          window.RIES_MODULE_LOADED = true;
          console.log("RIES module loaded successfully");
          
          // Run a quick test calculation to verify it works
          try {
            const originalPrint = window.riesModuleInstance.print;
            
            // Capture output with a special print function
            window.riesModuleInstance.print = function(text) {
              console.log("Test calculation output:", text);
            };
            
            // Run a simple test calculation
            window.riesModuleInstance.callMain(["-F3", "3.14159"]);
            
            // Restore original print function
            window.riesModuleInstance.print = originalPrint;
            console.log("Test calculation completed successfully");
          } catch (error) {
            console.error("Test calculation failed:", error);
          }
        } catch (error) {
          console.error("Failed to initialize RIES module:", error);
        }
      }
    };
    
    // Initialize the module
    initRIESModule();
  }, []);
  
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
    
    // Update URL if the value is not empty
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
          
          <InputForm 
            value={inputValue} 
            onChange={handleInputChange} 
          />
          
          <EquationDisplay targetValue={inputValue} />
          
          {inputValue && <DebugPanel targetValue={inputValue} />}
        </div>
      </MathJaxProvider>
    </QueryClientProvider>
  );
}

export default App;