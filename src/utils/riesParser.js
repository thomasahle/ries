// Utilities for parsing RIES output
import { convertForthToLatex } from './forthParser.js';

// Regular expressions for FORTH format output
// For regular equations with offsets like "x + 1 = 2 x for x = T - 3.5"
const EQUATION_REGEX = /^\s*(.*?)\s*=\s*(.*?)\s+for\s+x\s*=\s*T\s*([+\-]\s*[\d\.e\+\-]+)/;

// For exact match equations like "x = 8 dup* 5 + ('exact' match)"
const EXACT_MATCH_REGEX = /^\s*(.*?)\s*=\s*(.*?)\s+\(['"]exact['"]\s+match\)/;

// The target value as echoed in the banner, e.g. "T = -2.5", "T = 1e+15", "T = 69."
const TARGET_REGEX = /T = (-?[\d.]+(?:e[+-]?\d+)?)/;

/**
 * Parse the RIES output to extract equations in FORTH format.
 * @param {string} text - The raw output from RIES.
 * @returns {Array} Parsed equations with LaTeX formatting; empty when RIES
 *   printed no equations (see getRIESMessage for the explanation to show).
 */
export function parseRIESOutput(text) {
  if (!text || text.length === 0) {
    return [];
  }
  
  const lines = text.split("\n");
  
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

/**
 * Explain why RIES output contains no equations, as plain text (not LaTeX).
 * Option errors are printed as "<program>: <reason>" followed by the usage
 * text; a banner without equations means nothing matched the target.
 * @param {string} text - The raw output from RIES.
 * @returns {string} A human-readable message, or "" if there is nothing to say.
 */
export function getRIESMessage(text) {
  if (!text) {
    return "";
  }
  
  const usageIndex = text.indexOf("Usage:");
  if (usageIndex !== -1) {
    const reason = text
      .slice(0, usageIndex)
      .split("\n")
      .map((line, index) => (index === 0 ? line.replace(/^\S+:\s+/, "") : line).trim())
      .filter(Boolean)
      .join(" ");
    return reason || "RIES could not run with the selected options.";
  }
  
  if (text.includes("Your target value:") || text.includes("mrob.com/ries")) {
    const matchedValue = text.match(TARGET_REGEX);
    return matchedValue
      ? `No equations found for ${matchedValue[1].replace(/\.$/, "")}.`
      : "No equations found.";
  }
  
  return "";
}
