// src/utils/searchLevels.js
//
// The search depth, RIES's -l option. Level 2 is RIES's own default (ries.c
// changelog, 20101218: "ries 1.234" is the same as "ries -l2 1.234"), so it
// adds no flag and the command line for the default stays exactly as it was.

export const DEFAULT_SEARCH_LEVEL = 2;

// Levels offered in the options panel. Each level searches roughly ten times
// as many equations as the previous one; in the browser -l3 takes about a
// second and -l4 a few seconds.
export const SEARCH_LEVELS = [
  { value: 1, label: 'Quick (-l1)' },
  { value: 2, label: 'Normal (-l2, default)' },
  { value: 3, label: 'Deep (-l3)' },
  { value: 4, label: 'Deeper (-l4)' },
];

/**
 * Parse a search level from the URL, history state or options.
 * @param {*} raw - A level as a number or string (e.g. 3 or "3")
 * @returns {number} The level if it is one we offer, else the default
 */
export function parseSearchLevel(raw) {
  const level = Number(raw);
  return SEARCH_LEVELS.some(({ value }) => value === level) ? level : DEFAULT_SEARCH_LEVEL;
}

/**
 * The RIES flag for a search level.
 * @param {*} level - The level as stored in the options (may be absent)
 * @returns {string|null} "-l<n>", or null for the default level, which needs no flag
 */
export function searchLevelArg(level) {
  const parsed = parseSearchLevel(level);
  return parsed === DEFAULT_SEARCH_LEVEL ? null : `-l${parsed}`;
}
