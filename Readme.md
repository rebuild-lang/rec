# Rebuild Experimental Compiler (C++)

[![Test Runner](https://github.com/rebuild-lang/rec/actions/workflows/test_runner.yml/badge.svg)](https://github.com/rebuild-lang/rec/actions/workflows/test_runner.yml)

A compiler written in modern C++, that explores the [Rebuild Language concepts](https://github.com/rebuild-lang/rfcs).

## Traits

All is experimental. The current experiments explore the following traits.

Fully embrace compile time code execution to simplify the language, enable high level abstractions and low level optimizations.

Consequences:
* No keywords in the language
  * Compiler provides a compile time API
* Allow Tokens and Expressions as function arguments at compile time
  * Complex overload resolution
  
… many mored details

## Requirements

This is a spare time project, we use what makes the job easier and more enjoyable:
* C++20
* CoRoutines (as implemented right now)

Windows:
* Visual Studio 2022
* Qbs 1.20

Linux:
* Clang 13 + libc++
* Qbs 1.20

## Status

As the name of the project implies all of this is experimental and work in progress.
If you like the concepts feel free to participate.

## Contributions

Feel free to create an issue to ask questions or if you want to contribute.
There are a lot of easy tasks left.
