# Quick Start ⚡

This tutorial demonstrates how to bring up a PCAL9555A expander using the library on any platform.

1. **Include the header and create an I2C implementation**

```cpp
#include "pacl95555.hpp"

class MyI2C : public PACL95555::i2cBus {
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
gpio.resetToDefault(); // all pins become inputs with pull-ups
gpio.setPinDirection(0, PACL95555::GPIODir::Output);
```

4. **Toggle outputs and read inputs**

```cpp
gpio.writePin(0, true);
bool input = gpio.readPin(1);
```

5. **Handle interrupts (optional)**

```cpp
auto callback = [](uint16_t status) {
    // respond to interrupts here
};
gpio.setInterruptCallback(callback);
```

For more advanced configuration, see the [Configuration guide](./configuration.md).

---

**Navigation**  
⬅️ [Installation](./installation.md) • ➡️ [Configuration](./configuration.md)
