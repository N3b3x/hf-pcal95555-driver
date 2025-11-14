---
layout: default
title: "🛠️ Installation"
description: "Installation and integration instructions for the PCAL95555 driver"
nav_order: 1
parent: "📚 Documentation"
permalink: /docs/installation/
---

# Installation

This guide covers how to integrate the PCAL95555 driver into your project.

## Prerequisites

Before installing the driver, ensure you have:

- **C++11 Compiler**: GCC 4.8+, Clang 3.3+, or MSVC 2013+
- **I2C Interface**: Your platform's I2C driver (ESP-IDF, STM32 HAL, Arduino Wire, etc.)

## Integration

This driver is a **header-only template library** designed to be integrated directly into your project. There's no separate build step required.

### Option 1: Copy Files

Copy the following files into your project:

```
inc/
  └── pcal95555.hpp
src/
  └── pcal95555.cpp
```

**Note**: The driver uses a header-only template design where `pcal95555.cpp` is included by `pcal95555.hpp`. You typically only need to include the header file in your project.

### Option 2: Git Submodule

```bash
cd your-project
git submodule add https://github.com/N3b3x/hf-pcal95555-driver.git drivers/pcal95555
```

Then add to your include path:
```cmake
target_include_directories(your_target PRIVATE drivers/pcal95555/inc)
```

## Including the Header

Simply include the header in your code:

```cpp
#include "pcal95555.hpp"
```

The implementation is included automatically via the header file.

## Next Steps

- Follow the [Quick Start](quickstart.md) guide to create your first application
- Review [Hardware Setup](hardware_setup.md) for wiring instructions
- Check [Platform Integration](platform_integration.md) to implement the I2C interface

---

**Navigation**
⬅️ [Back to Index](index.md) | [Next: Quick Start ➡️](quickstart.md)

