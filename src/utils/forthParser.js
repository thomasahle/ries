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
  try {
    // Preprocess: replace 'dup*' (square operation) with '2 **' for consistent handling
    // This allows us to use the power operation instead of a special case
    let processedInput = input.replace(/\bdup\*/g, '2 **');
    
    // Split the input by spaces to get tokens
    const tokens = processedInput.split(/\s+/).filter(t => t.length > 0);
    const tree = parsePostfix(tokens);
    
    return toLatex(tree);
  } catch (error) {
    console.error(`Error parsing ${input}: ${error.message}`);
    return "\\text{Error: " + error.message + "}";
  }
}

/**
 * Parse the tokens (postfix) into an expression tree.
 * @param {string[]} tokens - Array of tokens in postfix notation
 * @return {Node} The root node of the expression tree
 */
function parsePostfix(tokens) {
  const stack = [];
  
  for (let i = 0; i < tokens.length; i++) {
    const token = tokens[i];
    
    // Numeric literals including fractions and negative numbers
    if (/^-?\d+(\.\d+)?$/.test(token) || /^-?\d+\/\d+$/.test(token)) {
      stack.push(new Node("num", token));
      continue;
    }
    
    // Variables (single letters)
    if (/^[a-z]$/.test(token)) {
      stack.push(new Node("var", token));
      continue;
    }
    
    // Constants
    if (token in CONSTANTS) {
      stack.push(new Node("const", CONSTANTS[token]));
      continue;
    }
    
    // Unary operators (functions)
    if (token in UNARY_OPS) {
      if (stack.length < 1) {
        throw new Error(`Insufficient operands for unary op ${token}`);
      }
      const arg = stack.pop();
      
      // Most functions don't need brackets for exponentiation since they have
      // their own delimiters, but 'exp' functions do need brackets when exponentiated
      const requiresBrackets = token === 'exp';
      
      stack.push(new Node("func", token, [arg], requiresBrackets));
      continue;
    }
    
    // Binary operators
    if (token in BINARY_OPS) {
      if (stack.length < 2) {
        throw new Error(`Insufficient operands for binary op ${token}`);
      }
      const right = stack.pop();
      const left = stack.pop();
      
      // Set requiresBrackets flag for operations that need parentheses when exponentiated
      const needsBrackets = token === "+" || token === "-" || 
                           token === "/" || token === "root" ||
                           token === "^" || token === "**";
      
      stack.push(new Node("op", token, [left, right], needsBrackets));
      continue;
    }
    
    // If we got here, the token is not recognized
    throw new Error(`Unknown token: ${token}`);
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
    
    // Special case for 'neg' with complex expressions
    if (node.value === 'neg' && node.children[0].type === 'op' && 
        (node.children[0].value === '+' || node.children[0].value === '-')) {
      const argLatex = toLatex(node.children[0], 0, true);
      return `-(${argLatex})`;
    }
    
    const childNode = node.children[0];
    const argLatex = toLatex(childNode, 0, true);
    
    // Pass the child node's requiresBracketsForExponentiation flag to the function
    return fn(argLatex, childNode.requiresBracketsForExponentiation);
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
      
      // The need for parentheses is determined solely by the requiresBracketsForExponentiation flag
      const baseRequiresParentheses = baseNode.requiresBracketsForExponentiation;
      
      // Handle fraction exponents specially for formatting
      if (node.children[1].type === "op" && node.children[1].value === "/") {
        const numerator = toLatex(node.children[1].children[0], 0, true);
        const denominator = toLatex(node.children[1].children[1], 0, true);
        
        // For simple fractions like 1/e, 1/phi, 1/pi, 1/2, etc. use the simpler notation
        const fracExponent = (numerator === "1" || 
            (numerator.length <= 2 && denominator.length <= 2) || 
            (/^-?\d+$/.test(numerator) && /^-?\d+$/.test(denominator))) ?
            `${numerator}/${denominator}` : `\\frac{${numerator}}{${denominator}}`;
        
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
    
    // Special case: for any operation inside an exponentiation that would get double-parenthesized
    // (This catches both the addition issue and potential issues with other operations)
    if (node.requiresBracketsForExponentiation && parentPrec === 3) { // Exponentiation has precedence 3
      
      // Generate the expression without parentheses - the parent exponentiation
      // will add them based on our requiresBracketsForExponentiation flag
      const left = toLatex(node.children[0], prec, inFunc);
      const right = toLatex(node.children[1], prec, inFunc);
      return opInfo.op(left, right);
    }
    
    // Normal case for all other operations
    const left = toLatex(node.children[0], prec, inFunc);
    const right = toLatex(node.children[1], prec, inFunc);
    
    let expr = opInfo.op(left, right);
    
    // Add parentheses only when necessary for operator precedence
    if (!inFunc && prec < parentPrec) {
      expr = `(${expr})`;
    }
    
    return expr;
  }
  
  throw new Error("Unknown node type: " + node.type);
}