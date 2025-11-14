---
layout: default
title: "🐛 Troubleshooting"
description: "Common issues and solutions for the PCAL95555 driver"
nav_order: 8
parent: "📚 Documentation"
permalink: /docs/troubleshooting/
---

# Troubleshooting

This guide helps you diagnose and resolve common issues when using the PCAL95555 driver.

## Common Error Messages

### Error: Device Not Detected

**Symptoms:**
- `ResetToDefault()` or other operations fail
- No response from device

**Causes:**
- Wrong I2C address
- Hardware connections incorrect
- Pull-up resistors missing

**Solutions:**
1. **Check I2C address**:
   - Default is 0x20 (all A0-A2 pins to GND)
   - Use I2C scanner to verify device address
   - Update address in constructor if different

2. **Verify hardware connections**:
   - Check SDA/SCL connections
   - Verify 4.7kΩ pull-up resistors on SCL and SDA
   - Ensure power connections (VDD and GND)

3. **Test I2C bus**:
   - Verify I2C bus is properly initialized
   - Check I2C bus speed (try 100 kHz if 400 kHz fails)

---

### Error: Invalid Pin

**Symptoms:**
- Methods return `false`
- Pin operations fail

**Causes:**
- Pin number >= 16
- Invalid pin parameter

**Solutions:**
```cpp
// Valid pins are 0-15
if (pin < 16) {
    gpio.SetPinDirection(pin, dir);
}
```

---

### Error: I2C Communication Failures

**Symptoms:**
- Operations fail intermittently
- Timeout errors

**Solutions:**
1. **Increase retries**:
   ```cpp
   gpio.SetRetries(3); // Default is 0
   ```

2. **Check bus speed**: Reduce I2C speed if using long wires

3. **Verify signal integrity**: Check for noise on I2C lines

---

## Hardware Issues

### Device Not Responding

**Symptoms:**
- No response to I2C commands
- Initialization fails

**Checklist:**
- [ ] Verify power supply voltage (1.65V-5.5V)
- [ ] Check all connections are secure
- [ ] Verify pull-up resistors (4.7kΩ) on SCL and SDA
- [ ] Check I2C address configuration (A0-A2 pins)
- [ ] Use I2C scanner to detect device address
- [ ] Verify ground connection

---

### Incorrect Pin States

**Symptoms:**
- Pins don't respond to writes
- Reads return wrong values

**Checklist:**
- [ ] Verify pin direction is set correctly
- [ ] Check pin is configured as output before writing
- [ ] Verify pull resistors are configured if needed
- [ ] Check for external loads affecting pin state

---

## Software Issues

### Compilation Errors

**Error: "No matching function"**

**Solution:**
- Ensure you've implemented all required `I2cInterface` methods
- Check method signatures match exactly

**Error: "Undefined reference"**

**Solution:**
- Verify you're including the header file
- Check include paths are correct

---

### Runtime Errors

**Initialization Fails**

**Checklist:**
- [ ] I2C interface is properly implemented
- [ ] I2C bus is initialized
- [ ] Hardware connections are correct
- [ ] I2C address matches hardware configuration

**Pin Operations Fail**

**Checklist:**
- [ ] Pin number is valid (0-15)
- [ ] Pin direction is set correctly
- [ ] I2C communication is working
- [ ] Error flags are checked

---

## Debugging Tips

### Enable Debug Output

Add debug prints to your I2C interface:

```cpp
bool write(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) {
    printf("I2C Write: addr=0x%02X, reg=0x%02X, len=%zu\n", addr, reg, len);
    // ... your implementation
    bool result = /* ... */;
    printf("Result: %s\n", result ? "OK" : "FAIL");
    return result;
}
```

### Check Error Flags

```cpp
gpio.ClearErrorFlags(); // Clear all errors
// Perform operation
// Check if errors occurred
```

### Use I2C Scanner

Scan the I2C bus to verify device detection:

```cpp
void i2c_scanner() {
    for (uint8_t addr = 0x08; addr < 0x78; addr++) {
        // Try to communicate with address
        if (/* device responds */) {
            printf("Device found at 0x%02X\n", addr);
        }
    }
}
```

---

## FAQ

### Q: Why do my pin writes not work?

**A:** Common causes:
1. Pin not configured as output: Call `SetPinDirection(pin, GPIODir::Output)` first
2. Wrong pin number: Valid pins are 0-15
3. I2C communication failure: Check I2C bus and connections

### Q: How do I use interrupts?

**A:**
1. Configure pin as input
2. Enable input latching: `EnableInputLatch(pin, true)`
3. Configure interrupt mask: `ConfigureInterruptMask(~(1 << pin))`
4. Set callback: `SetInterruptCallback(callback)`
5. Call `HandleInterrupt()` when interrupt occurs

### Q: Can I use multiple PCAL9555A devices?

**A:** Yes! Configure different I2C addresses via A0-A2 pins, then create separate driver instances:
```cpp
pcal95555::PCAL95555<MyI2c> gpio1(&i2c, 0x20); // First device
pcal95555::PCAL95555<MyI2c> gpio2(&i2c, 0x21); // Second device
```

### Q: What's the difference between pull-up and pull-down?

**A:**
- **Pull-up**: Resistor connects pin to VDD (default high when floating)
- **Pull-down**: Resistor connects pin to GND (default low when floating)

Choose based on your circuit requirements.

---

## Getting More Help

If you're still experiencing issues:

1. Check the [API Reference](api_reference.md) for method details
2. Review [Examples](examples.md) for working code
3. Search existing issues on GitHub
4. Open a new issue with:
   - Description of the problem
   - Steps to reproduce
   - Hardware setup details
   - Error messages/logs
   - I2C bus analyzer output (if available)

---

**Navigation**
⬅️ [Examples](examples.md) | [Back to Index](index.md)

