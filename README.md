# RIES Inverse Symbolic Calculator

A web interface for RIES ([RILYBOT Inverse Equation Solver](https://www.mrob.com/pub/ries/index.html#:~:text=acronym%20for%20RILYBOT%20Inverse%20Equation%20Solver)) by Robert Munafo.

## Overview

This web application allows users to enter a numerical value and find mathematical equations that produce that value. For example, entering π (3.14159) will find equations like:

- x² = 10 - π (an identity involving π)
- x ln(x) = 1 (the value where x ln(x) equals 1)
- e^(x/2) = 4 (the value where e raised to x/2 equals 4)

RIES is particularly useful for mathematical research, identifying constants, and finding unexpected relationships between mathematical expressions.

## Features

- WebAssembly-powered RIES equation solver
- Responsive, React-based UI
- MathJax for beautiful mathematical formula display
- Support for a wide range of mathematical functions and constants

## Getting Started

1. Clone this repository
2. Install dependencies with `npm install`
3. Run the development server with `npm run dev`
4. Open your browser to the URL shown in the terminal (usually http://localhost:3000)

## Building for Production

```
npm run build
```

This will create a production-ready build in the `dist` directory.

## Testing

To run tests for the FORTH parser:

```
npm test
```

## Building WASM Module

To rebuild the RIES WebAssembly module:

```bash
# Install and activate Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

# Build the WASM module
emcc -O2 ries.c -o public/ries.js \
     -s WASM=1 \
     -s MODULARIZE=1 \
     -s 'EXPORT_NAME="createRIESModule"' \
     -s INVOKE_RUN=0 \
     -s EXPORTED_FUNCTIONS='["_main"]' \
     -s EXPORTED_RUNTIME_METHODS='["ccall","callMain"]' \
     -s ALLOW_MEMORY_GROWTH=1 \
     -lm
```

## How It Works

The application uses:

1. **WebAssembly**: The core RIES algorithm is compiled to WebAssembly from C for performance
2. **FORTH Parser**: A custom parser converts FORTH notation (used by RIES) to LaTeX
3. **React**: The UI is built with React and React Query for state management
4. **MathJax**: Mathematical equations are rendered using MathJax

## Code Organization

- `/src/utils/forthParser.js` - FORTH parser for converting RIES output to LaTeX
- `/src/components/` - React components for the UI
- `/src/hooks/useRIESCalculation.js` - React hook for RIES calculations
- `/public/ries.js` and `/public/ries.wasm` - Compiled WebAssembly module

## Credits

- RIES was created by [Robert Munafo](https://mrob.com/pub/ries/index.html)
- This web interface was developed by Thomas Ahle

## License

This project is licensed under the GPL-3.0 License - see the LICENSE file for details.
