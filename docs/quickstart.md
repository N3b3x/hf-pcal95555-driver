# Quick Start ⚡

This tutorial demonstrates how to bring up a PCAL9555A expander using the library on any platform.

1. **Include the header and create an I2C implementation**

```cpp
#include "pcal95555.hpp"

class MyI2C : public pcal95555::I2cInterface<MyI2C> {
    bool write(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) override;
    bool read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) override;
};
```

2. **Instantiate the driver**

```cpp
MyI2C i2c;
PACL95555 gpio(&i2c, 0x20); // default I2C address
```

3. **Reset the expander to a known state and configure pins**

```cpp
gpio.ResetToDefault(); // all pins become inputs with pull-ups
gpio.SetPinDirection(0, PACL95555::GPIODir::Output);
```

4. **Toggle outputs and read inputs**

```cpp
gpio.WritePin(0, true);
bool input = gpio.ReadPin(1);
```

5. **Handle interrupts (optional)**

```cpp
auto callback = [](uint16_t status) {
    // respond to interrupts here
};
gpio.SetInterruptCallback(callback);
```

These minimal steps bring the expander online. The library exposes many more
functions for configuring pull resistors, drive strength and polarity. Refer to
the [API Reference](./api_reference.md) once you are comfortable with the basics.

For more advanced configuration, see the [Configuration guide](./configuration.md).

---

**Navigation**
⬅️ [Installation](./installation.md) • [Back to Index](./index.md) • ➡️ [Configuration](./configuration.md)
