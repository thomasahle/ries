import { convertForthToLatex } from "../src/utils/forthParser.js";

// Add event listener early to ensure it's attached
document.addEventListener("DOMContentLoaded", initializeApp);

/**
 * Initialize the application after DOM content is loaded:
 * - Set up event listeners
 * - Read ?T= parameter from URL
 * - Run solver if URL has a target value
 */
function initializeApp() {
  // Initialize DOM element references
  outputElement = document.getElementById("output");
  inputElement = document.getElementById("numberInput");
  equationsContainer = document.getElementById("equations");
  toggleDebugButton = document.getElementById("toggleDebug");
  randomTButton = document.getElementById("randomT");
  
  // Initial MathJax typesetting is handled in the MathJax config
  
  // Set up debug toggle
  toggleDebugButton.addEventListener("click", function() {
    const isHidden = outputElement.classList.contains("hidden");
    if (isHidden) {
      outputElement.classList.remove("hidden");
      toggleDebugButton.textContent = "Hide Debug Output";
      toggleDebugButton.classList.add("active");
    } else {
      outputElement.classList.add("hidden");
      toggleDebugButton.textContent = "Show Debug Output";
      toggleDebugButton.classList.remove("active");
    }
  });
  
  // Set up random T button
  randomTButton.addEventListener("click", function() {
    // Generate a random number between 0 and 10
    const randomValue = (Math.random() * 10).toFixed(6);
    
    // Set the input value
    inputElement.value = randomValue;
    
    // Run RIES with the new value
    runRIES();
    
    // Add active class briefly for visual feedback
    randomTButton.classList.add("active");
    setTimeout(() => randomTButton.classList.remove("active"), 300);
  });
  
  // Run immediately on input to give responsive feedback
  inputElement.addEventListener("input", runRIES);
  
  // Check URL parameters for initial value
  const params = new URLSearchParams(window.location.search);
  const Tparam = params.get("T");
  if (Tparam) {
    inputElement.value = Tparam;
    // Automatically run RIES on page load if we have a value
    runRIES();
  }
}

/**
 * 2) Listen for back-button/forward-button events:
 *    - If we have a saved state with T, re-run the solver for that T.
 */
window.addEventListener("popstate", (event) => {
  if (event.state && typeof event.state.T !== "undefined" && inputElement) {
    inputElement.value = event.state.T;
    runRIES(); // re-run solver
  }
});


// Global references to DOM elements
let outputElement;
let inputElement;
let equationsContainer;
let toggleDebugButton;
let randomTButton;

// Track current calculation to cancel pending ones
let currentCalculation = null;

// Equations are calculated immediately on input for responsive UX

/**
 * Main: run RIES, parse the results, display them.
 * Also push a new history state so back button works.
 */
async function runRIES() {
  // Check if elements exist
  if (!outputElement || !equationsContainer || !inputElement) {
    console.error("DOM elements not initialized");
    return;
  }
  
  const numStr = inputElement.value.trim();
  
  // Handle special cases
  if (!numStr) return;
  
  // Handle zero case specially with our own mathematical identities
  if (numStr === "0" || parseFloat(numStr) === 0) {
    // Create custom equation display for T=0
    equationsContainer.innerHTML = '';
    
    // Create special equations that are true when x=0
    const zeroEquations = [
      // Equivalence class
      { lhs: "x", rhs: "0" },
      { lhs: "x", rhs: "\\sin(0)" },
      { lhs: "x", rhs: "\\tan(0)" },
      { lhs: "x", rhs: "e^0-1" },
      
      // Trigonometric identities
      { lhs: "\\sin(x)", rhs: "0" },
      { lhs: "\\tan(x)", rhs: "0" },
      { lhs: "\\cos(x)-1", rhs: "0" },
      { lhs: "\\sin^2(x)+\\cos^2(x)-1", rhs: "0" },
      
      // Logarithmic/exponential identities
      { lhs: "\\ln(1)", rhs: "0" },
      { lhs: "e^x-1", rhs: "0" },
      { lhs: "\\ln(e^x)", rhs: "x" },
      { lhs: "\\ln(1+x)", rhs: "0" },
      
      // Polynomial identities
      { lhs: "x^2", rhs: "0" },
      { lhs: "x \\cdot \\pi", rhs: "0" },
      { lhs: "x \\cdot \\phi", rhs: "0" },
      
      // Limits
      { lhs: "\\lim_{n \\to 0} \\frac{\\sin(n)}{n}", rhs: "1" }
    ];
    
    // Use existing display function to show these equations
    displayAligned(zeroEquations, "0");
    return;
  }
  
  // Handle other invalid inputs
  const num = parseFloat(numStr);
  if (isNaN(num)) {
    showErrorMessage("Please enter a valid number like π ≈ 3.14159 or e ≈ 2.71828");
    return;
  }
  
  // Make container visible with blur effect for immediate visual feedback
  equationsContainer.classList.add('blurred');
  equationsContainer.style.visibility = 'visible';
  
  // Push a new entry into the browser history
  const url = new URL(window.location);
  url.searchParams.set("T", numStr);
  window.history.pushState({ T: numStr }, "", url);

  // Clear output element for new RIES output
  outputElement.textContent = "";
  
  try {
    // Check if the RIES module is available
    if (typeof createRIESModule !== 'function') {
      console.error("createRIESModule is not available", typeof createRIESModule);
      outputElement.textContent = "Error: RIES module is not loaded properly";
      return;
    }
    
    // Cancel any previous calculation and create a new one
    if (currentCalculation) currentCalculation.abort();
    const abortController = new AbortController();
    currentCalculation = abortController;
    
    // Execute RIES in a non-blocking way to allow the UI to update with blur effect first
    setTimeout(async () => {
      // Skip if already aborted
      if (abortController.signal.aborted) return;
      try {
        // Create module instance - use a global variable to avoid recreating it each time
        if (!window.riesModuleInstance) {
          try {
            window.riesModuleInstance = await createRIESModule({
              print: text => outputElement.textContent += text + "\n",
              printErr: text => console.error("RIES error:", text),
            });
          } catch (error) {
            // Handle module initialization error
            showErrorMessage("There was a problem initializing the calculator. Please refresh the page and try again.");
            if (currentCalculation === abortController) currentCalculation = null;
            return;
          }
        }
        
        // Clear output before each run
        outputElement.textContent = "";
        
        // Skip if calculation was aborted
        if (abortController.signal.aborted) return;
        
        // Call RIES with the FORTH format flag -F3 and proper error handling
        try {
          window.riesModuleInstance.callMain(["-F3", numStr]);
        } catch (error) {
          // Handle memory errors with a user-friendly message
          if (error.name === 'RuntimeError' && error.message.includes('memory access out of bounds')) {
            showErrorMessage("This calculation requires too much memory. Try a different number or a simpler calculation.");
            if (currentCalculation === abortController) currentCalculation = null;
            return;
          }
          
          // Re-throw other errors to be caught by the main catch block
          throw error;
        }
        
        // Skip if calculation was aborted after execution
        if (abortController.signal.aborted) return;
        
        // Process results after RIES completes
        const rawOutput = outputElement.textContent;
        const T = parseTargetValue(rawOutput);
        const eqs = parseRIESOutput(rawOutput);
        
        // Skip if calculation was aborted
        if (abortController.signal.aborted) return;
        
        // Hide container during content update
        equationsContainer.style.visibility = 'hidden';
        equationsContainer.innerHTML = "";
        
        // Display equations - this will make them visible after rendering
        displayAligned(eqs, T);
        
        // Cleanup: remove blur and clear reference if still active
        equationsContainer.classList.remove('blurred');
        if (currentCalculation === abortController) currentCalculation = null;
      } catch (error) {
        console.error("Error in RIES calculation:", error);
        
        // Show error message to user
        showErrorMessage("An error occurred while calculating. Please try again.");
        
        // Clean up
        if (currentCalculation === abortController) currentCalculation = null;
      }
    }, 0); // Using setTimeout with 0 delay to defer execution but keep it in the same thread
  } catch (err) {
    console.error("Error setting up RIES calculation:", err);
    showErrorMessage("An error occurred setting up the calculation. Please try again.");
  }
}

/**
 * Parse "Your target value: T = 0.12345" from RIES output.
 */
function parseTargetValue(text) {
  const m = text.match(/Your target value:\s*T\s*=\s*([-\d.e+]+)/);
  return m ? m[1] : null;
}

/**
 * Parse lines like:
 *   x 1 - = 2 recip   for x = T - 0.00659165 {50}
 * from FORTH format output
 */
function parseRIESOutput(text) {
  const lines = text.split("\n");
  // Regular expression for FORTH format
  const eqRegex = /^(.*?)=(.*?)for x = T\s*([+\-]\s*\S+)\s*\{(\d+)\}/;
  const results = [];

  for (const line of lines) {
    const m = line.match(eqRegex);
    if (m) {
      let lhsRaw = m[1].trim();
      let rhsRaw = m[2].trim();
      const offsetRaw = m[3].trim();

      // Convert to LaTeX using the forth_parser
      results.push({
        lhs: convertForthToLatex(lhsRaw),
        rhs: convertForthToLatex(rhsRaw),
        offset: offsetRaw,
      });
    }
  }
  return results;
}

/**
 * Build a single aligned environment for all equations, so the "=" lines up.
 * Adds spacing between equations and thin horizontal lines every 3 equations.
 * e.g.
 * $$
 * \require{color}
 * \begin{aligned}
 * x+2 &= \sqrt{5} & \text{for } x &= 0.23\textcolor{gray}{606798} \\[6pt]
 * 6\,x &= \sqrt{2} & \text{for } x &= 0.23\textcolor{gray}{702286} \\[6pt]
 * x^2 &= 3 & \text{for } x &= 0.23\textcolor{gray}{707292} \\[6pt]
 * \rule{15em}{0.4pt} \\[8pt]
 * ...
 * \end{aligned}
 * $$
 */
function displayAligned(eqs, T) {
  // Handle case with no equations
  if (!eqs.length) {
    showErrorMessage("No equations found. Try a different value like π ≈ 3.14159");
    return;
  }

  let Tnum = null;
  let decimals = 12; // fallback
  if (T) {
    Tnum = parseFloat(T);
    // Count decimal places in T
    const decMatch = T.match(/\.(\d+)$/);
    if (decMatch) decimals = decMatch[1].length;
  }

  // Clear the container
  equationsContainer.innerHTML = "";
  
  // Use document fragment for better performance when adding multiple elements
  const fragment = document.createDocumentFragment();
  
  // Create equation rows - batching DOM operations
  for (let i = 0; i < eqs.length; i++) {
    const eq = eqs[i];
    let xValLatex = "T"; // fallback
    
    // Calculate x value with highlighting
    if (eq.offset && Tnum !== null) {
      const offsetVal = parseFloat((eq.offset || "").replace(/\s+/g, "")) || 0;
      const xVal = (Tnum + offsetVal).toFixed(decimals);
      xValLatex = highlightDifference(T, xVal);
    } else if (T === "0") {
      xValLatex = "0";
    }
    
    // Create equation row
    const rowDiv = document.createElement("div");
    rowDiv.className = "equation-row";
    
    // Create LHS div with equals sign
    const lhsDiv = document.createElement("div");
    lhsDiv.className = "equation-lhs";
    lhsDiv.innerHTML = `$$${eq.lhs} =$$`;
    
    // Create RHS div
    const rhsDiv = document.createElement("div");
    rhsDiv.className = "equation-rhs";
    rhsDiv.innerHTML = `$$${eq.rhs}$$`;
    
    // Create x-value div
    const xValueDiv = document.createElement("div");
    xValueDiv.className = "equation-x-value";
    xValueDiv.innerHTML = `$$x = ${xValLatex}$$`;
    
    // Add all parts to the row
    rowDiv.appendChild(lhsDiv);
    rowDiv.appendChild(rhsDiv);
    rowDiv.appendChild(xValueDiv);
    
    // Add click handler to copy equation to clipboard
    const copyEquation = () => {
      // Create LaTeX representation of the equation
      const latexEquation = `${eq.lhs} = ${eq.rhs}`;
      
      // Copy to clipboard
      navigator.clipboard.writeText(latexEquation)
        .then(() => {
          // Flash feedback on the entire row
          rowDiv.style.outline = '2px solid rgba(255, 165, 0, 0.7)';
          
          // Reset after a moment
          setTimeout(() => {
            rowDiv.style.outline = '';
          }, 300); // Slightly faster animation
        })
        .catch(err => console.error('Failed to copy equation to clipboard', err));
    };
    
    // Attach the click handler to the entire row
    rowDiv.addEventListener('click', copyEquation);
    
    // Add the row to the fragment (not directly to DOM)
    fragment.appendChild(rowDiv);
    
    // Add a separator after every 3rd equation (except at the end)
    if ((i + 1) % 3 === 0 && i < eqs.length - 1) {
      const separatorDiv = document.createElement("div");
      separatorDiv.className = "equation-separator";
      fragment.appendChild(separatorDiv);
    }
  }
  
  // Add the fragment to the container in a single DOM operation
  equationsContainer.appendChild(fragment);
  
  // Hide container while MathJax renders (we'll show it after rendering completes)
  equationsContainer.style.visibility = 'hidden';
  
  // Render with MathJax and make visible when done
  if (window.MathJax && window.MathJax.typesetPromise) {
    try {
      // Render math and show when done
      window.MathJax.typesetPromise([equationsContainer])
        .then(() => equationsContainer.style.visibility = 'visible')
        .catch(() => equationsContainer.style.visibility = 'visible');
    } catch (err) {
      // Show on error
      equationsContainer.style.visibility = 'visible';
    }
  } else {
    // MathJax not available, show anyway
    equationsContainer.style.visibility = 'visible';
  }
}

/**
 * Shows an error message in the equations container
 * @param {string} message - The error message to display
 */
function showErrorMessage(message) {
  equationsContainer.innerHTML = `
    <div class="equation-block message-block">
      <p>${message}</p>
    </div>
  `;
  equationsContainer.style.visibility = 'visible';
  equationsContainer.classList.remove('blurred');
}

/**
 * highlightDifference("0.23490942","0.23606798")
 * => "0.23\\textcolor{gray}{606798}"
 */
function highlightDifference(strA, strB) {
  // Find the point where the strings start to differ
  let i = 0;
  const len = Math.min(strA.length, strB.length);
  while (i < len && strA[i] === strB[i]) i++;
  
  // Split into matching prefix and different remainder
  const prefix = strB.slice(0, i);
  const remainder = strB.slice(i);
  
  // Return with LaTeX formatting to highlight the difference
  return remainder ? prefix + `\\textcolor{gray}{${remainder}}` : prefix;
}
