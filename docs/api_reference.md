---
layout: default
title: "📖 API Reference"
description: "Complete API documentation for the PCAL95555 driver"
nav_order: 6
parent: "📚 Documentation"
permalink: /docs/api_reference/
---

# API Reference

Complete reference documentation for all public methods and types in the PCAL95555 driver.

## Source Code

- **Main Header**: [`inc/pcal95555.hpp`](../inc/pcal95555.hpp)
- **Implementation**: [`src/pcal95555.cpp`](../src/pcal95555.cpp)

## Core Classes

### `PCAL95555<I2cType>`

Main driver class for interfacing with the PCAL9555A I/O expander.

**Template Parameter**: `I2cType` - Your I2C interface implementation (must inherit from `pcal95555::I2cInterface<I2cType>`)

**Location**: [`inc/pcal95555.hpp`](../inc/pcal95555.hpp)

**Constructor:**
```cpp
PCAL95555(I2cType* bus, uint8_t address);
```

**Parameters:**
- `bus`: Pointer to your I2C interface implementation
- `address`: 7-bit I2C address of the PCAL9555A (0x20 default)

## Methods

### Initialization

#### `ResetToDefault()`

Reset the device to its power-on default state.

**Signature:**
```cpp
void ResetToDefault();
```

**Location**: [`src/pcal95555.cpp#L49`](../src/pcal95555.cpp#L49)

**Description**: Sets all registers to their default values. All pins become inputs with pull-ups enabled.

**Example:**
```cpp
gpio.ResetToDefault();
```

---

#### `InitFromConfig()`

Initialize using compile-time Kconfig settings.

**Signature:**
```cpp
void InitFromConfig();
```

**Location**: [`src/pcal95555.cpp#L78`](../src/pcal95555.cpp#L78)

**Note**: Only works if `CONFIG_PCAL95555_INIT_FROM_KCONFIG` is enabled.

---

### Pin Direction

#### `SetPinDirection(pin, dir)`

Set the direction of a single pin.

**Signature:**
```cpp
bool SetPinDirection(uint16_t pin, GPIODir dir);
```

**Location**: [`src/pcal95555.cpp#L110`](../src/pcal95555.cpp#L110)

**Parameters:**
- `pin`: Pin number (0-15)
- `dir`: `GPIODir::Input` or `GPIODir::Output`

**Returns:**
- `true` on success
- `false` on failure (invalid pin or I2C error)

---

#### `SetMultipleDirections(mask, dir)`

Set direction for multiple pins at once.

**Signature:**
```cpp
bool SetMultipleDirections(uint16_t mask, GPIODir dir);
```

**Location**: [`src/pcal95555.cpp#L126`](../src/pcal95555.cpp#L126)

**Parameters:**
- `mask`: Bitmask of pins to configure (bit N = pin N)
- `dir`: Direction for all selected pins

---

### Pin I/O

#### `ReadPin(pin)`

Read the state of an input pin.

**Signature:**
```cpp
bool ReadPin(uint16_t pin);
```

**Location**: [`src/pcal95555.cpp#L155`](../src/pcal95555.cpp#L155)

**Parameters:**
- `pin`: Pin number (0-15)

**Returns:**
- `true` if pin is high
- `false` if pin is low or error occurred

---

#### `WritePin(pin, value)`

Write the state of an output pin.

**Signature:**
```cpp
bool WritePin(uint16_t pin, bool value);
```

**Location**: [`src/pcal95555.cpp#L172`](../src/pcal95555.cpp#L172)

**Parameters:**
- `pin`: Pin number (0-15)
- `value`: `true` for high, `false` for low

**Returns:**
- `true` on success
- `false` on failure

---

#### `TogglePin(pin)`

Toggle the state of an output pin.

**Signature:**
```cpp
bool TogglePin(uint16_t pin);
```

**Location**: [`src/pcal95555.cpp#L189`](../src/pcal95555.cpp#L189)

---

### Pull Resistors

#### `SetPullEnable(pin, enable)`

Enable or disable pull resistor on a pin.

**Signature:**
```cpp
bool SetPullEnable(uint16_t pin, bool enable);
```

**Location**: [`inc/pcal95555.hpp#L586`](../inc/pcal95555.hpp#L586)

---

#### `SetPullDirection(pin, pull_up)`

Set pull-up or pull-down direction.

**Signature:**
```cpp
bool SetPullDirection(uint16_t pin, bool pull_up);
```

**Location**: [`inc/pcal95555.hpp#L594`](../inc/pcal95555.hpp#L594)

**Parameters:**
- `pull_up`: `true` for pull-up, `false` for pull-down

---

### Drive Strength

#### `SetDriveStrength(pin, level)`

Set output drive strength for a pin.

**Signature:**
```cpp
bool SetDriveStrength(uint16_t pin, DriveStrength level);
```

**Location**: [`inc/pcal95555.hpp#L603`](../inc/pcal95555.hpp#L603)

**Drive Strength Levels**:
- `DriveStrength::Quarter`
- `DriveStrength::Half`
- `DriveStrength::ThreeQuarter`
- `DriveStrength::Full`

---

### Interrupts

#### `ConfigureInterruptMask(mask)`

Configure which pins can generate interrupts.

**Signature:**
```cpp
bool ConfigureInterruptMask(uint16_t mask);
```

**Location**: [`inc/pcal95555.hpp#L611`](../inc/pcal95555.hpp#L611)

**Parameters:**
- `mask`: Bitmask (0 = interrupt enabled, 1 = masked/disabled)

---

#### `GetInterruptStatus()`

Get the interrupt status register.

**Signature:**
```cpp
uint16_t GetInterruptStatus();
```

**Location**: [`inc/pcal95555.hpp#L617`](../inc/pcal95555.hpp#L617)

**Returns**: Bitmask indicating which pins triggered interrupts

---

#### `SetInterruptCallback(callback)`

Set callback function for interrupts.

**Signature:**
```cpp
void SetInterruptCallback(const std::function<void(uint16_t)>& callback);
```

**Location**: [`inc/pcal95555.hpp#L668`](../inc/pcal95555.hpp#L668)

---

#### `HandleInterrupt()`

Process interrupt status and call callback.

**Signature:**
```cpp
void HandleInterrupt();
```

**Location**: [`inc/pcal95555.hpp#L675`](../inc/pcal95555.hpp#L675)

---

### Configuration

#### `SetOutputMode(port_0_od, port_1_od)`

Set output mode (push-pull or open-drain) per port.

**Signature:**
```cpp
bool SetOutputMode(bool port_0_open_drain, bool port_1_open_drain);
```

**Location**: [`inc/pcal95555.hpp#L626`](../inc/pcal95555.hpp#L626)

---

#### `SetPinPolarity(pin, polarity)`

Set input polarity inversion.

**Signature:**
```cpp
bool SetPinPolarity(uint16_t pin, Polarity polarity);
```

**Location**: [`inc/pcal95555.hpp#L635`](../inc/pcal95555.hpp#L635)

---

#### `EnableInputLatch(pin, enable)`

Enable input latching for interrupt detection.

**Signature:**
```cpp
bool EnableInputLatch(uint16_t pin, bool enable);
```

**Location**: [`inc/pcal95555.hpp#L653`](../inc/pcal95555.hpp#L653)

---

### Error Handling

#### `SetRetries(retries)`

Set number of I2C retry attempts.

**Signature:**
```cpp
void SetRetries(int retries);
```

**Location**: [`inc/pcal95555.hpp#L503`](../inc/pcal95555.hpp#L503)

---

#### `ClearErrorFlags(mask)`

Clear error flags.

**Signature:**
```cpp
void ClearErrorFlags(uint16_t mask = 0xFFFF);
```

**Location**: [`inc/pcal95555.hpp#L517`](../inc/pcal95555.hpp#L517)

---

## Type Definitions

### `GPIODir` Enum

Pin direction enumeration.

```cpp
enum class GPIODir {
    Input = 0,
    Output = 1
};
```

### `DriveStrength` Enum

Drive strength levels.

```cpp
enum class DriveStrength {
    Quarter = 0,
    Half = 1,
    ThreeQuarter = 2,
    Full = 3
};
```

### `Polarity` Enum

Input polarity.

```cpp
enum class Polarity {
    Normal = 0,
    Inverted = 1
};
```

## Thread Safety

The driver is **not thread-safe**. If used in a multi-threaded environment:
- Each `PCAL95555` instance should be used by a single thread
- Use external synchronization (mutex, etc.) for shared access
- I2C bus access must be thread-safe in your I2C interface implementation

---

**Navigation**
⬅️ [Configuration](configuration.md) | [Next: Examples ➡️](examples.md) | [Back to Index](index.md)

