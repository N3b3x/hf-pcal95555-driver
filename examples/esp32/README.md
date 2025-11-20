# PCAL9555 ESP32-S3 Comprehensive Test Suite

This directory contains comprehensive test suites for the PCAL9555 16-bit I/O expander driver using the ESP32-S3.

## 📋 Table of Contents

- [Hardware Overview](#-hardware-overview)
- [Pin Connections](#-pin-connections)
- [Hardware Setup](#-hardware-setup)
- [Building the Tests](#-building-the-tests)
- [Running the Tests](#-running-the-tests)
- [Test Suites](#-test-suites)
- [Troubleshooting](#-troubleshooting)

---

## 🔌 Hardware Overview

### ESP32-S3

The ESP32-S3 serves as the host controller for communicating with the PCAL9555 GPIO expander via I2C.

```
┌─────────────────────────────────────────────────┐
│        ESP32-S3 Development Board               │
│                                                 │
│  ┌──────────────────────────────────────────┐   │
│  │        ESP32-S3 Microcontroller          │   │
│  │                                          │   │
│  │  GPIO Pins:                              │   │
│  │  • I2C: SDA (GPIO4), SCL (GPIO5)         │   │
│  │  • Interrupt: INT (GPIO7)                │   │
│  │  • Address Control:                      │   │
│  │   - A0 (GPIO45), A1 (GPIO48), A2 (GPIO47)│   │
│  │  • Test Indicator: GPIO14                │   │
│  └──────────────────────────────────────────┘   │
│                                                 │
│  USB-C Connector                                │
│  (Power + Serial Communication)                 │
└─────────────────────────────────────────────────┘
```

### PCAL9555 GPIO Expander

The PCAL9555 is a 16-bit I/O expander with I²C interface, providing 16 GPIO pins organized into two 8-bit ports.

```
┌─────────────────────────────────────────────────┐
│      PCAL9555 GPIO Expander                     │
│                                                 │
│  ┌──────────────────────────────────────────┐   │
│  │        PCAL9555 IC                       │   │
│  │                                          │   │
│  │  Features:                               │   │
│  │  • 16 GPIO pins (PORT_0: P0.0-P0.7)      │   │
│  │  • 16 GPIO pins (PORT_1: P1.0-P1.7)      │   │
│  │  • I2C interface (7-bit address)         │   │
│  │  • Configurable pull-up/pull-down        │   │
│  │  • Programmable drive strength           │   │
│  │  • Interrupt support                     │   │
│  │  • Input latch capability                │   │
│  │  • Polarity inversion                    │   │
│  │  • Open-drain output mode                │   │
│  └──────────────────────────────────────────┘   │
│                                                 │
│  I2C Connections:                               │
│  • SDA (I2C Data)                               │
│  • SCL (I2C Clock)                              │
│  • VDD (3.3V Power)                             │
│  • GND (Ground)                                 │
│  • INT (Interrupt Output, optional)             │
└─────────────────────────────────────────────────┘
```

---

## 📌 Pin Connections

### I2C Bus Connections

| PCAL9555 Pin | ESP32-S3 GPIO | Function | Notes |
|--------------|---------------|----------|-------|
| SDA | GPIO4 | I2C Data | With 4.7kΩ pull-up to 3.3V |
| SCL | GPIO5 | I2C Clock | With 4.7kΩ pull-up to 3.3V |
| VDD | 3.3V | Power Supply | |
| GND | GND | Ground | |
| INT | GPIO7 | Interrupt Output | Open-drain, requires pull-up |

### Address Pin Control (GPIO-Controlled)

The comprehensive test uses GPIO pins to dynamically control the PCAL9555 address pins:

| PCAL9555 Pin | ESP32-S3 GPIO | Function | Notes |
|--------------|---------------|----------|-------|
| A0 | GPIO45 | Address Bit 0 | Controlled by MCU (output) |
| A1 | GPIO48 | Address Bit 1 | Controlled by MCU (output) |
| A2 | GPIO47 | Address Bit 2 | Controlled by MCU (output) |

**Note**: The driver automatically configures these GPIOs as outputs and sets them during initialization based on the constructor parameters. This allows dynamic address configuration without hardware changes.

### Test Indicator

| Signal | ESP32-S3 GPIO | Function |
|--------|---------------|----------|
| Test Progress | GPIO14 | Visual test progression indicator |

### I2C Address Configuration

The PCAL9555 I2C address is determined by the A0, A1, and A2 address pins:

| A2 | A1 | A0 | I2C Address (7-bit) | I2C Address (8-bit) |
|----|----|----|---------------------|---------------------|
| LOW | LOW | LOW | 0x20 | 0x40 |
| LOW | LOW | HIGH | 0x21 | 0x42 |
| LOW | HIGH | LOW | 0x22 | 0x44 |
| LOW | HIGH | HIGH | 0x23 | 0x46 |
| HIGH | LOW | LOW | 0x24 | 0x48 |
| HIGH | LOW | HIGH | 0x25 | 0x4A |
| HIGH | HIGH | LOW | 0x26 | 0x4C |
| HIGH | HIGH | HIGH | 0x27 | 0x4E |

**Default**: All address pins LOW = **0x20** (used in comprehensive test)

The comprehensive test configures address pins via GPIO:
- **A0**: GPIO45 (default: LOW)
- **A1**: GPIO48 (default: LOW)
- **A2**: GPIO47 (default: LOW)

This results in I2C address **0x20** by default.

---

## 🛠️ Hardware Setup

### Basic Setup for Comprehensive Test

1. **Connect I2C Bus**:
   - Connect PCAL9555 SDA to ESP32-S3 GPIO4
   - Connect PCAL9555 SCL to ESP32-S3 GPIO5
   - Add 4.7kΩ pull-up resistors on both SDA and SCL to 3.3V

2. **Power Connections**:
   - Connect PCAL9555 VDD to ESP32-S3 3.3V
   - Connect PCAL9555 GND to ESP32-S3 GND

3. **Interrupt Connection** (for interrupt tests):
   - Connect PCAL9555 INT to ESP32-S3 GPIO7
   - Add 4.7kΩ pull-up resistor to 3.3V (INT is open-drain)

4. **Address Pin Control** (GPIO-controlled):
   - Connect PCAL9555 A0 to ESP32-S3 GPIO45
   - Connect PCAL9555 A1 to ESP32-S3 GPIO48
   - Connect PCAL9555 A2 to ESP32-S3 GPIO47
   - **Note**: These are controlled by the MCU, not hardwired to GND/VDD

5. **Test Indicator** (optional):
   - Connect LED to ESP32-S3 GPIO14 (with current-limiting resistor)
   - Provides visual feedback of test progress

### Test Setup

For comprehensive testing, you can connect:
- LEDs to output pins (with current-limiting resistors)
- Switches/buttons to input pins
- External pull-up/pull-down resistors for testing
- Logic analyzer on I2C bus for protocol verification

---

## 🚀 Building the Tests

### Prerequisites

1. **Install ESP-IDF** (if not already installed):

   ```bash
   # Clone ESP-IDF
   git clone --recursive https://github.com/espressif/esp-idf.git
   cd esp-idf
   
   # Checkout release version 5.5
   git checkout release/v5.5
   git submodule update --init --recursive
   
   # Install ESP-IDF (Linux/macOS)
   ./install.sh esp32s3
   
   # Set up environment (add to ~/.bashrc or ~/.zshrc for persistence)
   . ./export.sh
   ```

2. **Navigate to ESP32 Examples**:

   ```bash
   cd examples/esp32
   ```

3. **Setup Repository** (First time only):

   ```bash
   # Make scripts executable and setup the build environment
   chmod +x scripts/*.sh
   ./scripts/setup_repo.sh
   ```

### Available Test Applications

The test suites use a centralized build system with scripts. Available applications:

| **Application Name** | **Description** | **Hardware Required** |
|----------------------|----------------|----------------------|
| `pcal9555_comprehensive_test` | Comprehensive PCAL9555 GPIO expander testing with all features | PCAL9555 board |

### List Available Applications

```bash
# List all available applications
./scripts/build_app.sh list
```

### Build an Application

```bash
# Build comprehensive test (Debug build)
./scripts/build_app.sh pcal9555_comprehensive_test Debug

# Build comprehensive test (Release build)
./scripts/build_app.sh pcal9555_comprehensive_test Release
```

---

## 📤 Running the Tests

### Flash Application

```bash
# Flash the application to ESP32-S3
./scripts/flash_app.sh pcal95555_comprehensive_test Debug

# Or manually:
idf.py -p /dev/ttyUSB0 flash
```

### Monitor Output

```bash
# Monitor serial output
idf.py -p /dev/ttyUSB0 monitor

# Or use the flash script which includes monitoring
./scripts/flash_app.sh pcal95555_comprehensive_test Debug
```

### Auto-detect Port

```bash
# The scripts can auto-detect the port
./scripts/detect_ports.sh
```

---

## 🧪 Test Suites

### Comprehensive Test Suite

**Application**: `pcal95555_comprehensive_test`

This comprehensive test suite validates all PCAL9555 functionality. For detailed documentation on each test, see [Comprehensive Test Documentation](docs/comprehensive_test.md).

**Key Features**:
- GPIO-controlled address pins (A0-A2 via GPIO45, GPIO48, GPIO47)
- Hardware interrupt support (INT pin on GPIO7)
- Complete feature coverage (all PCAL9555 registers and functions)

This comprehensive test suite validates all PCAL9555 functionality:

#### Test Sections

1. **Initialization Tests**
   - I2C bus initialization
   - Driver initialization
   - Reset to default state

2. **GPIO Direction Tests**
   - Single pin direction configuration (input/output)
   - Multiple pin direction configuration
   - Port-level direction control

3. **GPIO Read/Write Tests**
   - Pin write operations (HIGH/LOW)
   - Pin read operations
   - Pin toggle operations
   - Port-level read/write

4. **Pull Resistor Tests**
   - Pull-up resistor configuration
   - Pull-down resistor configuration
   - Pull enable/disable
   - Per-pin pull configuration

5. **Drive Strength Tests**
   - All drive strength levels (Level0-Level3)
   - Per-pin drive strength configuration

6. **Output Mode Tests**
   - Push-pull mode configuration
   - Open-drain mode configuration
   - Port-level output mode control

7. **Polarity Tests**
   - Input polarity inversion (normal/inverted)
   - Single pin polarity configuration
   - Multiple pin polarity configuration

8. **Input Latch Tests**
   - Input latch enable/disable
   - Single pin latch configuration
   - Multiple pin latch configuration

9. **Interrupt Tests**
   - Interrupt mask configuration
   - Interrupt status reading
   - Interrupt callback registration

10. **Port Operation Tests**
    - Port 0 operations (pins 0-7)
    - Port 1 operations (pins 8-15)
    - Mixed port operations

11. **Error Handling Tests**
    - Invalid pin handling
    - Error flag management
    - Error recovery

12. **Stress Tests**
    - Rapid pin operations
    - Continuous read/write cycles
    - Multi-pin simultaneous operations

#### Test Configuration

You can enable/disable specific test sections by editing the test file:

```cpp
// In PCAL9555ComprehensiveTest.cpp
static constexpr bool ENABLE_INITIALIZATION_TESTS = true;
static constexpr bool ENABLE_GPIO_DIRECTION_TESTS = true;
// ... etc
```

#### Test Results

The test framework provides:
- Automatic pass/fail tracking
- Execution time measurement
- GPIO14 progress indicator (toggles on each test)
- Comprehensive test summary
- Success percentage calculation

---

## 🔧 Configuration

### I2C Bus Configuration

Default I2C configuration (can be modified in test file):

```cpp
Esp32Pcal9555Bus::I2CConfig config;
config.port = I2C_NUM_0;
config.sda_pin = GPIO_NUM_4;      // SDA pin (ESP32-S3)
config.scl_pin = GPIO_NUM_5;       // SCL pin (ESP32-S3)
config.frequency = 400000;         // 400 kHz
config.pullup_enable = true;        // Enable internal pullups

// Address pin GPIO control (ESP32-S3)
config.a0_pin = GPIO_NUM_45;       // A0 address pin control
config.a1_pin = GPIO_NUM_48;       // A1 address pin control
config.a2_pin = GPIO_NUM_47;       // A2 address pin control
```

### PCAL9555 Address Configuration

The driver uses GPIO-controlled address pins. Default address configuration:

```cpp
// Address pin levels (A2, A1, A0)
static constexpr bool PCAL9555_A0_LEVEL = false;  // LOW = 0x20
static constexpr bool PCAL9555_A1_LEVEL = false;  // LOW = 0x20
static constexpr bool PCAL9555_A2_LEVEL = false;  // LOW = 0x20
// Results in I2C address 0x20
```

The driver automatically sets the GPIO pins during initialization to configure the I2C address.

---

## 🐛 Troubleshooting

### I2C Communication Failures

**Symptoms**: Tests fail with I2C errors

**Solutions**:
1. **Check I2C connections**:
   - Verify SDA/SCL connections
   - Check pull-up resistors (4.7kΩ recommended)
   - Ensure proper power connections

2. **Verify I2C address**:
   - Check A0, A1, A2 pin configuration
   - Use I2C scanner to detect device address
   - Update `PCAL9555_I2C_ADDRESS` if different

3. **Check I2C bus speed**:
   - Reduce frequency if using long wires
   - Try 100 kHz instead of 400 kHz

4. **Verify power supply**:
   - Ensure 3.3V is stable
   - Check for voltage drops

### Build Errors

**Symptoms**: CMake or compilation errors

**Solutions**:
1. **Verify ESP-IDF version**:
   ```bash
   idf.py --version
   # Should show ESP-IDF v5.5 or compatible
   ```

2. **Clean and rebuild**:
   ```bash
   idf.py fullclean
   ./scripts/build_app.sh pcal95555_comprehensive_test Debug
   ```

3. **Check component paths**:
   - Verify component CMakeLists.txt paths
   - Ensure source files are accessible

### Test Failures

**Symptoms**: Specific tests fail

**Solutions**:
1. **Check hardware connections**:
   - Verify all pins are properly connected
   - Check for loose connections

2. **Review test logs**:
   - Check which specific test failed
   - Review error messages in serial output

3. **Verify device state**:
   - Reset PCAL9555 (power cycle)
   - Run reset test first

---

## 📚 Additional Resources

- [Comprehensive Test Documentation](docs/comprehensive_test.md) - Detailed test documentation
- [PCAL9555 Datasheet](../../datasheet/PCAL9555A.pdf)
- [Driver API Documentation](../../docs/api_reference.md)
- [Platform Integration Guide](../../docs/platform_integration.md)
- [Hardware Overview](../../docs/hardware_overview.md)

---

## 🎯 Quick Reference

### Build Commands

```bash
# List available apps
./scripts/build_app.sh list

# Build comprehensive test
./scripts/build_app.sh pcal95555_comprehensive_test Debug

# Flash and monitor
./scripts/flash_app.sh pcal95555_comprehensive_test Debug
```

### Test Execution

The comprehensive test suite runs automatically on boot and provides:
- Real-time test progress via GPIO14 indicator
- Serial output with detailed test results
- Automatic test summary at completion

### GPIO14 Test Indicator

GPIO14 toggles between HIGH/LOW for each completed test, providing visual feedback:
- Use oscilloscope or logic analyzer to monitor
- Useful for automated test verification
- Blinks 5 times at section start/end

---

## 📝 Notes

- **I2C Pull-ups**: External pull-up resistors (4.7kΩ) are recommended even if internal pullups are enabled
- **Address Configuration**: Default address is 0x20. Modify if using different A0/A1/A2 configuration
- **Test Duration**: Comprehensive test suite takes approximately 2-5 minutes to complete
- **Hardware Requirements**: Basic tests work without external hardware; some tests benefit from LEDs/switches

---

<div style="text-align: center; margin: 2em 0; padding: 1em; background: #f8f9fa; border-radius: 8px;">
  <strong>🎯 Ready to test the PCAL9555?</strong><br>
  Start with: <code>./scripts/build_app.sh pcal95555_comprehensive_test Debug</code><br>
  <small>See <a href="docs/comprehensive_test.md">Comprehensive Test Documentation</a> for detailed test information</small>
</div>

