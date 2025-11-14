---
layout: default
title: "⚡ Quick Start"
description: "Get up and running with the PCAL95555 driver in minutes"
nav_order: 2
parent: "📚 Documentation"
permalink: /docs/quickstart/
---

# Quick Start

This guide will get you up and running with the PCAL95555 driver in just a few steps.

## Prerequisites

- [Driver installed](installation.md)
- [Hardware wired](hardware_setup.md)
- [I2C interface implemented](platform_integration.md)

## Minimal Example

Here's a complete working example:

```cpp
#include "pcal95555.hpp"

// 1. Implement the I2C interface
class MyI2c : public pcal95555::I2cInterface<MyI2c> {
public:
    bool write(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) {
        // Your I2C write implementation
        return true;
    }
    
    bool read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) {
        // Your I2C read implementation
        return true;
    }
};

// 2. Create instances
MyI2c i2c;
pcal95555::PCAL95555<MyI2c> gpio(&i2c, 0x20); // 0x20 is default I2C address

// 3. Initialize
gpio.ResetToDefault(); // all pins become inputs with pull-ups

// 4. Configure and use
gpio.SetPinDirection(0, pcal95555::PCAL95555<MyI2c>::GPIODir::Output);
gpio.WritePin(0, true);
bool input = gpio.ReadPin(1);
```

## Step-by-Step Explanation

### Step 1: Include the Header

```cpp
#include "pcal95555.hpp"
```

### Step 2: Implement the I2C Interface

You need to implement the `I2cInterface` for your platform. See [Platform Integration](platform_integration.md) for detailed examples for ESP32, STM32, and Arduino.

The interface requires two methods:
- `write(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len)` - Write data to a register
- `read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len)` - Read data from a register

### Step 3: Create Driver Instance

```cpp
MyI2c i2c;
pcal95555::PCAL95555<MyI2c> gpio(&i2c, 0x20);
```

The constructor takes:
- Pointer to your I2C interface implementation
- I2C address (0x20 is the default, can be changed via A0-A2 pins)

### Step 4: Initialize

```cpp
gpio.ResetToDefault();
```

`ResetToDefault()` puts the device in a known state (all pins as inputs with pull-ups enabled).

### Step 5: Configure and Use Pins

```cpp
// Set pin 0 as output
gpio.SetPinDirection(0, pcal95555::PCAL95555<MyI2c>::GPIODir::Output);

// Write to pin 0
gpio.WritePin(0, true);

// Read from pin 1 (input)
bool value = gpio.ReadPin(1);
```

## Expected Output

When running this example:
- Pin 0 should be set high (if connected to an LED, it should light up)
- Pin 1 value should be read correctly
- No error messages should appear

## Troubleshooting

If you encounter issues:

- **Compilation errors**: Check that you've implemented all required I2C interface methods
- **Initialization fails**: Verify hardware connections and I2C address
- **No response**: Check I2C bus and pull-up resistors
- **See**: [Troubleshooting](troubleshooting.md) for common issues

## Next Steps

- Explore [Examples](examples.md) for more advanced usage
- Review the [API Reference](api_reference.md) for all available methods
- Check [Configuration](configuration.md) for customization options

---

**Navigation**
⬅️ [Installation](installation.md) | [Next: Hardware Setup ➡️](hardware_setup.md) | [Back to Index](index.md)

