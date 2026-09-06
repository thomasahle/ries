// tests/equationDisplay.test.js
// Verifies that a value waiting out the calculation debounce is shown as
// pending rather than as "No equations found" (as happened on a fresh load)
import React from 'react'
// Rows need MathJax; render them as plain text instead
jest.mock('../src/components/EquationRow', () => {
  const React = require('react')
  return ({ equation }) =>
    React.createElement('div', { className: 'equation-row' }, `${equation.lhs} = ${equation.rhs}`)
})
// Control when the worker answers
jest.mock('../src/workers/riesWorkerClient', () => ({ sendRiesRequest: jest.fn() }))
import { render, screen, act } from '@testing-library/react'
import '@testing-library/jest-dom'
import { QueryClient, QueryClientProvider } from '@tanstack/react-query'
import { sendRiesRequest } from '../src/workers/riesWorkerClient'
import EquationDisplay from '../src/components/EquationDisplay'

const OPTIONS = { solutionType: '', solveForX: false }
const RIES_OUTPUT = `Your target value: T = 2.5
x + 1 = 2 x for x = T - 3.5
`

function display(client, targetValue, options = OPTIONS) {
  return (
    <QueryClientProvider client={client}>
      <EquationDisplay targetValue={targetValue} riesOptions={options} />
    </QueryClientProvider>
  )
}

describe('Pending calculation', () => {
  let client

  beforeEach(() => {
    jest.useFakeTimers()
    client = new QueryClient()
    sendRiesRequest.mockReset()
  })

  afterEach(() => {
    client.clear()
    jest.useRealTimers()
  })

  it('shows the loading indicator while the first value waits out the debounce', async () => {
    let answer
    sendRiesRequest.mockReturnValue(new Promise((resolve) => { answer = resolve }))

    // As on a fresh load: the value arrives after the first (blank) render
    const { rerender } = render(display(client, ''))
    expect(screen.getByText('Enter a numerical value to find equations.')).toBeInTheDocument()
    rerender(display(client, '2.5'))

    // Nothing has been calculated yet, but nothing is missing either
    expect(sendRiesRequest).not.toHaveBeenCalled()
    expect(screen.getByText('Calculating equations for 2.5...')).toBeInTheDocument()
    expect(screen.queryByText(/No equations found/)).not.toBeInTheDocument()

    // The debounce elapses: the calculation starts and the indicator stays
    await act(async () => { jest.advanceTimersByTime(200) })
    expect(sendRiesRequest).toHaveBeenCalledTimes(1)
    expect(sendRiesRequest).toHaveBeenCalledWith('2.5', OPTIONS)
    expect(screen.getByText('Calculating equations for 2.5...')).toBeInTheDocument()
    expect(screen.queryByText(/No equations found/)).not.toBeInTheDocument()

    await act(async () => { answer({ rawOutput: RIES_OUTPUT, stderr: '', exitCode: 0 }) })
    expect(await screen.findByText('x + 1 = 2 x')).toBeInTheDocument()
    expect(screen.queryByText(/Calculating equations/)).not.toBeInTheDocument()
  })

  it('keeps the previous rows, blurred, while a new value waits out the debounce', async () => {
    let answer
    sendRiesRequest
      .mockReturnValueOnce(new Promise((resolve) => { answer = resolve }))
      .mockReturnValueOnce(new Promise(() => {}))
    const { rerender, container } = render(display(client, '2.5'))
    await act(async () => { answer({ rawOutput: RIES_OUTPUT, stderr: '', exitCode: 0 }) })
    // The query notifies its observers on a timer, so wait for the rows
    expect(await screen.findByText('x + 1 = 2 x')).toBeInTheDocument()
    expect(container.querySelector('.equations-container.blurred')).toBeNull()

    rerender(display(client, '2.55'))
    expect(sendRiesRequest).toHaveBeenCalledTimes(1)
    expect(screen.getByText('x + 1 = 2 x')).toBeInTheDocument()
    expect(container.querySelector('.equations-container.blurred')).not.toBeNull()
    expect(screen.queryByText(/No equations found/)).not.toBeInTheDocument()

    await act(async () => { jest.advanceTimersByTime(200) })
    expect(sendRiesRequest).toHaveBeenCalledTimes(2)
    expect(sendRiesRequest).toHaveBeenLastCalledWith('2.55', OPTIONS)
  })
})

describe('Deeper search hint', () => {
  let client

  beforeEach(() => {
    jest.useFakeTimers()
    client = new QueryClient()
    sendRiesRequest.mockReset()
    sendRiesRequest.mockReturnValue(new Promise(() => {}))
  })

  afterEach(() => {
    client.clear()
    jest.useRealTimers()
  })

  it('says the search is deeper while a level-3 calculation is pending', async () => {
    const options = { ...OPTIONS, searchLevel: 3 }
    render(display(client, '2.5', options))
    expect(screen.getByText('Calculating equations for 2.5...')).toBeInTheDocument()
    expect(screen.getByText(/Searching deeper \(-l3\)/)).toBeInTheDocument()

    await act(async () => { jest.advanceTimersByTime(200) })
    expect(sendRiesRequest).toHaveBeenCalledWith('2.5', options)
    expect(screen.getByText(/Searching deeper \(-l3\)/)).toBeInTheDocument()
  })

  it('shows no hint at the default level', () => {
    render(display(client, '2.5'))
    expect(screen.getByText('Calculating equations for 2.5...')).toBeInTheDocument()
    expect(screen.queryByText(/Searching deeper/)).not.toBeInTheDocument()
  })
})
