# API Reference 📑

This section lists the most important methods provided by `PACL95555`. For a
full definition see [`pcal95555.hpp`](../inc/pcal95555.hpp). The examples below
assume an instance named `gpio` constructed with a platform specific `i2cBus`
implementation.

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

`setPinDirection` takes a pin index `0-15` and a `GPIODir` enumerator. Pins start
as inputs after reset. Writing a value only affects pins configured as outputs.

## Advanced Features

- **Pull Resistors**
  - `setPullEnable(pin, bool)` to enable/disable
  - `setPullDirection(pin, bool)` for pull-up or pull-down
- **Drive Strength**
  - `setDriveStrength(pin, Level0..Level3)` adjusts output current
- **Interrupts**
  - `setInterruptCallback(cb)` registers a handler
  - `getInterruptStatus()` returns pending interrupt flags

Example interrupt setup:

```cpp
auto handler = [](uint16_t mask) {
    if (mask & (1 << 4)) {
        // pin 4 changed state
    }
};
gpio.setInterruptCallback(handler);
gpio.enableInputLatch(4, true);
```

`enableInputLatch` latches a pin's logic level when an interrupt occurs so the
callback can safely read the latched value later via `getInterruptStatus()`.

For further detail consult the inline comments in [`pcal95555.hpp`](../inc/pcal95555.hpp).

---

**Navigation**
⬅️ [Configuration](./configuration.md) • [Back to Index](./index.md) • ➡️ [Platform Integration](./platform_integration.md)
