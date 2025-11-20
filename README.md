---
layout: default
title: "HardFOC PCAL95555 Driver"
description: "Hardware-agnostic C++ driver for the NXP PCAL9555A 16-bit I/O expander"
nav_order: 1
permalink: /
---

# HF-PCAL95555 Driver
**Hardware-agnostic C++ driver for the NXP PCAL9555A 16-bit I/O expander**

[![C++](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11)
[![License](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![CI](https://github.com/N3b3x/hf-pcal95555-driver/actions/workflows/esp32-examples-build-ci.yml/badge.svg?branch=main)](https://github.com/N3b3x/hf-pcal95555-driver/actions/workflows/esp32-examples-build-ci.yml)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://n3b3x.github.io/hf-pcal95555-driver/)

## 📚 Table of Contents
1. [Overview](#-overview)
2. [Features](#-features)
3. [Quick Start](#-quick-start)
4. [Installation](#-installation)
5. [API Reference](#-api-reference)
6. [Examples](#-examples)
7. [Documentation](#-documentation)
8. [Contributing](#-contributing)
9. [License](#-license)

## 📦 Overview

> **📖 [📚🌐 Live Complete Documentation](https://n3b3x.github.io/hf-pcal95555-driver/)** - 
> Interactive guides, examples, and step-by-step tutorials

The **PCAL9555A** is a 16-bit I/O expander from NXP Semiconductors that communicates via I²C. It provides 16 GPIO pins organized into two 8-bit ports (PORT_0: pins 0-7, PORT_1: pins 8-15), allowing you to expand your microcontroller's I/O capabilities. The chip features per-pin interrupt capability, configurable pull-up/pull-down resistors, adjustable drive strength, and support for both push-pull and open-drain output modes.

This driver provides a hardware-agnostic C++ interface that abstracts all register-level operations, requiring only an implementation of the `I2cInterface` for your platform. The driver uses the CRTP (Curiously Recurring Template Pattern) for zero-overhead hardware abstraction, making it suitable for resource-constrained embedded systems.

## ✨ Features

- ✅ **16 GPIO Pins**: Two 8-bit ports (PORT_0: 0-7, PORT_1: 8-15)
- ✅ **Per-Pin Configuration**: Direction, pull-up/pull-down, drive strength, polarity
- ✅ **Interrupt Support**: Per-pin interrupt capability with input latching
- ✅ **Hardware Agnostic**: CRTP-based I2C interface for platform independence
- ✅ **Modern C++**: C++11 compatible with template-based design
- ✅ **Zero Overhead**: CRTP design for compile-time polymorphism
- ✅ **Kconfig Integration**: Optional compile-time configuration via Kconfig
- ✅ **Error Handling**: Comprehensive error reporting with retry support

## 🚀 Quick Start

```cpp
#include "pcal95555.hpp"

// 1. Implement the I2C interface (see platform_integration.md)
class MyI2c : public pcal95555::I2cInterface<MyI2c> {
public:
    bool write(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len);
    bool read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len);
};

// 2. Create driver instance
MyI2c i2c;
// Option 1: Using address directly (recommended)
pcal95555::PCAL95555<MyI2c> gpio(&i2c, 0x20); // Address 0x20 (default)

// Option 2: Using pin levels
// pcal95555::PCAL95555<MyI2c> gpio(&i2c, false, false, false); // A0=LOW, A1=LOW, A2=LOW -> 0x20

// 3. Initialize and use
gpio.ResetToDefault(); // all pins become inputs with pull-ups
gpio.SetPinDirection(0, pcal95555::PCAL95555<MyI2c>::GPIODir::Output);
gpio.WritePin(0, true);
bool input = gpio.ReadPin(1);
```

For detailed setup, see [Installation](docs/installation.md) and [Quick Start Guide](docs/quickstart.md).

## 🔧 Installation

This driver is designed to be integrated into your project. It's a header-only template library with implementation included.

1. **Add to your project**: Copy or clone the driver files into your project
2. **Implement the I2C interface** for your platform (see [Platform Integration](docs/platform_integration.md))
3. **Include the header** in your code:
   ```cpp
   #include "pcal95555.hpp"
   ```
4. Compile with a **C++11** or newer compiler

For detailed installation instructions, see [docs/installation.md](docs/installation.md).

## 📖 API Reference

| Method | Description |
|--------|-------------|
| `ResetToDefault()` | Reset device to power-on default state |
| `SetPinDirection(pin, dir)` | Set pin direction (input/output) |
| `ReadPin(pin)` | Read pin state |
| `WritePin(pin, value)` | Write pin state |
| `SetPullEnable(pin, enable)` | Enable/disable pull resistor |
| `SetPullDirection(pin, pull_up)` | Set pull-up or pull-down |
| `SetDriveStrength(pin, level)` | Set output drive strength |
| `GetInterruptStatus()` | Get interrupt status register |

For complete API documentation with source code links, see [docs/api_reference.md](docs/api_reference.md).

## 📊 Examples

For ESP32 examples, see the [examples/esp32](examples/esp32/) directory.
Additional examples for other platforms are available in the [examples](examples/) directory.

Detailed example walkthroughs are available in [docs/examples.md](docs/examples.md).

## 📚 Documentation

For complete documentation, see the [docs directory](docs/index.md).

## 🤝 Contributing

Pull requests and suggestions are welcome! Please follow the existing code style and include tests for new features.

## 📄 License

This project is licensed under the **GNU General Public License v3.0**.
See the [LICENSE](LICENSE) file for details.

