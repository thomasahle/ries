/**
 * Highlights the difference between two similar numbers in LaTeX format
 * E.g. highlightDifference("0.23490942","0.23606798") => "0.23\\textcolor{gray}{606798}"
 * 
 * @param {string} strA - First number as string
 * @param {string} strB - Second number as string
 * @returns {string} LaTeX formatted string with difference highlighted
 */
export function highlightDifference(strA, strB) {
  // Find the point where the strings start to differ
  let i = 0;
  const len = Math.min(strA.length, strB.length);
  while (i < len && strA[i] === strB[i]) i++;
  
  // Split into matching prefix and different remainder
  const prefix = strB.slice(0, i);
  const remainder = strB.slice(i);
  
  // Return with LaTeX formatting to highlight the difference
  return remainder ? prefix + `\\textcolor{lightgray}{${remainder}}` : prefix;
}