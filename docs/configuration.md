---
layout: default
title: "⚙️ Configuration"
description: "Configuration options for the PCAL95555 driver"
nav_order: 5
parent: "📚 Documentation"
permalink: /docs/configuration/
---

# Configuration

This guide covers all configuration options available for the PCAL95555 driver.

## I2C Address Configuration

The PCAL9555A I2C address is configured via hardware pins A0-A2. Each pin represents one bit of the 3-bit address field.

| Pin | Address Bit | Description |
|-----|-------------|-------------|
| A0  | Bit 0       | Least significant address bit |
| A1  | Bit 1       | |
| A2  | Bit 2       | Most significant address bit |

**Default I2C address**: `0x20` (all address pins to GND)

**Address Range**: 0x20 to 0x27 (7-bit I2C addresses)

**Example**: To set address 0x21, connect A0 to VDD and A1-A2 to GND.

## Kconfig Configuration (Optional)

If your project uses Kconfig (e.g., ESP-IDF), the driver supports compile-time configuration:

**Location**: [`Kconfig`](../Kconfig)

### Configuration Options

- **Per-pin direction**: Configure each pin as input (1) or output (0)
- **Per-pin pull-up/pull-down**: Enable pull resistors and select direction
- **Per-pin initial output**: Set initial output state
- **Port open-drain**: Configure ports for open-drain or push-pull mode

### Using Kconfig

1. Run `idf.py menuconfig` (or your Kconfig tool)
2. Navigate to component configuration
3. Configure per-pin settings as needed
4. Use `InitFromConfig()` to apply settings:

```cpp
gpio.InitFromConfig(); // Apply Kconfig settings
```

## Runtime Configuration

### Pin Direction

Set individual pin direction:

```cpp
// Set pin 0 as output
gpio.SetPinDirection(0, pcal95555::PCAL95555<MyI2c>::GPIODir::Output);

// Set pin 1 as input
gpio.SetPinDirection(1, pcal95555::PCAL95555<MyI2c>::GPIODir::Input);

// Set multiple pins at once
gpio.SetMultipleDirections(0x000F, pcal95555::PCAL95555<MyI2c>::GPIODir::Output); // Pins 0-3
```

### Pull Resistors

Configure pull-up/pull-down resistors:

```cpp
// Enable pull-up on pin 0
gpio.SetPullEnable(0, true);
gpio.SetPullDirection(0, true); // true = pull-up, false = pull-down

// Enable pull-down on pin 1
gpio.SetPullEnable(1, true);
gpio.SetPullDirection(1, false);
```

### Drive Strength

Set output drive strength:

```cpp
gpio.SetDriveStrength(0, pcal95555::PCAL95555<MyI2c>::DriveStrength::Full);
```

**Drive Strength Levels**:
- `Quarter`: 25% drive strength
- `Half`: 50% drive strength
- `ThreeQuarter`: 75% drive strength
- `Full`: 100% drive strength

### Output Mode

Configure port output mode (push-pull or open-drain):

```cpp
// Set PORT_0 to open-drain, PORT_1 to push-pull
gpio.SetOutputMode(true, false);
```

### Polarity Inversion

Invert input polarity:

```cpp
// Invert pin 0 polarity
gpio.SetPinPolarity(0, pcal95555::PCAL95555<MyI2c>::Polarity::Inverted);

// Invert multiple pins
gpio.SetMultiplePolarities(0x00FF, pcal95555::PCAL95555<MyI2c>::Polarity::Inverted);
```

## Default Configuration

### ResetToDefault()

Resets all registers to power-on defaults:

```cpp
gpio.ResetToDefault();
```

**Default State**:
- All pins: Inputs with pull-ups enabled
- Drive strength: Full
- Output mode: Push-pull
- Interrupts: Masked (disabled)
- Polarity: Normal (not inverted)

**Location**: [`src/pcal95555.cpp#L49`](../src/pcal95555.cpp#L49)

## Interrupt Configuration

### Enable Interrupts

```cpp
// Configure interrupt mask (0 = enabled, 1 = masked)
gpio.ConfigureInterruptMask(0xFF00); // Enable interrupts on PORT_1 (pins 8-15)

// Set interrupt callback
gpio.SetInterruptCallback([](uint16_t status) {
    // Handle interrupt
    printf("Interrupt on pins: 0x%04X\n", status);
});

// Handle interrupt (call from ISR or polling)
gpio.HandleInterrupt();
```

### Get Interrupt Status

```cpp
uint16_t status = gpio.GetInterruptStatus();
// Check which pins triggered interrupt
```

## Recommended Settings

### For Input Pins

```cpp
gpio.SetPinDirection(pin, pcal95555::PCAL95555<MyI2c>::GPIODir::Input);
gpio.SetPullEnable(pin, true);
gpio.SetPullDirection(pin, true); // Pull-up
```

### For Output Pins

```cpp
gpio.SetPinDirection(pin, pcal95555::PCAL95555<MyI2c>::GPIODir::Output);
gpio.SetDriveStrength(pin, pcal95555::PCAL95555<MyI2c>::DriveStrength::Full);
gpio.WritePin(pin, false); // Set initial state
```

### For Interrupt-Driven Inputs

```cpp
gpio.SetPinDirection(pin, pcal95555::PCAL95555<MyI2c>::GPIODir::Input);
gpio.SetPullEnable(pin, true);
gpio.SetPullDirection(pin, true);
gpio.EnableInputLatch(pin, true); // Latch input changes
gpio.ConfigureInterruptMask(~(1 << pin)); // Enable interrupt for this pin
```

## Next Steps

- See [Examples](examples.md) for configuration examples
- Review [API Reference](api_reference.md) for all configuration methods

---

**Navigation**
⬅️ [Platform Integration](platform_integration.md) | [Next: API Reference ➡️](api_reference.md) | [Back to Index](index.md)

