import React, { useRef, useState, useEffect } from 'react';
import { useMathJax } from '../utils/MathJaxContext';
import { highlightDifference } from '../utils/formatting';
import './EquationRow.css';

export default function EquationRow({ equation, targetValue, decimals }) {
  const { isReady, typesetPromise } = useMathJax();
  const hiddenRef = useRef(null);
  const [renderedHTML, setRenderedHTML] = useState('');
  const [isCopied, setIsCopied] = useState(false);

  // Compute x-value based on the frozen targetValue.
  const offsetVal = parseFloat((equation.offset || "").replace(/\s+/g, "")) || 0;
  const Tnum = parseFloat(targetValue);
  const xVal = (!isNaN(Tnum)) ? (Tnum + offsetVal).toFixed(decimals) : targetValue;
  const xValLatex = highlightDifference(targetValue, xVal);

  // Build LaTeX for each part.
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
        setTimeout(() => setIsCopied(false), 800);
      })
      .catch(err => console.error('Failed to copy equation to clipboard', err));
  };

  // Pre-render offscreen: render raw LaTeX into a hidden container, typeset it, then update renderedHTML.
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
    <>
      <div
        className={`equation-row ${isCopied ? 'equation-copied' : ''}`}
        onClick={copyEquation}
        dangerouslySetInnerHTML={{ __html: renderedHTML }}
      />
      <div ref={hiddenRef} className="offscreen" />
    </>
  );
}
