// test_forth_parser.js
import { convertForthToLatex } from "./src/utils/forthParser.js";

const tests = [
  // "phi" → just "phi"
  { input: "phi", expected: "\\phi" },
  // "phi^2" → postfix: "phi 2 ^" → "phi2^" (no spaces)
  { input: "phi 2 ^", expected: "\\phi^{2}" },
  // "phi^-1" → "phi -1 ^" → "phi-1^"
  { input: "phi -1 ^", expected: "\\phi^{-1}" },
  // "phi^(1/2)" → we represent as "phi 1/2 ^" → "phi1/2^"
  { input: "phi 1/2 ^", expected: "\\phi^{1/2}" },
  // "1/(x+phi)" → postfix: "x phi + 1 /" → "xphi+1/"
  { input: "1 x phi + /", expected: "\\frac{1}{x + \\phi}" },
  // "cospi(1/9)" → postfix: "1/9 cospi" → "1/9cospi"
  { input: "1/9 cospi", expected: "\\cos\\bigl(\\frac{\\pi}{9}\\bigr)" },
  // "atan2(a,b)" → postfix: "a b atan2" → "abatan2"
  { input: "a b atan2", expected: "\\operatorname{atan_2}(a,b)" },
  // "sinpi(sinpi(x))" → postfix: "x sinpi sinpi" → "xsinpisinpi"
  { input: "x sinpi sinpi", expected: "\\sin\\bigl(\\pi \\sin\\bigl(\\pi x\\bigr)\\bigr)" },
  // "atan2(e,(1/8))" → postfix: "e 1/8 atan2" → "e1/8/atan2"
  { input: "e 1/8 atan2", expected: "\\operatorname{atan_2}(e,1/8)" },
  // "sqrt(pi sqrt(3))" → first, note that sqrt(…) is in infix;
  // In FORTH, this is written as "pi 3 sqrt sqrt" → "pi3sqrtsqrt"
  { input: "pi 3 sqrt * sqrt", expected: "\\sqrt{\\pi \\sqrt{3}}" },
  // "1/-9" → postfix: "1 -9 /" → "1-9/"
  { input: "1 -9 /", expected: "\\frac{1}{-9}" },
  // "(log_2(x))^2/x" → we use token "log2" for log base2.
  // In postfix: "x log2 2 ^ x /" → "xlog22^x/"
  { input: "x log2 2 ^ x /", expected: "\\frac{\\log_{2}(x)^{2}}{x}" },
  // "sinpi(atan2(1,pi))" → postfix: "1 pi atan2 sinpi" → "1piatan2sinpi"
  { input: "1 pi atan2 sinpi", expected: "\\sin\\bigl(\\pi \\operatorname{atan_2}(1,\\pi)\\bigr)" },
  // "1/(atan2(cospi(x),1))^2" → postfix representation
  { input: "1 x cospi 1 atan2 2 ^ /", expected: "\\frac{1}{\\operatorname{atan_2}(\\cos\\bigl(\\pi x\\bigr),1)^{2}}" },
  // "1/e^(4\"/pi)" → here, we interpret 4"/pi as a notation.
  // Using Forth notation for "e to the power of pi to the power of 1/4", then take reciprocal:
  { input: "1 e pi 1/4 ^ ^ /", expected: "\\frac{1}{e^{\\pi^{1/4}}}" },
  // "cospi(1/7\"/4)" → using Forth notation for "cospi of 1 over 4 to the power of 1/7"
  { input: "1 4 1/7 ^ / cospi", expected: "\\cos\\bigl(\\pi \\frac{1}{4^{1/7}}\\bigr)" },
  // "4-atan2(6)" → postfix: "4 6 atan2 -" in correct postfix form
  { input: "4 6 1 atan2 -", expected: "4 - \\operatorname{atan_2}(6,1)" },
  // "ln(4)" → postfix: "4 ln" → "4ln"
  { input: "4 ln", expected: "\\ln(4)" },
  // "phi\"/3" → using Forth notation for "3 to power of 1/phi"
  { input: "3 1 phi / ^", expected: "3^{1/\\phi}" },
  // "phi^3/2 + 1" → postfix: "phi 3 ^ 2 / 1 +" → "phi3^2/1+"
  { input: "phi 3 ^ 2 / 1 +", expected: "\\frac{\\phi^{3}}{2} + 1" },
  // "1/(2-1/e)^2" → postfix representation in correct form
  // For this representation, we expect 2-1/e to be calculated first, then raised to power 2
  { input: "1 2 1 e / - 2 ^ /", expected: "\\frac{1}{(2 - \\frac{1}{e})^{2}}" },
  // "e^(1/e)" → postfix: "e 1 e / ^" → "e1e/^"
  { input: "e 1 e / ^", expected: "e^{1/e}" },
  // "atan2((pi-3),4)" → postfix: For (pi-3), we use "pi 3 -" → "pi3-" then push 4, then atan2 → "pi3-4atan2"
  { input: "pi 3 - 4 atan2", expected: "\\operatorname{atan_2}(\\pi - 3,4)" },

  // NEW CHALLENGING TESTS
  
  // "sqrt(sqrt(sqrt(x)))" → postfix: "x sqrt sqrt sqrt"
  { input: "x sqrt sqrt sqrt", expected: "\\sqrt{\\sqrt{\\sqrt{x}}}" },
  
  // "(ln(x))^(ln(ln(x)))" → postfix: "x ln x ln ln ^"
  { input: "x ln x ln ln ^", expected: "\\ln(x)^{\\ln(\\ln(x))}" },
  
  // "1/(1/x+1)" → simpler postfix representation
  { input: "1 1 x / 1 + /", expected: "\\frac{1}{\\frac{1}{x} + 1}" },
  
  // "sinpi(cospi(tanpi(x)))" → postfix: "x tanpi cospi sinpi"
  { input: "x tanpi cospi sinpi", expected: "\\sin\\bigl(\\pi \\cos\\bigl(\\pi \\tan\\bigl(\\pi x\\bigr)\\bigr)\\bigr)" },
  
  // "log2(atan2(sinpi(x),cospi(y)))" → postfix: "x sinpi y cospi atan2 log2"
  { input: "x sinpi y cospi atan2 log2", expected: "\\log_{2}(\\operatorname{atan_2}(\\sin\\bigl(\\pi x\\bigr),\\cos\\bigl(\\pi y\\bigr)))" },
  
  // "phi^(1/phi) / (e^(1/e) * pi^(1/pi))" → postfix: "phi 1 phi / ^ e 1 e / ^ pi 1 pi / ^ * /"
  { input: "phi 1 phi / ^ e 1 e / ^ pi 1 pi / ^ * /", expected: "\\frac{\\phi^{1/\\phi}}{e^{1/e} \\pi^{1/\\pi}}" },
  
  // "1/(sqrt(1+sqrt(1+sqrt(1+x))))" → postfix representation in correct order for a stack machine
  { input: "1 1 x + sqrt 1 + sqrt 1 + sqrt /", expected: "\\frac{1}{\\sqrt{\\sqrt{\\sqrt{1 + x} + 1} + 1}}" },
  
  // Nested atan2 with multiple arguments and operations
  // "atan2(atan2(x,y), atan2(a,b))" → postfix: "x y atan2 a b atan2 atan2"
  { input: "x y atan2 a b atan2 atan2", expected: "\\operatorname{atan_2}(\\operatorname{atan_2}(x,y),\\operatorname{atan_2}(a,b))" },
  
  // Complex expression with multiple root notations
  // "(pi\"/e)^(phi\"/3)" → postfix: using standard notation instead
  { input: "e 1 pi / ^ 3 1 phi / ^ ^", expected: "(e^{1/\\pi})^{3^{1/\\phi}}" },
  
  // Deeply nested fractions
  // "(1/(2/(3/(4/x))))" → postfix: "1 2 3 4 x / / / /"
  { input: "1 2 3 4 x / / / /", expected: "\\frac{1}{\\frac{2}{\\frac{3}{\\frac{4}{x}}}}" },
  
  // Deeply nested powers with mixed operations
  // "(((x^2)^3)^4)^5" → postfix: "x 2 ^ 3 ^ 4 ^ 5 ^"
  { input: "x 2 ^ 3 ^ 4 ^ 5 ^", expected: "(((x^{2})^{3})^{4})^{5}" },
  
  // Complex combination of operations
  // "ln(sqrt(x^2 + y^2)) / log2(atan2(sinpi(a), cospi(b)))"
  { input: "x 2 ^ y 2 ^ + sqrt ln a sinpi b cospi atan2 log2 /", expected: "\\frac{\\ln(\\sqrt{x^{2} + y^{2}})}{\\log_{2}(\\operatorname{atan_2}(\\sin\\bigl(\\pi a\\bigr),\\cos\\bigl(\\pi b\\bigr)))}" },
  
  // Very complex triple nested root notation with operations
  // "(1\"/((2\"/3)+(4\"/5)))" → transformed to more standard notation
  { input: "3 1 2 / ^ 5 1 4 / ^ + -1 ^", expected: "(3^{1/2} + 5^{1/4})^{-1}" },
  
  // Test with negative fractions in powers
  // "x^(-3/4) + y^(-2/5)" → postfix: "x -3/4 ^ y -2/5 ^ +"
  { input: "x -3/4 ^ y -2/5 ^ +", expected: "x^{-3/4} + y^{-2/5}" },
  
  // Tests for FORTH-specific tokens
  
  // "sqrt(x)^x" - using sqrt and root (x-th root)
  { input: "x sqrt x root", expected: "\\sqrt{x}^{\\frac{1}{x}}" },
  
  // "1/x * e^2" - using recip (reciprocal) and exp (e^x)
  { input: "x recip 2 exp *", expected: "e^{2} \\frac{1}{x}" },
  
  // "x^2 * log_cospi(x)(x)" - using dup* (square) and logN (log base N)
  { input: "x dup* x x cospi logN *", expected: "x^{2} \\log_{\\cos\\bigl(\\pi x\\bigr)}(x)" },
  
  // "x^pi * 7" - using ** (power)
  { input: "x pi ** 7 *", expected: "7 x^{\\pi}" },
  
  // "e^3 - 1" - using exp (e^x)
  { input: "3 exp 1 -", expected: "e^{3} - 1" },
  
  // "sqrt(x)" - using sqrt
  { input: "x sqrt", expected: "\\sqrt{x}" },
  
  // "x^2" - using dup* for squaring
  { input: "x dup*", expected: "x^{2}" },
  
  // "-x/2 - 1" - using neg (negation)
  { input: "x 2 / neg 1 -", expected: "-\\frac{x}{2} - 1" },
  
  // "-(x+1)" - using neg with addition
  { input: "x 1 + neg", expected: "-(x + 1)" },
  
  // "(e+1/5)^2" - adding a constant and reciprocal, then squaring
  { input: "e 5 recip + dup*", expected: "(e + \\frac{1}{5})^{2}" },
  // "(4^π)^2" - testing nested exponentiation with dup*
  { input: "4 pi ** dup*", expected: "(4^{\\pi})^{2}"},
  // "((x^2)^(1/3))^2" - testing complex nesting with root and multiple dup*
  { input: "x dup* 3 root dup*", expected: "((x^{2})^{\\frac{1}{3}})^{2}"},
  // "((x^2)^(1/3))^(1/4)" - testing even more complex nesting with multiple exponentiation
  { input: "x dup* 3 root 4 root", expected: "((x^{2})^{\\frac{1}{3}})^{\\frac{1}{4}}"},
  // "(x + 1/5)^5" - testing addition with reciprocal followed by exponentiation
  { input: "x 5 recip + 5 **", expected: "(x + \\frac{1}{5})^{5}"},
  // "(sin(π/64))^2" - testing trig function with squaring
  { input: "8 dup* recip sinpi dup*", expected: "\\sin\\bigl(\\pi \\frac{1}{8^{2}}\\bigr)^{2}"},
   {input: "x neg 1 + pi logN", expected: "\\log_{\\pi}(-x + 1)"},
   {input: "pi recip exp dup* recip", expected: "\\frac{1}{(e^{\\frac{1}{\\pi}})^{2}}"},
   {input: "8 dup* pi **", expected: "(8^{2})^{\\pi}"},
];

let passCount = 0;
let failCount = 0;

tests.forEach(({ input, expected }) => {
  try {
    const actual = convertForthToLatex(input);
    if (actual === expected) {
      console.log(`PASS: "${input}" => "${actual}"`);
      passCount++;
    } else {
      console.error(`FAIL: "${input}" => got "${actual}", expected "${expected}"`);
      failCount++;
    }
  } catch (err) {
    console.error(`ERROR processing "${input}": ${err.message}`);
    failCount++;
  }
});

console.log(`\nTests complete. Passed=${passCount}, Failed=${failCount}`);
