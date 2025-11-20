# PCAL95555 Comprehensive Test Suite Documentation

## Overview

The comprehensive test suite (`pcal95555_comprehensive_test`) provides thorough validation of all PCAL9555 GPIO expander functionality. This document describes each test, what it validates, and how to interpret the results.

## Test Configuration

### Hardware Target
- **MCU**: ESP32-S3
- **I2C Frequency**: 400 kHz
- **Default I2C Address**: 0x20 (A2=LOW, A1=LOW, A0=LOW)

### Pin Configuration

| Signal | ESP32-S3 GPIO | PCAL9555 Pin | Function |
|--------|---------------|--------------|----------|
| SDA | GPIO4 | SDA | I2C Data Line |
| SCL | GPIO5 | SCL | I2C Clock Line |
| INT | GPIO7 | INT | Interrupt Output (open-drain) |
| A0 | GPIO45 | A0 | Address Bit 0 (controlled by MCU) |
| A1 | GPIO48 | A1 | Address Bit 1 (controlled by MCU) |
| A2 | GPIO47 | A2 | Address Bit 2 (controlled by MCU) |
| Test Indicator | GPIO14 | - | Visual test progress indicator |

### Address Pin Control

The test suite uses GPIO-controlled address pins, allowing dynamic address configuration:
- **A0**: Controlled via GPIO45
- **A1**: Controlled via GPIO48
- **A2**: Controlled via GPIO47

The driver automatically sets these pins during initialization based on the constructor parameters.

---

## Test Sections

### 1. Initialization Tests

#### `test_i2c_bus_initialization()`
**Purpose**: Validates I2C bus setup and configuration.

**What it tests**:
- I2C master bus creation
- GPIO pin configuration for SDA/SCL
- Address pin GPIO configuration (A0, A1, A2)
- Bus initialization verification

**Expected Result**: I2C bus successfully initialized with all pins configured.

**Failure Indications**:
- I2C bus creation failure
- GPIO configuration errors
- Missing address pin configuration

#### `test_driver_initialization()`
**Purpose**: Validates PCAL9555 driver initialization and default state.

**What it tests**:
- Driver instance creation
- I2C communication verification
- Reset to default state
- Error flag checking and clearing

**Expected Result**: Driver successfully initialized, device responds to I2C commands.

**Failure Indications**:
- I2C communication failure
- Device not responding
- Persistent error flags

---

### 2. GPIO Direction Tests

#### `test_single_pin_direction()`
**Purpose**: Validates individual pin direction configuration.

**What it tests**:
- Setting each pin (0-15) to output mode
- Setting each pin (0-15) to input mode
- Direction register read/write operations

**Expected Result**: All pins can be configured as input or output individually.

**Failure Indications**:
- Pin direction not changing
- Register write failures
- Invalid pin handling

#### `test_multiple_pin_direction()`
**Purpose**: Validates bulk pin direction configuration.

**What it tests**:
- Setting multiple pins simultaneously (port 0: pins 0-7)
- Setting multiple pins simultaneously (port 1: pins 8-15)
- Mask-based direction control

**Expected Result**: Multiple pins can be configured simultaneously using bit masks.

**Failure Indications**:
- Mask operations failing
- Incorrect pin states
- Port-level operations not working

---

### 3. GPIO Read/Write Tests

#### `test_pin_write()`
**Purpose**: Validates pin output functionality.

**What it tests**:
- Writing HIGH to a pin
- Writing LOW to a pin
- Output state persistence
- Pin 0 output operations

**Expected Result**: Pin outputs can be set HIGH and LOW reliably.

**Failure Indications**:
- Output not changing state
- Write operations failing
- State not persisting

#### `test_pin_read()`
**Purpose**: Validates pin input functionality.

**What it tests**:
- Reading pin state (HIGH/LOW)
- Reading all 16 pins sequentially
- Input register reading

**Expected Result**: Pin states can be read correctly.

**Failure Indications**:
- Read operations failing
- Incorrect pin states reported
- Input register access issues

#### `test_pin_toggle()`
**Purpose**: Validates pin toggle functionality.

**What it tests**:
- Toggling pin state multiple times
- State transitions (HIGH→LOW→HIGH)
- Toggle operation reliability

**Expected Result**: Pin can be toggled reliably between states.

**Failure Indications**:
- Toggle operations failing
- State not changing
- Multiple toggle issues

---

### 4. Pull Resistor Tests

#### `test_pull_resistor_config()`
**Purpose**: Validates internal pull-up/pull-down resistor configuration.

**What it tests**:
- Enabling pull-up resistor on a pin
- Enabling pull-down resistor on a pin
- Disabling pull resistors
- Pull enable/disable register operations

**Expected Result**: Pull resistors can be configured per pin.

**Failure Indications**:
- Pull configuration not working
- Register write failures
- Pull state not persisting

---

### 5. Drive Strength Tests

#### `test_drive_strength()`
**Purpose**: Validates programmable output drive strength.

**What it tests**:
- Setting drive strength Level 0 (lowest)
- Setting drive strength Level 1
- Setting drive strength Level 2
- Setting drive strength Level 3 (highest)
- Drive strength register configuration

**Expected Result**: All drive strength levels can be configured.

**Failure Indications**:
- Drive strength not changing
- Invalid level handling
- Register access failures

---

### 6. Output Mode Tests

#### `test_output_mode()`
**Purpose**: Validates output mode configuration (push-pull vs open-drain).

**What it tests**:
- Push-pull mode (default)
- Open-drain mode for port 0
- Open-drain mode for port 1
- Open-drain mode for both ports
- Output mode register configuration

**Expected Result**: Output modes can be configured per port.

**Failure Indications**:
- Mode configuration not working
- Port-level control issues
- Register write failures

---

### 7. Polarity Tests

#### `test_polarity_inversion()`
**Purpose**: Validates input polarity inversion functionality.

**What it tests**:
- Normal polarity (non-inverted)
- Inverted polarity
- Single pin polarity configuration
- Multiple pin polarity configuration (mask-based)

**Expected Result**: Input polarity can be inverted per pin.

**Failure Indications**:
- Polarity not changing
- Inversion not working correctly
- Mask operations failing

---

### 8. Input Latch Tests

#### `test_input_latch()`
**Purpose**: Validates input latch functionality.

**What it tests**:
- Enabling input latch on a pin
- Disabling input latch on a pin
- Multiple pin latch configuration
- Latch register operations

**Expected Result**: Input latch can be enabled/disabled per pin.

**Failure Indications**:
- Latch configuration not working
- Latch state not persisting
- Register access issues

---

### 9. Interrupt Tests

#### `test_interrupt_mask_config()`
**Purpose**: Validates interrupt mask configuration.

**What it tests**:
- Enabling interrupts on specific pins (0, 2, 4, 6)
- Enabling interrupts on all pins
- Disabling interrupts on all pins (default state)
- Interrupt mask register operations

**Expected Result**: Interrupt mask can be configured to enable/disable interrupts per pin.

**Failure Indications**:
- Mask configuration not working
- Interrupts not enabling/disabling correctly
- Register write failures

#### `test_interrupt_status()`
**Purpose**: Validates interrupt status reading.

**What it tests**:
- Reading interrupt status register
- Status register clearing behavior
- Initial interrupt status (should be 0)

**Expected Result**: Interrupt status can be read and clears after reading.

**Failure Indications**:
- Status register not readable
- Status not clearing
- Incorrect status values

#### `test_pin_interrupt_callbacks()`
**Purpose**: Validates per-pin interrupt callback registration.

**What it tests**:
- Registering rising edge callback on pin 0
- Registering falling edge callback on pin 1
- Registering both edges callback on pin 2
- Registering rising edge callback on pin 3
- Global interrupt callback registration
- Callback invocation mechanism

**Expected Result**: Per-pin callbacks can be registered with different edge conditions.

**Failure Indications**:
- Callback registration failing
- Callbacks not being invoked
- Edge detection not working

#### `test_interrupt_handler_registration()`
**Purpose**: Validates hardware interrupt handler setup.

**What it tests**:
- GPIO interrupt pin configuration (GPIO7)
- Interrupt handler registration with I2C bus
- FreeRTOS interrupt task setup
- Interrupt queue creation

**Expected Result**: Hardware interrupt handler successfully registered and ready to process interrupts.

**Failure Indications**:
- GPIO configuration failing
- Handler registration failing
- Interrupt infrastructure not set up

**Note**: This test will pass even if the INT pin is not physically connected (graceful degradation).

#### `test_interrupt_callback_unregistration()`
**Purpose**: Validates interrupt callback removal.

**What it tests**:
- Registering a callback for pin 5
- Unregistering the callback
- Attempting to unregister already-unregistered callback
- Invalid pin handling

**Expected Result**: Callbacks can be registered and unregistered correctly.

**Failure Indications**:
- Unregistration failing
- Callbacks still being invoked after unregistration
- Invalid pin handling issues

#### `test_interrupt_config()`
**Purpose**: Legacy interrupt configuration test.

**What it tests**:
- Basic interrupt mask configuration
- Interrupt status reading
- Simple interrupt setup

**Expected Result**: Basic interrupt functionality works.

---

### 10. Port Operation Tests

#### `test_port_operations()`
**Purpose**: Validates port-level operations.

**What it tests**:
- Configuring port 0 (pins 0-7) as output
- Configuring port 1 (pins 8-15) as input
- Writing to port 0 pins
- Reading from port 1 pins
- Port-level register operations

**Expected Result**: Port-level operations work correctly.

**Failure Indications**:
- Port operations failing
- Incorrect pin states
- Port register access issues

---

### 11. Error Handling Tests

#### `test_error_handling()`
**Purpose**: Validates error handling and recovery mechanisms.

**What it tests**:
- Invalid pin number handling (pin 16)
- Error flag reading
- Error flag clearing
- Graceful error recovery

**Expected Result**: Invalid operations are handled gracefully without crashing.

**Failure Indications**:
- Invalid pin operations succeeding (should fail)
- Error flags not being set
- Error recovery not working

---

### 12. Stress Tests

#### `test_rapid_operations()`
**Purpose**: Validates system stability under rapid operations.

**What it tests**:
- Rapid pin toggle operations (100 cycles)
- Continuous read/write cycles
- System stability under load
- Timing and performance

**Expected Result**: System remains stable under rapid operations.

**Failure Indications**:
- Operations failing under load
- System instability
- Timing issues
- I2C communication errors

---

## Test Execution Flow

1. **Initialization Phase**
   - I2C bus setup
   - Driver creation
   - Device reset

2. **Configuration Phase**
   - GPIO direction setup
   - Feature configuration (pull, drive strength, etc.)

3. **Functional Testing Phase**
   - Read/write operations
   - Feature-specific tests
   - Interrupt setup

4. **Validation Phase**
   - Error handling
   - Stress testing
   - Port operations

5. **Cleanup Phase**
   - Resource cleanup
   - Test summary reporting

---

## Test Results Interpretation

### Success Indicators
- ✅ Test name: Test passed successfully
- All operations completed without errors
- Expected behavior observed

### Warning Indicators
- ⚠️ Warning messages: Non-critical issues (e.g., INT pin not connected)
- Tests may still pass with warnings

### Failure Indicators
- ❌ Error messages: Critical failures
- Test execution stopped
- Error flags set

### Test Summary
At the end of execution, a summary is printed showing:
- Total tests run
- Tests passed
- Tests failed
- Success percentage
- Execution time

---

## Enabling/Disabling Tests

Individual test sections can be enabled or disabled by modifying the test configuration flags:

```cpp
static constexpr bool ENABLE_INITIALIZATION_TESTS = true;
static constexpr bool ENABLE_GPIO_DIRECTION_TESTS = true;
static constexpr bool ENABLE_GPIO_READ_WRITE_TESTS = true;
static constexpr bool ENABLE_PULL_RESISTOR_TESTS = true;
static constexpr bool ENABLE_DRIVE_STRENGTH_TESTS = true;
static constexpr bool ENABLE_OUTPUT_MODE_TESTS = true;
static constexpr bool ENABLE_POLARITY_TESTS = true;
static constexpr bool ENABLE_INPUT_LATCH_TESTS = true;
static constexpr bool ENABLE_INTERRUPT_TESTS = true;
static constexpr bool ENABLE_PORT_OPERATION_TESTS = true;
static constexpr bool ENABLE_ERROR_HANDLING_TESTS = true;
static constexpr bool ENABLE_STRESS_TESTS = true;
```

Set to `false` to skip a test section.

---

## Hardware Requirements

### Minimum Requirements
- PCAL9555 GPIO expander board
- ESP32-S3 development board
- I2C connections (SDA, SCL)
- Power connections (3.3V, GND)

### Optional for Full Testing
- INT pin connection (for interrupt tests)
- LEDs on output pins (for visual verification)
- Switches/buttons on input pins (for input testing)
- Logic analyzer (for protocol verification)

---

## Troubleshooting

### Common Issues

1. **I2C Communication Failures**
   - Check SDA/SCL connections
   - Verify pull-up resistors (4.7kΩ recommended)
   - Check I2C address configuration
   - Verify power supply stability

2. **Interrupt Tests Failing**
   - Verify INT pin connection (GPIO7)
   - Check pull-up resistor on INT pin
   - Ensure interrupt mask is configured correctly

3. **Address Pin Issues**
   - Verify GPIO connections (A0=GPIO45, A1=GPIO48, A2=GPIO47)
   - Check address pin levels match expected address
   - Verify GPIO configuration as outputs

4. **Test Timeouts**
   - Check I2C bus speed (may need to reduce frequency)
   - Verify device is responding
   - Check for I2C bus conflicts

---

## Test Duration

Typical test execution time: **2-5 minutes**

The duration depends on:
- Number of enabled test sections
- I2C communication speed
- System load
- Hardware response time

---

## Additional Notes

- **GPIO14 Test Indicator**: Toggles on each completed test for visual progress tracking
- **Serial Output**: Detailed test results are printed to serial console
- **Error Recovery**: Tests attempt to recover from errors and continue
- **Graceful Degradation**: Some tests (e.g., interrupt handler) will pass even if optional hardware is not connected

