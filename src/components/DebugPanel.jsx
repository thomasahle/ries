import React, { useState } from 'react';
import RIESOptions, { SolutionTypeSelector } from './RIESOptions';
import './RIESOptions.css';

const DebugPanel = ({ targetValue, rawOutput, riesOptions, onRIESOptionChange }) => {
  // RIES options tab not initially active
  const [activePanel, setActivePanel] = useState(null);

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
          aria-expanded={activePanel === 'ries'}
        >
          {activePanel === 'ries' ? 'Hide Options' : 'Show Options'}
        </button>
        {/* Solve-for-x toggle as button, matching debug-toggle style */}
        <button
          type="button"
          className={`debug-toggle solve-toggle ${riesOptions.solveForX ? 'active' : ''}`}
          onClick={() => onRIESOptionChange({ solveForX: !riesOptions.solveForX })}
          aria-pressed={riesOptions.solveForX}
        >
          Solve for x
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