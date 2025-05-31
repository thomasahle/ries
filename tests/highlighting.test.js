// highlightDifference tests
import test from 'node:test';
import assert from 'node:assert/strict';
import { highlightDifference, formatNumberForLatex } from '../src/utils/formatting.js';

const tests = [
  // Basic decimal comparison
  {
    a: "0.23490942",
    b: "0.23606798", 
    expected: "0.23\\textcolor{lightgray}{606798}"
  },
  
  // Comparing a large number with its scientific notation
  {
    a: "12345600000", 
    b: "1.23456 \\cdot 10^{10}",
    expected: "1.23456 \\cdot 10^{10}" // All digits are correct
  },
  
  // Comparing a large number with slightly different scientific notation
  {
    a: "12345600000", 
    b: "1.234567 \\cdot 10^{10}",
    expected: "1.23456\\textcolor{lightgray}{7} \\cdot 10^{10}" // Only the 7 is wrong
  },
  
  // Comparing a large number with significantly different scientific notation
  {
    a: "12345600000", 
    b: "1.23499 \\cdot 10^{10}",
    expected: "1.234\\textcolor{lightgray}{99} \\cdot 10^{10}" // 99 is wrong
  },

  // Testing with e notation instead of cdot
  {
    a: "12345600000", 
    b: "1.23456e10",
    expected: "1.23456e10" // All digits are correct
  },
  
  // Regular number with a slight difference
  {
    a: "123.456", 
    b: "123.457",
    expected: "123.45\\textcolor{lightgray}{7}" // Just the 7 is wrong
  },
  
  // Exactly equal numbers
  {
    a: "123.456", 
    b: "123.456",
    expected: "123.456" // Exact match, no highlighting
  },

  // Scientific notation with imprecise original
  {
    a: "1.2e10", 
    b: "1.23456 \\cdot 10^{10}",
    expected: "1.2\\textcolor{lightgray}{3456} \\cdot 10^{10}" // Only 1.2 is confirmed
  },
  
  // Different scales of precision
  {
    a: "12345678901234", 
    b: "1.234567890123 \\cdot 10^{13}",
    expected: "1.234567890123 \\cdot 10^{13}" // All digits match
  },
  
  // Testing with trailing zeros
  {
    a: "123000", 
    b: "1.23 \\cdot 10^{5}",
    expected: "1.23 \\cdot 10^{5}" // All significant digits match
  },
  
  // Testing with leading zeros
  {
    a: "0.0001234", 
    b: "1.234 \\cdot 10^{-4}",
    expected: "1.234 \\cdot 10^{-4}" // All digits should match
  },
  
  // ADDITIONAL CHALLENGING TESTS
  
  // Negative scientific notation
  {
    a: "-98765432100", 
    b: "-9.87654321 \\cdot 10^{10}",
    expected: "-9.87654321 \\cdot 10^{10}" 
  },
  
  // Different representation but same value (PI)
  {
    a: "3.14159265359", 
    b: "3.1415926536 \\cdot 10^{0}",
    expected: "3.141592653\\textcolor{lightgray}{6}" 
  },
  
  // Extreme precision with slight difference
  {
    a:        "2.718281828459045235360287471", 
    b:        "2.718281828459045235360287472 \\cdot 10^{0}",
    expected: "2.71828182845904523536028747\\textcolor{lightgray}{2}" 
  },
  
  // Very small number with trailing differences
  {
    a: "0.0000000000000000000000000001", 
    b: "1.000000000000000000000000002 \\cdot 10^{-28}",
    expected: "1.00000000000000000000000000\\textcolor{lightgray}{2} \\cdot 10^{-28}" 
  },
  
  // Different e notation base but same value
  {
    a: "1.23e-5", 
    b: "12.3 \\cdot 10^{-6}",
    expected: "12.3 \\cdot 10^{-6}" 
  },
  
  // Two scientific notations, different precision
  {
    a: "6.626e-34", 
    b: "6.62607015 \\cdot 10^{-34}",
    expected: "6.626\\textcolor{lightgray}{07015} \\cdot 10^{-34}" 
  },
  
  // Zeros in the middle
  {
    a: "10020003000", 
    b: "1.002000300 \\cdot 10^{10}",
    expected: "1.002000300 \\cdot 10^{10}" 
  },
  
  // Scientific notation comparing to similar but different value
  {
    a:         "299792458", // Speed of light
    b:        "2.99792460 \\cdot 10^{8}", // Slightly off
    expected: "2.997924\\textcolor{lightgray}{60} \\cdot 10^{8}" 
  },
  {
    a:         "3",
    b:        "3.0000000",
    expected: "3.0000000"
  },
  {
    a:        "3.1",
    b:        "3.1004410",
    expected: "3.100\\textcolor{lightgray}{4410}"
  },
  {
    a:        "3.",
    b:        "3.0123",
    expected: "3.0\\textcolor{lightgray}{123}"
  }
];

// Run tests using Node's built-in test runner
for (const { a, b, expected } of tests) {
  test(`highlightDifference("${a}", "${b}")`, () => {
    assert.strictEqual(
      highlightDifference(a, b),
      expected,
      `for inputs a="${a}" and b="${b}"`
    );
  });
}
