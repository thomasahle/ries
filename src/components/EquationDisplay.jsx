import React from 'react';
import { useRIESCalculation } from '../hooks/useRIESCalculation';
import EquationRow from './EquationRow';
import './EquationDisplay.css';

export default function EquationDisplay({ targetValue }) {
  // Destructure computedTarget from the hook so that the displayed equations stay in sync.
  const { equations, isLoading, isFetching, isError, error, computedTarget } = useRIESCalculation(targetValue);

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

  // Handle "no equations found"
  if (!equations || equations.length === 0) {
    return (
      <div className="equations-container">
        <div className="message-block">
          <p>No equations found. Try a different value like π ≈ 3.14159</p>
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
