// tests/searchDepth.test.js
// Verifies the search depth control (RIES's -l flag) in the options panel:
// the levels it offers, the value it reports, and that only levels other
// than RIES's default (2) add a flag to the command line
import React from 'react'
import { render, screen, fireEvent } from '@testing-library/react'
import '@testing-library/jest-dom'
import RIESOptions from '../src/components/RIESOptions.jsx'

const BASE = {
  solutionType: '',
  solveForX: false,
  neverUseSymbols: '',
  onlyOneSymbols: '',
  onlyUseSymbols: '',
  targetValue: '2.5029',
  rawOutput: ''
}

function panel(riesOptions, onOptionsChange = () => {}) {
  return render(
    <RIESOptions
      isVisible={true}
      onToggle={() => {}}
      onOptionsChange={onOptionsChange}
      riesOptions={{ ...BASE, ...riesOptions }}
    />
  )
}

function commandLine(container) {
  return container.querySelector('.command-text').textContent.trim()
}

describe('Search depth in the options panel', () => {
  it('offers the levels and defaults to Normal', () => {
    panel({ searchLevel: 2 })
    const select = screen.getByLabelText(/Search depth/)
    expect(select.value).toBe('2')
    expect(Array.from(select.options).map((option) => option.textContent)).toEqual([
      'Quick (-l1)',
      'Normal (-l2, default)',
      'Deep (-l3)',
      'Deeper (-l4)'
    ])
  })

  it('reports a chosen level as a number', () => {
    const onOptionsChange = jest.fn()
    panel({ searchLevel: 2 }, onOptionsChange)
    fireEvent.change(screen.getByLabelText(/Search depth/), { target: { value: '3' } })
    expect(onOptionsChange).toHaveBeenCalledWith(expect.objectContaining({ searchLevel: 3 }))
  })

  it('shows the same command line as before for the default level', () => {
    const { container } = panel({ searchLevel: 2 })
    expect(commandLine(container)).toBe('ries 2.5029')
  })

  it('shows the same command line when the level is absent', () => {
    const { container } = panel({})
    expect(commandLine(container)).toBe('ries 2.5029')
  })

  it.each([
    [1, 'ries 2.5029 -l1'],
    [3, 'ries 2.5029 -l3'],
    [4, 'ries 2.5029 -l4']
  ])('shows -l for level %i', (searchLevel, expected) => {
    const { container } = panel({ searchLevel })
    expect(commandLine(container)).toBe(expected)
  })

  it('keeps the level with the other flags', () => {
    const { container } = panel({ searchLevel: 3, solutionType: 'i', solveForX: true, neverUseSymbols: 'p' })
    expect(commandLine(container)).toBe('ries 2.5029 -Np -i -l3 -s')
  })
})
