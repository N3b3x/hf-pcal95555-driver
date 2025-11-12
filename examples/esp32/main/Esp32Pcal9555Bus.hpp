/**
 * @file Esp32Pcal9555Bus.hpp
 * @brief ESP32 I2C bus implementation for PCAL9555 driver
 * 
 * This file provides the ESP32-specific implementation of the PCAL95555::i2cBus
 * interface using ESP-IDF's I2C master driver.
 * 
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "pcal95555.hpp"
#include <memory>
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#ifdef __cplusplus
}
#endif

static const char* TAG_I2C = "PCAL9555_I2C";

/**
 * @class Esp32Pcal9555Bus
 * @brief ESP32 implementation of PCAL95555::i2cBus interface
 * 
 * This class provides I2C communication for the PCAL9555 driver using
 * ESP-IDF's I2C master driver API.
 */
class Esp32Pcal9555Bus : public PCAL95555::i2cBus {
public:
    /**
     * @brief I2C bus configuration structure
     */
    struct I2CConfig {
        i2c_port_t port = I2C_NUM_0;           ///< I2C port number
        gpio_num_t sda_pin = GPIO_NUM_4;       ///< SDA pin (default GPIO4)
        gpio_num_t scl_pin = GPIO_NUM_5;       ///< SCL pin (default GPIO5)
        uint32_t frequency = 400000;           ///< I2C frequency in Hz (default 400kHz)
        bool pullup_enable = true;             ///< Enable internal pullups
    };

    /**
     * @brief Constructor with default configuration
     */
    Esp32Pcal9555Bus() : Esp32Pcal9555Bus(I2CConfig{}) {}

    /**
     * @brief Constructor with custom I2C configuration
     * @param config I2C bus configuration
     */
    explicit Esp32Pcal9555Bus(const I2CConfig& config) 
        : config_(config), bus_handle_(nullptr), initialized_(false) {
    }

    /**
     * @brief Destructor - cleans up I2C resources
     */
    ~Esp32Pcal9555Bus() override {
        Deinit();
    }

    /**
     * @brief Initialize the I2C bus
     * @return true if successful, false otherwise
     */
    bool Init() noexcept {
        if (initialized_) {
            ESP_LOGW(TAG_I2C, "I2C bus already initialized");
            return true;
        }

        ESP_LOGI(TAG_I2C, "Initializing I2C bus on port %d (SDA:GPIO%d, SCL:GPIO%d, Freq:%lu Hz)",
                 config_.port, config_.sda_pin, config_.scl_pin, config_.frequency);

        // Configure I2C master bus
        i2c_master_bus_config_t bus_config = {
            .i2c_port = config_.port,
            .sda_io_num = config_.sda_pin,
            .scl_io_num = config_.scl_pin,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .flags = {
                .enable_internal_pullup = config_.pullup_enable,
            },
        };

        esp_err_t ret = i2c_new_master_bus(&bus_config, &bus_handle_);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG_I2C, "Failed to create I2C master bus: %s", esp_err_to_name(ret));
            return false;
        }

        initialized_ = true;
        ESP_LOGI(TAG_I2C, "I2C bus initialized successfully");
        return true;
    }

    /**
     * @brief Deinitialize the I2C bus
     */
    void Deinit() noexcept {
        if (!initialized_) {
            return;
        }

        if (bus_handle_ != nullptr) {
            i2c_del_master_bus(bus_handle_);
            bus_handle_ = nullptr;
        }

        initialized_ = false;
        ESP_LOGI(TAG_I2C, "I2C bus deinitialized");
    }

    /**
     * @brief Write bytes to a device register
     * @param addr 7-bit I2C address of the target device
     * @param reg Register address to write to
     * @param data Pointer to the data buffer containing bytes to send
     * @param len Number of bytes to write from the buffer
     * @return true if the device acknowledges the transfer; false on NACK or error
     */
    bool write(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) override {
        if (!initialized_ || bus_handle_ == nullptr) {
            ESP_LOGE(TAG_I2C, "I2C bus not initialized");
            return false;
        }

        // Create device handle if not exists for this address
        i2c_master_dev_handle_t dev_handle = nullptr;
        i2c_device_config_t dev_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = addr,
            .scl_speed_hz = config_.frequency,
        };

        esp_err_t ret = i2c_master_bus_add_device(bus_handle_, &dev_config, &dev_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG_I2C, "Failed to add device 0x%02X: %s", addr, esp_err_to_name(ret));
            return false;
        }

        // Prepare write buffer: register address + data
        uint8_t write_buffer[32]; // Max 32 bytes (register + 31 data bytes)
        if (len > 31) {
            ESP_LOGE(TAG_I2C, "Write length %zu exceeds maximum (31 bytes)", len);
            return false;
        }

        write_buffer[0] = reg;
        if (len > 0 && data != nullptr) {
            memcpy(&write_buffer[1], data, len);
        }

        // Perform I2C write transaction
        ret = i2c_master_transmit(dev_handle, write_buffer, len + 1, pdMS_TO_TICKS(1000));
        
        // Remove device handle (cleanup)
        if (dev_handle != nullptr) {
            i2c_master_bus_rm_device(dev_handle);
        }

        if (ret != ESP_OK) {
            ESP_LOGE(TAG_I2C, "I2C write failed: %s (addr=0x%02X, reg=0x%02X, len=%zu)",
                     esp_err_to_name(ret), addr, reg, len);
            return false;
        }

        return true;
    }

    /**
     * @brief Read bytes from a device register
     * @param addr 7-bit I2C address of the target device
     * @param reg Register address to read from
     * @param data Pointer to the buffer to store received data
     * @param len Number of bytes to read into the buffer
     * @return true if the read succeeds; false on NACK or error
     */
    bool read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) override {
        if (!initialized_ || bus_handle_ == nullptr) {
            ESP_LOGE(TAG_I2C, "I2C bus not initialized");
            return false;
        }

        if (data == nullptr || len == 0) {
            ESP_LOGE(TAG_I2C, "Invalid read parameters");
            return false;
        }

        // Create device handle if not exists for this address
        i2c_master_dev_handle_t dev_handle = nullptr;
        i2c_device_config_t dev_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = addr,
            .scl_speed_hz = config_.frequency,
        };

        esp_err_t ret = i2c_master_bus_add_device(bus_handle_, &dev_config, &dev_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG_I2C, "Failed to add device 0x%02X: %s", addr, esp_err_to_name(ret));
            return false;
        }

        // Write register address, then read data
        ret = i2c_master_transmit_receive(dev_handle, &reg, 1, data, len, pdMS_TO_TICKS(1000));

        // Remove device handle (cleanup)
        if (dev_handle != nullptr) {
            i2c_master_bus_rm_device(dev_handle);
        }

        if (ret != ESP_OK) {
            ESP_LOGE(TAG_I2C, "I2C read failed: %s (addr=0x%02X, reg=0x%02X, len=%zu)",
                     esp_err_to_name(ret), addr, reg, len);
            return false;
        }

        return true;
    }

    /**
     * @brief Get the I2C configuration
     * @return Reference to the I2C configuration
     */
    const I2CConfig& getConfig() const noexcept {
        return config_;
    }

    /**
     * @brief Check if the bus is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const noexcept {
        return initialized_;
    }

private:
    I2CConfig config_;
    i2c_master_bus_handle_t bus_handle_;
    bool initialized_;
};

/**
 * @brief Factory function to create an ESP32 I2C bus instance
 * @param config I2C configuration (optional, uses defaults if not provided)
 * @return Unique pointer to Esp32Pcal9555Bus instance
 */
inline std::unique_ptr<Esp32Pcal9555Bus> CreateEsp32Pcal9555Bus(
    const Esp32Pcal9555Bus::I2CConfig& config = Esp32Pcal9555Bus::I2CConfig{}) {
    auto bus = std::make_unique<Esp32Pcal9555Bus>(config);
    if (!bus->Init()) {
        ESP_LOGE(TAG_I2C, "Failed to initialize I2C bus");
        return nullptr;
    }
    return bus;
}

