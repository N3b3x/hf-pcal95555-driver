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
 *   bool write(...) { ... }
 *   bool read(...) { ... }
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
  bool write(uint8_t addr, uint8_t reg, const uint8_t* data, size_t len) {
    return static_cast<Derived*>(this)->write(addr, reg, data, len);
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
  bool read(uint8_t addr, uint8_t reg, uint8_t* data, size_t len) {
    return static_cast<Derived*>(this)->read(addr, reg, data, len);
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

