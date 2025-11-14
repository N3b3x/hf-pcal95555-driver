# Examples 💡

A set of minimal projects is provided in the [examples](../examples) directory.
Each subfolder demonstrates the driver on a different platform.

- **arduino/** – Sketch using the Arduino `Wire` library.
- **esp32/** – ESP-IDF application.

- **stm32/** – STM32 HAL example.

After copying an example into your project directory, build it with the normal
toolchain commands for that platform. For example `idf.py build` for the ESP32
or the Arduino IDE for AVR boards. Flash the resulting firmware to quickly test
that the expander is working as expected.

To build an example, copy the code into your own project and adjust the I²C pins
and device address for your hardware. The examples mirror the instructions in
[Platform Integration](./platform_integration.md).

---

**Navigation**
⬅️ [Platform Integration](./platform_integration.md) • [Back to Index](./index.md) • ➡️ [Hardware Overview](./hardware_overview.md)
