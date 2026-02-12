/**
 * @file esp32_pcal95555_test_config.hpp
 * @brief Hardware configuration for PCAL9555 driver on ESP32-S3
 *
 * This file contains the actual hardware configuration that is used by the HAL
 * and example applications. Modify these values to match your hardware setup.
 *
 * @copyright Copyright (c) 2024-2025 HardFOC. All rights reserved.
 */

#pragma once

#include <cstdint>

//==============================================================================
// COMPILE-TIME CONFIGURATION FLAGS
//==============================================================================

/**
 * @brief Enable detailed I2C transaction logging
 *
 * @details
 * When enabled (set to 1), the Esp32Pcal9555I2cBus will log detailed
 * information about each I2C transaction including:
 * - Register read/write addresses and values
 * - Port configuration changes
 * - Interrupt status reads
 *
 * When disabled (set to 0), only basic error logging is performed.
 *
 * Default: 0 (disabled) - Set to 1 to enable for debugging
 */
#ifndef ESP32_PCAL95555_ENABLE_DETAILED_I2C_LOGGING
#define ESP32_PCAL95555_ENABLE_DETAILED_I2C_LOGGING 0
#endif

namespace PCAL95555_TestConfig {

/**
 * @brief I2C Pin Configuration for ESP32-S3
 *
 * These pins are used for I2C communication with the PCAL9555.
 * Ensure your hardware matches these pin assignments or modify accordingly.
 */
struct I2CPins {
    static constexpr uint8_t SDA = 4;           ///< GPIO4 - I2C SDA (data)
    static constexpr uint8_t SCL = 5;           ///< GPIO5 - I2C SCL (clock)
};

/**
 * @brief Hardware Address Pin Configuration
 *
 * GPIO pins connected to the PCAL9555's A0-A2 address inputs.
 * These pins determine the device's I2C address.
 * Set to -1 if hardwired (not controlled by GPIO).
 */
struct AddressPins {
    static constexpr uint8_t A0 = 45;           ///< GPIO45 - Address pin A0
    static constexpr uint8_t A1 = 48;           ///< GPIO48 - Address pin A1
    static constexpr uint8_t A2 = 47;           ///< GPIO47 - Address pin A2
    static constexpr uint8_t A0_LEVEL = 0;      ///< A0 logic level (0=LOW, 1=HIGH)
    static constexpr uint8_t A1_LEVEL = 0;      ///< A1 logic level (0=LOW, 1=HIGH)
    static constexpr uint8_t A2_LEVEL = 0;      ///< A2 logic level (0=LOW, 1=HIGH)
};

/**
 * @brief Control GPIO Pins for PCAL9555
 *
 * Interrupt and reset pins for device control.
 * Set to -1 if not connected/configured.
 */
struct ControlPins {
    static constexpr int8_t INT = 7;            ///< GPIO7 - Interrupt output (active low, open-drain)
    static constexpr int8_t RST = -1;           ///< Reset pin (not available on PCAL9555)
};

/**
 * @brief I2C Communication Parameters
 *
 * The PCAL9555 supports I2C frequencies up to 400kHz (Fast Mode).
 *
 * I2C Addressing (per PCAL9555 datasheet):
 * - Base address: 0x20 (all address pins LOW)
 * - Address range: 0x20 - 0x27 (3 address bits: A0-A2)
 */
struct I2CParams {
    static constexpr uint32_t FREQUENCY = 400000;      ///< 400kHz I2C frequency (Fast Mode)
    static constexpr uint8_t BASE_ADDRESS = 0x20;      ///< Base 7-bit I2C address
    static constexpr bool PULLUP_ENABLE = true;         ///< Enable internal pullups
};

/**
 * @brief GPIO Expander Specifications
 *
 * PCAL9555 is a 16-bit I2C GPIO expander with interrupt support.
 */
struct GPIOSpecs {
    static constexpr uint8_t NUM_PINS = 16;          ///< Total number of GPIO pins
    static constexpr uint8_t NUM_PORTS = 2;          ///< Number of 8-bit ports (Port 0, Port 1)
    static constexpr uint8_t PINS_PER_PORT = 8;      ///< Pins per port
};

/**
 * @brief Supply Voltage Specifications (volts)
 *
 * VDD: Logic supply for PCAL9555
 */
struct SupplyVoltage {
    static constexpr float VDD_MIN = 2.3f;     ///< Minimum VDD voltage (V)
    static constexpr float VDD_NOM = 3.3f;     ///< Nominal VDD voltage (V)
    static constexpr float VDD_MAX = 5.5f;     ///< Maximum VDD voltage (V)
};

/**
 * @brief Temperature Specifications (celsius)
 *
 * Operating temperature range from PCAL9555 datasheet.
 */
struct Temperature {
    static constexpr int16_t OPERATING_MIN = -40;    ///< Minimum operating temperature (°C)
    static constexpr int16_t OPERATING_MAX = 85;     ///< Maximum operating temperature (°C)
    static constexpr int16_t WARNING_THRESHOLD = 75; ///< Temperature warning threshold (°C)
};

/**
 * @brief Timing Parameters
 *
 * Timing requirements from the PCAL9555 datasheet.
 */
struct Timing {
    static constexpr uint16_t POWER_ON_DELAY_MS = 10;       ///< Power-on initialization delay (ms)
    static constexpr uint16_t RESET_DELAY_MS = 1;           ///< Reset recovery delay (ms)
};

/**
 * @brief Diagnostic Thresholds
 *
 * Thresholds for health monitoring and error detection.
 */
struct Diagnostics {
    static constexpr uint16_t POLL_INTERVAL_MS = 100;      ///< Diagnostic polling interval (ms)
    static constexpr uint8_t MAX_RETRY_COUNT = 3;          ///< Maximum communication retries
};

/**
 * @brief Test Configuration
 *
 * Default parameters for testing.
 */
struct TestConfig {
    static constexpr uint16_t TEST_DURATION_MS = 5000;      ///< Test duration (ms)
    static constexpr uint16_t TOGGLE_DELAY_MS = 100;        ///< GPIO toggle delay for tests (ms)
    static constexpr uint16_t INTERRUPT_TIMEOUT_MS = 1000;   ///< Interrupt wait timeout (ms)
};

/**
 * @brief Application-specific Configuration
 *
 * Configuration values that can be adjusted per application.
 */
struct AppConfig {
    // Logging
    static constexpr bool ENABLE_DEBUG_LOGGING = true;     ///< Enable detailed debug logs
    static constexpr bool ENABLE_I2C_LOGGING = false;      ///< Enable I2C transaction logs

    // Performance
    static constexpr bool ENABLE_PERFORMANCE_MONITORING = true;  ///< Enable performance metrics
    static constexpr uint16_t STATS_REPORT_INTERVAL_MS = 10000;  ///< Statistics reporting interval

    // Error handling
    static constexpr bool ENABLE_AUTO_RECOVERY = true;     ///< Enable automatic error recovery
    static constexpr uint8_t MAX_ERROR_COUNT = 10;         ///< Maximum errors before failsafe
};

} // namespace PCAL95555_TestConfig

/**
 * @brief Hardware configuration validation
 *
 * Compile-time checks to ensure configuration is valid.
 */
static_assert(PCAL95555_TestConfig::I2CParams::FREQUENCY <= 400000,
              "I2C frequency exceeds PCAL9555 maximum of 400kHz");

static_assert(PCAL95555_TestConfig::I2CParams::BASE_ADDRESS >= 0x20 &&
              PCAL95555_TestConfig::I2CParams::BASE_ADDRESS <= 0x27,
              "PCAL9555 I2C base address must be in range 0x20-0x27");

static_assert(PCAL95555_TestConfig::GPIOSpecs::NUM_PINS == 16,
              "PCAL9555 has exactly 16 GPIO pins");

static_assert(PCAL95555_TestConfig::AddressPins::A0_LEVEL <= 1 &&
              PCAL95555_TestConfig::AddressPins::A1_LEVEL <= 1 &&
              PCAL95555_TestConfig::AddressPins::A2_LEVEL <= 1,
              "Address pin levels must be 0 or 1");

/**
 * @brief Helper macro for compile-time GPIO pin validation
 */
#define PCAL95555_VALIDATE_GPIO(pin) \
    static_assert((pin) >= 0 && (pin) < 49, "Invalid GPIO pin number for ESP32-S3")
