// highlightDifference tests
import { highlightDifference, formatNumberForLatex } from '../../src/utils/formatting.js';

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
    expected: "3.1415926536 \\cdot 10^{0}" 
  },
  
  // Extreme precision with slight difference
  {
    a: "2.718281828459045235360287471", 
    b: "2.718281828459045235360287472 \\cdot 10^{0}",
    expected: "2.71828182845904523536028747\\textcolor{lightgray}{2} \\cdot 10^{0}" 
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
    a: "299792458", // Speed of light
    b: "2.99792460 \\cdot 10^{8}", // Slightly off
    expected: "2.9979245\\textcolor{lightgray}{60} \\cdot 10^{8}" 
  }
];

// Run the tests
let passCount = 0;
let failCount = 0;

console.log("Running highlightDifference tests...\n");

tests.forEach((test, index) => {
  const actual = highlightDifference(test.a, test.b);
  if (actual === test.expected) {
    console.log(`✅ Test #${index + 1} PASSED: "${test.a}" vs "${test.b}" => "${actual}"`);
    passCount++;
  } else {
    console.error(`❌ Test #${index + 1} FAILED: "${test.a}" vs "${test.b}"`);
    console.error(`   Expected: "${test.expected}"`);
    console.error(`   Actual:   "${actual}"`);
    failCount++;
  }
});

console.log(`\nTests complete. Passed=${passCount}, Failed=${failCount}`);