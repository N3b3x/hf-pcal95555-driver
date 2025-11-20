---
layout: default
title: "📖 API Reference"
description: "Complete API reference for the PCAL95555 driver"
nav_order: 6
parent: "📚 Documentation"
permalink: /docs/api_reference/
---

# API Reference

Complete reference documentation for all public methods and types in the PCAL95555 driver.

## Source Code

- **Main Header**: [`inc/pcal95555.hpp`](../inc/pcal95555.hpp)
- **Implementation**: [`src/pcal95555.cpp`](../src/pcal95555.cpp)

## Core Class

### `PCAL95555<I2cType>`

Main driver class for interfacing with the PCAL9555A I/O expander.

**Template Parameter**: `I2cType` - Your I2C interface implementation (must inherit from `pcal95555::I2cInterface<I2cType>`)

**Location**: [`inc/pcal95555.hpp`](../inc/pcal95555.hpp)

**Constructors:**

**Constructor 1: Using address pin levels**
```cpp
PCAL95555(I2cType* bus, bool a0_level, bool a1_level, bool a2_level);
```

**Parameters:**
- `bus`: Pointer to your I2C interface implementation
- `a0_level`: Initial state of A0 pin (true = HIGH/VDD, false = LOW/GND)
- `a1_level`: Initial state of A1 pin (true = HIGH/VDD, false = LOW/GND)
- `a2_level`: Initial state of A2 pin (true = HIGH/VDD, false = LOW/GND)

**Constructor 2: Using I2C address directly**
```cpp
PCAL95555(I2cType* bus, uint8_t address);
```

**Parameters:**
- `bus`: Pointer to your I2C interface implementation
- `address`: 7-bit I2C address (0x20 to 0x27). Address bits are calculated automatically.

**Description:**
Both constructors calculate the I2C address and attempt to set address pins via `SetAddressPins()` if supported by the I2C interface, then verify communication afterward.

**Examples:**
```cpp
// Using pin levels - A2=LOW, A1=LOW, A0=LOW -> address 0x20 (default)
PCAL95555 driver1(bus, false, false, false);

// Using pin levels - A2=LOW, A1=LOW, A0=HIGH -> address 0x21
PCAL95555 driver2(bus, true, false, false);

// Using address directly - address 0x20 (default)
PCAL95555 driver3(bus, 0x20);

// Using address directly - address 0x21
PCAL95555 driver4(bus, 0x21);

// Using address directly - address 0x25
PCAL95555 driver5(bus, 0x25);
```

**Location**: [`inc/pcal95555.hpp`](../inc/pcal95555.hpp)

## Methods

### Initialization

| Method | Signature | Location |
|--------|-----------|----------|
| `EnsureInitialized()` | `bool EnsureInitialized()` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `ResetToDefault()` | `void ResetToDefault()` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `InitFromConfig()` | `void InitFromConfig()` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |

### Pin Direction

| Method | Signature | Location |
|--------|-----------|----------|
| `SetPinDirection()` | `bool SetPinDirection(uint16_t pin, GPIODir dir)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `SetDirections()` | `bool SetDirections(std::initializer_list<std::pair<uint16_t, GPIODir>> configs)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `SetMultipleDirections()` | `bool SetMultipleDirections(uint16_t mask, GPIODir dir)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |

### Pin I/O

| Method | Signature | Location |
|--------|-----------|----------|
| `ReadPin()` | `bool ReadPin(uint16_t pin)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `ReadPins()` | `std::vector<std::pair<uint16_t, bool>> ReadPins(std::initializer_list<uint16_t> pins)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `WritePin()` | `bool WritePin(uint16_t pin, bool value)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `WritePins()` | `bool WritePins(std::initializer_list<std::pair<uint16_t, bool>> configs)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `TogglePin()` | `bool TogglePin(uint16_t pin)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |

### Pull-up/Pull-down

| Method | Signature | Location |
|--------|-----------|----------|
| `SetPullEnable()` | `bool SetPullEnable(uint16_t pin, bool enable)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `SetPullEnables()` | `bool SetPullEnables(std::initializer_list<std::pair<uint16_t, bool>> configs)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `SetPullDirection()` | `bool SetPullDirection(uint16_t pin, bool pull_up)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `SetPullDirections()` | `bool SetPullDirections(std::initializer_list<std::pair<uint16_t, bool>> configs)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |

### Drive Strength

| Method | Signature | Location |
|--------|-----------|----------|
| `SetDriveStrength()` | `bool SetDriveStrength(uint16_t pin, DriveStrength level)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `SetDriveStrengths()` | `bool SetDriveStrengths(std::initializer_list<std::pair<uint16_t, DriveStrength>> configs)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |

### Interrupts

| Method | Signature | Location |
|--------|-----------|----------|
| `ConfigureInterrupt()` | `bool ConfigureInterrupt(uint16_t pin, InterruptState state)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `ConfigureInterrupts()` | `bool ConfigureInterrupts(std::initializer_list<std::pair<uint16_t, InterruptState>> configs)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `ConfigureInterruptMask()` | `bool ConfigureInterruptMask(uint16_t mask)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `GetInterruptStatus()` | `uint16_t GetInterruptStatus()` | [`src/pcal95555.cpp#L300`](../src/pcal95555.cpp#L300) |
| `RegisterPinInterrupt()` | `bool RegisterPinInterrupt(uint16_t pin, InterruptEdge edge, std::function<void(uint16_t, bool)> callback)` | [`src/pcal95555.cpp#L317`](../src/pcal95555.cpp#L317) |
| `UnregisterPinInterrupt()` | `bool UnregisterPinInterrupt(uint16_t pin)` | [`src/pcal95555.cpp#L342`](../src/pcal95555.cpp#L342) |
| `SetInterruptCallback()` | `void SetInterruptCallback(const std::function<void(uint16_t)>& callback)` | [`src/pcal95555.cpp#L360`](../src/pcal95555.cpp#L360) |
| `RegisterInterruptHandler()` | `bool RegisterInterruptHandler()` | [`src/pcal95555.cpp#L366`](../src/pcal95555.cpp#L366) |
| `HandleInterrupt()` | `void HandleInterrupt()` | [`src/pcal95555.cpp#L385`](../src/pcal95555.cpp#L385) |

### Output Mode

| Method | Signature | Location |
|--------|-----------|----------|
| `SetOutputMode()` | `bool SetOutputMode(bool port_0_open_drain, bool port_1_open_drain)` | [`src/pcal95555.cpp#L626`](../src/pcal95555.cpp#L626) |

### Polarity

| Method | Signature | Location |
|--------|-----------|----------|
| `SetPinPolarity()` | `bool SetPinPolarity(uint16_t pin, Polarity polarity)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `SetPolarities()` | `bool SetPolarities(std::initializer_list<std::pair<uint16_t, Polarity>> configs)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `SetMultiplePolarities()` | `bool SetMultiplePolarities(uint16_t mask, Polarity polarity)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |

### Input Latch

| Method | Signature | Location |
|--------|-----------|----------|
| `EnableInputLatch()` | `bool EnableInputLatch(uint16_t pin, bool enable)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `EnableInputLatches()` | `bool EnableInputLatches(std::initializer_list<std::pair<uint16_t, bool>> configs)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `EnableMultipleInputLatches()` | `bool EnableMultipleInputLatches(uint16_t mask, bool enable)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |

### Address Management

| Method | Signature | Location |
|--------|-----------|----------|
| `GetAddress()` | `uint8_t GetAddress() const` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `GetAddressBits()` | `uint8_t GetAddressBits() const` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `ChangeAddress()` | `bool ChangeAddress(bool a0_level, bool a1_level, bool a2_level)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |
| `ChangeAddress()` | `bool ChangeAddress(uint8_t address)` | [`src/pcal95555.cpp`](../src/pcal95555.cpp) |

### Error Handling

| Method | Signature | Location |
|--------|-----------|----------|
| `SetRetries()` | `void SetRetries(int retries)` | [`src/pcal95555.cpp#L46`](../src/pcal95555.cpp#L46) |
| `GetErrorFlags()` | `uint16_t GetErrorFlags() const` | [`src/pcal95555.cpp#L525`](../src/pcal95555.cpp#L525) |
| `ClearErrorFlags()` | `void ClearErrorFlags(uint16_t mask = 0xFFFF)` | [`src/pcal95555.cpp#L530`](../src/pcal95555.cpp#L530) |

## Types

### Enumerations

| Type | Values | Description | Location |
|------|--------|-------------|----------|
| `GPIODir` | `Input`, `Output` | GPIO pin direction | [`inc/pcal95555.hpp`](../inc/pcal95555.hpp) |
| `DriveStrength` | `Level0`, `Level1`, `Level2`, `Level3` | Output drive strength (Level0=25%, Level1=50%, Level2=75%, Level3=100%) | [`inc/pcal95555.hpp`](../inc/pcal95555.hpp) |
| `Polarity` | `Normal`, `Inverted` | Input polarity inversion | [`inc/pcal95555.hpp`](../inc/pcal95555.hpp) |
| `OutputMode` | `PushPull`, `OpenDrain` | Output mode (per port) | [`inc/pcal95555.hpp`](../inc/pcal95555.hpp) |
| `InterruptState` | `Enabled`, `Disabled` | Interrupt enable/disable state | [`inc/pcal95555.hpp`](../inc/pcal95555.hpp) |
| `InterruptEdge` | `Rising`, `Falling`, `Both` | Interrupt edge trigger type | [`inc/pcal95555.hpp`](../inc/pcal95555.hpp) |
| `Error` | `None`, `InvalidPin`, `InvalidMask`, `I2CReadFail`, `I2CWriteFail` | Error conditions (bitmask) | [`inc/pcal95555.hpp`](../inc/pcal95555.hpp) |

---

**Navigation**
⬅️ [Configuration](configuration.md) | [Next: Examples ➡️](examples.md) | [Back to Index](index.md)
