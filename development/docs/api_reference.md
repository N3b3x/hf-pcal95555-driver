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

**Constructor:**
```cpp
PCAL95555(I2cType* bus, uint8_t address);
```

**Location**: [`inc/pcal95555.hpp`](../inc/pcal95555.hpp)

## Methods

### Initialization

| Method | Signature | Location |
|--------|-----------|----------|
| `ResetToDefault()` | `void ResetToDefault()` | [`src/pcal95555.cpp#L49`](../src/pcal95555.cpp#L49) |
| `InitFromConfig()` | `void InitFromConfig()` | [`src/pcal95555.cpp#L78`](../src/pcal95555.cpp#L78) |

### Pin Direction

| Method | Signature | Location |
|--------|-----------|----------|
| `SetPinDirection()` | `bool SetPinDirection(uint16_t pin, GPIODir dir)` | [`src/pcal95555.cpp#L110`](../src/pcal95555.cpp#L110) |
| `SetMultipleDirections()` | `bool SetMultipleDirections(uint16_t mask, GPIODir dir)` | [`src/pcal95555.cpp#L552`](../src/pcal95555.cpp#L552) |

### Pin I/O

| Method | Signature | Location |
|--------|-----------|----------|
| `ReadPin()` | `bool ReadPin(uint16_t pin)` | [`src/pcal95555.cpp#L560`](../src/pcal95555.cpp#L560) |
| `WritePin()` | `bool WritePin(uint16_t pin, bool value)` | [`src/pcal95555.cpp#L569`](../src/pcal95555.cpp#L569) |
| `TogglePin()` | `bool TogglePin(uint16_t pin)` | [`src/pcal95555.cpp#L577`](../src/pcal95555.cpp#L577) |

### Pull-up/Pull-down

| Method | Signature | Location |
|--------|-----------|----------|
| `SetPullEnable()` | `bool SetPullEnable(uint16_t pin, bool enable)` | [`src/pcal95555.cpp#L586`](../src/pcal95555.cpp#L586) |
| `SetPullDirection()` | `bool SetPullDirection(uint16_t pin, bool pull_up)` | [`src/pcal95555.cpp#L594`](../src/pcal95555.cpp#L594) |

### Drive Strength

| Method | Signature | Location |
|--------|-----------|----------|
| `SetDriveStrength()` | `bool SetDriveStrength(uint16_t pin, DriveStrength level)` | [`src/pcal95555.cpp#L603`](../src/pcal95555.cpp#L603) |

### Interrupts

| Method | Signature | Location |
|--------|-----------|----------|
| `ConfigureInterruptMask()` | `bool ConfigureInterruptMask(uint16_t mask)` | [`src/pcal95555.cpp#L611`](../src/pcal95555.cpp#L611) |
| `GetInterruptStatus()` | `uint16_t GetInterruptStatus()` | [`src/pcal95555.cpp#L617`](../src/pcal95555.cpp#L617) |
| `SetInterruptCallback()` | `void SetInterruptCallback(const std::function<void(uint16_t)>& callback)` | [`src/pcal95555.cpp#L668`](../src/pcal95555.cpp#L668) |
| `HandleInterrupt()` | `void HandleInterrupt()` | [`src/pcal95555.cpp#L675`](../src/pcal95555.cpp#L675) |

### Output Mode

| Method | Signature | Location |
|--------|-----------|----------|
| `SetOutputMode()` | `bool SetOutputMode(bool port_0_open_drain, bool port_1_open_drain)` | [`src/pcal95555.cpp#L626`](../src/pcal95555.cpp#L626) |

### Polarity

| Method | Signature | Location |
|--------|-----------|----------|
| `SetPinPolarity()` | `bool SetPinPolarity(uint16_t pin, Polarity polarity)` | [`src/pcal95555.cpp#L635`](../src/pcal95555.cpp#L635) |
| `SetMultiplePolarities()` | `bool SetMultiplePolarities(uint16_t mask, Polarity polarity)` | [`src/pcal95555.cpp#L644`](../src/pcal95555.cpp#L644) |

### Input Latch

| Method | Signature | Location |
|--------|-----------|----------|
| `EnableInputLatch()` | `bool EnableInputLatch(uint16_t pin, bool enable)` | [`src/pcal95555.cpp#L653`](../src/pcal95555.cpp#L653) |
| `EnableMultipleInputLatches()` | `bool EnableMultipleInputLatches(uint16_t mask, bool enable)` | [`src/pcal95555.cpp#L662`](../src/pcal95555.cpp#L662) |

### Error Handling

| Method | Signature | Location |
|--------|-----------|----------|
| `SetRetries()` | `void SetRetries(int retries)` | [`inc/pcal95555.hpp#L503`](../inc/pcal95555.hpp#L503) |
| `GetErrorFlags()` | `uint16_t GetErrorFlags() const` | [`inc/pcal95555.hpp#L510`](../inc/pcal95555.hpp#L510) |
| `ClearErrorFlags()` | `void ClearErrorFlags(uint16_t mask = 0xFFFF)` | [`inc/pcal95555.hpp#L517`](../inc/pcal95555.hpp#L517) |

## Types

### Enumerations

| Type | Values | Location |
|------|--------|----------|
| `GPIODir` | `Input`, `Output` | [`inc/pcal95555.hpp`](../inc/pcal95555.hpp) |
| `DriveStrength` | `Level0`, `Level1`, `Level2`, `Level3` | [`inc/pcal95555.hpp`](../inc/pcal95555.hpp) |
| `Polarity` | `Normal`, `Inverted` | [`inc/pcal95555.hpp`](../inc/pcal95555.hpp) |

---

**Navigation**
⬅️ [Configuration](configuration.md) | [Next: Examples ➡️](examples.md) | [Back to Index](index.md)
