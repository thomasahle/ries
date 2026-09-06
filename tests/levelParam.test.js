// tests/levelParam.test.js
// Verifies that the search depth is read from and written to the URL as
// ?l=<n> (left out for RIES's default level 2), gets its own history entry
// when changed, and is restored on Back/Forward
import React from 'react'
// Stub out the heavy components; the debug panel stub exposes the level and
// a way to change it, as the options panel does
jest.mock('../src/components/EquationDisplay', () => () => null);
jest.mock('../src/components/DebugPanel', () => {
  const React = require('react')
  return ({ riesOptions, onRIESOptionChange }) =>
    React.createElement('div', null,
      React.createElement('output', { 'data-testid': 'level' }, String(riesOptions.searchLevel)),
      React.createElement('button', {
        onClick: () => onRIESOptionChange({ ...riesOptions, searchLevel: 3 })
      }, 'Deep'))
})
import { render, screen, fireEvent, act } from '@testing-library/react'
import '@testing-library/jest-dom'
import App from '../src/App.jsx'

function loadWith(search) {
  window.history.replaceState({}, '', `/${search}`)
  render(<App />)
  return screen.getByPlaceholderText('Enter a number')
}

function urlParams() {
  return new URLSearchParams(window.location.search)
}

function level() {
  return screen.getByTestId('level').textContent
}

describe('l query parameter', () => {
  it('defaults to level 2 and leaves the parameter out', () => {
    loadWith('?T=2.5029')
    expect(level()).toBe('2')
    expect(urlParams().get('l')).toBeNull()
    expect(urlParams().get('T')).toBe('2.5029')
  })

  it('restores the level from the URL', () => {
    const input = loadWith('?T=2.5029&l=3')
    expect(input.value).toBe('2.5029')
    expect(level()).toBe('3')
    expect(urlParams().get('l')).toBe('3')
  })

  it('keeps solve and l together', () => {
    loadWith('?T=1&solve=true&l=4')
    expect(level()).toBe('4')
    expect(urlParams().get('solve')).toBe('true')
    expect(urlParams().get('l')).toBe('4')
  })

  it.each(['0', '5', '2.5', 'abc', ''])('falls back to level 2 for l=%j', (value) => {
    loadWith(`?T=1&l=${encodeURIComponent(value)}`)
    expect(level()).toBe('2')
    expect(urlParams().get('l')).toBeNull()
  })

  it('writes a changed level as its own history entry and restores it on Back/Forward', () => {
    const input = loadWith('?T=2.5029')
    const entries = window.history.length

    fireEvent.click(screen.getByText('Deep'))
    expect(level()).toBe('3')
    expect(urlParams().get('l')).toBe('3')
    expect(urlParams().get('T')).toBe('2.5029')
    expect(window.history.length).toBe(entries + 1)

    // Selecting the current level again adds nothing
    fireEvent.click(screen.getByText('Deep'))
    expect(window.history.length).toBe(entries + 1)

    // Typing keeps the level
    fireEvent.change(input, { target: { value: '1.5' } })
    expect(urlParams().get('T')).toBe('1.5')
    expect(urlParams().get('l')).toBe('3')

    // Back to the entry with the default level
    act(() => {
      window.history.back()
      window.dispatchEvent(new PopStateEvent('popstate', { state: { T: '2.5029', solve: false, l: 2 } }))
    })
    expect(level()).toBe('2')
    expect(input.value).toBe('2.5029')

    // Forward again
    act(() => {
      window.history.forward()
      window.dispatchEvent(new PopStateEvent('popstate', { state: { T: '2.5029', solve: false, l: 3 } }))
    })
    expect(level()).toBe('3')
  })

  it('takes the level from the URL for a history entry without one', () => {
    loadWith('?T=2.5029&l=3')
    // As on a real Back: the URL has already changed when popstate fires
    act(() => {
      window.history.replaceState({}, '', '/?T=0.5')
      window.dispatchEvent(new PopStateEvent('popstate', { state: { T: '0.5' } }))
    })
    expect(level()).toBe('2')

    act(() => {
      window.history.replaceState({}, '', '/?T=0.5&l=4')
      window.dispatchEvent(new PopStateEvent('popstate', { state: { T: '0.5' } }))
    })
    expect(level()).toBe('4')
  })
})
