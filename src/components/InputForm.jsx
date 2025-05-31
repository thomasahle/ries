import React from 'react';
import './InputForm.css';

const InputForm = ({ value, onChange, onRandom }) => {
  // Run calculation immediately on input change
  const handleInputChange = (e) => {
    onChange(e.target.value);
  };

  return (
    <div className="input-row">
      <input
        type="number"
        value={value}
        onChange={handleInputChange}
        step="any"
        placeholder="Enter a number"
      />
      {onRandom && (
        <button
          type="button"
          className="random-btn"
          onClick={onRandom}
          aria-label="Random value"
        >
          🎲
        </button>
      )}
    </div>
  );
};

export default InputForm;