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
// Option 1: Using address directly (recommended)
pcal95555::PCAL95555<MyI2c> gpio(&i2c, 0x20); // Address 0x20 (default)

// Option 2: Using pin levels
// pcal95555::PCAL95555<MyI2c> gpio(&i2c, false, false, false);

void app_main(void) {
    // Initialize driver (lazy initialization happens automatically, but explicit is clearer)
    if (!gpio.EnsureInitialized()) {
        ESP_LOGE("APP", "Failed to initialize PCAL95555");
        return;
    }
    
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

## Example 2: Multiple Pins Configuration

This example demonstrates configuring multiple pins at once with individual settings.

```cpp
// Configure multiple pins with individual directions
gpio.SetDirections({
    {0, GPIODir::Output},
    {1, GPIODir::Input},
    {5, GPIODir::Output},
    {10, GPIODir::Input}
});

// Write to multiple pins at once
gpio.WritePins({
    {0, true},
    {5, false},
    {10, true}
});

// Read multiple pins at once
auto results = gpio.ReadPins({0, 1, 5, 10});
for (const auto& [pin, value] : results) {
    printf("Pin %d: %s\n", pin, value ? "HIGH" : "LOW");
}

// Configure pull resistors for multiple pins
gpio.SetPullEnables({
    {1, true},   // Enable pull on pin 1
    {10, true}   // Enable pull on pin 10
});

gpio.SetPullDirections({
    {1, true},   // Pull-up on pin 1
    {10, false}  // Pull-down on pin 10
});

// Configure drive strength for multiple pins
gpio.SetDriveStrengths({
    {0, DriveStrength::Level3},  // Full strength
    {5, DriveStrength::Level1}   // 50% strength
});
```

---

## Example 3: Interrupt Handling

This example shows how to use interrupts with per-pin callbacks.

```cpp
// Configure pin 8 as input with interrupt
gpio.SetPinDirection(8, pcal95555::PCAL95555<MyI2c>::GPIODir::Input);
gpio.SetPullEnable(8, true);
gpio.SetPullDirection(8, true); // Pull-up
gpio.EnableInputLatch(8, true);
gpio.ConfigureInterrupt(8, InterruptState::Enabled); // Enable interrupt

// Register per-pin callback for rising edge
gpio.RegisterPinInterrupt(8, InterruptEdge::Rising, [](uint16_t pin, bool state) {
    printf("Pin %d interrupt: went HIGH\n", pin);
});

// Register interrupt handler with I2C interface (for hardware INT pin)
gpio.RegisterInterruptHandler(); // Sets up INT pin handling

// In your main loop or ISR (if using polling)
gpio.HandleInterrupt();
```

### Configure Multiple Interrupts

```cpp
// Configure interrupts for multiple pins at once
gpio.ConfigureInterrupts({
    {0, InterruptState::Enabled},   // Enable on pin 0
    {5, InterruptState::Enabled},   // Enable on pin 5
    {10, InterruptState::Enabled},  // Enable on pin 10
    {3, InterruptState::Disabled}   // Disable on pin 3
});
```

### Global Interrupt Callback

```cpp
// Set global callback for all interrupts
gpio.SetInterruptCallback([](uint16_t status) {
    printf("Interrupt on pins: 0x%04X\n", status);
    // Handle interrupt
});
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
gpio.SetDriveStrength(0, pcal95555::PCAL95555<MyI2c>::DriveStrength::Level1); // 50% strength
```

---

## Example 6: Address Management

This example shows how to work with I2C addresses.

```cpp
// Option 1: Create driver using address directly (recommended)
pcal95555::PCAL95555<MyI2c> gpio(&i2c, 0x21); // Address 0x21

// Option 2: Create driver using pin levels
// pcal95555::PCAL95555<MyI2c> gpio(&i2c, true, false, false); // A0=HIGH -> 0x21

// Get current address
uint8_t addr = gpio.GetAddress(); // Returns 0x21
uint8_t bits = gpio.GetAddressBits(); // Returns 1 (binary: 001)

// Change address dynamically using address value (if I2C interface supports GPIO control)
if (gpio.ChangeAddress(0x20)) {
    printf("Address changed to 0x%02X\n", gpio.GetAddress()); // Now 0x20
}

// Change address using pin levels
if (gpio.ChangeAddress(false, false, false)) {
    printf("Address changed to 0x%02X\n", gpio.GetAddress()); // Now 0x20
}
```

## Example 7: Error Handling

This example shows error handling and retry configuration.

```cpp
// Configure retry mechanism
gpio.SetRetries(3); // Retry up to 3 times on I2C failure

// Perform operations
gpio.SetPinDirection(0, pcal95555::PCAL95555<MyI2c>::GPIODir::Output);
gpio.WritePin(0, true);

// Check for errors
uint16_t errors = gpio.GetErrorFlags();
if (errors != 0) {
    if (errors & static_cast<uint16_t>(Error::I2CReadFail)) {
        printf("I2C read failed\n");
    }
    if (errors & static_cast<uint16_t>(Error::I2CWriteFail)) {
        printf("I2C write failed\n");
    }
    if (errors & static_cast<uint16_t>(Error::InvalidPin)) {
        printf("Invalid pin number\n");
    }
    
    // Clear errors
    gpio.ClearErrorFlags();
}
```

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

