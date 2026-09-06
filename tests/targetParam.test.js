// tests/targetParam.test.js
// Verifies that the T query parameter only reaches the input (and RIES) when
// it is a number the number input can display; anything else is replaced by
// a random value, as when the parameter is absent
import React from 'react'
// Stub out heavy components so tests only focus on the input behavior
jest.mock('../src/components/EquationDisplay', () => () => null);
jest.mock('../src/components/DebugPanel', () => () => null);
import { render, screen } from '@testing-library/react'
import '@testing-library/jest-dom'
import App from '../src/App.jsx'

// The fallback: a random value between 0 and 10 with six decimals
const RANDOM_VALUE = /^\d\.\d{6}$/

function loadWith(value) {
  window.history.replaceState({}, '', `/?T=${encodeURIComponent(value)}`)
  render(<App />)
  return screen.getByPlaceholderText('Enter a number')
}

function urlTarget() {
  return new URLSearchParams(window.location.search).get('T')
}

describe('T query parameter', () => {
  it.each(['42', '-2.5', '.5', '1e-7', '2E3'])('accepts %s', (value) => {
    const input = loadWith(value)
    expect(input.value).toBe(value)
    expect(urlTarget()).toBe(value)
  })

  it('trims surrounding whitespace', () => {
    const input = loadWith(' 2.5 ')
    expect(input.value).toBe('2.5')
    expect(urlTarget()).toBe('2.5')
  })

  it.each(['+5', '5.', 'abc', '1abc', '1e400', 'Infinity', 'NaN', '0x10', '1_000', '--1', ''])(
    'replaces %j with a random value',
    (value) => {
      const input = loadWith(value)
      expect(input.value).toMatch(RANDOM_VALUE)
      expect(urlTarget()).toBe(input.value)
    }
  )
})
