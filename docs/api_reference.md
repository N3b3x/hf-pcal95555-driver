# API Reference

This section lists the most important methods provided by `PACL95555`. For a full definition see [`pacl95555.hpp`](../src/pacl95555.hpp).

## Construction

```cpp
PACL95555(i2cBus *bus, uint8_t addr);
```

- `bus`: pointer to an object implementing the `i2cBus` interface.
- `addr`: I²C address of the PCAL9555A device.

## Basic I/O

| Method | Description |
| ------ | ----------- |
| `setPinDirection(pin, dir)` | Set pin as input or output |
| `readPin(pin)` | Read logic level |
| `writePin(pin, value)` | Drive output pin |
| `togglePin(pin)` | Toggle output state |

Example:

```cpp
gpio.setPinDirection(3, PACL95555::GPIODir::Output);
gpio.writePin(3, true);
```

## Advanced Features

- **Pull Resistors**
  - `setPullEnable(pin, bool)` to enable/disable
  - `setPullDirection(pin, bool)` for pull-up or pull-down
- **Drive Strength**
  - `setDriveStrength(pin, Level0..Level3)` adjusts output current
- **Interrupts**
  - `setInterruptCallback(cb)` registers a handler
  - `getInterruptStatus()` returns pending interrupt flags

For further detail consult the inline comments in [`pacl95555.hpp`](../src/pacl95555.hpp).
