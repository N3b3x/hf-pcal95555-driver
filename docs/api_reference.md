# API Reference 📑

This section lists the most important methods provided by `PACL95555`. For a
full definition see [`pcal95555.hpp`](../inc/pcal95555.hpp). The examples below
assume an instance named `gpio` constructed with a platform specific `I2cInterface`
implementation.

## Construction

```cpp
PCAL95555(I2cInterface *bus, uint8_t addr);
```

- `bus`: pointer to an object implementing the `I2cInterface` interface.
- `addr`: I²C address of the PCAL9555A device.

## Basic I/O

| Method | Description |
| ------ | ----------- |
| `SetPinDirection(pin, dir)` | Set pin as input or output |
| `ReadPin(pin)` | Read logic level |
| `WritePin(pin, value)` | Drive output pin |
| `TogglePin(pin)` | Toggle output state |

Example:

```cpp
gpio.SetPinDirection(3, PACL95555::GPIODir::Output);
gpio.WritePin(3, true);
```

`SetPinDirection` takes a pin index `0-15` and a `GPIODir` enumerator. Pins start
as inputs after reset. Writing a value only affects pins configured as outputs.

## Advanced Features

- **Pull Resistors**
  - `SetPullEnable(pin, bool)` to enable/disable
  - `SetPullDirection(pin, bool)` for pull-up or pull-down
- **Drive Strength**
  - `SetDriveStrength(pin, Level0..Level3)` adjusts output current
- **Interrupts**
  - `SetInterruptCallback(cb)` registers a handler
  - `GetInterruptStatus()` returns pending interrupt flags

Example interrupt setup:

```cpp
auto handler = [](uint16_t mask) {
    if (mask & (1 << 4)) {
        // pin 4 changed state
    }
};
gpio.SetInterruptCallback(handler);
gpio.EnableInputLatch(4, true);
```

`EnableInputLatch` latches a pin's logic level when an interrupt occurs so the
callback can safely read the latched value later via `GetInterruptStatus()`.

For further detail consult the inline comments in [`pcal95555.hpp`](../inc/pcal95555.hpp).

---

**Navigation**
⬅️ [Configuration](./configuration.md) • [Back to Index](./index.md) • ➡️ [Platform Integration](./platform_integration.md)
