/**
 * @file pcal95555_i2c_interface.hpp
 * @brief CRTP-based I2C interface for PCAL95555 driver
 *
 * This header defines the hardware-agnostic I2C communication interface
 * using the CRTP (Curiously Recurring Template Pattern) for compile-time
 * polymorphism. Platform-specific implementations should inherit from this
 * template with themselves as the template parameter.
 *
 * @author Nebiyu Tadesse
 * @date 2025-05-21
 * @version 1.0
 */
#ifndef PCAL95555_I2C_INTERFACE_HPP
#define PCAL95555_I2C_INTERFACE_HPP

#include <cstddef>
#include <cstdint>
#include <functional>

namespace pcal95555 {

/**
 * @brief CRTP-based template interface for I2C bus operations
 *
 * This template class provides a hardware-agnostic interface for I2C communication
 * using the CRTP pattern. Platform-specific implementations should inherit from
 * this template with themselves as the template parameter.
 *
 * Benefits of CRTP:
 * - Compile-time polymorphism (no virtual function overhead)
 * - Static dispatch instead of dynamic dispatch
 * - Better optimization opportunities for the compiler
 *
 * Example usage:
 * @code
 * class MyI2C : public pcal95555::I2cInterface<MyI2C> {
 * public:
 *   bool Write(...) { ... }
 *   bool Read(...) { ... }
 * };
 * @endcode
 *
 * @tparam Derived The derived class type (CRTP pattern)
 */
template <typename Derived>
class I2cInterface {
public:
  /**
   * @brief Write bytes to a device register.
   *
   * @param addr 7-bit I2C address of the target device.
   * @param reg Register address to write to.
   * @param data Pointer to the data buffer containing bytes to send.
   * @param len Number of bytes to write from the buffer.
   * @return true if the device acknowledges the transfer; false on NACK or
   * error.
   */
  bool Write(uint8_t addr, uint8_t reg, const uint8_t* data, size_t len) noexcept {
    return static_cast<Derived*>(this)->Write(addr, reg, data, len);
  }

  /**
   * @brief Read bytes from a device register.
   *
   * @param addr 7-bit I2C address of the target device.
   * @param reg Register address to read from.
   * @param data Pointer to the buffer to store received data.
   * @param len Number of bytes to read into the buffer.
   * @return true if the read succeeds; false on NACK or error.
   */
  bool Read(uint8_t addr, uint8_t reg, uint8_t* data, size_t len) noexcept {
    return static_cast<Derived*>(this)->Read(addr, reg, data, len);
  }

  /**
   * @brief Set address pin levels (for controlling address pins A2-A0).
   *
   * This method allows the I2C interface implementation to control GPIO pins
   * that are connected to the PCAL95555 address selection pins (A2, A1, A0).
   * The default implementation returns false, indicating that GPIO control is
   * not supported. Derived classes should override this method if they support
   * dynamic address pin control.
   *
   * @param a0_level true to set A0 pin HIGH (VDD), false to set LOW (GND)
   * @param a1_level true to set A1 pin HIGH (VDD), false to set LOW (GND)
   * @param a2_level true to set A2 pin HIGH (VDD), false to set LOW (GND)
   * @return true if GPIO control is supported and pins were set successfully;
   *         false if not supported or on error.
   *
   * @note This is an optional feature. Most implementations will return false
   *       as address pins are typically hardwired. Only override if your
   *       hardware allows software control of A2-A0 pins.
   *
   * @example
   *   // Set A2=HIGH, A1=LOW, A0=HIGH (address bits = 0b101 = 5)
   *   bool success = i2c->SetAddressPins(true, false, true);
   */
  bool SetAddressPins(bool a0_level, bool a1_level, bool a2_level) noexcept {
    // Default implementation: not supported
    (void)a0_level;  // Suppress unused parameter warning
    (void)a1_level;
    (void)a2_level;
    return false;
  }

  /**
   * @brief Ensure the I2C bus is initialized and ready for communication.
   *
   * This method performs lazy initialization of the I2C bus. It should initialize
   * the I2C hardware, configure pins, set up the bus, and verify it's ready for
   * communication. On subsequent calls, it should return true immediately if
   * already initialized.
   *
   * @return true if initialization succeeded or was already initialized;
   *         false if initialization failed.
   *
   * @note This is a required method. All I2C interface implementations must
   *       provide this method to ensure the bus is ready before communication.
   *
   * @example
   *   bool EnsureInitialized() {
   *       if (initialized_) {
   *           return true;
   *       }
   *       // Initialize I2C hardware, configure pins, etc.
   *       initialized_ = true;
   *       return true;
   *   }
   */
  bool EnsureInitialized() noexcept {
    return static_cast<Derived*>(this)->EnsureInitialized();
  }

  /**
   * @brief Register an interrupt handler to be called when INT pin fires.
   *
   * This method allows the PCAL95555 driver to register its interrupt handler
   * with the I2C interface. When the INT pin (open-drain interrupt output from
   * PCAL95555) fires, the I2C interface should call this handler.
   *
   * The handler will typically read the interrupt status registers and invoke
   * user-registered callbacks for pins that triggered interrupts.
   *
   * @param handler Function to call when INT pin interrupt occurs.
   *                The handler should be called from interrupt context or a
   *                dedicated interrupt task.
   * @return true if interrupt handler registration is supported and successful;
   *         false if not supported or on error.
   *
   * @note This is an optional feature. The default implementation does nothing.
   *       Platform-specific implementations should override this to set up GPIO
   *       interrupt handling for the INT pin.
   *
   * @note The INT pin is open-drain and requires an external pull-up resistor.
   *       The interrupt is active-low (falling edge).
   *
   * @example
   *   // In PCAL95555 class:
   *   i2c_->RegisterInterruptHandler([this]() {
   *       HandleInterrupt();  // Reads status and calls user callbacks
   *   });
   */
  bool RegisterInterruptHandler(std::function<void()> handler) noexcept {
    // Default implementation: not supported
    (void)handler;  // Suppress unused parameter warning
    return false;
  }

protected:
  /**
   * @brief Protected constructor to prevent direct instantiation
   */
  I2cInterface() = default;

  // Prevent copying
  I2cInterface(const I2cInterface&) = delete;
  I2cInterface& operator=(const I2cInterface&) = delete;

  // Allow moving
  I2cInterface(I2cInterface&&) = default;
  I2cInterface& operator=(I2cInterface&&) = default;

  /**
   * @brief Protected destructor
   * @note Derived classes can have public destructors
   */
  ~I2cInterface() = default;
};

} // namespace pcal95555

#endif // PCAL95555_I2C_INTERFACE_HPP

