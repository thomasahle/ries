import React from 'react';
import { useRIESCalculation } from '../hooks/useRIESCalculation';
import { getRIESMessage } from '../utils/riesParser.js';
import EquationRow from './EquationRow';
import './EquationDisplay.css';

export default function EquationDisplay({ targetValue, riesOptions, onRawOutputUpdate }) {
  // Use the hook with options and callback for raw output.
  // stderr and exitCode are only present when the worker reports them.
  const { 
    equations, 
    rawOutput,
    stderr,
    exitCode,
    isLoading, 
    isFetching, 
    isError, 
    error, 
    computedTarget 
  } = useRIESCalculation(targetValue, riesOptions, onRawOutputUpdate);

  // When input is empty, show a friendly message.
  if (!targetValue || targetValue.trim() === '') {
    return (
      <div className="equations-container">
        <div className="message-block">
          <p>Enter a numerical value to find equations.</p>
        </div>
      </div>
    );
  }

  // Show loading indicator if no data is available yet.
  if (isLoading && (!equations || equations.length === 0)) {
    return (
      <div className="equations-container blurred">
        <div className="loading-indicator">
          <p>Calculating equations for {targetValue}...</p>
          <div className="spinner"></div>
        </div>
      </div>
    );
  }

  // Handle error state.
  if (isError) {
    return (
      <div className="equations-container">
        <div className="message-block">
          <p>{error?.message || "An error occurred during calculation. Please try again."}</p>
        </div>
      </div>
    );
  }

  // Handle "no equations found": show RIES's own explanation (option errors,
  // banner without matches) and any stderr / exit code as plain text.
  if (!equations || equations.length === 0) {
    const message = getRIESMessage(rawOutput) || 'No equations found.';
    const details = [];
    if (typeof stderr === 'string' && stderr.trim()) {
      details.push(stderr.trim());
    }
    if (typeof exitCode === 'number' && exitCode !== 0) {
      details.push(`RIES exited with code ${exitCode}.`);
    }
    return (
      <div className="equations-container">
        <div className="message-block">
          <p>{message}</p>
          {details.map((detail, index) => (
            <pre key={index} className="message-details">{detail}</pre>
          ))}
          <p>Try a different value like π ≈ 3.14159 or adjust your RIES options.</p>
        </div>
      </div>
    );
  }

  return (
    <div className="equations-container-wrapper" style={{ position: 'relative' }}>
      
      {/* Apply the blurred class when a new request is pending */}
      <div className={`equations-container ${isFetching ? 'blurred' : ''}`}>
        {equations.map((equation, index) => (
          <React.Fragment key={index}>
            <EquationRow
              equation={equation}
              targetValue={computedTarget}  // Use frozen target from the worker result
              decimals={8} // adjust as needed
            />
            {(index + 1) % 3 === 0 && index < equations.length - 1 && (
              <div className="equation-separator"></div>
            )}
          </React.Fragment>
        ))}
      </div>
      {isFetching && (
        <div className="loading-overlay">
          <div className="spinner"></div>
        </div>
      )}
    </div>
  );
}