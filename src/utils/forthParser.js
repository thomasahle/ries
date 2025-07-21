// forth_parser.js
// A FORTH–style (Format 0) parser for RIES output.
// It tokenizes a compact postfix expression, builds an expression tree,
// and then converts that tree into canonical LaTeX.

/**
 * Creates a function to handle trigonometric functions with pi arguments
 * This handles both regular and fractional pi arguments for sin, cos, tan
 * @param {string} trigName - Name of the trigonometric function (sin, cos, tan)
 * @return {Function} A function that formats trigonometric expressions with pi
 */
function createTrigPiFunction(trigName) {
  return (arg) => {
    // If arg is a simple fraction a/b, output \sin\bigl(\frac{\pi}{b}\bigr)
    const m = arg.match(/^(\d+)\s*\/\s*(\d+)$/);
    if (m) {
      return `\\${trigName}\\bigl(\\frac{\\pi}{${m[2]}}\\bigr)`;
    }
    return `\\${trigName}\\bigl(\\pi ${arg}\\bigr)`;
  };
}

// Define our unary operators (functions).
const UNARY_OPS = {
  // q: square root → \sqrt{...}
  q: (arg) => `\\sqrt{${arg}}`,
  // l/ln: natural logarithm → \ln(...)
  l: (arg) => `\\ln(${arg})`,
  ln: (arg) => `\\ln(${arg})`,
  // s: sine → \sin(...). (We do not add extra parentheses when in function context.)
  s: (arg) => `\\sin(${arg})`,
  // r: reciprocal → \frac{1}{...}
  r: (arg) => `\\frac{1}{${arg}}`,
  recip: (arg) => `\\frac{1}{${arg}}`,
  // log2: logarithm base 2 → \log_{2}(...)
  log2: (arg) => `\\log_{2}(${arg})`,
  sqrt: (arg) => `\\sqrt{${arg}}`,
  // exp: e^x
  exp: (arg) => `e^{${arg}}`,
  // Note: dup* is handled via preprocessing by replacing it with '2 **'
  // neg: negation
  neg: (arg) => `-${arg}`,
  // sinpi, cospi, tanpi: prepend \pi appropriately.
  // Helper function for trigonometric functions with pi
  // Handles both sinpi, cospi, and tanpi
  sinpi: createTrigPiFunction('sin'),
  cospi: createTrigPiFunction('cos'),
  tanpi: createTrigPiFunction('tan')
};

// Define binary operators.
const BINARY_OPS = {
  "+": { prec: 1, op: (a, b) => `${a} + ${b}` },
  "-": { prec: 1, op: (a, b) => `${a} - ${b}` },
  "*": { 
    prec: 2, 
    op: (a, b) => {
      // Check if the second operand is a simple number (like "7" but not "x^2")
      const isSimpleNumber = /^\d+$/.test(b) || /^e\^{\d+}$/.test(b);
      // If b is a simple number, swap the order to put the number on the left
      return isSimpleNumber ? `${b} ${a}` : `${a} ${b}`;
    } 
  },
  "/": { prec: 2, op: (a, b) => `\\frac{${a}}{${b}}` },
  "^": { prec: 3, op: (a, b) => `${a}^{${b}}` },
  "**": { prec: 3, op: (a, b) => `${a}^{${b}}` },  // Power operator in FORTH format
  // We treat atan2 as a binary operator.
  atan2: { prec: 1, op: (a, b) => `\\operatorname{atan_2}(${a},${b})` },
  // Log base N (logN)
  logN: { prec: 3, op: (a, b) => `\\log_{${b}}(${a})` },
  // Root (N-th root of x)
  root: { 
    prec: 3, 
    op: (a, b) => {
      // Check if the base already has an exponent
      const needsParens = a.includes('^');
      
      // Add parentheses if needed
      if (needsParens) {
        return `(${a})^{\\frac{1}{${b}}}`;
      }
      return `${a}^{\\frac{1}{${b}}}`;
    }
  }
};

/**
 * Constants used in FORTH expressions
 * Maps token names to their LaTeX representation
 * @type {Object.<string, string>}
 */
const CONSTANTS = {
  pi: "\\pi",
  p: "\\pi",
  phi: "\\phi",
  e: "e", 
  a: "a",
  b: "b",
  x: "x",
  y: "y"
};

/**
 * Expression tree node class.
 * Represents a node in the abstract syntax tree for a FORTH expression.
 */
class Node {
  /**
   * Create a new node
   * @param {string} type - Type of node: "num", "var", "const", "op", or "func"
   * @param {string} value - The token value or operator symbol
   * @param {Node[]} children - Child nodes (operands)
   * @param {boolean} requiresBracketsForExponentiation - Whether this node needs to be wrapped in brackets when exponentiated
   */
  constructor(type, value, children = [], requiresBracketsForExponentiation = false) {
    this.type = type;
    this.value = value;
    this.children = children;
    this.requiresBracketsForExponentiation = requiresBracketsForExponentiation;
  }
}

/**
 * Main exported function.
 * Converts a FORTH-format postfix expression into canonical LaTeX.
 * @param {string} input - The FORTH-format postfix expression to convert
 * @return {string} Converted LaTeX expression
 */
export function convertForthToLatex(input) {
  // Preprocess: replace 'dup*' (square operation) with '2 **'
  const processedInput = input.replace(/\bdup\*/g, '2 **');

  // Parse the input and convert to LaTeX
  const tokens = processedInput.split(/\s+/).filter(t => t.length > 0);
  const tree = parsePostfix(tokens);
  return toLatex(tree);
}

/**
 * Parse the tokens (postfix) into an expression tree.
 * @param {string[]} tokens - Array of tokens in postfix notation
 * @return {Node} The root node of the expression tree
 */
// Precompile numeric literal regexes for decimals and simple fractions
const DECIMAL_RE = /^-?(?:\d+(?:\.\d*)?|\.\d+)$/;
const FRACTION_RE = /^-?\d+\/\d+$/;
function parsePostfix(tokens) {
  const stack = [];
  
  for (const token of tokens) {
    // Numeric literals: decimals or simple integer fractions
    if (DECIMAL_RE.test(token) || FRACTION_RE.test(token)) {
      stack.push(new Node("num", token));
    }
    // Variables (single letters)
    else if (/^[a-z]$/.test(token)) {
      stack.push(new Node("var", token));
    }
    // Constants
    else if (token in CONSTANTS) {
      stack.push(new Node("const", CONSTANTS[token]));
    }
    // Unary operators (functions)
    else if (token in UNARY_OPS) {
      if (stack.length < 1) {
        throw new Error(`Insufficient operands for unary op ${token}`);
      }
      const arg = stack.pop();
      
      // For exponentiation, we need parentheses around exp() and negation
      const requiresBrackets = token === 'exp' || token === 'neg';
      
      stack.push(new Node("func", token, [arg], requiresBrackets));
    }
    // Binary operators
    else if (token in BINARY_OPS) {
      if (stack.length < 2) {
        throw new Error(`Insufficient operands for binary op ${token}`);
      }
      const right = stack.pop();
      const left = stack.pop();
      
      // Operations that need brackets when exponentiated
      const needsBrackets = 
          token === "+" || token === "-" || 
          token === "/" || token === "root" ||
          token === "^" || token === "**" ||
          token === "*";
      
      stack.push(new Node("op", token, [left, right], needsBrackets));
    }
    // Unrecognized token
    else {
      throw new Error(`Unknown token: ${token}`);
    }
  }
  
  if (stack.length !== 1) {
    throw new Error(`Invalid postfix expression; stack=${stack.length}`);
  }
  
  return stack[0];
}


/**
 * Convert an expression tree into LaTeX.
 * @param {Node} node - The root node of the expression tree to convert
 * @param {number} parentPrec - The precedence of the parent operator (for proper parenthesization)
 * @param {boolean} inFunc - Whether we're in a function argument context (if true, no extra parentheses)
 * @return {string} The LaTeX representation of the expression tree
 */
function toLatex(node, parentPrec = 0, inFunc = false) {
  if (node.type === "num" || node.type === "var" || node.type === "const") {
    return node.value;
  } else if (node.type === "func") {
    const fn = UNARY_OPS[node.value];
    if (!fn) throw new Error("Unknown unary op: " + node.value);
    // Special case: negating a sum or difference should parenthesize
    if (node.value === 'neg' && node.children[0].type === 'op' && 
        (node.children[0].value === '+' || node.children[0].value === '-')) {
      const expr = toLatex(node.children[0], 0, true);
      return `-(${expr})`;
    }
    // Otherwise just apply the unary formatting function
    const argLatex = toLatex(node.children[0], 0, true);
    return fn(argLatex);
  } else if (node.type === "op") {
    const opInfo = BINARY_OPS[node.value];
    if (!opInfo) throw new Error("Unknown binary op: " + node.value);
    const prec = opInfo.prec;
    
    // For fractions, we need special handling of nested expressions
    if (node.value === "/") {
      const numerator = toLatex(node.children[0], 0, true);
      const denominator = toLatex(node.children[1], 0, true);
      
      // No extra parentheses by default
      return `\\frac{${numerator}}{${denominator}}`;
    }
    
    // Special handling for powers (both ^ and **)
    if (node.value === "^" || node.value === "**") {
      // Generate base LaTeX
      const baseNode = node.children[0];
      const base = toLatex(baseNode, prec, inFunc);
      const baseRequiresParentheses = baseNode.requiresBracketsForExponentiation;
      
      // Handle fraction exponents specially for formatting
      if (node.children[1].type === "op" && node.children[1].value === "/") {
        const numerator = toLatex(node.children[1].children[0], 0, true);
        const denominator = toLatex(node.children[1].children[1], 0, true);
        
        // Use simpler notation for basic fractions
        const isSimpleFraction = numerator === "1" || 
            (numerator.length <= 2 && denominator.length <= 2) || 
            (/^-?\d+$/.test(numerator) && /^-?\d+$/.test(denominator));
            
        const fracExponent = isSimpleFraction ? 
            `${numerator}/${denominator}` : 
            `\\frac{${numerator}}{${denominator}}`;
        
        return baseRequiresParentheses ? 
            `(${base})^{${fracExponent}}` : 
            `${base}^{${fracExponent}}`;
      }
      
      // For non-fraction exponents
      const exponent = toLatex(node.children[1], 0, true);
      
      return baseRequiresParentheses ? 
          `(${base})^{${exponent}}` : 
          `${base}^{${exponent}}`;
    }
    
    // Special handling for multiplication to ensure proper parenthesization
    if (node.value === "*") {
      // Process left and right operands
      const leftNode = node.children[0];
      const rightNode = node.children[1];
      
      // If both operands are numeric literals, render explicit dot multiplication
      if (leftNode.type === "num" && rightNode.type === "num") {
        const left = toLatex(leftNode, 0, inFunc);
        const right = toLatex(rightNode, 0, inFunc);
        return `${left} \\cdot ${right}`;
      }
      
      // For multiplication, we need parentheses only for + and - operations
      const needsLeftParens = leftNode.type === "op" && (leftNode.value === "+" || leftNode.value === "-");
      const needsRightParens = rightNode.type === "op" && (rightNode.value === "+" || rightNode.value === "-");
      
      // Get the raw expressions
      const left = toLatex(leftNode, 0, inFunc);
      const right = toLatex(rightNode, 0, inFunc);
      
      // Apply parentheses where needed
      const formattedLeft = needsLeftParens ? `(${left})` : left;
      const formattedRight = needsRightParens ? `(${right})` : right;
      
      // Check for numeric right operand to follow conventional notation (e.g., "2 x" instead of "x 2")
      const isRightSimpleNumber = /^\d+$/.test(right) || /^e\^{\d+}$/.test(right);
      
      return isRightSimpleNumber ? 
          `${right} ${formattedLeft}` : 
          `${formattedLeft} ${formattedRight}`;
    }
    
    // Special case for operations inside exponentiation to avoid double-parenthesization
    if (node.requiresBracketsForExponentiation && parentPrec === 3) {
      const left = toLatex(node.children[0], prec, inFunc);
      const right = toLatex(node.children[1], 0, true);
      return opInfo.op(left, right);
    }
    
    // Normal case for all other binary operations
    const left = toLatex(node.children[0], prec, inFunc);
    
    // For subtraction, check if the right operand needs parentheses for disambiguation
    // Only + and - on the right side of subtraction create ambiguity
    let rightPrec = prec;
    if (node.value === "-" && node.children[1].type === "op") {
      const rightOp = node.children[1].value;
      if (rightOp === "+" || rightOp === "-") {
        rightPrec = Infinity; // Force parentheses
      }
    }
    
    const right = toLatex(node.children[1], rightPrec, inFunc);
    
    let expr = opInfo.op(left, right);
    
    // Add parentheses if needed based on operator precedence
    if (!inFunc && prec < parentPrec && node.requiresBracketsForExponentiation) {
      expr = `(${expr})`;
    }
    
    return expr;
  }
  
  throw new Error("Unknown node type: " + node.type);
}
