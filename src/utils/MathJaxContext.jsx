// src/utils/MathJaxContext.js
import React, { createContext, useContext, useEffect, useState } from 'react';

const MathJaxContext = createContext({
  isReady: false,
  typeset: () => {},
  typesetPromise: () => Promise.resolve(),
});

export const MathJaxProvider = ({ children }) => {
  const [isReady, setIsReady] = useState(false);

  useEffect(() => {
    if (window.MathJax && window.MathJax.typesetPromise) {
      setIsReady(true);
      return;
    }
    const checkInterval = setInterval(() => {
      if (window.MathJax && window.MathJax.typesetPromise) {
        setIsReady(true);
        clearInterval(checkInterval);
      }
    }, 100);
    return () => clearInterval(checkInterval);
  }, []);

  const typesetPromise = (elements) => {
    if (window.MathJax && window.MathJax.typesetPromise) {
      return window.MathJax.typesetPromise(elements || undefined)
        .catch((err) => console.error("MathJax typeset failed:", err));
    } else if (window.MathJax && window.MathJax.typeset) {
      window.MathJax.typeset(elements);
      return Promise.resolve();
    }
    return Promise.resolve();
  };

  const typeset = (elements) => {
    typesetPromise(elements);
  };

  return (
    <MathJaxContext.Provider value={{ isReady, typeset, typesetPromise }}>
      {children}
    </MathJaxContext.Provider>
  );
};

export const useMathJax = () => useContext(MathJaxContext);
