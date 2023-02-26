# leano
This is a very early prototype of a (minimal) portable, Deno-compatible JavaScript runtime that is built on QuickJS.

Note: leano is currently just a toy that I'm using to run some of my Deno scripts/tools on uncommon platforms. I doubt anyone else is interested, anyway.

## Motivation
As of this writing, Deno doesn't support Raspberry Pi (either 32-bit or 64-bit), but I would like to be able to run some tools I built with Deno on my (v1) Raspberry Pi.

## Architecture
In order to support other architectures, rather than attempt to port Deno directly (a worthwhile, but far too ambitious goal), this tool simply embeds QuickJS (which is portable) and then provides a (minimal) implementation of Deno's API (along with related web APIs that aren't implemented in QuickJS).

## Usage
1. Build QuickJS's `qjs` command line tool
1. If needed, use a bundler/transpiler to convert TypeScript code to JavaScript
1. Run `qjs --std -I leano.js <script>`
