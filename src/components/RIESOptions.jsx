import React, { useState } from 'react';
import './RIESOptions.css';

// Define all RIES symbols in a clean layout at module level
const initialSymbolGroups = [
  // Row 1 
  [
    { id: 'W', display: 'W', label: 'Lambert W function' },
    { id: 'l', display: 'ln', label: 'Natural log' },
    { id: 'L', display: 'logₐb', label: 'Log base A of B' },
    { id: 'p', display: 'π', label: 'Pi' },
    { id: 'e', display: 'e', label: 'Euler\'s number' },
    { id: 'f', display: 'φ', label: 'Golden ratio (phi)' },
    { id: '/', display: '÷', label: 'Divide' }
  ],
  // Row 2
  [
    { id: '', display: '', label: '' },
    { id: 'E', display: 'e^x', label: 'e to power x' },
    { id: 'v', display: 'ⁿ√x', label: 'nth root' },
    { id: '7', display: '7', label: 'Number 7' },
    { id: '8', display: '8', label: 'Number 8' },
    { id: '9', display: '9', label: 'Number 9' },
    { id: '*', display: '×', label: 'Multiply' }
  ],
  // Row 3
  [
    { id: '', display: '', label: '' },
    { id: '^', display: 'x^y', label: 'Power' },
    { id: 'q', display: '√x', label: 'Square root' },
    { id: '4', display: '4', label: 'Number 4' },
    { id: '5', display: '5', label: 'Number 5' },
    { id: '6', display: '6', label: 'Number 6' },
    { id: '-', display: '−', label: 'Subtract' }
  ],
  // Row 4
  [
    { id: 'C', display: 'cos', label: 'Cosine' },
    { id: 'S', display: 'sin', label: 'Sine' },
    { id: 's', display: 'x²', label: 'Square' },
    { id: '1', display: '1', label: 'Number 1' },
    { id: '2', display: '2', label: 'Number 2' },
    { id: '3', display: '3', label: 'Number 3' },
    { id: '+', display: '+', label: 'Add' }
  ],
  // Row 5
  [
    { id: 'T', display: 'tan', label: 'Tangent' },
    { id: 'A', display: 'atan2', label: 'Two-arg arctangent' },
    { id: 'r', display: '1/x', label: 'Reciprocal' },
    { id: '', display: '', label: '' },
    { id: '0', display: '0', label: 'Number 0' },
    { id: 'n', display: '±', label: 'Negative' },
    { id: 'x', display: 'x', label: 'Variable' },
  ]
];

// Solution type options
const solutionTypes = [
  { value: '', label: 'Any' },
  { value: 'i', label: 'Integer' },
  { value: 'r', label: 'Rational' },
  { value: 'c', label: 'Constructible' },
  { value: 'a', label: 'Algebraic' }
];

const RIESOptions = ({ isVisible, onToggle, onOptionsChange, riesOptions }) => {
  // Initialize symbol states from the current riesOptions
  const [symbolStates, setSymbolStates] = useState(() => {
    const states = {};
    
    // Extract symbols from riesOptions and set their states
    if (riesOptions) {
      // Handle "never use" symbols (-N)
      riesOptions.neverUseSymbols?.split('').forEach(symbol => {
        states[symbol] = 'never';
      });
      
      // Handle "only once" symbols (-O)
      riesOptions.onlyOneSymbols?.split('').forEach(symbol => {
        states[symbol] = 'once';
      });
      
      // Handle "only use" symbols (-S)
      riesOptions.onlyUseSymbols?.split('').forEach(symbol => {
        states[symbol] = 'only';
      });
    }
    
    return states;
  });
  
  // Initialize options with passed in riesOptions or defaults
  const [options, setOptions] = useState({
    // Solution types
    solutionType: riesOptions?.solutionType || '', // '', 'i', 'r', 'c', 'a'
    
    // Output format
    solveForX: riesOptions?.solveForX || false,
    
    // Symbol options
    neverUseSymbols: riesOptions?.neverUseSymbols || '',
    onlyOneSymbols: riesOptions?.onlyOneSymbols || '',
    onlyUseSymbols: riesOptions?.onlyUseSymbols || '',
  });

  // Handle symbol state toggle
  const handleSymbolClick = (symbolId) => {
    if (!symbolId) return;
    
    setSymbolStates(prevStates => {
      // Cycle through states: enabled -> never -> once -> only -> enabled
      const currentState = prevStates[symbolId] || 'enabled';
      let nextState;
      
      switch (currentState) {
        case 'enabled': nextState = 'never'; break;
        case 'never': nextState = 'once'; break;
        case 'once': nextState = 'only'; break;
        case 'only': nextState = 'enabled'; break;
        default: nextState = 'enabled';
      }
      
      const newStates = { ...prevStates, [symbolId]: nextState };
      
      // Build options based on the current states
      updateOptionsFromSymbolStates(newStates);
      
      return newStates;
    });
  };

  // Update options based on symbol states
  const updateOptionsFromSymbolStates = (states) => {
    const neverSymbols = Object.entries(states)
      .filter(([_, state]) => state === 'never')
      .map(([symbol]) => symbol)
      .join('');
      
    const onceSymbols = Object.entries(states)
      .filter(([_, state]) => state === 'once')
      .map(([symbol]) => symbol)
      .join('');
      
    const onlySymbols = Object.entries(states)
      .filter(([_, state]) => state === 'only')
      .map(([symbol]) => symbol)
      .join('');
    
    const newOptions = { 
      ...options, 
      neverUseSymbols: neverSymbols,
      onlyOneSymbols: onceSymbols,
      onlyUseSymbols: onlySymbols
    };
    
    setOptions(newOptions);
    onOptionsChange(newOptions);
  };

  // Don't update options in the initial render effect
  // This was causing the React "Cannot update a component while rendering" error
  React.useEffect(() => {
    if (!riesOptions) return;
    
    // Skip the first render to prevent setState during render
    const timer = setTimeout(() => {
      // Update local options state
      setOptions({
        solutionType: riesOptions.solutionType || '',
        solveForX: riesOptions.solveForX || false,
        neverUseSymbols: riesOptions.neverUseSymbols || '',
        onlyOneSymbols: riesOptions.onlyOneSymbols || '',
        onlyUseSymbols: riesOptions.onlyUseSymbols || '',
      });
      
      // Update symbol states
      const states = {};
      
      // Handle "never use" symbols (-N)
      riesOptions.neverUseSymbols?.split('').forEach(symbol => {
        states[symbol] = 'never';
      });
      
      // Handle "only once" symbols (-O)
      riesOptions.onlyOneSymbols?.split('').forEach(symbol => {
        states[symbol] = 'once';
      });
      
      // Handle "only use" symbols (-S)
      riesOptions.onlyUseSymbols?.split('').forEach(symbol => {
        states[symbol] = 'only';
      });
      
      setSymbolStates(states);
    }, 0);
    
    return () => clearTimeout(timer);
  }, [riesOptions]);

  // Handle general option changes
  const handleOptionChange = (e) => {
    const { name, value, type, checked } = e.target;
    const newValue = type === 'checkbox' ? checked : value;
    
    setOptions(prevOptions => {
      const newOptions = { ...prevOptions, [name]: newValue };
      onOptionsChange(newOptions);
      return newOptions;
    });
  };

  // Determine button class based on state
  const getButtonClass = (symbol) => {
    if (!symbol.id) return 'key empty-key';
    
    const state = symbolStates[symbol.id] || 'enabled';
    let type = 'function';
    
    // Determine button type by symbol
    if ('0123456789'.includes(symbol.id)) {
      type = 'number';
    } else if ('+-*/'.includes(symbol.id)) {
      type = 'operator';
    }
    
    return `key ${type}-key ${state}`;
  };

  // Build RIES command parameters
  const getRIESParams = () => {
    const params = [];
    
    // Symbol selection
    if (options.neverUseSymbols) params.push(`-N${options.neverUseSymbols}`);
    if (options.onlyOneSymbols) params.push(`-O${options.onlyOneSymbols}`);
    if (options.onlyUseSymbols) params.push(`-S${options.onlyUseSymbols}`);
    
    // Solution types
    if (options.solutionType) params.push(`-${options.solutionType}`);
    
    // Output format
    if (options.solveForX) params.push('-s');
    
    return params.join(' ');
  };

  // Just render the panel when it's visible, not the button
  if (!isVisible) return null;
  
  return (
    <div className="ries-panel">
      {/* Symbol Selection - Users can toggle RIES functions and operators */}
      <div className="options-section">
        <h3>Symbol Selection</h3>
        <div className="symbol-selection">
          <div className="legend-container">
            <div className="legend-title">Select symbols to include/exclude:</div>
            <div className="legend">
              <div className="legend-item">
                <div className="legend-dot enabled"></div>
                <span>Enabled (default)</span>
              </div>
              <div className="legend-item">
                <div className="legend-dot never"></div>
                <span>Never use (-N)</span>
              </div>
              <div className="legend-item">
                <div className="legend-dot once"></div>
                <span>Use at most once (-O)</span>
              </div>
              <div className="legend-item">
                <div className="legend-dot only"></div>
                <span>Only use these (-S)</span>
              </div>
            </div>
          </div>
          
          <div className="keypad-container">
            {initialSymbolGroups.map((row, rowIndex) => (
              <div key={rowIndex} className="keypad-row">
                {row.map((symbol, colIndex) => (
                  <button
                    key={`${rowIndex}-${colIndex}`}
                    className={getButtonClass(symbol)}
                    onClick={() => handleSymbolClick(symbol.id)}
                    title={symbol.label}
                    disabled={!symbol.id}
                  >
                    <span className="key-text">{symbol.display}</span>
                    {symbol.id && symbolStates[symbol.id] && symbolStates[symbol.id] !== 'enabled' && (
                      <span className="state-indicator">
                        {symbolStates[symbol.id] === 'never' ? 'N' : 
                         symbolStates[symbol.id] === 'once' ? 'O' : 'S'}
                      </span>
                    )}
                  </button>
                ))}
              </div>
            ))}
          </div>
        </div>
      </div>
      
      {/* Options - Controls for equation type and format */}
      <div className="options-section">
        <h3>Options</h3>
        <div className="options-wrapper">
          <div className="checkbox-option">
            <label>
              <input
                type="checkbox"
                name="solveForX"
                checked={options.solveForX}
                onChange={handleOptionChange}
              />
              Try to solve for x
            </label>
          </div>
          
          <div className="select-option">
            <label>
              Solution type:
              <SolutionTypeSelector 
                value={options.solutionType || ''}
                onChange={handleOptionChange}
              />
            </label>
          </div>
        </div>
      </div>

      {/* Debug Output - Shows command and output */}
      <div className="options-section">
        <h3>Debug Output</h3>
        <div className="debug-output-section">
          <div className="command-text">
            ries {riesOptions.targetValue || "[value]"} {getRIESParams()}
          </div>
          
          <pre className="debug-output">
            {riesOptions.rawOutput || 'No output captured yet. Enter a value to calculate.'}
          </pre>
        </div>
      </div>
    </div>
  );
};

// Export solution type selector separately so it can be used in the main controls
export const SolutionTypeSelector = ({ value, onChange }) => {
  const handleChange = (e) => {
    onChange({
      target: {
        name: 'solutionType',
        value: e.target.value,
        type: 'select'
      }
    });
  };

  return (
    <select
      value={value}
      onChange={handleChange}
      className="solution-type-select"
    >
      {solutionTypes.map(type => (
        <option key={type.value} value={type.value}>
          {type.label}
        </option>
      ))}
    </select>
  );
};

export default RIESOptions;