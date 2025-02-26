import React, { useState, useEffect } from 'react';
import './EquationRow.css';
import { highlightDifference } from '../utils/formatting';
import { useMathJax } from '../utils/MathJaxContext';

const EquationRow = ({ equation, targetValue, decimals }) => {
  const [isCopied, setIsCopied] = useState(false);
  
  // Calculate x value with highlighting
  let xValLatex = "T"; // fallback
  if (equation.offset && targetValue) {
    const offsetVal = parseFloat((equation.offset || "").replace(/\s+/g, "")) || 0;
    const Tnum = parseFloat(targetValue);
    const xVal = (Tnum + offsetVal).toFixed(decimals);
    xValLatex = highlightDifference(targetValue, xVal);
  } else if (targetValue === "0") {
    xValLatex = "0";
  }

  // Copy equation to clipboard when clicked
  const copyEquation = () => {
    // Create LaTeX representation of the equation
    const latexEquation = `${equation.lhs} = ${equation.rhs}`;
    
    navigator.clipboard.writeText(latexEquation)
      .then(() => {
        setIsCopied(true);
        setTimeout(() => setIsCopied(false), 800);
      })
      .catch(err => console.error('Failed to copy equation to clipboard', err));
  };

  // Get MathJax context
  const { typeset, isReady } = useMathJax();
  
  // Use useEffect to trigger MathJax rendering when isCopied changes
  useEffect(() => {
    if (isReady && isCopied) {
      typeset();
    }
  }, [isCopied, isReady, typeset]);

  return (
    <div 
      className={`equation-row ${isCopied ? 'equation-copied' : ''}`}
      onClick={copyEquation}
    >
      <div className="equation-lhs" dangerouslySetInnerHTML={{ __html: `\\(${equation.lhs} =\\)` }}></div>
      <div className="equation-rhs" dangerouslySetInnerHTML={{ __html: `\\(${equation.rhs}\\)` }}></div>
      <div className="equation-x-value" dangerouslySetInnerHTML={{ __html: `\\(x = ${xValLatex}\\)` }}></div>
    </div>
  );
};

export default EquationRow;