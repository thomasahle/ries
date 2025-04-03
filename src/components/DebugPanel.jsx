import React, { useState } from 'react';
import RIESOptions, { SolutionTypeSelector } from './RIESOptions';
import './RIESOptions.css';

const DebugPanel = ({ targetValue, rawOutput, onRandomValue, riesOptions, onRIESOptionChange }) => {
  // RIES options tab not initially active
  const [activePanel, setActivePanel] = useState(null);
  
  // Generate a random number between 0 and 10
  const generateRandomValue = () => {
    const randomValue = (Math.random() * 10).toFixed(6);
    // Use window.location to set the URL and trigger a new calculation
    const url = new URL(window.location);
    url.searchParams.set("T", randomValue);
    window.history.pushState({ T: randomValue }, "", url);
    // Update the App's inputValue through the onRandomValue callback
    onRandomValue(randomValue);
  };

  // Toggle panel visibility
  const togglePanel = (panel) => {
    if (activePanel === panel) {
      setActivePanel(null);
    } else {
      setActivePanel(panel);
    }
  };
  
  return (
    <div className="debug-section">
      <div className="debug-controls">
        <button 
          className={`debug-toggle ${activePanel === 'ries' ? 'active' : ''}`}
          onClick={() => togglePanel('ries')}
        >
          {activePanel === 'ries' ? 'Hide RIES Options' : 'Show RIES Options'}
        </button>
        
        <button 
          className="debug-toggle" 
          onClick={generateRandomValue}
        >
          Random T
        </button>
      </div>

      {activePanel === 'ries' && (
        <RIESOptions 
          isVisible={true}
          onToggle={() => togglePanel('ries')}
          onOptionsChange={onRIESOptionChange}
          riesOptions={{...riesOptions, rawOutput, targetValue}}
        />
      )}
    </div>
  );
};

export default DebugPanel;