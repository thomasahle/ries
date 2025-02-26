import React, { useRef, useState, useEffect } from 'react';
import { useMathJax } from '../utils/MathJaxContext';
import { highlightDifference, formatNumberForLatex } from '../utils/formatting';
import './EquationRow.css';

export default function EquationRow({ equation, targetValue, decimals }) {
  const { isReady, typesetPromise } = useMathJax();
  const hiddenRef = useRef(null);
  const [renderedHTML, setRenderedHTML] = useState('');
  const [isCopied, setIsCopied] = useState(false);

  // Compute x-value based on the target value.
  const offsetVal = parseFloat((equation.offset || "").replace(/\s+/g, "")) || 0;
  const Tnum = parseFloat(targetValue);
  
  // Get the raw x value
  let xVal = !isNaN(Tnum) ? (Tnum + offsetVal) : targetValue;
  
  // Format the value depending on its magnitude
  let formattedXVal;
  if (typeof xVal === 'number') {
    // Check if it's a very large or small number that would be shown in scientific notation
    const absXVal = Math.abs(xVal);
    if (absXVal > 0 && (absXVal >= 1e10 || absXVal < 1e-4)) {
      // For scientific notation, use exponential format and convert to LaTeX
      formattedXVal = formatNumberForLatex(xVal.toExponential(decimals));
    } else {
      // For regular numbers, use fixed precision
      formattedXVal = xVal.toFixed(decimals);
    }
  } else {
    formattedXVal = String(xVal);
  }
  
  // Add highlighting for differences
  const xValLatex = highlightDifference(String(targetValue), formattedXVal);

  // Build LaTeX strings for each part.
  const lhsLatex = `\\(${equation.lhs} =\\)`;
  const rhsLatex = `\\(${equation.rhs}\\)`;
  const xLatex   = `\\(x = ${xValLatex}\\)`;
  const fullLatex = `
    <span class="equation-lhs">${lhsLatex}</span>
    <span class="equation-rhs">${rhsLatex}</span>
    <span class="equation-x-value">${xLatex}</span>
  `;

  // Copy equation to clipboard.
  const copyEquation = () => {
    const latexEquation = `${equation.lhs} = ${equation.rhs}`;
    navigator.clipboard.writeText(latexEquation)
      .then(() => {
        setIsCopied(true);
        setTimeout(() => setIsCopied(false), 1500);
      })
      .catch(err => console.error('Failed to copy equation to clipboard', err));
  };

  // Pre-render offscreen: insert raw LaTeX, typeset with MathJax, then store final HTML.
  useEffect(() => {
    if (!isReady || !hiddenRef.current) return;
    hiddenRef.current.innerHTML = fullLatex;
    typesetPromise([hiddenRef.current])
      .then(() => {
        setRenderedHTML(hiddenRef.current.innerHTML);
      })
      .catch(err => console.error('MathJax typeset error:', err));
  }, [fullLatex, isReady, typesetPromise]);

  return (
    <div className="equation-row-wrapper">
      <div
        className="equation-row"
        onClick={copyEquation}
        dangerouslySetInnerHTML={{ __html: renderedHTML }}
      />
      {isCopied && (
        <div className="copy-message-overlay">Copied to clipboard!</div>
      )}
      <div ref={hiddenRef} className="offscreen" />
    </div>
  );
}
