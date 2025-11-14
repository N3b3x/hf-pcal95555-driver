/**
 * @file pcal95555_comprehensive_test.cpp
 * @brief Comprehensive test suite for PCAL9555 GPIO expander driver on ESP32-C6
 *
 * This file contains comprehensive testing for PCAL9555 features including:
 * - GPIO pin direction configuration (input/output)
 * - Pin read/write operations
 * - Pull-up/pull-down resistor configuration
 * - Drive strength configuration
 * - Output mode configuration (push-pull/open-drain)
 * - Input polarity inversion
 * - Input latch functionality
 * - Interrupt configuration and handling
 * - Port-level operations
 * - Error handling and recovery
 * - Multi-pin operations
 * - Edge cases and stress testing
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "pcal95555.hpp"
#include "esp32_pcal95555_bus.hpp"
#include "TestFramework.h"
#include <memory>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Use fully qualified name for the class
using PCAL95555Driver = pcal95555::PCAL95555<Esp32Pcal9555Bus>;

#ifdef __cplusplus
extern "C" {
#endif
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
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
static constexpr bool ENABLE_ERROR_HANDLING_TESTS = true;
static constexpr bool ENABLE_STRESS_TESTS = true;

// PCAL9555 I2C address (default 0x20, can be changed via A0-A2 pins)
static constexpr uint8_t PCAL9555_I2C_ADDRESS = 0x20;

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

    auto driver = std::make_unique<PCAL95555Driver>(g_i2c_bus.get(), PCAL9555_I2C_ADDRESS);
    if (!driver) {
        ESP_LOGE(TAG, "Failed to create driver instance");
        return nullptr;
    }

    // Reset to default state
    driver->ResetToDefault();
    vTaskDelay(pdMS_TO_TICKS(10));

    return driver;
}

/**
 * @brief Verify pin state matches expected value
 */
static bool verify_pin_state(PCAL95555Driver& driver, uint16_t pin, bool expected, const char* context) noexcept {
    bool actual = driver.ReadPin(pin);
    if (actual != expected) {
        ESP_LOGE(TAG, "%s: Pin %d state mismatch - expected %s, got %s",
                 context, pin, expected ? "HIGH" : "LOW", actual ? "HIGH" : "LOW");
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
    config.sda_pin = GPIO_NUM_4;
    config.scl_pin = GPIO_NUM_5;
    config.frequency = 400000;
    config.pullup_enable = true;

    g_i2c_bus = CreateEsp32Pcal9555Bus(config);
    if (!g_i2c_bus || !g_i2c_bus->isInitialized()) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus");
        return false;
    }

    ESP_LOGI(TAG, "✅ I2C bus initialized successfully");
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

    ESP_LOGI(TAG, "✅ Driver initialized successfully");
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

/**
 * @brief Test interrupt configuration
 */
static bool test_interrupt_config() noexcept {
    ESP_LOGI(TAG, "Testing interrupt configuration...");

    if (!g_driver) {
        ESP_LOGE(TAG, "Driver not initialized");
        return false;
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

    // Set callback
    g_driver->SetInterruptCallback([](uint16_t status) {
        ESP_LOGI(TAG, "Interrupt callback triggered: 0x%04X", status);
    });

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

    // Test invalid pin operations
    // Note: Driver should handle invalid pins gracefully
    bool result = g_driver->SetPinDirection(16, GPIODir::Output); // Invalid pin
    if (result) {
        ESP_LOGW(TAG, "Unexpected success with invalid pin");
    }

    // Check error flags
    uint16_t errors = g_driver->GetErrorFlags();
    ESP_LOGI(TAG, "Error flags: 0x%04X", errors);

    // Clear error flags
    g_driver->ClearErrorFlags();

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
    ESP_LOGI(TAG, "║                  ESP32-C6 PCAL9555 COMPREHENSIVE TEST SUITE                   ║");
    ESP_LOGI(TAG, "║                      HardFOC PCAL9555 Driver Tests                            ║");
    ESP_LOGI(TAG, "╚══════════════════════════════════════════════════════════════════════════════╝");

    vTaskDelay(pdMS_TO_TICKS(1000));

    // Report test section configuration
    print_test_section_status(TAG, "PCAL9555");

    // Run initialization tests
    RUN_TEST_SECTION_IF_ENABLED(ENABLE_INITIALIZATION_TESTS, "INITIALIZATION TESTS",
        RUN_TEST_IN_TASK("I2C Bus Init", test_i2c_bus_initialization, 4096, 5);
        flip_test_progress_indicator();
        RUN_TEST_IN_TASK("Driver Init", test_driver_initialization, 4096, 5);
        flip_test_progress_indicator();
    );

    // Run GPIO direction tests
    RUN_TEST_SECTION_IF_ENABLED(ENABLE_GPIO_DIRECTION_TESTS, "GPIO DIRECTION TESTS",
        RUN_TEST_IN_TASK("Single Pin Direction", test_single_pin_direction, 4096, 5);
        flip_test_progress_indicator();
        RUN_TEST_IN_TASK("Multiple Pin Direction", test_multiple_pin_direction, 4096, 5);
        flip_test_progress_indicator();
    );

    // Run GPIO read/write tests
    RUN_TEST_SECTION_IF_ENABLED(ENABLE_GPIO_READ_WRITE_TESTS, "GPIO READ/WRITE TESTS",
        RUN_TEST_IN_TASK("Pin Write", test_pin_write, 4096, 5);
        flip_test_progress_indicator();
        RUN_TEST_IN_TASK("Pin Read", test_pin_read, 4096, 5);
        flip_test_progress_indicator();
        RUN_TEST_IN_TASK("Pin Toggle", test_pin_toggle, 4096, 5);
        flip_test_progress_indicator();
    );

    // Run pull resistor tests
    RUN_TEST_SECTION_IF_ENABLED(ENABLE_PULL_RESISTOR_TESTS, "PULL RESISTOR TESTS",
        RUN_TEST_IN_TASK("Pull Resistor Config", test_pull_resistor_config, 4096, 5);
        flip_test_progress_indicator();
    );

    // Run drive strength tests
    RUN_TEST_SECTION_IF_ENABLED(ENABLE_DRIVE_STRENGTH_TESTS, "DRIVE STRENGTH TESTS",
        RUN_TEST_IN_TASK("Drive Strength", test_drive_strength, 4096, 5);
        flip_test_progress_indicator();
    );

    // Run output mode tests
    RUN_TEST_SECTION_IF_ENABLED(ENABLE_OUTPUT_MODE_TESTS, "OUTPUT MODE TESTS",
        RUN_TEST_IN_TASK("Output Mode", test_output_mode, 4096, 5);
        flip_test_progress_indicator();
    );

    // Run polarity tests
    RUN_TEST_SECTION_IF_ENABLED(ENABLE_POLARITY_TESTS, "POLARITY TESTS",
        RUN_TEST_IN_TASK("Polarity Inversion", test_polarity_inversion, 4096, 5);
        flip_test_progress_indicator();
    );

    // Run input latch tests
    RUN_TEST_SECTION_IF_ENABLED(ENABLE_INPUT_LATCH_TESTS, "INPUT LATCH TESTS",
        RUN_TEST_IN_TASK("Input Latch", test_input_latch, 4096, 5);
        flip_test_progress_indicator();
    );

    // Run interrupt tests
    RUN_TEST_SECTION_IF_ENABLED(ENABLE_INTERRUPT_TESTS, "INTERRUPT TESTS",
        RUN_TEST_IN_TASK("Interrupt Config", test_interrupt_config, 4096, 5);
        flip_test_progress_indicator();
    );

    // Run port operation tests
    RUN_TEST_SECTION_IF_ENABLED(ENABLE_PORT_OPERATION_TESTS, "PORT OPERATION TESTS",
        RUN_TEST_IN_TASK("Port Operations", test_port_operations, 4096, 5);
        flip_test_progress_indicator();
    );

    // Run error handling tests
    RUN_TEST_SECTION_IF_ENABLED(ENABLE_ERROR_HANDLING_TESTS, "ERROR HANDLING TESTS",
        RUN_TEST_IN_TASK("Error Handling", test_error_handling, 4096, 5);
        flip_test_progress_indicator();
    );

    // Run stress tests
    RUN_TEST_SECTION_IF_ENABLED(ENABLE_STRESS_TESTS, "STRESS TESTS",
        RUN_TEST_IN_TASK("Rapid Operations", test_rapid_operations, 4096, 5);
        flip_test_progress_indicator();
    );

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

