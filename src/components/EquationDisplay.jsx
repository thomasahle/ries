import React, { useEffect, useRef } from 'react';
import { useRIESCalculation } from '../hooks/useRIESCalculation';
import { useMathJax } from '../utils/MathJaxContext';
import EquationRow from './EquationRow';
import './EquationDisplay.css';

const EquationDisplay = ({ targetValue }) => {
  const containerRef = useRef(null);
  const { equations, isLoading, isError, error, rawOutput } = useRIESCalculation(targetValue);
  
  // Get access to MathJax context for equation rendering
  const { typeset, isReady } = useMathJax();
  
  // Typeset equations with MathJax after they're rendered or updated
  useEffect(() => {
    if (isReady && containerRef.current && equations?.length > 0) {
      // Schedule typesetting in the next microtask to ensure DOM is updated
      setTimeout(() => {
        try {
          typeset([containerRef.current]);
        } catch (err) {
          console.error("MathJax typesetting error:", err);
        }
      }, 0);
    }
  }, [equations, isReady, typeset]);

  // If no targetValue provided yet, show empty state
  if (!targetValue || targetValue.trim() === '') {
    return (
      <div 
        ref={containerRef} 
        className="equations-container"
      >
        <div className="message-block">
          <p>Enter a numerical value to find equations.</p>
        </div>
      </div>
    );
  }

  // Show loading state
  if (isLoading) {
    return (
      <div 
        ref={containerRef} 
        className="equations-container blurred"
      >
        <div className="loading-indicator">
          <p>Calculating equations for {targetValue}...</p>
          <div className="spinner"></div>
        </div>
      </div>
    );
  }

  // Show error state
  if (isError) {
    return (
      <div 
        ref={containerRef}
        className="equations-container"
      >
        <div className="message-block">
          <p>{error?.message || "An error occurred during calculation. Please try again."}</p>
        </div>
      </div>
    );
  }

  // Handle case with no equations
  if (!equations || equations.length === 0) {
    return (
      <div 
        ref={containerRef}
        className="equations-container"
      >
        <div className="message-block">
          <p>No equations found. Try a different value like π ≈ 3.14159</p>
        </div>
      </div>
    );
  }

  // Calculate T display precision for highlighting
  let decimals = 8; // fallback
  if (targetValue) {
    const decMatch = targetValue.match(/\.(\d+)$/);
    if (decMatch) decimals = decMatch[1].length;
  }

  // Show equation results
  return (
    <div 
      ref={containerRef} 
      className="equations-container"
    >
      {equations.map((equation, index) => (
        <React.Fragment key={index}>
          <EquationRow 
            equation={equation} 
            targetValue={targetValue}
            decimals={decimals}
          />
          
          {/* Add separator after every 3rd equation */}
          {(index + 1) % 3 === 0 && index < equations.length - 1 && (
            <div className="equation-separator"></div>
          )}
        </React.Fragment>
      ))}
    </div>
  );
};

export default EquationDisplay;