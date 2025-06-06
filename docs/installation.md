# Installation

This section explains how to acquire the source, build the library and verify that everything works.

## Prerequisites

- **C++11** compatible compiler (e.g. `g++`, `clang++`)
- `make` build tool

For embedded targets, ensure your toolchain is configured to compile standard C++ code and provides access to an I²C implementation.

## Cloning the Repository

```bash
git clone https://github.com/yourusername/pacl95555.git
cd pacl95555
```

## Building the Static Library

A simple `Makefile` is included:

```bash
make          # builds build/libpcal95555.a
```

The output `libpcal95555.a` can be linked into your application.

## Running Unit Tests

The library ships with a mock-based unit test suite. Run:

```bash
make test
build/test    # executes the test binary
```

You should see `All tests passed.`

Continue with the [Quick Start](./quickstart.md) once the build succeeds.
