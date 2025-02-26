# RIES Codebase Guidelines

## Commands
- `npm run dev` - Start development server
- `npm run build` - Build for production
- `npm run preview` - Preview production build
- `npm test` - Run all tests
- `node test_forth_parser.js` - Run FORTH parser tests only

## Code Style
- **Imports**: Group React/library imports first, then local components, then utilities/hooks, then styles
- **Components**: Use functional components with hooks, keep complex logic in custom hooks
- **Error Handling**: Use try/catch with specific error types, provide user-friendly messages
- **State Management**: Use React Query for async data, React state for UI
- **Naming**: camelCase for variables/functions, PascalCase for components, ALL_CAPS for constants
- **Documentation**: JSDoc comments for functions, inline comments for complex logic
- **LaTeX Formatting**: Use MathJax for rendering, FORTH notation converted via forthParser.js

## Project Structure
- `/src/components` - UI components
- `/src/hooks` - Custom React hooks
- `/src/utils` - Utility functions and context providers
- `/public` - Static assets including WASM files