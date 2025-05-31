#!/usr/bin/env node
import { execSync } from 'child_process';
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

// Emulate __dirname in ES modules
const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Simple test runner: recursively execute all *.test.js under this directory
console.log(`Starting test runner in ${__dirname}`);

function runTests(dir) {
  const entries = fs.readdirSync(dir, { withFileTypes: true });
  for (const entry of entries) {
    const fullPath = path.join(dir, entry.name);
    if (entry.isDirectory()) {
      runTests(fullPath);
    } else if (entry.isFile() && entry.name.endsWith('.test.js')) {
      console.log(`\n=== Running ${fullPath} ===`);
      execSync(`node ${fullPath}`, { stdio: 'inherit' });
    }
  }
}

// Start from this script's directory (tests/)
runTests(__dirname);