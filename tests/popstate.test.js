// tests/popstate.test.js
// Verifies that browser Back/Forward (popstate) events sync the input state
import React from 'react'
// Stub out heavy components so tests only focus on the input behavior
jest.mock('../src/components/EquationDisplay', () => () => null);
jest.mock('../src/components/DebugPanel', () => () => null);
import { render, screen, fireEvent, act } from '@testing-library/react'
import '@testing-library/jest-dom'
import App from '../src/App.jsx'

describe('Browser history (popstate) sync', () => {
  beforeEach(() => {
    // reset URL
    window.history.replaceState({}, '', '/')
  })

  it('reverts input value on Back navigation', () => {
    render(<App />)
    const input = screen.getByPlaceholderText('Enter a number')

    // 1) First value
    fireEvent.change(input, { target: { value: '0.123' } })
    expect(input.value).toBe('0.123')
    expect(window.location.search).toBe('?T=0.123')

    // 2) Second value
    fireEvent.change(input, { target: { value: '-1' } })
    expect(input.value).toBe('-1')
    expect(window.location.search).toBe('?T=-1')

    // 3) Simulate Back
    act(() => {
      window.history.back()
      window.dispatchEvent(new PopStateEvent('popstate', { state: { T: '0.123' } }))
    })
    expect(input.value).toBe('0.123')
  })

  it('applies input value on Forward navigation', () => {
    render(<App />)
    const input = screen.getByPlaceholderText('Enter a number')

    fireEvent.change(input, { target: { value: '1.234' } })
    fireEvent.change(input, { target: { value: '5.678' } })

    // Back to first
    act(() => {
      window.history.back()
      window.dispatchEvent(new PopStateEvent('popstate', { state: { T: '1.234' } }))
    })
    expect(input.value).toBe('1.234')

    // Forward to second
    act(() => {
      window.history.forward()
      window.dispatchEvent(new PopStateEvent('popstate', { state: { T: '5.678' } }))
    })
    expect(input.value).toBe('5.678')
  })
})