// tests/parsing.test.js - Tests for RIES output parsing
// Bug discovered: Scientific notation with e+ (like "T - 2.2178e+07") isn't parsed correctly

import assert from 'assert';
import { parseRIESOutput } from "../src/utils/riesParser.js";

console.log("TESTING RIES OUTPUT PARSING WITH SCIENTIFIC NOTATION");
console.log("===================================================");

// These test cases have full RIES output with different types of scientific notation and edge cases
const testCases = [
  {
    description: "Regular decimal offset",
    input: `Your target value: 10
x + 1 = 2 x for x = T - 3.5
`,
    expected: {
      lhs: "x + 1",
      rhs: "2 x",
      offset: "- 3.5"
    }
  },
  {
    description: "Negative scientific notation (e-)",
    input: `Your target value: 0.142857
x ln = 1/7 for x = T - 2.2178e-07
`,
    expected: {
      lhs: "x ln",
      rhs: "1/7",
      offset: "- 2.2178e-07"
    }
  },
  {
    description: "Scientific notation with e+ (FAILS WITH BUG)",
    input: `Your target value: 3.14159
x 2 ^ = phi for x = T - 2.2178e+07
`,
    expected: {
      lhs: "x 2 ^",
      rhs: "phi",
      offset: "- 2.2178e+07"
    }
  },
  {
    description: "Positive offset with scientific notation (FAILS WITH BUG)",
    input: `Your target value: 42
x e ^ = phi 2 ^ for x = T + 1.234e+05
`,
    expected: {
      lhs: "x e ^",
      rhs: "phi 2 ^",
      offset: "+ 1.234e+05"
    }
  },
  {
    description: "Exact match line (FAILS WITH BUG)",
    input: `Your target value: T = 69.                                   
x = 8 dup* 5 +                              ('exact' match)         {64}
`,
    expected: {
      lhs: "x",
      // Note: We're expecting the LaTeX-converted version now
      rhs: "8^{2} + 5",
      offset: "0" // Exact match means no offset
    }
  }
];

// Helper function to calculate what the actual value would be
function calculateActualValue(targetValue, offsetStr) {
  if (!offsetStr) return NaN;
  
  const targetNum = parseFloat(targetValue);
  let offsetNum = 0;
  
  try {
    // Clean up the offset string by separating sign and number
    const cleanOffset = offsetStr.replace(/([+\-])/, " $1").trim();
    const sign = cleanOffset.charAt(0);
    const numStr = cleanOffset.substring(1).trim();
    
    offsetNum = parseFloat(numStr);
    if (sign === '-') offsetNum = -offsetNum;
  } catch (e) {
    console.error(`Error parsing offset: ${e.message}`);
    return NaN;
  }
  
  return targetNum + offsetNum;
}

// Format number for display
function formatNumber(num) {
  if (isNaN(num)) return "NaN";
  if (Math.abs(num) < 0.0001 || Math.abs(num) > 10000000) {
    return num.toExponential(4);
  }
  return num.toFixed(6);
}

// Adjust test title for clarity
console.log("\nTESTING RIES OUTPUT PARSING WITH SCIENTIFIC NOTATION AND EXACT MATCHES");
console.log("================================================================");

// Run the tests
console.log("\nRUNNING TESTS:");
console.log("==============");

let passedTests = 0;
let failedTests = 0;

// Function to check if exact match test case passed
function checkExactMatchTestCase(testCase) {
  // Extract expected fields
  const { lhs, rhs } = testCase.expected;
  
  // Extract the equation line directly
  const lines = testCase.input.split('\n');
  const equationLine = lines.find(line => line.includes("('exact' match)"));
  
  // This test should fail with current implementation
  if (!equationLine) {
    return { passed: false, error: "No exact match line found" };
  }
  
  // Check if the equation parts are properly extracted
  const result = parseRIESOutput(testCase.input);
  if (result.length === 0) {
    return { 
      passed: false, 
      error: "Exact match line not parsed at all",
      details: "The parser did not extract any equations from the input with ('exact' match)"
    };
  }
  
  // This might pass unexpectedly in some cases, so we'll look deeper
  const parsedEquation = result[0];
  
  // Check if the exact match was properly identified
  const isExactMatch = equationLine.includes("('exact' match)");
  const hasProperOffset = parsedEquation.offset === "0";
  
  return {
    passed: parsedEquation.lhs === lhs && parsedEquation.rhs === rhs && hasProperOffset,
    actual: parsedEquation,
    expected: testCase.expected,
    isExactMatch,
    error: !hasProperOffset ? "Offset not properly set for exact match" : null
  };
}

testCases.forEach((testCase, index) => {
  console.log(`\nTest #${index + 1}: ${testCase.description}`);
  
  // Special handling for exact match test
  if (testCase.description.includes("Exact match")) {
    const result = checkExactMatchTestCase(testCase);
    
    if (result.passed) {
      console.log(`✅ PASS: Successfully parsed exact match equation`);
      passedTests++;
    } else {
      failedTests++;
      console.log(`❌ FAILED: ${result.error || "Failed to correctly parse exact match"}`);
      console.log("\nDiagnostic information:");
      console.log(`Raw input with exact match: "${testCase.input.trim()}"`);
      
      if (result.actual) {
        console.log(`Expected LHS: "${testCase.expected.lhs}", got: "${result.actual.lhs}"`);
        console.log(`Expected RHS: "${testCase.expected.rhs}", got: "${result.actual.rhs}"`);
        console.log(`Expected offset: "${testCase.expected.offset}", got: "${result.actual.offset}"`);
      } else {
        console.log("The parser didn't extract any equation from the input.");
      }
    }
    return;
  }
  
  // Standard tests for regular equations
  try {
    // Call the actual parseRIESOutput function from the source code
    const result = parseRIESOutput(testCase.input);
    
    // There should be exactly one equation parsed
    assert.strictEqual(
      result.length, 
      1, 
      `Expected to parse exactly 1 equation from the input, but got ${result.length}`
    );
    
    const parsedEquation = result[0];
    
    // NOTE: We're only testing the OFFSET parsing bug, not the LaTeX conversion
    // Skipping LHS/RHS tests as they involve convertForthToLatex which has its own tests
    
    // Test offset - this is where the bug is!
    assert.strictEqual(
      parsedEquation.offset,
      testCase.expected.offset,
      `Expected offset "${testCase.expected.offset}", but got "${parsedEquation.offset}"`
    );
    
    console.log(`✅ PASS: Successfully parsed equation with ${testCase.description}`);
    passedTests++;
    
  } catch (err) {
    failedTests++;
    console.log(`❌ FAILED: ${err.message}`);
    
    // Show detailed diagnostic information
    const result = parseRIESOutput(testCase.input);
    if (result && result.length > 0) {
      const parsedEquation = result[0];
      
      // Show what was parsed versus what was expected
      console.log("\nDiagnostic information:");
      console.log(`Raw input: "${testCase.input.trim()}"`);
      console.log(`Expected offset: "${testCase.expected.offset}"`);
      console.log(`Actual parsed offset: "${parsedEquation.offset}"`);
      
      // Show the numerical impact
      const targetValueMatch = testCase.input.match(/Your target value: (?:T = )?([\d\.]+)/);
      const targetValue = targetValueMatch ? targetValueMatch[1] : '0';
      const expectedValue = calculateActualValue(targetValue, testCase.expected.offset);
      const actualValue = calculateActualValue(targetValue, parsedEquation.offset);
      
      console.log(`\nNumerical impact of the bug:`);
      console.log(`Target value: ${targetValue}`);
      console.log(`With correct offset: value = ${formatNumber(expectedValue)} (CORRECT)`);
      console.log(`With parsed offset: value = ${formatNumber(actualValue)} (WRONG)`);
      
      if (!isNaN(actualValue) && !isNaN(expectedValue) && actualValue !== 0) {
        const errorFactor = Math.abs(expectedValue / actualValue);
        const errorDirection = errorFactor > 1 ? 'too small' : 'too large';
        console.log(`Error magnitude: ${errorFactor.toExponential(2)}x ${errorDirection}`);
      }
    }
  }
});

// Summary
console.log("\n\nTEST SUMMARY:");
console.log("=============");
console.log(`Passed: ${passedTests} test cases`);
console.log(`Failed: ${failedTests} test cases`);

if (failedTests > 0) {
  console.log("\nBUGS CONFIRMED:");
  console.log("1. parseRIESOutput doesn't properly handle scientific notation with e+");
  console.log("2. parseRIESOutput doesn't handle 'exact match' lines");
  
  console.log("\nSuggested fixes:");
  console.log("1. For scientific notation: Fix the regex pattern to include '+' in scientific notation:");
  console.log(`   FROM: /^\\s*(.*?)\\s*=\\s*(.*?)\\s+for\\s+x\\s*=\\s*T\\s*([+\\-]\\s*[\\d\\.e\\-]+)/`);
  console.log(`   TO:   /^\\s*(.*?)\\s*=\\s*(.*?)\\s+for\\s+x\\s*=\\s*T\\s*([+\\-]\\s*[\\d\\.e\\+\\-]+)/`);
  
  console.log("2. For exact matches: Add a second regex pattern to match lines with ('exact' match):");
  console.log(`   Add: const EXACT_MATCH_REGEX = /^\\s*(.*?)\\s*=\\s*(.*?)\\s+\\(['"]exact['"]\\s+match\\)/;`);
  
  // Force a proper error for CI/CD environments
  assert.strictEqual(failedTests, 0, `${failedTests} tests failed - parseRIESOutput has bugs with scientific notation and exact matches`);
} else {
  console.log("\nALL TESTS PASSED: parseRIESOutput correctly handles all notation formats and exact matches");
}