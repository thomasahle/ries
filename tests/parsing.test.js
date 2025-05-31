// tests/parsing.test.js
import test from 'node:test';
import assert from 'node:assert/strict';
import { parseRIESOutput } from '../src/utils/riesParser.js';

const cases = [
  {
    description: 'Regular decimal offset',
    input: `Your target value: 10
x + 1 = 2 x for x = T - 3.5
`,
    expected: { lhs: 'x + 1', rhs: '2 x', offset: '- 3.5' }
  },
  {
    description: 'Negative scientific notation (e-)',
    input: `Your target value: 0.142857
x ln = 1/7 for x = T - 2.2178e-07
`,
    expected: { lhs: '\\ln(x)', rhs: '1/7', offset: '- 2.2178e-07' }
  },
  {
    description: 'Scientific notation with e+',
    input: `Your target value: 3.14159
x 2 ^ = phi for x = T - 2.2178e+07
`,
    expected: { lhs: 'x^{2}', rhs: '\\phi', offset: '- 2.2178e+07' }
  },
  {
    description: 'Positive offset with scientific notation',
    input: `Your target value: 42
x e ^ = phi 2 ^ for x = T + 1.234e+05
`,
    expected: { lhs: 'x^{e}', rhs: '\\phi^{2}', offset: '+ 1.234e+05' }
  },
  {
    description: 'Exact match line',
    input: `Your target value: T = 69.
x = 8 dup* 5 +                              ('exact' match)         {64}
`,
    expected: { lhs: 'x', rhs: '8^{2} + 5', offset: '0' }
  }
];

for (const { description, input, expected } of cases) {
  test(description, () => {
    const result = parseRIESOutput(input);
    assert.strictEqual(result.length, 1, 'Expected exactly one equation');
    const eq = result[0];
    assert.strictEqual(eq.lhs, expected.lhs, 'lhs mismatch');
    assert.strictEqual(eq.rhs, expected.rhs, 'rhs mismatch');
    assert.strictEqual(eq.offset, expected.offset, 'offset mismatch');
  });
}