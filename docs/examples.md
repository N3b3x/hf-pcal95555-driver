---
layout: default
title: "💡 Examples"
description: "Complete example walkthroughs for the PCAL95555 driver"
nav_order: 7
parent: "📚 Documentation"
permalink: /docs/examples/
---

# Examples

This guide provides complete, working examples demonstrating various use cases for the PCAL95555 driver.

## Example 1: Basic GPIO Control

This example shows basic input/output operations.

```cpp
#include "pcal95555.hpp"

// Implement I2C interface (see platform_integration.md)
class MyI2c : public pcal95555::I2cInterface<MyI2c> {
    // ... implement write() and read()
};

MyI2c i2c;
pcal95555::PCAL95555<MyI2c> gpio(&i2c, 0x20);

void app_main(void) {
    gpio.ResetToDefault();
    
    // Configure pin 0 as output
    gpio.SetPinDirection(0, pcal95555::PCAL95555<MyI2c>::GPIODir::Output);
    gpio.WritePin(0, true);
    
    // Configure pin 1 as input
    gpio.SetPinDirection(1, pcal95555::PCAL95555<MyI2c>::GPIODir::Input);
    bool value = gpio.ReadPin(1);
}
```

### Explanation

1. **Reset**: Put device in known state
2. **Configure**: Set pin directions
3. **Use**: Read and write pin states

---

## Example 2: Multiple Pins

This example demonstrates configuring multiple pins at once.

```cpp
// Set pins 0-7 as outputs
gpio.SetMultipleDirections(0x00FF, pcal95555::PCAL95555<MyI2c>::GPIODir::Output);

// Write to multiple pins
for (int i = 0; i < 8; i++) {
    gpio.WritePin(i, (i % 2 == 0)); // Alternate pattern
}
```

---

## Example 3: Interrupt Handling

This example shows how to use interrupts.

```cpp
// Configure pin 8 as input with interrupt
gpio.SetPinDirection(8, pcal95555::PCAL95555<MyI2c>::GPIODir::Input);
gpio.SetPullEnable(8, true);
gpio.SetPullDirection(8, true); // Pull-up
gpio.EnableInputLatch(8, true);
gpio.ConfigureInterruptMask(~(1 << 8)); // Enable interrupt

// Set callback
gpio.SetInterruptCallback([](uint16_t status) {
    printf("Interrupt on pins: 0x%04X\n", status);
    // Handle interrupt
});

// In your main loop or ISR
gpio.HandleInterrupt();
```

---

## Example 4: Pull Resistors

This example demonstrates pull resistor configuration.

```cpp
// Configure pin as input with pull-down
gpio.SetPinDirection(5, pcal95555::PCAL95555<MyI2c>::GPIODir::Input);
gpio.SetPullEnable(5, true);
gpio.SetPullDirection(5, false); // Pull-down
```

---

## Example 5: Drive Strength

This example shows drive strength configuration.

```cpp
// Set pin as output with reduced drive strength
gpio.SetPinDirection(0, pcal95555::PCAL95555<MyI2c>::GPIODir::Output);
gpio.SetDriveStrength(0, pcal95555::PCAL95555<MyI2c>::DriveStrength::Half);
```

---

## Running the Examples

### ESP32

The examples are available in the [`examples/esp32`](../examples/esp32/) directory.

```bash
cd examples/esp32
idf.py build flash monitor
```

### Other Platforms

Adapt the I2C interface implementation for your platform (see [Platform Integration](platform_integration.md)) and compile with your platform's toolchain.

## Next Steps

- Review the [API Reference](api_reference.md) for method details
- Check [Troubleshooting](troubleshooting.md) if you encounter issues
- Explore the [examples directory](../examples/) for more examples

---

**Navigation**
⬅️ [API Reference](api_reference.md) | [Next: Troubleshooting ➡️](troubleshooting.md) | [Back to Index](index.md)

