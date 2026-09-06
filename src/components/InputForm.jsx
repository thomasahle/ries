import React from 'react';
import './InputForm.css';

const InputForm = ({ value, onChange, onCommit, onRandom }) => {
  // Run calculation immediately on input change
  const handleInputChange = (e) => {
    onChange(e.target.value);
  };

  // Enter commits the typed value (blur does too)
  const handleKeyDown = (e) => {
    if (e.key === 'Enter' && onCommit) {
      onCommit();
    }
  };

  return (
    <div className="input-row">
      <input
        type="number"
        value={value}
        onChange={handleInputChange}
        onKeyDown={handleKeyDown}
        onBlur={onCommit}
        step="any"
        placeholder="Enter a number"
        aria-label="Number to find equations for"
      />
      {onRandom && (
        <button
          type="button"
          className="random-btn"
          onClick={onRandom}
          title="Surprise me!"
          aria-label="Surprise me!"
        >
          🎲
        </button>
      )}
    </div>
  );
};

export default InputForm;
