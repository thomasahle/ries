// src/utils/MathJaxContext.js
import React, { createContext, useCallback, useContext, useEffect, useMemo, useState } from 'react';

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

  // Stable identities: consumers use these as effect dependencies, so a new
  // function per render would re-typeset every row on every keystroke.
  const typesetPromise = useCallback((elements) => {
    if (window.MathJax && window.MathJax.typesetPromise) {
      return window.MathJax.typesetPromise(elements || undefined)
        .catch((err) => console.error("MathJax typeset failed:", err));
    } else if (window.MathJax && window.MathJax.typeset) {
      window.MathJax.typeset(elements);
      return Promise.resolve();
    }
    return Promise.resolve();
  }, []);

  const typeset = useCallback((elements) => {
    typesetPromise(elements);
  }, [typesetPromise]);

  const value = useMemo(
    () => ({ isReady, typeset, typesetPromise }),
    [isReady, typeset, typesetPromise]
  );

  return (
    <MathJaxContext.Provider value={value}>
      {children}
    </MathJaxContext.Provider>
  );
};

export const useMathJax = () => useContext(MathJaxContext);
