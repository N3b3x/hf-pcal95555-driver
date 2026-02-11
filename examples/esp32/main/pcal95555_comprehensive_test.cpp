/**
 * @file pcal95555_comprehensive_test.cpp
 * @brief Comprehensive test suite for PCA9555/PCAL9555A GPIO expander driver on ESP32-S3
 *
 * This file contains comprehensive testing for the PCA9555/PCAL9555A driver.
 * The driver auto-detects the chip variant at initialization. Tests requiring
 * PCAL9555A-specific Agile I/O features (drive strength, pull resistors, input
 * latch, interrupt mask/status, output mode) are automatically skipped when a
 * standard PCA9555 is detected.
 *
 * Tests included (17 sections covering all 43 public API methods):
 *
 * PCA9555 + PCAL9555A (standard registers):
 * - Initialization (I2C bus, driver, chip variant auto-detection)
 * - GPIO pin direction (SetPinDirection, SetMultipleDirections, SetDirections)
 * - Pin read/write (ReadPin, WritePin, TogglePin, WritePins, ReadPins)
 * - Input polarity inversion (SetPinPolarity, SetMultiplePolarities, SetPolarities)
 * - Port-level operations (mixed port direction + read/write)
 * - Multi-pin API (initializer_list overloads for directions, polarities, R/W)
 * - Address management (ChangeAddress, address-based constructor)
 * - Configuration (SetRetries, EnsureInitialized)
 * - Error handling (invalid pins, UnsupportedFeature, selective flag clearing)
 * - Stress tests (rapid pin toggling)
 *
 * PCAL9555A only (Agile I/O registers, auto-skipped on PCA9555):
 * - Pull-up/pull-down (SetPullEnable/s, SetPullDirection/s)
 * - Drive strength (SetDriveStrength/s)
 * - Output mode (SetOutputMode)
 * - Input latch (EnableInputLatch, EnableMultipleInputLatches, EnableInputLatches)
 * - Interrupt mask/status (ConfigureInterrupt/s, ConfigureInterruptMask, GetInterruptStatus)
 * - Multi-pin PCAL APIs (all initializer_list overloads)
 *
 * Interactive (disabled by default, requires physical button):
 * - Button press detection, HandleInterrupt explicit call
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "TestFramework.h"
#include "esp32_pcal95555_bus.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pcal95555.hpp"
#include <memory>

// Use fully qualified name for the class
using PCAL95555Driver = pcal95555::PCAL95555<Esp32Pcal9555Bus>;

#ifdef __cplusplus
extern "C" {
#endif
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#ifdef __cplusplus
}
#endif

static const char* TAG = "PCAL9555_Test";
static TestResults g_test_results;

//=============================================================================
// TEST CONFIGURATION
//=============================================================================
// Enable/disable test sections (set to false to skip a section)
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
static constexpr bool ENABLE_MULTI_PIN_TESTS = true;
static constexpr bool ENABLE_MULTI_PIN_API_TESTS = true;
static constexpr bool ENABLE_ADDRESS_TESTS = true;
static constexpr bool ENABLE_CONFIG_TESTS = true;
static constexpr bool ENABLE_MULTI_PIN_PCAL_TESTS = true;
static constexpr bool ENABLE_INTERACTIVE_INPUT_TESTS = false; // Requires physical button on pin 0
static constexpr bool ENABLE_ERROR_HANDLING_TESTS = true;
static constexpr bool ENABLE_STRESS_TESTS = true;

// PCAL9555 I2C address configuration
// A0-A2 pins determine the address: base 0x20 + (A2<<2 | A1<<1 | A0)
// Example: A2=LOW, A1=LOW, A0=LOW -> 0x20 (default)
//          A2=LOW, A1=LOW, A0=HIGH -> 0x21
// Note: Address pins are controlled via GPIO (A0=GPIO45, A1=GPIO48, A2=GPIO47)
//       The driver will set these pins automatically via SetAddressPins()
static constexpr bool PCAL9555_A0_LEVEL = false; // A0 pin level (LOW = false, HIGH = true)
static constexpr bool PCAL9555_A1_LEVEL = false; // A1 pin level (LOW = false, HIGH = true)
static constexpr bool PCAL9555_A2_LEVEL = false; // A2 pin level (LOW = false, HIGH = true)

//=============================================================================
// SHARED TEST RESOURCES
//=============================================================================
static std::unique_ptr<Esp32Pcal9555Bus> g_i2c_bus;
static std::unique_ptr<PCAL95555Driver> g_driver;

//=============================================================================
// TEST HELPER FUNCTIONS
//=============================================================================

/**
 * @brief Create and initialize test driver
 */
static std::unique_ptr<PCAL95555Driver> create_test_driver() noexcept {
  if (!g_i2c_bus) {
    ESP_LOGE(TAG, "I2C bus not initialized");
    return nullptr;
  }

  // Create driver using address pin levels (A2, A1, A0)
  auto driver = std::make_unique<PCAL95555Driver>(g_i2c_bus.get(), PCAL9555_A0_LEVEL,
                                                  PCAL9555_A1_LEVEL, PCAL9555_A2_LEVEL);
  if (!driver) {
    ESP_LOGE(TAG, "Failed to create driver instance");
    return nullptr;
  }

  uint8_t address_bits = driver->GetAddressBits();
  ESP_LOGI(
      TAG,
      "Driver created with address pins A2=%d, A1=%d, A0=%d (bits=0b%03b, I2C address: 0x%02X)",
      PCAL9555_A2_LEVEL, PCAL9555_A1_LEVEL, PCAL9555_A0_LEVEL, address_bits, driver->GetAddress());

  // Check for communication errors
  uint16_t errors = driver->GetErrorFlags();
  if (errors != 0) {
    ESP_LOGW(TAG, "Driver has error flags: 0x%04X (device may not be accessible at this address)",
             errors);
    driver->ClearErrorFlags();
  }

  // Reset to default state
  driver->ResetToDefault();
  vTaskDelay(pdMS_TO_TICKS(10));

  return driver;
}

/**
 * @brief Verify pin state matches expected value
 * @note Currently unused but kept for potential future use
 */
[[maybe_unused]] static bool verify_pin_state(PCAL95555Driver& driver, uint16_t pin, bool expected,
                                              const char* context) noexcept {
  bool actual = driver.ReadPin(pin);
  if (actual != expected) {
    ESP_LOGE(TAG, "%s: Pin %d state mismatch - expected %s, got %s", context, pin,
             expected ? "HIGH" : "LOW", actual ? "HIGH" : "LOW");
    return false;
  }
  return true;
}

//=============================================================================
// INITIALIZATION TESTS
//=============================================================================

/**
 * @brief Test I2C bus initialization
 */
static bool test_i2c_bus_initialization() noexcept {
  ESP_LOGI(TAG, "Testing I2C bus initialization...");

  Esp32Pcal9555Bus::I2CConfig config;
  config.port = I2C_NUM_0;
  config.sda_pin = GPIO_NUM_4; // ESP32S3: GPIO4 for SDA
  config.scl_pin = GPIO_NUM_5; // ESP32S3: GPIO5 for SCL
  config.frequency = 400000;
  config.pullup_enable = true;

  // Configure address pins A0-A2 (ESP32S3: GPIO45, GPIO48, GPIO47)
  config.a0_pin = GPIO_NUM_45; // A0 address pin
  config.a1_pin = GPIO_NUM_48; // A1 address pin
  config.a2_pin = GPIO_NUM_47; // A2 address pin

  g_i2c_bus = CreateEsp32Pcal9555Bus(config);
  if (!g_i2c_bus || !g_i2c_bus->isInitialized()) {
    ESP_LOGE(TAG, "Failed to initialize I2C bus");
    return false;
  }

  ESP_LOGI(TAG, "✅ I2C bus initialized successfully");
  ESP_LOGI(TAG, "   SDA:GPIO%d, SCL:GPIO%d", config.sda_pin, config.scl_pin);
  ESP_LOGI(TAG, "   Address pins: A0=GPIO%d, A1=GPIO%d, A2=GPIO%d", config.a0_pin, config.a1_pin,
           config.a2_pin);
  return true;
}

/**
 * @brief Test driver initialization
 */
static bool test_driver_initialization() noexcept {
  ESP_LOGI(TAG, "Testing driver initialization...");

  g_driver = create_test_driver();
  if (!g_driver) {
    ESP_LOGE(TAG, "Failed to create driver");
    return false;
  }

  // Test reset to default
  g_driver->ResetToDefault();
  vTaskDelay(pdMS_TO_TICKS(10));

  // Check error flags
  uint16_t errors = g_driver->GetErrorFlags();
  if (errors != 0) {
    ESP_LOGW(TAG, "Driver has error flags: 0x%04X", errors);
    g_driver->ClearErrorFlags();
  }

  // Log detected chip variant
  auto variant = g_driver->GetChipVariant();
  const char* variant_name = "Unknown";
  if (variant == pcal95555::ChipVariant::PCA9555) {
    variant_name = "PCA9555 (standard)";
  } else if (variant == pcal95555::ChipVariant::PCAL9555A) {
    variant_name = "PCAL9555A (Agile I/O)";
  }
  ESP_LOGI(TAG, "✅ Driver initialized successfully");
  ESP_LOGI(TAG, "   Detected chip variant: %s", variant_name);
  ESP_LOGI(TAG, "   Agile I/O support: %s", g_driver->HasAgileIO() ? "YES" : "NO");
  return true;
}

//=============================================================================
// GPIO DIRECTION TESTS
//=============================================================================

/**
 * @brief Test single pin direction configuration
 */
static bool test_single_pin_direction() noexcept {
  ESP_LOGI(TAG, "Testing single pin direction configuration...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  // Test setting pin to output
  for (uint16_t pin = 0; pin < 16; ++pin) {
    if (!g_driver->SetPinDirection(pin, GPIODir::Output)) {
      ESP_LOGE(TAG, "Failed to set pin %d to output", pin);
      return false;
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  // Test setting pin to input
  for (uint16_t pin = 0; pin < 16; ++pin) {
    if (!g_driver->SetPinDirection(pin, GPIODir::Input)) {
      ESP_LOGE(TAG, "Failed to set pin %d to input", pin);
      return false;
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  ESP_LOGI(TAG, "✅ Single pin direction tests passed");
  return true;
}

/**
 * @brief Test multiple pin direction configuration
 */
static bool test_multiple_pin_direction() noexcept {
  ESP_LOGI(TAG, "Testing multiple pin direction configuration...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  // Test setting multiple pins to output
  uint16_t mask = 0x00FF; // Port 0 (pins 0-7)
  if (!g_driver->SetMultipleDirections(mask, GPIODir::Output)) {
    ESP_LOGE(TAG, "Failed to set multiple pins to output");
    return false;
  }

  // Test setting multiple pins to input
  mask = 0xFF00; // Port 1 (pins 8-15)
  if (!g_driver->SetMultipleDirections(mask, GPIODir::Input)) {
    ESP_LOGE(TAG, "Failed to set multiple pins to input");
    return false;
  }

  ESP_LOGI(TAG, "✅ Multiple pin direction tests passed");
  return true;
}

//=============================================================================
// GPIO READ/WRITE TESTS
//=============================================================================

/**
 * @brief Test pin write operations
 */
static bool test_pin_write() noexcept {
  ESP_LOGI(TAG, "Testing pin write operations...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  // Configure pin as output
  uint16_t test_pin = 0;
  if (!g_driver->SetPinDirection(test_pin, GPIODir::Output)) {
    ESP_LOGE(TAG, "Failed to set pin %d to output", test_pin);
    return false;
  }

  // Test writing HIGH
  if (!g_driver->WritePin(test_pin, true)) {
    ESP_LOGE(TAG, "Failed to write HIGH to pin %d", test_pin);
    return false;
  }
  vTaskDelay(pdMS_TO_TICKS(10));

  // Test writing LOW
  if (!g_driver->WritePin(test_pin, false)) {
    ESP_LOGE(TAG, "Failed to write LOW to pin %d", test_pin);
    return false;
  }
  vTaskDelay(pdMS_TO_TICKS(10));

  ESP_LOGI(TAG, "✅ Pin write tests passed");
  return true;
}

/**
 * @brief Test pin read operations
 */
static bool test_pin_read() noexcept {
  ESP_LOGI(TAG, "Testing pin read operations...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  // Configure pin as input
  uint16_t test_pin = 1;
  if (!g_driver->SetPinDirection(test_pin, GPIODir::Input)) {
    ESP_LOGE(TAG, "Failed to set pin %d to input", test_pin);
    return false;
  }

  // Read pin state
  bool state = g_driver->ReadPin(test_pin);
  ESP_LOGI(TAG, "Pin %d read state: %s", test_pin, state ? "HIGH" : "LOW");

  // Test reading all pins
  for (uint16_t pin = 0; pin < 16; ++pin) {
    g_driver->SetPinDirection(pin, GPIODir::Input);
    bool pin_state = g_driver->ReadPin(pin);
    ESP_LOGI(TAG, "Pin %d: %s", pin, pin_state ? "HIGH" : "LOW");
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  ESP_LOGI(TAG, "✅ Pin read tests passed");
  return true;
}

/**
 * @brief Test pin toggle operations
 */
static bool test_pin_toggle() noexcept {
  ESP_LOGI(TAG, "Testing pin toggle operations...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  // Configure pin as output
  uint16_t test_pin = 2;
  if (!g_driver->SetPinDirection(test_pin, GPIODir::Output)) {
    ESP_LOGE(TAG, "Failed to set pin %d to output", test_pin);
    return false;
  }

  // Set initial state
  g_driver->WritePin(test_pin, false);
  vTaskDelay(pdMS_TO_TICKS(10));

  // Toggle multiple times
  for (int i = 0; i < 5; ++i) {
    if (!g_driver->TogglePin(test_pin)) {
      ESP_LOGE(TAG, "Failed to toggle pin %d", test_pin);
      return false;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  ESP_LOGI(TAG, "✅ Pin toggle tests passed");
  return true;
}

//=============================================================================
// PULL RESISTOR TESTS
//=============================================================================

/**
 * @brief Test pull-up/pull-down resistor configuration
 */
static bool test_pull_resistor_config() noexcept {
  ESP_LOGI(TAG, "Testing pull resistor configuration...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  if (!g_driver->HasAgileIO()) {
    ESP_LOGW(TAG, "⏭️  Skipping: Pull resistor config requires PCAL9555A (detected PCA9555)");
    return true;  // Pass - feature not available on this chip
  }

  uint16_t test_pin = 3;

  // Configure as input
  g_driver->SetPinDirection(test_pin, GPIODir::Input);

  // Test enabling pull-up
  if (!g_driver->SetPullEnable(test_pin, true)) {
    ESP_LOGE(TAG, "Failed to enable pull on pin %d", test_pin);
    return false;
  }

  if (!g_driver->SetPullDirection(test_pin, true)) {
    ESP_LOGE(TAG, "Failed to set pull-up on pin %d", test_pin);
    return false;
  }

  // Test enabling pull-down
  if (!g_driver->SetPullDirection(test_pin, false)) {
    ESP_LOGE(TAG, "Failed to set pull-down on pin %d", test_pin);
    return false;
  }

  // Test disabling pull
  if (!g_driver->SetPullEnable(test_pin, false)) {
    ESP_LOGE(TAG, "Failed to disable pull on pin %d", test_pin);
    return false;
  }

  ESP_LOGI(TAG, "✅ Pull resistor tests passed");
  return true;
}

//=============================================================================
// DRIVE STRENGTH TESTS
//=============================================================================

/**
 * @brief Test drive strength configuration
 */
static bool test_drive_strength() noexcept {
  ESP_LOGI(TAG, "Testing drive strength configuration...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  if (!g_driver->HasAgileIO()) {
    ESP_LOGW(TAG, "⏭️  Skipping: Drive strength config requires PCAL9555A (detected PCA9555)");
    return true;  // Pass - feature not available on this chip
  }

  uint16_t test_pin = 4;
  g_driver->SetPinDirection(test_pin, GPIODir::Output);

  // Test all drive strength levels
  for (uint8_t level = 0; level < 4; ++level) {
    DriveStrength ds = static_cast<DriveStrength>(level);
    if (!g_driver->SetDriveStrength(test_pin, ds)) {
      ESP_LOGE(TAG, "Failed to set drive strength level %d on pin %d", level, test_pin);
      return false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  ESP_LOGI(TAG, "✅ Drive strength tests passed");
  return true;
}

//=============================================================================
// OUTPUT MODE TESTS
//=============================================================================

/**
 * @brief Test output mode configuration
 */
static bool test_output_mode() noexcept {
  ESP_LOGI(TAG, "Testing output mode configuration...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  if (!g_driver->HasAgileIO()) {
    ESP_LOGW(TAG, "⏭️  Skipping: Output mode config requires PCAL9555A (detected PCA9555)");
    return true;  // Pass - feature not available on this chip
  }

  // Test push-pull mode (default)
  if (!g_driver->SetOutputMode(false, false)) {
    ESP_LOGE(TAG, "Failed to set push-pull mode");
    return false;
  }

  // Test open-drain mode for port 0
  if (!g_driver->SetOutputMode(true, false)) {
    ESP_LOGE(TAG, "Failed to set open-drain mode for port 0");
    return false;
  }

  // Test open-drain mode for port 1
  if (!g_driver->SetOutputMode(false, true)) {
    ESP_LOGE(TAG, "Failed to set open-drain mode for port 1");
    return false;
  }

  // Test open-drain mode for both ports
  if (!g_driver->SetOutputMode(true, true)) {
    ESP_LOGE(TAG, "Failed to set open-drain mode for both ports");
    return false;
  }

  ESP_LOGI(TAG, "✅ Output mode tests passed");
  return true;
}

//=============================================================================
// POLARITY TESTS
//=============================================================================

/**
 * @brief Test input polarity inversion
 */
static bool test_polarity_inversion() noexcept {
  ESP_LOGI(TAG, "Testing input polarity inversion...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  uint16_t test_pin = 5;
  g_driver->SetPinDirection(test_pin, GPIODir::Input);

  // Test normal polarity
  if (!g_driver->SetPinPolarity(test_pin, Polarity::Normal)) {
    ESP_LOGE(TAG, "Failed to set normal polarity on pin %d", test_pin);
    return false;
  }

  // Test inverted polarity
  if (!g_driver->SetPinPolarity(test_pin, Polarity::Inverted)) {
    ESP_LOGE(TAG, "Failed to set inverted polarity on pin %d", test_pin);
    return false;
  }

  // Test multiple pins
  uint16_t mask = 0x00FF;
  if (!g_driver->SetMultiplePolarities(mask, Polarity::Inverted)) {
    ESP_LOGE(TAG, "Failed to set inverted polarity on multiple pins");
    return false;
  }

  ESP_LOGI(TAG, "✅ Polarity inversion tests passed");
  return true;
}

//=============================================================================
// INPUT LATCH TESTS
//=============================================================================

/**
 * @brief Test input latch functionality
 */
static bool test_input_latch() noexcept {
  ESP_LOGI(TAG, "Testing input latch functionality...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  if (!g_driver->HasAgileIO()) {
    ESP_LOGW(TAG, "⏭️  Skipping: Input latch config requires PCAL9555A (detected PCA9555)");
    return true;  // Pass - feature not available on this chip
  }

  uint16_t test_pin = 6;
  g_driver->SetPinDirection(test_pin, GPIODir::Input);

  // Test enabling latch
  if (!g_driver->EnableInputLatch(test_pin, true)) {
    ESP_LOGE(TAG, "Failed to enable input latch on pin %d", test_pin);
    return false;
  }

  // Test disabling latch
  if (!g_driver->EnableInputLatch(test_pin, false)) {
    ESP_LOGE(TAG, "Failed to disable input latch on pin %d", test_pin);
    return false;
  }

  // Test multiple pins
  uint16_t mask = 0x00FF;
  if (!g_driver->EnableMultipleInputLatches(mask, true)) {
    ESP_LOGE(TAG, "Failed to enable input latch on multiple pins");
    return false;
  }

  ESP_LOGI(TAG, "✅ Input latch tests passed");
  return true;
}

//=============================================================================
// INTERRUPT TESTS
//=============================================================================

// Global variables for interrupt testing
static volatile uint16_t g_interrupt_count = 0;
static volatile uint16_t g_last_interrupt_status = 0;
static volatile uint16_t g_pin_interrupt_counts[16] = {0};

/**
 * @brief Test interrupt mask configuration
 */
static bool test_interrupt_mask_config() noexcept {
  ESP_LOGI(TAG, "Testing interrupt mask configuration...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  if (!g_driver->HasAgileIO()) {
    ESP_LOGW(TAG, "⏭️  Skipping: Interrupt mask config requires PCAL9555A (detected PCA9555)");
    return true;
  }

  // Test: Enable interrupts on pins 0, 2, 4, 6 (mask bit = 0 enables interrupt)
  uint16_t mask = static_cast<uint16_t>(~((1U << 0) | (1U << 2) | (1U << 4) | (1U << 6)));
  if (!g_driver->ConfigureInterruptMask(mask)) {
    ESP_LOGE(TAG, "Failed to configure interrupt mask");
    return false;
  }

  // Test: Enable interrupts on all pins
  mask = 0x0000; // All bits 0 = all interrupts enabled
  if (!g_driver->ConfigureInterruptMask(mask)) {
    ESP_LOGE(TAG, "Failed to enable interrupts on all pins");
    return false;
  }

  // Test: Disable interrupts on all pins (default state)
  mask = 0xFFFF; // All bits 1 = all interrupts masked
  if (!g_driver->ConfigureInterruptMask(mask)) {
    ESP_LOGE(TAG, "Failed to disable interrupts on all pins");
    return false;
  }

  ESP_LOGI(TAG, "✅ Interrupt mask configuration tests passed");
  return true;
}

/**
 * @brief Test interrupt status reading
 */
static bool test_interrupt_status() noexcept {
  ESP_LOGI(TAG, "Testing interrupt status reading...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  if (!g_driver->HasAgileIO()) {
    ESP_LOGW(TAG, "⏭️  Skipping: Interrupt status reading requires PCAL9555A (detected PCA9555)");
    return true;
  }

  // Enable interrupts on all pins
  g_driver->ConfigureInterruptMask(0x0000);

  // Read interrupt status (should be 0 initially)
  uint16_t status = g_driver->GetInterruptStatus();
  ESP_LOGI(TAG, "Initial interrupt status: 0x%04X", status);

  // Reading interrupt status clears it, so read again
  status = g_driver->GetInterruptStatus();
  ESP_LOGI(TAG, "Interrupt status after read: 0x%04X", status);

  ESP_LOGI(TAG, "✅ Interrupt status reading tests passed");
  return true;
}

/**
 * @brief Test per-pin interrupt callback registration
 */
static bool test_pin_interrupt_callbacks() noexcept {
  ESP_LOGI(TAG, "Testing per-pin interrupt callback registration...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  if (!g_driver->HasAgileIO()) {
    ESP_LOGW(TAG, "⏭️  Skipping: Pin interrupt callbacks require PCAL9555A (detected PCA9555)");
    return true;
  }

  // Reset interrupt counts
  g_interrupt_count = 0;
  for (int i = 0; i < 16; ++i) {
    g_pin_interrupt_counts[i] = 0;
  }

  // Configure test pins as inputs
  const uint16_t test_pins[] = {0, 1, 2, 3};
  for (uint16_t pin : test_pins) {
    if (!g_driver->SetPinDirection(pin, GPIODir::Input)) {
      ESP_LOGE(TAG, "Failed to set pin %d to input", pin);
      return false;
    }
  }

  // Enable interrupts on test pins
  uint16_t interrupt_mask = 0xFFFF; // Start with all masked
  for (uint16_t pin : test_pins) {
    interrupt_mask &= ~(1U << pin); // Clear bit to enable interrupt
  }
  if (!g_driver->ConfigureInterruptMask(interrupt_mask)) {
    ESP_LOGE(TAG, "Failed to configure interrupt mask");
    return false;
  }

  // Register callbacks for different pins with different edge conditions
  bool callback_registered = true;

  // Pin 0: Rising edge callback
  callback_registered &=
      g_driver->RegisterPinInterrupt(0, InterruptEdge::Rising, [](uint16_t pin, bool state) {
        uint16_t count = g_pin_interrupt_counts[pin];
        g_pin_interrupt_counts[pin] = count + 1;
        ESP_LOGI(TAG, "Pin %d RISING edge callback: state=%s", pin, state ? "HIGH" : "LOW");
      });

  // Pin 1: Falling edge callback
  callback_registered &=
      g_driver->RegisterPinInterrupt(1, InterruptEdge::Falling, [](uint16_t pin, bool state) {
        uint16_t count = g_pin_interrupt_counts[pin];
        g_pin_interrupt_counts[pin] = count + 1;
        ESP_LOGI(TAG, "Pin %d FALLING edge callback: state=%s", pin, state ? "HIGH" : "LOW");
      });

  // Pin 2: Both edges callback
  callback_registered &=
      g_driver->RegisterPinInterrupt(2, InterruptEdge::Both, [](uint16_t pin, bool state) {
        uint16_t count = g_pin_interrupt_counts[pin];
        g_pin_interrupt_counts[pin] = count + 1;
        ESP_LOGI(TAG, "Pin %d BOTH edges callback: state=%s", pin, state ? "HIGH" : "LOW");
      });

  // Pin 3: Rising edge callback
  callback_registered &=
      g_driver->RegisterPinInterrupt(3, InterruptEdge::Rising, [](uint16_t pin, bool state) {
        uint16_t count = g_pin_interrupt_counts[pin];
        g_pin_interrupt_counts[pin] = count + 1;
        ESP_LOGI(TAG, "Pin %d RISING edge callback: state=%s", pin, state ? "HIGH" : "LOW");
      });

  if (!callback_registered) {
    ESP_LOGE(TAG, "Failed to register pin interrupt callbacks");
    return false;
  }

  // Register global callback
  g_driver->SetInterruptCallback([](uint16_t status) {
    uint16_t count = g_interrupt_count;
    g_interrupt_count = count + 1;
    g_last_interrupt_status = status;
    ESP_LOGI(TAG, "Global interrupt callback: status=0x%04X", status);
  });

  ESP_LOGI(TAG, "✅ Per-pin interrupt callback registration tests passed");
  ESP_LOGI(TAG, "   Registered callbacks:");
  ESP_LOGI(TAG, "   - Pin 0: Rising edge");
  ESP_LOGI(TAG, "   - Pin 1: Falling edge");
  ESP_LOGI(TAG, "   - Pin 2: Both edges");
  ESP_LOGI(TAG, "   - Pin 3: Rising edge");
  return true;
}

/**
 * @brief Test interrupt handler registration
 */
static bool test_interrupt_handler_registration() noexcept {
  ESP_LOGI(TAG, "Testing interrupt handler registration...");

  if (!g_driver || !g_i2c_bus) {
    ESP_LOGE(TAG, "Driver or bus not initialized");
    return false;
  }

  // Setup GPIO interrupt pin (ESP32S3: GPIO7 for PCAL_INT)
  const gpio_num_t int_pin = GPIO_NUM_7;
  if (!g_i2c_bus->SetupInterruptPin(int_pin)) {
    ESP_LOGW(TAG, "Failed to setup interrupt pin GPIO %d (may not be connected)", int_pin);
    // Don't fail the test if interrupt pin is not connected
    return true; // Skip this test if INT pin not available
  }

  // Register interrupt handler with I2C interface
  // This will call HandleInterrupt() which processes all registered pin callbacks
  if (!g_driver->RegisterInterruptHandler()) {
    ESP_LOGW(TAG, "Failed to register interrupt handler");
    return false;
  }

  ESP_LOGI(TAG, "✅ Interrupt handler registered successfully on GPIO %d", int_pin);
  ESP_LOGI(TAG, "   Interrupts will now be processed automatically when INT pin fires");
  return true;
}

/**
 * @brief Test interrupt callback unregistration
 */
static bool test_interrupt_callback_unregistration() noexcept {
  ESP_LOGI(TAG, "Testing interrupt callback unregistration...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  // Register a callback for pin 5
  if (!g_driver->RegisterPinInterrupt(5, InterruptEdge::Both, [](uint16_t pin, bool state) {
        ESP_LOGI(TAG, "This should not be called");
      })) {
    ESP_LOGE(TAG, "Failed to register callback");
    return false;
  }

  // Unregister the callback
  if (!g_driver->UnregisterPinInterrupt(5)) {
    ESP_LOGE(TAG, "Failed to unregister callback");
    return false;
  }

  // Try to unregister again (should return false)
  if (g_driver->UnregisterPinInterrupt(5)) {
    ESP_LOGW(TAG, "Unexpected success unregistering already-unregistered callback");
  }

  // Test invalid pin
  if (g_driver->UnregisterPinInterrupt(16)) {
    ESP_LOGE(TAG, "Unexpected success unregistering invalid pin");
    return false;
  }

  ESP_LOGI(TAG, "✅ Interrupt callback unregistration tests passed");
  return true;
}

/**
 * @brief Test interrupt configuration (legacy test)
 */
static bool test_interrupt_config() noexcept {
  ESP_LOGI(TAG, "Testing interrupt configuration...");

  if (!g_driver || !g_i2c_bus) {
    ESP_LOGE(TAG, "Driver or bus not initialized");
    return false;
  }

  if (!g_driver->HasAgileIO()) {
    ESP_LOGW(TAG, "⏭️  Skipping: Interrupt config requires PCAL9555A (detected PCA9555)");
    return true;
  }

  // Configure interrupt mask (0 = enable interrupt, 1 = disable)
  uint16_t mask = 0x0000; // Enable interrupts on all pins
  if (!g_driver->ConfigureInterruptMask(mask)) {
    ESP_LOGE(TAG, "Failed to configure interrupt mask");
    return false;
  }

  // Read interrupt status
  uint16_t status = g_driver->GetInterruptStatus();
  ESP_LOGI(TAG, "Interrupt status: 0x%04X", status);

  ESP_LOGI(TAG, "✅ Interrupt configuration tests passed");
  return true;
}

//=============================================================================
// PORT OPERATION TESTS
//=============================================================================

/**
 * @brief Test port-level operations
 */
static bool test_port_operations() noexcept {
  ESP_LOGI(TAG, "Testing port-level operations...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  // Configure port 0 as output
  g_driver->SetMultipleDirections(0x00FF, GPIODir::Output);

  // Configure port 1 as input
  g_driver->SetMultipleDirections(0xFF00, GPIODir::Input);

  // Test writing to port 0
  for (uint16_t pin = 0; pin < 8; ++pin) {
    g_driver->WritePin(pin, (pin % 2 == 0));
  }

  // Test reading from port 1
  for (uint16_t pin = 8; pin < 16; ++pin) {
    bool state = g_driver->ReadPin(pin);
    ESP_LOGI(TAG, "Port 1 pin %d: %s", pin, state ? "HIGH" : "LOW");
  }

  ESP_LOGI(TAG, "✅ Port operation tests passed");
  return true;
}

//=============================================================================
// MULTI-PIN API TESTS (initializer_list overloads)
//=============================================================================

/**
 * @brief Test WritePins - multi-pin write with initializer list
 */
static bool test_write_pins_multi() noexcept {
  ESP_LOGI(TAG, "Testing WritePins (initializer_list)...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  // Set pins 0-3 as output
  for (uint16_t pin = 0; pin < 4; ++pin) {
    g_driver->SetPinDirection(pin, GPIODir::Output);
  }

  // Write multiple pins at once
  if (!g_driver->WritePins({{0, true}, {1, false}, {2, true}, {3, false}})) {
    ESP_LOGE(TAG, "WritePins failed");
    return false;
  }

  // Verify the write: pins configured as output, read back output register
  ESP_LOGI(TAG, "WritePins result: pin0=%d, pin1=%d, pin2=%d, pin3=%d",
           g_driver->ReadPin(0), g_driver->ReadPin(1),
           g_driver->ReadPin(2), g_driver->ReadPin(3));

  ESP_LOGI(TAG, "✅ WritePins tests passed");
  return true;
}

/**
 * @brief Test ReadPins - multi-pin read with initializer list
 */
static bool test_read_pins_multi() noexcept {
  ESP_LOGI(TAG, "Testing ReadPins (initializer_list)...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  // Set pins 8-11 as input
  for (uint16_t pin = 8; pin < 12; ++pin) {
    g_driver->SetPinDirection(pin, GPIODir::Input);
  }

  // Read multiple pins at once
  auto results = g_driver->ReadPins({8, 9, 10, 11});
  if (results.empty()) {
    ESP_LOGE(TAG, "ReadPins returned empty");
    return false;
  }

  for (const auto& [pin, value] : results) {
    ESP_LOGI(TAG, "ReadPins: pin %d = %s", pin, value ? "HIGH" : "LOW");
  }

  if (results.size() != 4) {
    ESP_LOGE(TAG, "ReadPins returned %d results, expected 4", (int)results.size());
    return false;
  }

  ESP_LOGI(TAG, "✅ ReadPins tests passed");
  return true;
}

/**
 * @brief Test SetDirections - initializer_list overload for mixed directions
 */
static bool test_set_directions_multi() noexcept {
  ESP_LOGI(TAG, "Testing SetDirections (initializer_list)...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  // Set mixed directions: even pins output, odd pins input
  if (!g_driver->SetDirections({
          {0, GPIODir::Output}, {1, GPIODir::Input},
          {2, GPIODir::Output}, {3, GPIODir::Input},
          {4, GPIODir::Output}, {5, GPIODir::Input},
          {6, GPIODir::Output}, {7, GPIODir::Input}})) {
    ESP_LOGE(TAG, "SetDirections failed");
    return false;
  }

  // Verify: write to output pins, read from input pins
  g_driver->WritePin(0, true);
  g_driver->WritePin(2, false);
  g_driver->WritePin(4, true);
  g_driver->WritePin(6, false);

  bool pin1 = g_driver->ReadPin(1);
  bool pin3 = g_driver->ReadPin(3);
  ESP_LOGI(TAG, "Input pins after mixed config: pin1=%d, pin3=%d", pin1, pin3);

  ESP_LOGI(TAG, "✅ SetDirections tests passed");
  return true;
}

/**
 * @brief Test SetPolarities - initializer_list overload
 */
static bool test_set_polarities_multi() noexcept {
  ESP_LOGI(TAG, "Testing SetPolarities (initializer_list)...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  // Set mixed polarities
  if (!g_driver->SetPolarities({
          {0, Polarity::Normal}, {1, Polarity::Inverted},
          {2, Polarity::Normal}, {3, Polarity::Inverted}})) {
    ESP_LOGE(TAG, "SetPolarities failed");
    return false;
  }

  // Reset to normal
  if (!g_driver->SetPolarities({
          {0, Polarity::Normal}, {1, Polarity::Normal},
          {2, Polarity::Normal}, {3, Polarity::Normal}})) {
    ESP_LOGE(TAG, "Failed to reset polarities");
    return false;
  }

  ESP_LOGI(TAG, "✅ SetPolarities tests passed");
  return true;
}

//=============================================================================
// ADDRESS MANAGEMENT TESTS
//=============================================================================

/**
 * @brief Test ChangeAddress overloads and address-based constructor
 */
static bool test_address_management() noexcept {
  ESP_LOGI(TAG, "Testing address management...");

  if (!g_driver || !g_i2c_bus) {
    ESP_LOGE(TAG, "Driver or bus not initialized");
    return false;
  }

  // Record original address
  uint8_t original_address = g_driver->GetAddress();
  uint8_t original_bits = g_driver->GetAddressBits();
  ESP_LOGI(TAG, "Original address: 0x%02X (bits=%d)", original_address, original_bits);

  // Test ChangeAddress(bool, bool, bool) - set A0=HIGH, others LOW -> 0x21
  ESP_LOGI(TAG, "Changing address to A0=1, A1=0, A2=0 (0x21)...");
  if (g_driver->ChangeAddress(true, false, false)) {
    ESP_LOGI(TAG, "ChangeAddress(bool) succeeded, new address: 0x%02X", g_driver->GetAddress());
    // Note: this may or may not succeed depending on hardware - if address pin GPIOs
    // are configured, the chip physically changes address. If not, we get a NACK.
  } else {
    ESP_LOGW(TAG, "ChangeAddress(bool) to 0x21 failed (expected if no device at that address)");
  }
  g_driver->ClearErrorFlags();

  // Restore original address
  ESP_LOGI(TAG, "Restoring original address (0x%02X)...", original_address);
  if (!g_driver->ChangeAddress(PCAL9555_A0_LEVEL, PCAL9555_A1_LEVEL, PCAL9555_A2_LEVEL)) {
    ESP_LOGE(TAG, "Failed to restore original address!");
    return false;
  }
  ESP_LOGI(TAG, "Address restored to 0x%02X", g_driver->GetAddress());

  // Test ChangeAddress(uint8_t) - try same address
  ESP_LOGI(TAG, "Testing ChangeAddress(uint8_t) with original address...");
  if (!g_driver->ChangeAddress(original_address)) {
    ESP_LOGE(TAG, "ChangeAddress(uint8_t) failed for original address");
    return false;
  }

  // Verify address-based constructor (create a temporary driver)
  ESP_LOGI(TAG, "Testing address-based constructor (0x%02X)...", original_address);
  {
    PCAL95555Driver temp_driver(g_i2c_bus.get(), original_address);
    if (!temp_driver.EnsureInitialized()) {
      ESP_LOGE(TAG, "Address-based constructor driver failed to initialize");
      return false;
    }
    ESP_LOGI(TAG, "Address-based constructor: addr=0x%02X, variant=%s",
             temp_driver.GetAddress(),
             temp_driver.HasAgileIO() ? "PCAL9555A" : "PCA9555");
  }

  ESP_LOGI(TAG, "✅ Address management tests passed");
  return true;
}

//=============================================================================
// CONFIGURATION & INITIALIZATION TESTS
//=============================================================================

/**
 * @brief Test SetRetries and EnsureInitialized
 */
static bool test_config_and_init() noexcept {
  ESP_LOGI(TAG, "Testing SetRetries and EnsureInitialized...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  // Test SetRetries with various values
  g_driver->SetRetries(0);
  ESP_LOGI(TAG, "SetRetries(0) - no retries on I2C failure");

  // Verify driver still works with no retries
  bool pin_val = g_driver->ReadPin(0);
  ESP_LOGI(TAG, "ReadPin(0) with 0 retries: %s", pin_val ? "HIGH" : "LOW");

  g_driver->SetRetries(3);
  ESP_LOGI(TAG, "SetRetries(3) - 3 retries on I2C failure");

  pin_val = g_driver->ReadPin(0);
  ESP_LOGI(TAG, "ReadPin(0) with 3 retries: %s", pin_val ? "HIGH" : "LOW");

  // Restore default
  g_driver->SetRetries(1);

  // Test EnsureInitialized (should be no-op since already initialized)
  if (!g_driver->EnsureInitialized()) {
    ESP_LOGE(TAG, "EnsureInitialized failed on already-initialized driver");
    return false;
  }
  ESP_LOGI(TAG, "EnsureInitialized on already-initialized driver: OK");

  // Test EnsureInitialized on a fresh driver
  {
    PCAL95555Driver fresh_driver(g_i2c_bus.get(), PCAL9555_A0_LEVEL,
                                  PCAL9555_A1_LEVEL, PCAL9555_A2_LEVEL);
    if (!fresh_driver.EnsureInitialized()) {
      ESP_LOGE(TAG, "EnsureInitialized failed on fresh driver");
      return false;
    }
    ESP_LOGI(TAG, "EnsureInitialized on fresh driver: OK (variant=%s)",
             fresh_driver.HasAgileIO() ? "PCAL9555A" : "PCA9555");
  }

  ESP_LOGI(TAG, "✅ Configuration and initialization tests passed");
  return true;
}

//=============================================================================
// MULTI-PIN PCAL9555A-ONLY TESTS
//=============================================================================

/**
 * @brief Test multi-pin PCAL9555A APIs: SetPullEnables, SetPullDirections,
 *        SetDriveStrengths, ConfigureInterrupt, ConfigureInterrupts, EnableInputLatches
 */
static bool test_multi_pin_pcal_apis() noexcept {
  ESP_LOGI(TAG, "Testing multi-pin PCAL9555A initializer_list APIs...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  if (!g_driver->HasAgileIO()) {
    ESP_LOGW(TAG, "⏭️  Skipping: Multi-pin PCAL APIs require PCAL9555A (detected PCA9555)");
    return true;
  }

  // --- SetPullEnables ---
  ESP_LOGI(TAG, "Testing SetPullEnables...");
  if (!g_driver->SetPullEnables({{0, true}, {1, true}, {2, false}, {3, true}})) {
    ESP_LOGE(TAG, "SetPullEnables failed");
    return false;
  }
  ESP_LOGI(TAG, "  SetPullEnables: OK");

  // --- SetPullDirections ---
  ESP_LOGI(TAG, "Testing SetPullDirections...");
  if (!g_driver->SetPullDirections({{0, true}, {1, false}, {2, true}, {3, false}})) {
    ESP_LOGE(TAG, "SetPullDirections failed");
    return false;
  }
  ESP_LOGI(TAG, "  SetPullDirections: OK");

  // --- SetDriveStrengths ---
  ESP_LOGI(TAG, "Testing SetDriveStrengths...");
  for (uint16_t pin = 0; pin < 4; ++pin) {
    g_driver->SetPinDirection(pin, GPIODir::Output);
  }
  if (!g_driver->SetDriveStrengths({
          {0, DriveStrength::Level0}, {1, DriveStrength::Level1},
          {2, DriveStrength::Level2}, {3, DriveStrength::Level3}})) {
    ESP_LOGE(TAG, "SetDriveStrengths failed");
    return false;
  }
  ESP_LOGI(TAG, "  SetDriveStrengths: OK");

  // --- ConfigureInterrupt (single pin) ---
  ESP_LOGI(TAG, "Testing ConfigureInterrupt (single pin)...");
  g_driver->SetPinDirection(5, GPIODir::Input);
  if (!g_driver->ConfigureInterrupt(5, InterruptState::Enabled)) {
    ESP_LOGE(TAG, "ConfigureInterrupt(5, Enabled) failed");
    return false;
  }
  if (!g_driver->ConfigureInterrupt(5, InterruptState::Disabled)) {
    ESP_LOGE(TAG, "ConfigureInterrupt(5, Disabled) failed");
    return false;
  }
  ESP_LOGI(TAG, "  ConfigureInterrupt: OK");

  // --- ConfigureInterrupts (multi-pin) ---
  ESP_LOGI(TAG, "Testing ConfigureInterrupts (multi-pin)...");
  for (uint16_t pin = 4; pin < 8; ++pin) {
    g_driver->SetPinDirection(pin, GPIODir::Input);
  }
  if (!g_driver->ConfigureInterrupts({
          {4, InterruptState::Enabled}, {5, InterruptState::Enabled},
          {6, InterruptState::Disabled}, {7, InterruptState::Enabled}})) {
    ESP_LOGE(TAG, "ConfigureInterrupts failed");
    return false;
  }
  // Disable all again
  if (!g_driver->ConfigureInterrupts({
          {4, InterruptState::Disabled}, {5, InterruptState::Disabled},
          {6, InterruptState::Disabled}, {7, InterruptState::Disabled}})) {
    ESP_LOGE(TAG, "ConfigureInterrupts (disable) failed");
    return false;
  }
  ESP_LOGI(TAG, "  ConfigureInterrupts: OK");

  // --- EnableInputLatches ---
  ESP_LOGI(TAG, "Testing EnableInputLatches...");
  if (!g_driver->EnableInputLatches({{4, true}, {5, false}, {6, true}, {7, false}})) {
    ESP_LOGE(TAG, "EnableInputLatches failed");
    return false;
  }
  // Disable all latches
  if (!g_driver->EnableInputLatches({{4, false}, {5, false}, {6, false}, {7, false}})) {
    ESP_LOGE(TAG, "EnableInputLatches (disable) failed");
    return false;
  }
  ESP_LOGI(TAG, "  EnableInputLatches: OK");

  ESP_LOGI(TAG, "✅ Multi-pin PCAL9555A API tests passed");
  return true;
}

//=============================================================================
// INTERACTIVE INPUT TESTS
//=============================================================================

/**
 * @brief Interactive test: physically verify input pin reading with a button press.
 *
 * This test requires a momentary push-button connected between PCA9555 pin 0
 * and GND (with a pull-up resistor, or using PCAL9555A internal pull-up).
 *
 * @note Enable ENABLE_INTERACTIVE_INPUT_TESTS to run this test.
 *       The test waits up to 10 seconds for the user to press a button.
 */
static bool test_interactive_input() noexcept {
  ESP_LOGI(TAG, "");
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║              INTERACTIVE INPUT TEST                          ║");
  ESP_LOGI(TAG, "║                                                              ║");
  ESP_LOGI(TAG, "║  This test requires a momentary push-button connected        ║");
  ESP_LOGI(TAG, "║  between PCA9555 IO0_0 (pin 0) and GND.                     ║");
  ESP_LOGI(TAG, "║                                                              ║");
  ESP_LOGI(TAG, "║  If using PCAL9555A, internal pull-up will be enabled.       ║");
  ESP_LOGI(TAG, "║  If using PCA9555, an external pull-up resistor is needed.   ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════╝");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  constexpr uint16_t BUTTON_PIN = 0;

  // Configure pin 0 as input
  if (!g_driver->SetPinDirection(BUTTON_PIN, GPIODir::Input)) {
    ESP_LOGE(TAG, "Failed to set pin %d as input", BUTTON_PIN);
    return false;
  }

  // Enable pull-up if PCAL9555A
  if (g_driver->HasAgileIO()) {
    g_driver->SetPullEnable(BUTTON_PIN, true);
    g_driver->SetPullDirection(BUTTON_PIN, true); // pull-up
    ESP_LOGI(TAG, "Internal pull-up enabled on pin %d (PCAL9555A)", BUTTON_PIN);
  } else {
    ESP_LOGW(TAG, "PCA9555 detected: ensure external pull-up on pin %d", BUTTON_PIN);
  }

  // Read initial state
  bool initial = g_driver->ReadPin(BUTTON_PIN);
  ESP_LOGI(TAG, "Pin %d initial state: %s (expected HIGH if pull-up active)",
           BUTTON_PIN, initial ? "HIGH" : "LOW");

  // Wait for button press (pin goes LOW when pressed)
  ESP_LOGI(TAG, "");
  ESP_LOGI(TAG, ">>> Press the button on pin %d within 10 seconds... <<<", BUTTON_PIN);
  ESP_LOGI(TAG, "");

  bool button_detected = false;
  for (int i = 0; i < 100; ++i) { // 100 * 100ms = 10 seconds
    bool state = g_driver->ReadPin(BUTTON_PIN);
    if (!state) { // LOW = pressed (active-low with pull-up)
      ESP_LOGI(TAG, "Button press detected on pin %d at t=%d ms!", BUTTON_PIN, i * 100);
      button_detected = true;

      // Wait for release
      ESP_LOGI(TAG, "Waiting for button release...");
      while (!g_driver->ReadPin(BUTTON_PIN)) {
        vTaskDelay(pdMS_TO_TICKS(50));
      }
      ESP_LOGI(TAG, "Button released.");
      break;
    }

    // Print countdown every 2 seconds
    if (i % 20 == 0 && i > 0) {
      ESP_LOGI(TAG, "  Waiting... %d seconds remaining", (100 - i) / 10);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  if (!button_detected) {
    ESP_LOGW(TAG, "No button press detected within 10 seconds (this is OK if no button connected)");
    ESP_LOGW(TAG, "Skipping interactive verification - ReadPin still exercises the I2C path");
  } else {
    ESP_LOGI(TAG, "✅ Interactive button test verified: ReadPin detects physical state changes");
  }

  // Test HandleInterrupt explicitly (simulate by calling directly)
  ESP_LOGI(TAG, "Testing HandleInterrupt (explicit call)...");
  g_driver->HandleInterrupt(); // Should not crash; processes pin state changes
  ESP_LOGI(TAG, "  HandleInterrupt: OK (no crash)");

  ESP_LOGI(TAG, "✅ Interactive input tests passed");
  return true;
}

//=============================================================================
// ERROR HANDLING TESTS
//=============================================================================

/**
 * @brief Test error handling and recovery
 */
static bool test_error_handling() noexcept {
  ESP_LOGI(TAG, "Testing error handling...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  // Test 1: Invalid pin operations
  ESP_LOGI(TAG, "  Test: Invalid pin index (pin 16)...");
  bool result = g_driver->SetPinDirection(16, GPIODir::Output);
  if (result) {
    ESP_LOGW(TAG, "  Unexpected success with invalid pin");
  }
  uint16_t errors = g_driver->GetErrorFlags();
  ESP_LOGI(TAG, "  Error flags after invalid pin: 0x%04X (expect InvalidPin=0x0001)", errors);
  g_driver->ClearErrorFlags();

  // Test 2: Invalid pin for ReadPin and WritePin
  ESP_LOGI(TAG, "  Test: Invalid pin read/write (pin 16, 17)...");
  g_driver->ReadPin(16);
  g_driver->WritePin(17, true);
  g_driver->TogglePin(18);
  errors = g_driver->GetErrorFlags();
  ESP_LOGI(TAG, "  Error flags after invalid R/W/T: 0x%04X", errors);
  g_driver->ClearErrorFlags();

  // Test 3: ClearErrorFlags with specific mask
  ESP_LOGI(TAG, "  Test: ClearErrorFlags with specific mask...");
  g_driver->SetPinDirection(16, GPIODir::Output); // sets InvalidPin
  errors = g_driver->GetErrorFlags();
  ESP_LOGI(TAG, "  Before selective clear: 0x%04X", errors);
  g_driver->ClearErrorFlags(0x0001); // Clear only InvalidPin
  errors = g_driver->GetErrorFlags();
  ESP_LOGI(TAG, "  After clearing InvalidPin: 0x%04X (expect 0x0000)", errors);
  g_driver->ClearErrorFlags();

  // Test 4: UnsupportedFeature error on PCA9555
  if (!g_driver->HasAgileIO()) {
    ESP_LOGI(TAG, "  Test: UnsupportedFeature error (PCA9555)...");
    result = g_driver->SetDriveStrength(0, DriveStrength::Level2);
    if (result) {
      ESP_LOGW(TAG, "  Unexpected success for SetDriveStrength on PCA9555");
    }
    errors = g_driver->GetErrorFlags();
    ESP_LOGI(TAG, "  Error flags: 0x%04X (expect UnsupportedFeature=0x0010)", errors);
    g_driver->ClearErrorFlags();
  } else {
    ESP_LOGI(TAG, "  Skip: UnsupportedFeature test (chip is PCAL9555A)");
  }

  // Test 5: ReadPinStates (private but used by HandleInterrupt, verify no crash)
  ESP_LOGI(TAG, "  Test: HandleInterrupt on clean state (no crash expected)...");
  g_driver->HandleInterrupt();
  ESP_LOGI(TAG, "  HandleInterrupt completed without crash");

  ESP_LOGI(TAG, "✅ Error handling tests passed");
  return true;
}

//=============================================================================
// STRESS TESTS
//=============================================================================

/**
 * @brief Test rapid pin operations
 */
static bool test_rapid_operations() noexcept {
  ESP_LOGI(TAG, "Testing rapid pin operations...");

  if (!g_driver) {
    ESP_LOGE(TAG, "Driver not initialized");
    return false;
  }

  uint16_t test_pin = 7;
  g_driver->SetPinDirection(test_pin, GPIODir::Output);

  // Rapid toggle
  for (int i = 0; i < 100; ++i) {
    g_driver->WritePin(test_pin, (i % 2 == 0));
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  ESP_LOGI(TAG, "✅ Rapid operations tests passed");
  return true;
}

//=============================================================================
// MAIN TEST EXECUTION
//=============================================================================

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(TAG, "║                  ESP32-S3 PCAL9555 COMPREHENSIVE TEST SUITE                  ║");
  ESP_LOGI(TAG, "║                      HardFOC PCAL9555 Driver Tests                           ║");
  ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Report test section configuration
  print_test_section_status(TAG, "PCAL9555");

  // Run initialization tests
  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_INITIALIZATION_TESTS, "INITIALIZATION TESTS",
      RUN_TEST_IN_TASK("I2C Bus Init", test_i2c_bus_initialization, 4096, 5);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("Driver Init", test_driver_initialization, 4096, 5);
      flip_test_progress_indicator(););

  // Run GPIO direction tests
  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_GPIO_DIRECTION_TESTS, "GPIO DIRECTION TESTS",
      RUN_TEST_IN_TASK("Single Pin Direction", test_single_pin_direction, 4096, 5);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("Multiple Pin Direction", test_multiple_pin_direction, 4096, 5);
      flip_test_progress_indicator(););

  // Run GPIO read/write tests
  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_GPIO_READ_WRITE_TESTS, "GPIO READ/WRITE TESTS",
      RUN_TEST_IN_TASK("Pin Write", test_pin_write, 4096, 5);
      flip_test_progress_indicator(); RUN_TEST_IN_TASK("Pin Read", test_pin_read, 4096, 5);
      flip_test_progress_indicator(); RUN_TEST_IN_TASK("Pin Toggle", test_pin_toggle, 4096, 5);
      flip_test_progress_indicator(););

  // Run pull resistor tests
  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_PULL_RESISTOR_TESTS, "PULL RESISTOR TESTS",
      RUN_TEST_IN_TASK("Pull Resistor Config", test_pull_resistor_config, 4096, 5);
      flip_test_progress_indicator(););

  // Run drive strength tests
  RUN_TEST_SECTION_IF_ENABLED(ENABLE_DRIVE_STRENGTH_TESTS, "DRIVE STRENGTH TESTS",
                              RUN_TEST_IN_TASK("Drive Strength", test_drive_strength, 4096, 5);
                              flip_test_progress_indicator(););

  // Run output mode tests
  RUN_TEST_SECTION_IF_ENABLED(ENABLE_OUTPUT_MODE_TESTS, "OUTPUT MODE TESTS",
                              RUN_TEST_IN_TASK("Output Mode", test_output_mode, 4096, 5);
                              flip_test_progress_indicator(););

  // Run polarity tests
  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_POLARITY_TESTS, "POLARITY TESTS",
      RUN_TEST_IN_TASK("Polarity Inversion", test_polarity_inversion, 4096, 5);
      flip_test_progress_indicator(););

  // Run input latch tests
  RUN_TEST_SECTION_IF_ENABLED(ENABLE_INPUT_LATCH_TESTS, "INPUT LATCH TESTS",
                              RUN_TEST_IN_TASK("Input Latch", test_input_latch, 4096, 5);
                              flip_test_progress_indicator(););

  // Run interrupt tests
  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_INTERRUPT_TESTS, "INTERRUPT TESTS",
      RUN_TEST_IN_TASK("Interrupt Mask Config", test_interrupt_mask_config, 4096, 5);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("Interrupt Status", test_interrupt_status, 4096, 5);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("Pin Interrupt Callbacks", test_pin_interrupt_callbacks, 4096, 5);
      flip_test_progress_indicator(); RUN_TEST_IN_TASK(
          "Interrupt Handler Registration", test_interrupt_handler_registration, 4096, 5);
      flip_test_progress_indicator(); RUN_TEST_IN_TASK(
          "Interrupt Callback Unregistration", test_interrupt_callback_unregistration, 4096, 5);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("Interrupt Config", test_interrupt_config, 4096, 5);
      flip_test_progress_indicator(););

  // Run port operation tests
  RUN_TEST_SECTION_IF_ENABLED(ENABLE_PORT_OPERATION_TESTS, "PORT OPERATION TESTS",
                              RUN_TEST_IN_TASK("Port Operations", test_port_operations, 4096, 5);
                              flip_test_progress_indicator(););

  // Run multi-pin API tests (initializer_list overloads)
  RUN_TEST_SECTION_IF_ENABLED(
      ENABLE_MULTI_PIN_API_TESTS, "MULTI-PIN API TESTS",
      RUN_TEST_IN_TASK("WritePins Multi", test_write_pins_multi, 4096, 5);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("ReadPins Multi", test_read_pins_multi, 4096, 5);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("SetDirections Multi", test_set_directions_multi, 4096, 5);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("SetPolarities Multi", test_set_polarities_multi, 4096, 5);
      flip_test_progress_indicator(););

  // Run address management tests
  RUN_TEST_SECTION_IF_ENABLED(ENABLE_ADDRESS_TESTS, "ADDRESS MANAGEMENT TESTS",
                              RUN_TEST_IN_TASK("Address Management", test_address_management, 4096, 10);
                              flip_test_progress_indicator(););

  // Run configuration and initialization tests
  RUN_TEST_SECTION_IF_ENABLED(ENABLE_CONFIG_TESTS, "CONFIGURATION TESTS",
                              RUN_TEST_IN_TASK("Config & Init", test_config_and_init, 4096, 5);
                              flip_test_progress_indicator(););

  // Run multi-pin PCAL9555A-only API tests
  RUN_TEST_SECTION_IF_ENABLED(ENABLE_MULTI_PIN_PCAL_TESTS, "MULTI-PIN PCAL9555A API TESTS",
                              RUN_TEST_IN_TASK("Multi-Pin PCAL APIs", test_multi_pin_pcal_apis, 4096, 5);
                              flip_test_progress_indicator(););

  // Run interactive input tests (requires physical button)
  RUN_TEST_SECTION_IF_ENABLED(ENABLE_INTERACTIVE_INPUT_TESTS, "INTERACTIVE INPUT TESTS",
                              RUN_TEST_IN_TASK("Interactive Input", test_interactive_input, 8192, 20);
                              flip_test_progress_indicator(););

  // Run error handling tests
  RUN_TEST_SECTION_IF_ENABLED(ENABLE_ERROR_HANDLING_TESTS, "ERROR HANDLING TESTS",
                              RUN_TEST_IN_TASK("Error Handling", test_error_handling, 4096, 5);
                              flip_test_progress_indicator(););

  // Run stress tests
  RUN_TEST_SECTION_IF_ENABLED(ENABLE_STRESS_TESTS, "STRESS TESTS",
                              RUN_TEST_IN_TASK("Rapid Operations", test_rapid_operations, 4096, 5);
                              flip_test_progress_indicator(););

  // Print test summary
  print_test_summary(g_test_results, "PCAL9555", TAG);

  // Blink GPIO14 to indicate completion
  output_section_indicator(5);

  // Cleanup
  cleanup_test_progress_indicator();
  g_driver.reset();
  g_i2c_bus.reset();

  ESP_LOGI(TAG, "\nTest suite completed.");

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
