---
layout: default
title: "🤝 Contributing"
description: "Guidelines and information for contributing to the HardFOC PCAL95555 Driver"
nav_order: 5
parent: "🔧 HardFOC PCAL95555 Driver"
permalink: /CONTRIBUTING/
---

# 🤝 Contributing to HardFOC PCAL95555 Driver

Thank you for your interest in contributing to the HardFOC PCAL95555 Driver!
This document provides guidelines and information for contributors.

## 📋 **Code Standards**

### 🎯 **Coding Style and Best Practices**

**All contributors must follow the official HardFOC Coding Standards:**

📚 **[HardFOC Embedded Systems Coding Standards](https://github.com/hardfoc/org-discussions)**

The coding standards document provides comprehensive guidelines for:
- **Naming Conventions**: Functions, variables, constants, classes, and more
- **Code Organization**: File structure, include order, class organization
- **Error Handling**: Patterns for embedded systems
- **Memory Management**: Best practices for resource-constrained systems
- **Thread Safety**: Guidelines for multi-threaded code
- **Documentation Standards**: Doxygen and inline comments
- **Embedded Systems Best Practices**: Fixed-width types, volatile usage, and more

### 🎯 **Quick Reference for PCAL95555 Driver**

- **C++17 Standard Compliance** - All code must be compatible with C++17
- **Consistent Naming** - Follow the established naming conventions:
  - Classes: `PascalCase` (e.g., `PCAL95555`, `I2CBus`)
  - Public Functions: `PascalCase` (e.g., `SetPinMode`, `ReadPin`)
  - Private Functions: `camelCase` (e.g., `checkInitialized`, `writeRegister`)
  - Member Variables: `snake_case` with trailing underscore (e.g., `i2c_bus_`, `device_address_`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `PCAL95555_DEFAULT_ADDRESS`, `MAX_PIN_COUNT`)
  - Local Variables: `snake_case` (e.g., `pin_value`, `port_index`)
  - Parameters: `snake_case` (e.g., `pin`, `mode`)

### 🏗️ **Architecture Guidelines**

- **Hardware Abstraction** - Use hardware-agnostic I2C interface
- **Error Handling** - All functions use appropriate error handling patterns
- **Safety** - Use `noexcept` where appropriate for safety-critical code
- **Dependencies** - Keep dependencies minimal (freestanding where possible)
- **Pin Management** - Support all 16 pins with proper isolation

## 🧪 **Testing**

### 🔧 **Unit Tests and Hardware Validation Requirements**

- **Unit Tests** - Write comprehensive unit tests for all new functionality
- **Hardware Testing** - Test on actual PCAL95555 hardware with ESP32
- **Integration Tests** - Verify compatibility with existing HardFOC systems
- **Performance Tests** - Ensure real-time performance requirements are met
- **Safety Tests** - Validate safety features and error handling
- **Multi-Pin Tests** - Test all 16 pins independently and in parallel

## 📖 **Documentation**

### 📚 **Documentation Standards and Updates**

- **API Documentation** - Update documentation for all public interfaces
- **User Guides** - Create or update guides for new features
- **Example Code** - Provide working examples for GPIO control applications
- **Architecture Documentation** - Document design decisions and patterns
- **Doxygen Comments** - All public APIs must have Doxygen documentation

## 🐛 **Bug Reports**

### 🔍 **How to Report Bugs Effectively**

When reporting bugs, please include:

1. **Hardware Information**: PCAL95555 board, ESP32 version, pin configuration
2. **Environment Details**: ESP-IDF version, compiler version, operating system
3. **Reproduction Steps**: Minimal code example, configuration settings
4. **Hardware Configuration**: Connected peripherals, pin assignments, I2C settings
5. **Debugging Information**: Error messages, log output, stack traces
6. **Pin Information**: Which pin(s) are affected

## ✨ **Feature Requests**

### 🚀 **Proposing New Features and Enhancements**

When proposing new features:

1. **Use Case** - Describe the specific GPIO control use case
2. **Technical Specification** - Provide detailed technical requirements
3. **API Design** - Propose the interface design following established patterns
4. **Implementation Plan** - Outline the implementation approach
5. **Testing Strategy** - Describe how the feature will be tested

## 🔄 **Development Workflow**

### 📋 **Step-by-Step Development Process**

1. **Fork the Repository**
2. **Create a Feature Branch**
3. **Implement Your Changes** following the [Coding Standards](https://github.com/hardfoc/org-discussions)
4. **Write Tests** for your changes
5. **Document Your Changes** with examples
6. **Submit a Pull Request**

## 📋 **Code Quality Standards**

- **C++17 Compliance** - Code compiles without warnings
- **HardFOC Compatibility** - Tested on HardFOC boards
- **Error Handling** - All error conditions handled appropriately
- **Documentation** - All public APIs documented with Doxygen
- **Tests** - Adequate test coverage provided
- **Performance** - Real-time requirements met
- **Coding Standards** - Follows [HardFOC Coding Standards](https://github.com/hardfoc/org-discussions)

## 🔗 **Resources**

- **[HardFOC Coding Standards](https://github.com/hardfoc/org-discussions)** - Complete coding standards document
- **[PCAL95555 Datasheet](../datasheet/)** - Hardware datasheet
- **[Documentation](../docs/)** - Driver documentation
- **[Examples](../examples/)** - Example code and usage patterns

---

## 🚀 Thank You for Contributing to HardFOC

Your contributions help make HardFOC motor controller boards more accessible and powerful for everyone.

