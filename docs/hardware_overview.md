# Hardware Overview 🔍

The PCAL9555A is a 16-bit I/O expander with an I²C interface.
It offers two 8-bit ports that can be individually configured as inputs or outputs
and supports a variety of features often required in embedded designs.

Pins 0‒7 belong to **PORT0** while pins 8‒15 belong to **PORT1**. Each port has
its own set of configuration registers but the API treats all pins with a single
numbering scheme to simplify usage.

## Key Features

- Two 8-bit I/O ports for a total of 16 pins
- Per-pin interrupt capability with input latching
- Optional pull-up or pull-down resistors
- Adjustable output drive strength
- Selectable push-pull or open-drain outputs per port
- Polarity inversion on input pins

For the detailed register map,
electrical characteristics and timing diagrams consult the official [NXP PCAL9555A Datasheet](../datasheet/PCAL9555A.pdf).

## Registers

Each feature is controlled through registers accessible over I²C. Some of the most commonly used registers are:

| Register | Description |
| -------- | ----------- |
| `INPUT_PORT_0/1` | Current logic level on each pin |
| `OUTPUT_PORT_0/1` | Output values driven on pins |
| `CONFIG_PORT_0/1` | Direction (1=input, 0=output) |
| `PULL_ENABLE_0/1` | Enable internal pull resistor |
| `PULL_SELECT_0/1` | Choose pull-up or pull-down |
| `INT_STATUS_0/1` | Latched interrupt source |

Understanding the registers is helpful when troubleshooting or extending the driver. See the datasheet for a full list.

---

**Navigation**
⬅️ [Examples](./examples.md) • [Back to Index](./index.md) • ➡️ [Documentation Guidelines](./guidelines.md)
