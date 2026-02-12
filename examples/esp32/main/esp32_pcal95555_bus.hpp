/**
 * @file esp32_pcal95555_bus.hpp
 * @brief ESP32 I2C bus implementation for PCAL9555 driver
 *
 * This file provides the ESP32-specific implementation of the pcal95555::I2cInterface
 * interface using ESP-IDF's I2C master driver.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "pcal95555.hpp"
#include <array>
#include <cstring>
#include <functional>
#include <memory>

#ifdef __cplusplus
extern "C" {
#endif
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#ifdef __cplusplus
}
#endif

static constexpr const char* g_TAG_I2C = "PCAL9555_I2C";

class Esp32Pcal9555Bus : public pcal95555::I2cInterface<Esp32Pcal9555Bus> {
public:
  /**
   * @brief I2C bus configuration structure
   */
  struct I2CConfig {
    i2c_port_t port = I2C_NUM_0;     ///< I2C port number
    gpio_num_t sda_pin = GPIO_NUM_4; ///< SDA pin (default GPIO4)
    gpio_num_t scl_pin = GPIO_NUM_5; ///< SCL pin (default GPIO5)
    uint32_t frequency = 400000;     ///< I2C frequency in Hz (default 400kHz)
    bool pullup_enable = true;       ///< Enable internal pullups

    // Optional: GPIO pins for controlling A2-A0 address pins
    // Set to GPIO_NUM_NC if not used (default - address pins are hardwired)
    gpio_num_t a0_pin = GPIO_NUM_NC; ///< GPIO pin for A0 address control (optional)
    gpio_num_t a1_pin = GPIO_NUM_NC; ///< GPIO pin for A1 address control (optional)
    gpio_num_t a2_pin = GPIO_NUM_NC; ///< GPIO pin for A2 address control (optional)
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
    // Initialize address pins as outputs if configured
    if (config_.a0_pin != GPIO_NUM_NC || config_.a1_pin != GPIO_NUM_NC ||
        config_.a2_pin != GPIO_NUM_NC) {
      initAddressPins();
    }
  }

  /**
   * @brief Destructor - cleans up I2C resources
   */
  ~Esp32Pcal9555Bus() {
    Deinit();
  }

  /**
   * @brief Ensure the I2C bus is initialized and ready (required by I2cInterface)
   * @return true if successful, false otherwise
   */
  bool EnsureInitialized() noexcept {
    return Init();
  }

  /**
   * @brief Initialize the I2C bus
   * @return true if successful, false otherwise
   */
  bool Init() noexcept {
    if (initialized_) {
      ESP_LOGW(g_TAG_I2C, "I2C bus already initialized");
      return true;
    }

    ESP_LOGI(g_TAG_I2C, "Initializing I2C bus on port %d (SDA:GPIO%d, SCL:GPIO%d, Freq:%lu Hz)",
             config_.port, config_.sda_pin, config_.scl_pin, config_.frequency);

    // Configure I2C master bus
    i2c_master_bus_config_t bus_config = {};
    bus_config.i2c_port = config_.port;
    bus_config.sda_io_num = config_.sda_pin;
    bus_config.scl_io_num = config_.scl_pin;
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.flags.enable_internal_pullup = config_.pullup_enable;
    bus_config.intr_priority = 0;
    bus_config.trans_queue_depth = 0;

    esp_err_t ret = i2c_new_master_bus(&bus_config, &bus_handle_);
    if (ret != ESP_OK) {
      ESP_LOGE(g_TAG_I2C, "Failed to create I2C master bus: %s", esp_err_to_name(ret));
      return false;
    }

    initialized_ = true;
    ESP_LOGI(g_TAG_I2C, "I2C bus initialized successfully");
    return true;
  }

  /**
   * @brief Deinitialize the I2C bus
   */
  void Deinit() noexcept {
    if (!initialized_) {
      return;
    }

    // Remove cached device handle before deleting bus
    if (dev_handle_ != nullptr) {
      i2c_master_bus_rm_device(dev_handle_);
      dev_handle_ = nullptr;
      cached_dev_addr_ = 0xFF;
    }

    if (bus_handle_ != nullptr) {
      i2c_del_master_bus(bus_handle_);
      bus_handle_ = nullptr;
    }

    initialized_ = false;
    ESP_LOGI(g_TAG_I2C, "I2C bus deinitialized");
  }

  /**
   * @brief Write bytes to a device register
   * @param addr 7-bit I2C address of the target device
   * @param reg Register address to write to
   * @param data Pointer to the data buffer containing bytes to send
   * @param len Number of bytes to write from the buffer
   * @return true if the device acknowledges the transfer; false on NACK or error
   */
  bool Write(uint8_t addr, uint8_t reg, const uint8_t* data, size_t len) noexcept {
    if (!initialized_ || bus_handle_ == nullptr) {
      ESP_LOGE(g_TAG_I2C, "I2C bus not initialized");
      return false;
    }

    i2c_master_dev_handle_t dev = getOrCreateDeviceHandle(addr);
    if (dev == nullptr) {
      return false;
    }

    // Prepare write buffer: register address + data
    std::array<uint8_t, 32> write_buffer{}; // Max 32 bytes (register + 31 data bytes)
    if (len > 31) {
      ESP_LOGE(g_TAG_I2C, "Write length %zu exceeds maximum (31 bytes)", len);
      return false;
    }

    write_buffer[0] = reg;
    if (len > 0 && data != nullptr) {
      memcpy(&write_buffer[1], data, len);
    }

    // Perform I2C write transaction
    esp_err_t ret = i2c_master_transmit(dev, write_buffer.data(), len + 1, pdMS_TO_TICKS(1000));
    if (ret != ESP_OK) {
      ESP_LOGE(g_TAG_I2C, "I2C write failed: %s (addr=0x%02X, reg=0x%02X, len=%zu)",
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
  bool Read(uint8_t addr, uint8_t reg, uint8_t* data, size_t len) noexcept {
    if (!initialized_ || bus_handle_ == nullptr) {
      ESP_LOGE(g_TAG_I2C, "I2C bus not initialized");
      return false;
    }

    if (data == nullptr || len == 0) {
      ESP_LOGE(g_TAG_I2C, "Invalid read parameters");
      return false;
    }

    i2c_master_dev_handle_t dev = getOrCreateDeviceHandle(addr);
    if (dev == nullptr) {
      return false;
    }

    // Write register address, then read data
    esp_err_t ret = i2c_master_transmit_receive(dev, &reg, 1, data, len, pdMS_TO_TICKS(1000));
    if (ret != ESP_OK) {
      ESP_LOGE(g_TAG_I2C, "I2C read failed: %s (addr=0x%02X, reg=0x%02X, len=%zu)",
               esp_err_to_name(ret), addr, reg, len);
      return false;
    }

    return true;
  }

  /**
   * @brief Get the I2C configuration
   * @return Reference to the I2C configuration
   */
  [[nodiscard]] const I2CConfig& GetConfig() const noexcept {
    return config_;
  }

  /**
   * @brief Check if the bus is initialized
   * @return true if initialized, false otherwise
   */
  [[nodiscard]] bool IsInitialized() const noexcept {
    return initialized_;
  }

  /**
   * @brief Set address pin levels for controlling A2-A0 address pins
   *
   * This method controls the GPIO pins connected to the PCAL95555 address
   * selection pins (A2, A1, A0). If address pins are not configured in the
   * I2CConfig structure, this method returns false.
   *
   * @param a0_level true to set A0 pin HIGH (VDD), false to set LOW (GND)
   * @param a1_level true to set A1 pin HIGH (VDD), false to set LOW (GND)
   * @param a2_level true to set A2 pin HIGH (VDD), false to set LOW (GND)
   * @return true if GPIO control is supported and pins were set successfully;
   *         false if address pins are not configured or on error
   *
   * @note This requires A0-A2 pins to be configured in I2CConfig.
   *       Most implementations will return false as address pins are typically hardwired.
   *
   * @example
   *   // Configure address pins in I2CConfig
   *   Esp32Pcal9555Bus::I2CConfig config;
   *   config.a0_pin = GPIO_NUM_10;
   *   config.a1_pin = GPIO_NUM_11;
   *   config.a2_pin = GPIO_NUM_12;
   *   auto bus = CreateEsp32Pcal9555Bus(config);
   *
   *   // Set A2=HIGH, A1=LOW, A0=HIGH (address bits = 0b101 = 5)
   *   bus->SetAddressPins(true, false, true);
   */
  bool SetAddressPins(bool a0_level, bool a1_level, bool a2_level) noexcept {
    // Check if address pins are configured
    if (config_.a0_pin == GPIO_NUM_NC && config_.a1_pin == GPIO_NUM_NC &&
        config_.a2_pin == GPIO_NUM_NC) {
      // Address pins not configured - not supported
      return false;
    }

    esp_err_t ret = ESP_OK;

    // Set A0 pin if configured
    if (config_.a0_pin != GPIO_NUM_NC) {
      ret = gpio_set_level(config_.a0_pin, a0_level ? 1 : 0);
      if (ret != ESP_OK) {
        ESP_LOGE(g_TAG_I2C, "Failed to set A0 pin (GPIO%d): %s", config_.a0_pin,
                 esp_err_to_name(ret));
        return false;
      }
    }

    // Set A1 pin if configured
    if (config_.a1_pin != GPIO_NUM_NC) {
      ret = gpio_set_level(config_.a1_pin, a1_level ? 1 : 0);
      if (ret != ESP_OK) {
        ESP_LOGE(g_TAG_I2C, "Failed to set A1 pin (GPIO%d): %s", config_.a1_pin,
                 esp_err_to_name(ret));
        return false;
      }
    }

    // Set A2 pin if configured
    if (config_.a2_pin != GPIO_NUM_NC) {
      ret = gpio_set_level(config_.a2_pin, a2_level ? 1 : 0);
      if (ret != ESP_OK) {
        ESP_LOGE(g_TAG_I2C, "Failed to set A2 pin (GPIO%d): %s", config_.a2_pin,
                 esp_err_to_name(ret));
        return false;
      }
    }

    ESP_LOGI(g_TAG_I2C, "Address pins set: A2=%d, A1=%d, A0=%d", a2_level, a1_level, a0_level);
    // Allow device to settle after address pins change before first I2C transaction
    vTaskDelay(pdMS_TO_TICKS(5));
    return true;
  }

  /**
   * @brief Register interrupt handler for PCAL95555 INT pin
   *
   * This method implements the I2cInterface::RegisterInterruptHandler() method.
   * It configures the ESP32 GPIO pin connected to the PCAL95555 INT pin and sets
   * up an interrupt handler that will call the provided handler function when an
   * interrupt occurs.
   *
   * @param handler Function to call when INT pin interrupt occurs
   * @return true if setup successful, false otherwise
   *
   * @note The INT pin must be specified via SetupInterruptPin() first.
   * @note The INT pin is open-drain and requires an external pull-up resistor.
   *       The interrupt is configured for falling edge (active low).
   *
   * @example
   *   auto bus = CreateEsp32Pcal9555Bus();
   *   bus->SetupInterruptPin(GPIO_NUM_7);  // Configure INT pin
   *   auto driver = std::make_unique<PCAL95555Driver>(bus.get(), false, false, false);
   *   driver->RegisterInterruptHandler();  // Registers HandleInterrupt() with bus
   */
  bool RegisterInterruptHandler(std::function<void()> handler) noexcept {
    if (!handler) {
      ESP_LOGE(g_TAG_I2C, "Interrupt handler is null");
      return false;
    }

    // Check if INT pin is configured
    if (interrupt_pin_ == GPIO_NUM_NC) {
      ESP_LOGW(g_TAG_I2C, "INT pin not configured. Call SetupInterruptPin() first.");
      return false;
    }

    // Configure GPIO pin for interrupt if not already configured
    if (interrupt_queue_ == nullptr) {
      gpio_config_t io_conf = {};
      io_conf.intr_type = GPIO_INTR_NEGEDGE; // Falling edge (active low)
      io_conf.pin_bit_mask = (1ULL << interrupt_pin_);
      io_conf.mode = GPIO_MODE_INPUT;
      io_conf.pull_up_en = GPIO_PULLUP_ENABLE; // Enable pull-up (INT is open-drain)
      io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

      esp_err_t ret = gpio_config(&io_conf);
      if (ret != ESP_OK) {
        ESP_LOGE(g_TAG_I2C, "Failed to configure GPIO %d for interrupt: %s", interrupt_pin_,
                 esp_err_to_name(ret));
        return false;
      }

      // Create interrupt queue
      interrupt_queue_ = xQueueCreate(10, sizeof(uint32_t));
      if (interrupt_queue_ == nullptr) {
        ESP_LOGE(g_TAG_I2C, "Failed to create interrupt queue");
        return false;
      }

      // Install GPIO ISR service if not already installed
      static bool isr_service_installed = false;
      if (!isr_service_installed) {
        ret = gpio_install_isr_service(0);
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
          ESP_LOGE(g_TAG_I2C, "Failed to install GPIO ISR service: %s", esp_err_to_name(ret));
          return false;
        }
        isr_service_installed = true;
      }

      // Hook ISR handler for this pin
      ret = gpio_isr_handler_add(interrupt_pin_, interruptHandler, this);
      if (ret != ESP_OK) {
        ESP_LOGE(g_TAG_I2C, "Failed to add ISR handler for GPIO %d: %s", interrupt_pin_,
                 esp_err_to_name(ret));
        return false;
      }

      // Create task to process interrupts
      if (interruptTask_handle_ == nullptr) {
        xTaskCreate(interruptTask, "pcal9555_int", 4096, this, 5, &interruptTask_handle_);
        if (interruptTask_handle_ == nullptr) {
          ESP_LOGE(g_TAG_I2C, "Failed to create interrupt task");
          return false;
        }
      }
    }

    // Store handler for interrupt processing
    interrupt_callback_ = handler;

    ESP_LOGI(g_TAG_I2C, "Interrupt handler registered on GPIO %d", interrupt_pin_);
    return true;
  }

  /**
   * @brief Setup interrupt pin GPIO configuration
   *
   * This method configures the GPIO pin that will be used for the INT pin.
   * Must be called before RegisterInterruptHandler().
   *
   * @param int_pin GPIO pin number connected to PCAL95555 INT pin
   * @return true if setup successful, false otherwise
   */
  bool SetupInterruptPin(gpio_num_t int_pin) noexcept {
    interrupt_pin_ = int_pin;
    ESP_LOGI(g_TAG_I2C, "Interrupt pin configured: GPIO %d", int_pin);
    return true;
  }

  /**
   * @brief Legacy method - use RegisterInterruptHandler() instead
   * @deprecated Use SetupInterruptPin() + RegisterInterruptHandler() instead
   */
  [[deprecated("Use SetupInterruptPin() + RegisterInterruptHandler() instead")]]
  bool SetupInterrupt(gpio_num_t int_pin, std::function<void()> interrupt_callback) noexcept {
    if (!interrupt_callback) {
      ESP_LOGE(g_TAG_I2C, "Interrupt callback is null");
      return false;
    }

    // Configure GPIO pin for interrupt
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE; // Falling edge (active low)
    io_conf.pin_bit_mask = (1ULL << int_pin);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE; // Enable pull-up (INT is open-drain)
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
      ESP_LOGE(g_TAG_I2C, "Failed to configure GPIO %d for interrupt: %s", int_pin,
               esp_err_to_name(ret));
      return false;
    }

    // Create interrupt queue if not exists
    if (interrupt_queue_ == nullptr) {
      interrupt_queue_ = xQueueCreate(10, sizeof(uint32_t));
      if (interrupt_queue_ == nullptr) {
        ESP_LOGE(g_TAG_I2C, "Failed to create interrupt queue");
        return false;
      }
    }

    // Store callback for interrupt handler
    interrupt_callback_ = interrupt_callback;
    interrupt_pin_ = int_pin;

    // Install GPIO ISR service if not already installed
    static bool isr_service_installed = false;
    if (!isr_service_installed) {
      ret = gpio_install_isr_service(0);
      if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(g_TAG_I2C, "Failed to install GPIO ISR service: %s", esp_err_to_name(ret));
        return false;
      }
      isr_service_installed = true;
    }

    // Hook ISR handler for this pin
    ret = gpio_isr_handler_add(int_pin, interruptHandler, this);
    if (ret != ESP_OK) {
      ESP_LOGE(g_TAG_I2C, "Failed to add ISR handler for GPIO %d: %s", int_pin, esp_err_to_name(ret));
      return false;
    }

    // Create task to process interrupts
    if (interruptTask_handle_ == nullptr) {
      xTaskCreate(interruptTask, "pcal9555_int", 4096, this, 5, &interruptTask_handle_);
      if (interruptTask_handle_ == nullptr) {
        ESP_LOGE(g_TAG_I2C, "Failed to create interrupt task");
        return false;
      }
    }

    ESP_LOGI(g_TAG_I2C, "Interrupt setup complete on GPIO %d", int_pin);
    return true;
  }

  /**
   * @brief Remove interrupt handler for the INT pin
   */
  void RemoveInterrupt() noexcept {
    if (interrupt_pin_ != GPIO_NUM_NC) {
      gpio_isr_handler_remove(interrupt_pin_);
      interrupt_pin_ = GPIO_NUM_NC;
    }
    interrupt_callback_ = nullptr;
    if (interruptTask_handle_ != nullptr) {
      vTaskDelete(interruptTask_handle_);
      interruptTask_handle_ = nullptr;
    }
  }

private:
  I2CConfig config_;
  i2c_master_bus_handle_t bus_handle_;
  bool initialized_;

  // Cached device handle -- avoids add_device/rm_device per transaction
  i2c_master_dev_handle_t dev_handle_{nullptr};
  uint8_t cached_dev_addr_{0xFF};

  /**
   * @brief Get or create a cached I2C device handle for the given address.
   *
   * If the cached handle matches the requested address, it is reused.
   * Otherwise the old handle is removed and a new one is created.
   *
   * @param addr 7-bit I2C device address
   * @return Device handle, or nullptr on failure
   */
  i2c_master_dev_handle_t getOrCreateDeviceHandle(uint8_t addr) noexcept {
    if (dev_handle_ != nullptr && cached_dev_addr_ == addr) {
      return dev_handle_;  // Reuse cached handle
    }

    // Address changed or first call -- evict old handle
    if (dev_handle_ != nullptr) {
      i2c_master_bus_rm_device(dev_handle_);
      dev_handle_ = nullptr;
    }

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = config_.frequency,
        .scl_wait_us = 0,
        .flags = {},
    };

    esp_err_t ret = i2c_master_bus_add_device(bus_handle_, &dev_config, &dev_handle_);
    if (ret != ESP_OK) {
      ESP_LOGE(g_TAG_I2C, "Failed to add device 0x%02X: %s", addr, esp_err_to_name(ret));
      dev_handle_ = nullptr;
      cached_dev_addr_ = 0xFF;
      return nullptr;
    }

    cached_dev_addr_ = addr;
    return dev_handle_;
  }

  // Interrupt handling members
  gpio_num_t interrupt_pin_ = GPIO_NUM_NC;
  std::function<void()> interrupt_callback_;
  QueueHandle_t interrupt_queue_ = nullptr;
  TaskHandle_t interruptTask_handle_ = nullptr;

  /**
   * @brief Static interrupt handler (ISR context)
   */
  static void IRAM_ATTR interruptHandler(void* arg) {
    auto* bus = static_cast<Esp32Pcal9555Bus*>(arg);
    auto pin = static_cast<uint32_t>(bus->interrupt_pin_);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(bus->interrupt_queue_, &pin, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
      portYIELD_FROM_ISR();
    }
  }

  /**
   * @brief Interrupt processing task (task context)
   */
  static void interruptTask(void* arg) {
    auto* bus = static_cast<Esp32Pcal9555Bus*>(arg);
    uint32_t pin = 0;

    while (true) {
      if (xQueueReceive(bus->interrupt_queue_, &pin, portMAX_DELAY)) {
        if (bus->interrupt_callback_) {
          bus->interrupt_callback_();
        }
      }
    }
  }

  /**
   * @brief Initialize address pins as outputs
   */
  void initAddressPins() noexcept {
    gpio_config_t io_conf = {};
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    uint64_t pin_mask = 0;
    if (config_.a0_pin != GPIO_NUM_NC) {
      pin_mask |= (1ULL << config_.a0_pin);
    }
    if (config_.a1_pin != GPIO_NUM_NC) {
      pin_mask |= (1ULL << config_.a1_pin);
    }
    if (config_.a2_pin != GPIO_NUM_NC) {
      pin_mask |= (1ULL << config_.a2_pin);
    }

    if (pin_mask != 0) {
      io_conf.pin_bit_mask = pin_mask;
      esp_err_t ret = gpio_config(&io_conf);
      if (ret != ESP_OK) {
        ESP_LOGE(g_TAG_I2C, "Failed to configure address pins: %s", esp_err_to_name(ret));
      } else {
        ESP_LOGI(g_TAG_I2C, "Address pins configured: A0=GPIO%d, A1=GPIO%d, A2=GPIO%d",
                 config_.a0_pin, config_.a1_pin, config_.a2_pin);
      }
    }
  }
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
    ESP_LOGE(g_TAG_I2C, "Failed to initialize I2C bus");
    return nullptr;
  }
  return bus;
}
