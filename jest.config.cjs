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
  // Ignore tests meant for Node's built-in test runner
  testPathIgnorePatterns: [
    '<rootDir>/tests/parsing.test.js$'
  ]
};