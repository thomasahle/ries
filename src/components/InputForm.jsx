import React from 'react';

const InputForm = ({ value, onChange }) => {
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
    </div>
  );
};

export default InputForm;