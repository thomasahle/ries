// Utilities for parsing RIES output
import { convertForthToLatex } from './forthParser.js';

// Regular expressions for FORTH format output
// For regular equations with offsets like "x + 1 = 2 x for x = T - 3.5"
const EQUATION_REGEX = /^\s*(.*?)\s*=\s*(.*?)\s+for\s+x\s*=\s*T\s*([+\-]\s*[\d\.e\+\-]+)/;

// For exact match equations like "x = 8 dup* 5 + ('exact' match)"
const EXACT_MATCH_REGEX = /^\s*(.*?)\s*=\s*(.*?)\s+\(['"]exact['"]\s+match\)/;

/**
 * Parse the RIES output to extract equations in FORTH format.
 * @param {string} text - The raw output from RIES.
 * @returns {Array} Parsed equations with LaTeX formatting.
 */
export function parseRIESOutput(text) {
  if (!text || text.length === 0) {
    return [];
  }
  
  const lines = text.split("\n");
  
  // Find both regular offset equations and exact match equations
  const equationLines = lines.filter(line => 
    line.includes("=") && 
    (
      (line.includes("for x = T") && !line.includes("Your target value")) ||
      line.includes("('exact' match)")
    )
  );
  
  if (equationLines.length === 0) {
    if (text.includes("Your target value:") || text.includes("mrob.com/ries")) {
      const matchedValue = text.match(/T = ([\d\.]+)/);
      const valueDisplay = matchedValue ? matchedValue[1] : "value";
      return [{
        lhs: "x",
        rhs: `\\text{No equations found for } ${valueDisplay}`,
        offset: "0"
      }];
    }
    return [];
  }
  
  const results = [];
  for (const line of lines) {
    // Try to match as a regular equation with offset
    let match = line.match(EQUATION_REGEX);
    if (match) {
      const lhsRaw = match[1].trim();
      const rhsRaw = match[2].trim();
      const offsetRaw = match[3].trim();
      
      try {
        results.push({
          lhs: convertForthToLatex(lhsRaw),
          rhs: convertForthToLatex(rhsRaw),
          offset: offsetRaw
        });
      } catch (error) {
        results.push({
          lhs: lhsRaw,
          rhs: rhsRaw,
          offset: offsetRaw || "0"
        });
      }
      continue; // Found a match, proceed to the next line
    }
    
    // Try to match as an exact match equation
    match = line.match(EXACT_MATCH_REGEX);
    if (match) {
      const lhsRaw = match[1].trim();
      const rhsRaw = match[2].trim();
      
      try {
        results.push({
          lhs: convertForthToLatex(lhsRaw),
          rhs: convertForthToLatex(rhsRaw),
          offset: "0" // Exact match means offset is 0
        });
      } catch (error) {
        results.push({
          lhs: lhsRaw,
          rhs: rhsRaw,
          offset: "0"
        });
      }
    }
  }
  
  return results;
}
