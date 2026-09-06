module.exports = {
  testEnvironment: 'jsdom',
  moduleFileExtensions: ['js', 'jsx', 'json'],
  transform: {
    '^.+\\.jsx?$': 'babel-jest'
  },
  moduleNameMapper: {
    '\\.(css|less|scss)$': '<rootDir>/__mocks__/styleMock.js',
    '.*/workers/.*': '<rootDir>/__mocks__/riesWorkerClient.js'
  }
  ,
  // Ignore tests meant for Node's built-in test runner (run via `node --test` in `npm test`)
  testPathIgnorePatterns: [
    '<rootDir>/tests/forth_parser.test.js$',
    '<rootDir>/tests/highlighting.test.js$',
    '<rootDir>/tests/parsing.test.js$'
  ]
};
