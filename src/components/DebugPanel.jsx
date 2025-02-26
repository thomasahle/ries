import React, { useState, useEffect } from 'react';
import { useRIESCalculation } from '../hooks/useRIESCalculation';
import './DebugPanel.css';

const DebugPanel = ({ targetValue }) => {
  const [visible, setVisible] = useState(false);
  const { rawOutput } = useRIESCalculation(targetValue);
  
  // Generate a random number between 0 and 10
  const generateRandomValue = () => {
    const randomValue = (Math.random() * 10).toFixed(6);
    // Use window.location to set the URL and trigger a new calculation
    const url = new URL(window.location);
    url.searchParams.set("T", randomValue);
    window.history.pushState({ T: randomValue }, "", url);
    // Force page reload to ensure the calculation runs with the new value
    window.location.reload();
  };
  
  // Update UI immediately when rawOutput changes
  const [localOutput, setLocalOutput] = useState('');
  useEffect(() => {
    if (rawOutput) {
      setLocalOutput(rawOutput);
    }
  }, [rawOutput]);
  
  return (
    <div className="debug-section">
      <div className="debug-controls">
        <button 
          className={`debug-toggle ${visible ? 'active' : ''}`} 
          onClick={() => setVisible(!visible)}
        >
          {visible ? 'Hide Debug Output' : 'Show Debug Output'}
        </button>
        
        <button 
          className="debug-toggle" 
          onClick={generateRandomValue}
        >
          Random T
        </button>
      </div>
      
      {visible && (
        <>
          <h3>Current Calculation Output:</h3>
          <pre className="debug-output">
            {localOutput || rawOutput || 'No output captured yet. Enter a value to calculate.'}
          </pre>
          
          <h3>WASM Status:</h3>
          <pre className="debug-output">
            RIES_MODULE_LOADED: {window.RIES_MODULE_LOADED ? 'true' : 'false'}<br/>
            createRIESModule exists: {typeof window.createRIESModule === 'function' ? 'true' : 'false'}<br/>
            riesModuleInstance available: {window.riesModuleInstance ? 'true' : 'false'}<br/>
            Target Value: {targetValue || 'none'}<br/>
            lastRIESOutput length: {window.lastRIESOutput?.length || 0} chars<br/>
          </pre>
        </>
      )}
    </div>
  );
};

export default DebugPanel;