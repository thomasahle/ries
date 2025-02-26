import React, { createContext, useContext, useEffect, useState } from 'react';

// Create a context for MathJax
const MathJaxContext = createContext({
  isReady: false,
  typeset: () => {},
});

// Provider component that tracks MathJax loading state
export const MathJaxProvider = ({ children }) => {
  const [isReady, setIsReady] = useState(false);

  useEffect(() => {
    // Check if MathJax is already loaded
    if (window.MathJax && window.MathJax.typesetPromise) {
      setIsReady(true);
      return;
    }

    // Set up an interval to check for MathJax availability
    const checkInterval = setInterval(() => {
      if (window.MathJax && window.MathJax.typesetPromise) {
        setIsReady(true);
        clearInterval(checkInterval);
      }
    }, 100);

    // Clean up interval
    return () => clearInterval(checkInterval);
  }, []);

  // Function to typeset math in a safe way
  const typeset = (elements) => {
    if (window.MathJax && window.MathJax.typesetPromise) {
      window.MathJax.typesetPromise(elements || undefined)
        .catch((err) => console.error('MathJax typesetting failed:', err));
    } else if (window.MathJax && window.MathJax.typeset) {
      window.MathJax.typeset(elements);
    }
  };

  return (
    <MathJaxContext.Provider value={{ isReady, typeset }}>
      {children}
    </MathJaxContext.Provider>
  );
};

// Hook to use MathJax context
export const useMathJax = () => useContext(MathJaxContext);