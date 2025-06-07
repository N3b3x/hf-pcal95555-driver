# Configuration ⚙️

The driver can be configured programmatically or through `Kconfig` options. This section covers both approaches.

## Runtime Configuration

The simplest method is to call the appropriate member functions at runtime:

```cpp
gpio.setPinDirection(0, PACL95555::GPIODir::Output);
gpio.setPullEnable(0, true);
gpio.setPullDirection(0, true);  // true = pull-up
```

Runtime configuration gives you full flexibility and can be changed any time after initialization.

## Kconfig Integration

For projects that use Kconfig (e.g. Zephyr, ESP-IDF) the library provides a set of options under the `PCAL95555` menu. Important entries include:

- **PCAL95555_DEFAULT_ADDRESS** – I²C address of the expander
- **PCAL95555_INIT_FROM_KCONFIG** – automatically apply configuration at startup
- **Pin submenus** – specify direction, pull mode and initial level for each pin

Call `initFromConfig()` during startup to apply the selected values.

```cpp
PACL95555 gpio(&i2c, CONFIG_PCAL95555_DEFAULT_ADDRESS);
gpio.initFromConfig();
```

A minimal `Kconfig` snippet might look like:

```
config PCAL95555_DEFAULT_ADDRESS
    hex "Expander address"
    default 0x20

config PCAL95555_INIT_FROM_KCONFIG
    bool "Initialize expander at startup"
    default y
```

Use menuconfig or your build system's configuration tool to adjust these values.

Adjust `Kconfig` to match your hardware setup. Refer to the [Hardware Overview](./hardware_overview.md) for pin capabilities.

---

**Navigation**
⬅️ [Quick Start](./quickstart.md) • [Back to Index](./index.md) • ➡️ [API Reference](./api_reference.md)
