---
layout: default
title: "HardFOC PCAL95555 Driver"
description: "Hardware-agnostic C++ driver for the NXP PCAL9555A GPIO expander"
nav_order: 1
permalink: /
---

# HF-PCAL95555 Driver
**Hardware Agnostic PCAL95555 library - as used in the HardFOC-V1 controller**

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![CI Build](https://github.com/N3b3x/hf-pcal95555-driver/actions/workflows/esp32-component-ci.yml/badge.svg?branch=main)](https://github.com/N3b3x/hf-pcal95555-driver/actions/workflows/esp32-component-ci.yml)

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

**PACL95555** is a fully-featured, platform-independent C++ driver for the **PCAL9555A** GPIO expander by NXP Semiconductors. The PCAL9555A provides 16 general-purpose I/O pins (two 8-bit ports) accessible over I²C and supports advanced "Agile I/O" capabilities including interrupts, drive strength control, polarity inversion, input latching, and internal pull-up/pull-down resistors.

This library abstracts all of that into a clear and extensible C++ API, ready to be used across a wide range of embedded platforms such as STM32, ESP32 (ESP-IDF), Arduino, and more.

---

## 🚀 Features

- ✅ 16-bit I²C-controlled GPIO expander (2 x 8-bit ports)
- 🔁 Per-pin direction (input/output)
- 🔌 Digital read/write and toggle support
- 🔄 Input polarity inversion
- 📉 Internal pull-up/down resistors (100kΩ typ.)
- 🔩 Adjustable output drive strength (4 levels)
- 🔀 Open-drain or push-pull output mode per port
- 🧲 Input latching with interrupt capture
- 📡 INT interrupt output with per-pin interrupt masks
- 🧪 Built-in mock-based unit test suite
- 🔌 Hardware-agnostic `i2cBus` interface

---

## 📂 Project Structure

```text
├── datasheet/             # PCAL9555A datasheet PDF
├── examples/              # Sample usage and wiring examples
├── src/                   # Source files
│   ├── pacl95555.hpp      # Driver header
│   ├── pacl95555.cpp      # Driver implementation
│   └── pacl95555_test.cpp # Mock-based unit tests
├── LICENSE                # GNU GPLv3 license
└── README.md              # Project documentation
```

---

## 🔧 Installation

1. **Clone or copy** the `pacl95555.hpp` and `pcal95555.cpp` files into your project.
2. **Implement the `i2cBus` interface** for your platform (examples below).
3. Include the header in your code:

   ```cpp
   #include "pacl95555.hpp"
   ```
4. Compile with any **C++11 or newer** compiler.

---

## 🚀 Quick Start

```cpp
#include "pacl95555.hpp"
MyPlatformI2CBus i2c;               // Custom i2cBus implementation
PACL95555 gpio(&i2c, 0x20);        // 0x20 is default I2C address

gpio.resetToDefault();             // Safe known state (inputs w/ pull-ups)
// Optionally configure using values from Kconfig
gpio.initFromConfig();

gpio.setPinDirection(0, GPIODir::Output);
gpio.writePin(0, true);
bool isHigh = gpio.readPin(1);
```

---

## 📖 API Reference

| Method                             | Description                             |
| ---------------------------------- | --------------------------------------- |
| `setPinDirection(pin, dir)`        | Configure a single pin's direction      |
| `setMultipleDirections(mask, dir)` | Batch pin direction setting             |
| `readPin(pin)`                     | Read logic level of a pin               |
| `writePin(pin, value)`             | Set logic level of an output pin        |
| `togglePin(pin)`                   | Toggle output pin                       |
| `setPullEnable(pin, bool)`         | Enable/disable internal pull resistor   |
| `setPullDirection(pin, bool)`      | Choose pull-up (true) or pull-down      |
| `setDriveStrength(pin, level)`     | Adjust output drive (Level0–Level3)     |
| `setOutputMode(od0, od1)`          | Set port 0/1 to open-drain or push-pull |
| `setPinPolarity(pin, polarity)`    | Invert input polarity                   |
| `enableInputLatch(pin, bool)`      | Enable latching for input capture       |
| `configureInterruptMask(mask)`     | Configure per-pin interrupt masks       |
| `getInterruptStatus()`             | Read and clear interrupt source         |
| `setInterruptCallback(cb)`         | Set callback for interrupt handling     |
| `handleInterrupt()`                | Handle INT signal & invoke callback     |
| `getErrorFlags()`                  | Retrieve latched driver errors          |
| `clearErrorFlags(mask)`            | Clear selected error flags              |

### ❗ Error Handling

Each driver method sets an error flag when it fails (e.g. on I²C NACK or when an invalid pin is passed). The flags persist until the call succeeds or `clearErrorFlags()` is used to reset them.


---

## 🔌 Platform Integration

### ✅ ESP32 (ESP-IDF)

```cpp
class ESP32I2CBus : public PACL95555::i2cBus {
    bool write(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) override {
        uint8_t buf[1 + len];
        buf[0] = reg;
        memcpy(&buf[1], data, len);
        return i2c_master_write_to_device(I2C_NUM_0, addr, buf, len+1, 100 / portTICK_PERIOD_MS) == ESP_OK;
    }

    bool read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) override {
        return i2c_master_write_read_device(I2C_NUM_0, addr, &reg, 1, data, len, 100 / portTICK_PERIOD_MS) == ESP_OK;
    }
};
```

### ✅ STM32 (HAL)

```cpp
class STM32I2CBus : public PACL95555::i2cBus {
    bool write(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) override {
        return HAL_I2C_Mem_Write(&hi2c1, addr<<1, reg, 1, (uint8_t*)data, len, HAL_MAX_DELAY) == HAL_OK;
    }

    bool read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) override {
        return HAL_I2C_Mem_Read(&hi2c1, addr<<1, reg, 1, data, len, HAL_MAX_DELAY) == HAL_OK;
    }
};
```

### ✅ Arduino (Wire)

```cpp
class ArduinoI2CBus : public PACL95555::i2cBus {
    bool write(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) override {
        Wire.beginTransmission(addr);
        Wire.write(reg);
        Wire.write(data, len);
        return Wire.endTransmission() == 0;
    }

    bool read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) override {
        Wire.beginTransmission(addr);
        Wire.write(reg);
        if (Wire.endTransmission(false) != 0) return false;
        Wire.requestFrom(addr, len);
        for (size_t i = 0; i < len; ++i) data[i] = Wire.read();
        return true;
    }
};
```

---

## 📊 Examples

For ESP32 examples, see the [examples/esp32](examples/esp32/) directory. Additional examples for other platforms are available in the [examples](examples/) directory.

---

## 🧪 Unit Testing

To run the built-in unit tests on a desktop:

```bash
# Windows PowerShell
g++ -std=c++11 pacl95555.cpp pcal95555_test.cpp -o test.exe
./test.exe
```

These tests use a mock I2C class to validate:

- Register correctness
- Retry logic
- Polarity inversion
- Interrupt latching
- Pull resistor settings
- Output state reflection

Expected output:

```bash
All tests passed.
```

---

## 🛠 Makefile & Kconfig

A `Makefile` is included for building the library and unit tests.

```bash
make        # build build/libpcal95555.a
make test   # build and run the unit tests
make clean  # remove build directory
```


Configuration options for Kconfig-based projects are provided in the
`Kconfig` file. Enable the driver with `PCAL95555` and override the
default address using `PCAL95555_DEFAULT_ADDRESS`.
Each port contains a submenu for every pin so you can individually
configure direction, pull resistor settings and the initial output level.
Open-drain mode is still set per port. Call `initFromConfig()` at runtime
to apply the selected values. Set `PCAL95555_INIT_FROM_KCONFIG` to `n`
if you want `initFromConfig()` to do nothing at runtime.

## 📚 Documentation

For a full guide including installation steps, API usage, and platform-specific notes, see the [docs directory](docs/index.md). Generate Doxygen documentation with:
```bash
doxygen _config/Doxyfile
```

## 🤝 Contributing

Pull requests and suggestions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## 📄 License

This project is licensed under the **GNU General Public License v3.0**.  
See the [LICENSE](LICENSE) file for details.
