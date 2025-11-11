/**
 * @file PCAL95555.hpp
 * @brief Driver for the PCAL9555 16-bit I/O expander with I²C interface
 *
 * This header provides a comprehensive C++ interface for controlling the
 * PCAL9555/PCAL95555AHF 16-bit I/O expander. The driver handles all
 * register-level operations required to configure and operate the device
 * including:
 *   - Setting I/O directions (input/output)
 *   - Reading and writing pin states
 *   - Configuring pull-up/pull-down resistors
 *   - Managing interrupt functionality
 *   - Setting drive strength and output modes
 *
 * The implementation uses an abstract I2C interface that must be provided by
 * the user to accommodate different hardware platforms and I2C libraries.
 *
 * @author Nebiyu Tadesse
 * @date 2025-05-21
 * @version 1.0
 *
 * @note This driver supports the PCAL9555/PCAL95555AHF device with 16 GPIO pins
 *       divided into two 8-bit ports (PORT_0 and PORT_1).
 */
#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>

#ifndef CONFIG_PCAL95555_INIT_FROM_KCONFIG
#define CONFIG_PCAL95555_INIT_FROM_KCONFIG 1
#endif
#ifndef CONFIG_PCAL95555_PORT0_OD
#define CONFIG_PCAL95555_PORT0_OD 0
#endif
#ifndef CONFIG_PCAL95555_PORT1_OD
#define CONFIG_PCAL95555_PORT1_OD 0
#endif

/* Per-pin defaults */
#ifndef CONFIG_PCAL95555_P0_0_DIR
#define CONFIG_PCAL95555_P0_0_DIR 1
#endif
#ifndef CONFIG_PCAL95555_P0_1_DIR
#define CONFIG_PCAL95555_P0_1_DIR 1
#endif
#ifndef CONFIG_PCAL95555_P0_2_DIR
#define CONFIG_PCAL95555_P0_2_DIR 1
#endif
#ifndef CONFIG_PCAL95555_P0_3_DIR
#define CONFIG_PCAL95555_P0_3_DIR 1
#endif
#ifndef CONFIG_PCAL95555_P0_4_DIR
#define CONFIG_PCAL95555_P0_4_DIR 1
#endif
#ifndef CONFIG_PCAL95555_P0_5_DIR
#define CONFIG_PCAL95555_P0_5_DIR 1
#endif
#ifndef CONFIG_PCAL95555_P0_6_DIR
#define CONFIG_PCAL95555_P0_6_DIR 1
#endif
#ifndef CONFIG_PCAL95555_P0_7_DIR
#define CONFIG_PCAL95555_P0_7_DIR 1
#endif
#ifndef CONFIG_PCAL95555_P1_0_DIR
#define CONFIG_PCAL95555_P1_0_DIR 1
#endif
#ifndef CONFIG_PCAL95555_P1_1_DIR
#define CONFIG_PCAL95555_P1_1_DIR 1
#endif
#ifndef CONFIG_PCAL95555_P1_2_DIR
#define CONFIG_PCAL95555_P1_2_DIR 1
#endif
#ifndef CONFIG_PCAL95555_P1_3_DIR
#define CONFIG_PCAL95555_P1_3_DIR 1
#endif
#ifndef CONFIG_PCAL95555_P1_4_DIR
#define CONFIG_PCAL95555_P1_4_DIR 1
#endif
#ifndef CONFIG_PCAL95555_P1_5_DIR
#define CONFIG_PCAL95555_P1_5_DIR 1
#endif
#ifndef CONFIG_PCAL95555_P1_6_DIR
#define CONFIG_PCAL95555_P1_6_DIR 1
#endif
#ifndef CONFIG_PCAL95555_P1_7_DIR
#define CONFIG_PCAL95555_P1_7_DIR 1
#endif

#ifndef CONFIG_PCAL95555_P0_0_PULL
#define CONFIG_PCAL95555_P0_0_PULL 0
#endif
#ifndef CONFIG_PCAL95555_P0_1_PULL
#define CONFIG_PCAL95555_P0_1_PULL 0
#endif
#ifndef CONFIG_PCAL95555_P0_2_PULL
#define CONFIG_PCAL95555_P0_2_PULL 0
#endif
#ifndef CONFIG_PCAL95555_P0_3_PULL
#define CONFIG_PCAL95555_P0_3_PULL 0
#endif
#ifndef CONFIG_PCAL95555_P0_4_PULL
#define CONFIG_PCAL95555_P0_4_PULL 0
#endif
#ifndef CONFIG_PCAL95555_P0_5_PULL
#define CONFIG_PCAL95555_P0_5_PULL 0
#endif
#ifndef CONFIG_PCAL95555_P0_6_PULL
#define CONFIG_PCAL95555_P0_6_PULL 0
#endif
#ifndef CONFIG_PCAL95555_P0_7_PULL
#define CONFIG_PCAL95555_P0_7_PULL 0
#endif
#ifndef CONFIG_PCAL95555_P1_0_PULL
#define CONFIG_PCAL95555_P1_0_PULL 0
#endif
#ifndef CONFIG_PCAL95555_P1_1_PULL
#define CONFIG_PCAL95555_P1_1_PULL 0
#endif
#ifndef CONFIG_PCAL95555_P1_2_PULL
#define CONFIG_PCAL95555_P1_2_PULL 0
#endif
#ifndef CONFIG_PCAL95555_P1_3_PULL
#define CONFIG_PCAL95555_P1_3_PULL 0
#endif
#ifndef CONFIG_PCAL95555_P1_4_PULL
#define CONFIG_PCAL95555_P1_4_PULL 0
#endif
#ifndef CONFIG_PCAL95555_P1_5_PULL
#define CONFIG_PCAL95555_P1_5_PULL 0
#endif
#ifndef CONFIG_PCAL95555_P1_6_PULL
#define CONFIG_PCAL95555_P1_6_PULL 0
#endif
#ifndef CONFIG_PCAL95555_P1_7_PULL
#define CONFIG_PCAL95555_P1_7_PULL 0
#endif

#ifndef CONFIG_PCAL95555_P0_0_PULL_UP
#define CONFIG_PCAL95555_P0_0_PULL_UP 1
#endif
#ifndef CONFIG_PCAL95555_P0_1_PULL_UP
#define CONFIG_PCAL95555_P0_1_PULL_UP 1
#endif
#ifndef CONFIG_PCAL95555_P0_2_PULL_UP
#define CONFIG_PCAL95555_P0_2_PULL_UP 1
#endif
#ifndef CONFIG_PCAL95555_P0_3_PULL_UP
#define CONFIG_PCAL95555_P0_3_PULL_UP 1
#endif
#ifndef CONFIG_PCAL95555_P0_4_PULL_UP
#define CONFIG_PCAL95555_P0_4_PULL_UP 1
#endif
#ifndef CONFIG_PCAL95555_P0_5_PULL_UP
#define CONFIG_PCAL95555_P0_5_PULL_UP 1
#endif
#ifndef CONFIG_PCAL95555_P0_6_PULL_UP
#define CONFIG_PCAL95555_P0_6_PULL_UP 1
#endif
#ifndef CONFIG_PCAL95555_P0_7_PULL_UP
#define CONFIG_PCAL95555_P0_7_PULL_UP 1
#endif
#ifndef CONFIG_PCAL95555_P1_0_PULL_UP
#define CONFIG_PCAL95555_P1_0_PULL_UP 1
#endif
#ifndef CONFIG_PCAL95555_P1_1_PULL_UP
#define CONFIG_PCAL95555_P1_1_PULL_UP 1
#endif
#ifndef CONFIG_PCAL95555_P1_2_PULL_UP
#define CONFIG_PCAL95555_P1_2_PULL_UP 1
#endif
#ifndef CONFIG_PCAL95555_P1_3_PULL_UP
#define CONFIG_PCAL95555_P1_3_PULL_UP 1
#endif
#ifndef CONFIG_PCAL95555_P1_4_PULL_UP
#define CONFIG_PCAL95555_P1_4_PULL_UP 1
#endif
#ifndef CONFIG_PCAL95555_P1_5_PULL_UP
#define CONFIG_PCAL95555_P1_5_PULL_UP 1
#endif
#ifndef CONFIG_PCAL95555_P1_6_PULL_UP
#define CONFIG_PCAL95555_P1_6_PULL_UP 1
#endif
#ifndef CONFIG_PCAL95555_P1_7_PULL_UP
#define CONFIG_PCAL95555_P1_7_PULL_UP 1
#endif

#ifndef CONFIG_PCAL95555_P0_0_OUTPUT
#define CONFIG_PCAL95555_P0_0_OUTPUT 0
#endif
#ifndef CONFIG_PCAL95555_P0_1_OUTPUT
#define CONFIG_PCAL95555_P0_1_OUTPUT 0
#endif
#ifndef CONFIG_PCAL95555_P0_2_OUTPUT
#define CONFIG_PCAL95555_P0_2_OUTPUT 0
#endif
#ifndef CONFIG_PCAL95555_P0_3_OUTPUT
#define CONFIG_PCAL95555_P0_3_OUTPUT 0
#endif
#ifndef CONFIG_PCAL95555_P0_4_OUTPUT
#define CONFIG_PCAL95555_P0_4_OUTPUT 0
#endif
#ifndef CONFIG_PCAL95555_P0_5_OUTPUT
#define CONFIG_PCAL95555_P0_5_OUTPUT 0
#endif
#ifndef CONFIG_PCAL95555_P0_6_OUTPUT
#define CONFIG_PCAL95555_P0_6_OUTPUT 0
#endif
#ifndef CONFIG_PCAL95555_P0_7_OUTPUT
#define CONFIG_PCAL95555_P0_7_OUTPUT 0
#endif
#ifndef CONFIG_PCAL95555_P1_0_OUTPUT
#define CONFIG_PCAL95555_P1_0_OUTPUT 0
#endif
#ifndef CONFIG_PCAL95555_P1_1_OUTPUT
#define CONFIG_PCAL95555_P1_1_OUTPUT 0
#endif
#ifndef CONFIG_PCAL95555_P1_2_OUTPUT
#define CONFIG_PCAL95555_P1_2_OUTPUT 0
#endif
#ifndef CONFIG_PCAL95555_P1_3_OUTPUT
#define CONFIG_PCAL95555_P1_3_OUTPUT 0
#endif
#ifndef CONFIG_PCAL95555_P1_4_OUTPUT
#define CONFIG_PCAL95555_P1_4_OUTPUT 0
#endif
#ifndef CONFIG_PCAL95555_P1_5_OUTPUT
#define CONFIG_PCAL95555_P1_5_OUTPUT 0
#endif
#ifndef CONFIG_PCAL95555_P1_6_OUTPUT
#define CONFIG_PCAL95555_P1_6_OUTPUT 0
#endif
#ifndef CONFIG_PCAL95555_P1_7_OUTPUT
#define CONFIG_PCAL95555_P1_7_OUTPUT 0
#endif

/* Derived port-level masks from per-pin options */
#ifndef CONFIG_PCAL95555_PORT0_DIRECTION
#define CONFIG_PCAL95555_PORT0_DIRECTION                                                           \
  ((CONFIG_PCAL95555_P0_0_DIR << 0) | (CONFIG_PCAL95555_P0_1_DIR << 1) |                           \
   (CONFIG_PCAL95555_P0_2_DIR << 2) | (CONFIG_PCAL95555_P0_3_DIR << 3) |                           \
   (CONFIG_PCAL95555_P0_4_DIR << 4) | (CONFIG_PCAL95555_P0_5_DIR << 5) |                           \
   (CONFIG_PCAL95555_P0_6_DIR << 6) | (CONFIG_PCAL95555_P0_7_DIR << 7))
#endif
#ifndef CONFIG_PCAL95555_PORT1_DIRECTION
#define CONFIG_PCAL95555_PORT1_DIRECTION                                                           \
  ((CONFIG_PCAL95555_P1_0_DIR << 0) | (CONFIG_PCAL95555_P1_1_DIR << 1) |                           \
   (CONFIG_PCAL95555_P1_2_DIR << 2) | (CONFIG_PCAL95555_P1_3_DIR << 3) |                           \
   (CONFIG_PCAL95555_P1_4_DIR << 4) | (CONFIG_PCAL95555_P1_5_DIR << 5) |                           \
   (CONFIG_PCAL95555_P1_6_DIR << 6) | (CONFIG_PCAL95555_P1_7_DIR << 7))
#endif
#ifndef CONFIG_PCAL95555_PORT0_PULL_ENABLE
#define CONFIG_PCAL95555_PORT0_PULL_ENABLE                                                         \
  ((CONFIG_PCAL95555_P0_0_PULL << 0) | (CONFIG_PCAL95555_P0_1_PULL << 1) |                         \
   (CONFIG_PCAL95555_P0_2_PULL << 2) | (CONFIG_PCAL95555_P0_3_PULL << 3) |                         \
   (CONFIG_PCAL95555_P0_4_PULL << 4) | (CONFIG_PCAL95555_P0_5_PULL << 5) |                         \
   (CONFIG_PCAL95555_P0_6_PULL << 6) | (CONFIG_PCAL95555_P0_7_PULL << 7))
#endif
#ifndef CONFIG_PCAL95555_PORT1_PULL_ENABLE
#define CONFIG_PCAL95555_PORT1_PULL_ENABLE                                                         \
  ((CONFIG_PCAL95555_P1_0_PULL << 0) | (CONFIG_PCAL95555_P1_1_PULL << 1) |                         \
   (CONFIG_PCAL95555_P1_2_PULL << 2) | (CONFIG_PCAL95555_P1_3_PULL << 3) |                         \
   (CONFIG_PCAL95555_P1_4_PULL << 4) | (CONFIG_PCAL95555_P1_5_PULL << 5) |                         \
   (CONFIG_PCAL95555_P1_6_PULL << 6) | (CONFIG_PCAL95555_P1_7_PULL << 7))
#endif
#ifndef CONFIG_PCAL95555_PORT0_PULL_UP
#define CONFIG_PCAL95555_PORT0_PULL_UP                                                             \
  ((CONFIG_PCAL95555_P0_0_PULL_UP << 0) | (CONFIG_PCAL95555_P0_1_PULL_UP << 1) |                   \
   (CONFIG_PCAL95555_P0_2_PULL_UP << 2) | (CONFIG_PCAL95555_P0_3_PULL_UP << 3) |                   \
   (CONFIG_PCAL95555_P0_4_PULL_UP << 4) | (CONFIG_PCAL95555_P0_5_PULL_UP << 5) |                   \
   (CONFIG_PCAL95555_P0_6_PULL_UP << 6) | (CONFIG_PCAL95555_P0_7_PULL_UP << 7))
#endif
#ifndef CONFIG_PCAL95555_PORT1_PULL_UP
#define CONFIG_PCAL95555_PORT1_PULL_UP                                                             \
  ((CONFIG_PCAL95555_P1_0_PULL_UP << 0) | (CONFIG_PCAL95555_P1_1_PULL_UP << 1) |                   \
   (CONFIG_PCAL95555_P1_2_PULL_UP << 2) | (CONFIG_PCAL95555_P1_3_PULL_UP << 3) |                   \
   (CONFIG_PCAL95555_P1_4_PULL_UP << 4) | (CONFIG_PCAL95555_P1_5_PULL_UP << 5) |                   \
   (CONFIG_PCAL95555_P1_6_PULL_UP << 6) | (CONFIG_PCAL95555_P1_7_PULL_UP << 7))
#endif
#ifndef CONFIG_PCAL95555_PORT0_OUTPUT
#define CONFIG_PCAL95555_PORT0_OUTPUT                                                              \
  ((CONFIG_PCAL95555_P0_0_OUTPUT << 0) | (CONFIG_PCAL95555_P0_1_OUTPUT << 1) |                     \
   (CONFIG_PCAL95555_P0_2_OUTPUT << 2) | (CONFIG_PCAL95555_P0_3_OUTPUT << 3) |                     \
   (CONFIG_PCAL95555_P0_4_OUTPUT << 4) | (CONFIG_PCAL95555_P0_5_OUTPUT << 5) |                     \
   (CONFIG_PCAL95555_P0_6_OUTPUT << 6) | (CONFIG_PCAL95555_P0_7_OUTPUT << 7))
#endif
#ifndef CONFIG_PCAL95555_PORT1_OUTPUT
#define CONFIG_PCAL95555_PORT1_OUTPUT                                                              \
  ((CONFIG_PCAL95555_P1_0_OUTPUT << 0) | (CONFIG_PCAL95555_P1_1_OUTPUT << 1) |                     \
   (CONFIG_PCAL95555_P1_2_OUTPUT << 2) | (CONFIG_PCAL95555_P1_3_OUTPUT << 3) |                     \
   (CONFIG_PCAL95555_P1_4_OUTPUT << 4) | (CONFIG_PCAL95555_P1_5_OUTPUT << 5) |                     \
   (CONFIG_PCAL95555_P1_6_OUTPUT << 6) | (CONFIG_PCAL95555_P1_7_OUTPUT << 7))
#endif

#ifndef CONFIG_PCAL95555_INIT_DIRECTION
#define CONFIG_PCAL95555_INIT_DIRECTION                                                            \
  ((CONFIG_PCAL95555_PORT1_DIRECTION << 8) | CONFIG_PCAL95555_PORT0_DIRECTION)
#endif
#ifndef CONFIG_PCAL95555_INIT_PULL_ENABLE
#define CONFIG_PCAL95555_INIT_PULL_ENABLE                                                          \
  ((CONFIG_PCAL95555_PORT1_PULL_ENABLE << 8) | CONFIG_PCAL95555_PORT0_PULL_ENABLE)
#endif
#ifndef CONFIG_PCAL95555_INIT_PULL_UP
#define CONFIG_PCAL95555_INIT_PULL_UP                                                              \
  ((CONFIG_PCAL95555_PORT1_PULL_UP << 8) | CONFIG_PCAL95555_PORT0_PULL_UP)
#endif
#ifndef CONFIG_PCAL95555_INIT_OUTPUT
#define CONFIG_PCAL95555_INIT_OUTPUT                                                               \
  ((CONFIG_PCAL95555_PORT1_OUTPUT << 8) | CONFIG_PCAL95555_PORT0_OUTPUT)
#endif
#ifndef CONFIG_PCAL95555_INIT_OD_PORT0
#define CONFIG_PCAL95555_INIT_OD_PORT0 CONFIG_PCAL95555_PORT0_OD
#endif
#ifndef CONFIG_PCAL95555_INIT_OD_PORT1
#define CONFIG_PCAL95555_INIT_OD_PORT1 CONFIG_PCAL95555_PORT1_OD
#endif

/** PCAL95555 register map (all control registers). */
/**
 * @namespace PCAL95555_REG
 * @brief Contains register addresses for the PCAL9555 I/O expander chip
 *
 * The PCAL9555 is a 16-bit I/O expander with I²C interface. This namespace
 * defines the memory-mapped register addresses used to control and interact
 * with the device.
 *
 * Register groups:
 * - Input/Output registers (0x00-0x03): Read inputs and control outputs
 * - Configuration registers (0x04-0x07): Configure polarity and port direction
 * - Drive strength registers (0x40-0x43): Control output drive strength
 * - Input/Pull registers (0x44-0x49): Configure input latching and pull-up/down
 * resistors
 * - Interrupt registers (0x4A-0x4D): Manage interrupt functionality
 * - Output configuration (0x4F): General output configuration
 *
 * Each register controls 8 pins, with PORT_0 typically handling pins 0-7 and
 * PORT_1 handling pins 8-15.
 */
namespace PCAL95555_REG {
enum : uint8_t {
  INPUT_PORT_0 = 0x00,
  INPUT_PORT_1 = 0x01,
  OUTPUT_PORT_0 = 0x02,
  OUTPUT_PORT_1 = 0x03,
  POLARITY_INV_0 = 0x04,
  POLARITY_INV_1 = 0x05,
  CONFIG_PORT_0 = 0x06,
  CONFIG_PORT_1 = 0x07,
  DRIVE_STRENGTH_0 = 0x40,
  DRIVE_STRENGTH_1 = 0x41,
  DRIVE_STRENGTH_2 = 0x42,
  DRIVE_STRENGTH_3 = 0x43,
  INPUT_LATCH_0 = 0x44,
  INPUT_LATCH_1 = 0x45,
  PULL_ENABLE_0 = 0x46,
  PULL_ENABLE_1 = 0x47,
  PULL_SELECT_0 = 0x48,
  PULL_SELECT_1 = 0x49,
  INT_MASK_0 = 0x4A,
  INT_MASK_1 = 0x4B,
  INT_STATUS_0 = 0x4C,
  INT_STATUS_1 = 0x4D,
  OUTPUT_CONF = 0x4F
};
}

/** GPIO direction (1=input, 0=output). */
enum class GPIODir : uint8_t { Input = 1, Output = 0 };

/** Input polarity inversion (0=normal, 1=inverted). */
enum class Polarity : uint8_t { Normal = 0, Inverted = 1 };

/** Output drive strength levels (00=¼, 11=full). */
enum class DriveStrength : uint8_t { Level0 = 0, Level1 = 1, Level2 = 2, Level3 = 3 };

/** Output mode (push-pull vs open-drain). */
enum class OutputMode : uint8_t { PushPull = 0, OpenDrain = 1 };

/** Error conditions reported by the driver. */
enum class Error : uint16_t {
  None = 0,
  InvalidPin = 1 << 0,  ///< Provided pin index out of range
  InvalidMask = 1 << 1, ///< Mask contained bits outside 0-15
  I2CReadFail = 1 << 2, ///< An I2C read operation failed
  I2CWriteFail = 1 << 3 ///< An I2C write operation failed
};

inline Error operator|(Error a, Error b) {
  return static_cast<Error>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}
inline Error operator&(Error a, Error b) {
  return static_cast<Error>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
}

/**
 * @class PCAL95555
 * @brief Driver for the PCAL95555AHF / PCAL9555A I²C GPIO expander.
 */
class PCAL95555 {
public:
  /**
   * @brief Abstract interface for I2C bus operations.
   *
   * @details Users must implement this interface to provide low-level I2C
   * communication for the PCAL95555 driver. Provides basic read and write
   * methods with ACK/NACK feedback.
   */
  class i2cBus {
  public:
    virtual ~i2cBus() = default;
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
    virtual bool write(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) = 0;
    /**
     * @brief Read bytes from a device register.
     *
     * @param addr 7-bit I2C address of the target device.
     * @param reg Register address to read from.
     * @param data Pointer to the buffer to store received data.
     * @param len Number of bytes to read into the buffer.
     * @return true if the read succeeds; false on NACK or error.
     */
    virtual bool read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) = 0;
  };

  /**
   * @brief Construct a new PCAL95555 driver instance.
   *
   * @param bus Pointer to a user-implemented I2C bus interface.
   * @param address 7-bit I2C address of the PCAL95555 device (0x00 to 0x7F).
   */
  PCAL95555(PCAL95555::i2cBus *bus, uint8_t address);

  /**
   * @brief Configure retry mechanism for I2C transactions.
   *
   * @param retries Maximum number of retry attempts on failure.
   *                Note: setting N will result in N+1 total attempts per
   * transfer.
   */
  void setRetries(int retries);

  /**
   * @brief Retrieve currently latched error flags.
   *
   * @return Bitmask composed of @ref Error values.
   */
  uint16_t getErrorFlags() const;

  /**
   * @brief Clear specific error flags.
   *
   * @param mask Bitmask of errors to clear (default: all).
   */
  void clearErrorFlags(uint16_t mask = 0xFFFF);

  /**
   * @brief Reset all registers to their power-on default state.
   *
   * @details Available registers are set to inputs, pull-ups enabled,
   * interrupts masked, drive strength set to full, and output mode
   * configured as push-pull as specified by the datasheet.
   */
  void resetToDefault();

  /**
   * @brief Initialize the device using values from Kconfig.
   *
   * @details This writes the configuration registers using the
   * CONFIG_PCAL95555_INIT_* options defined at compile time.
   */
  void initFromConfig();

  /**
   * @brief Set the direction of a single GPIO pin.
   *
   * @param pin Zero-based pin index (0-15).
   * @param dir Direction enum: Input or Output.
   * @return true on success; false on I2C failure.
   */
  bool setPinDirection(uint16_t pin, GPIODir dir);

  /**
   * @brief Set the direction for multiple GPIO pins at once.
   *
   * @param mask Bitmask where each set bit indicates a pin to configure.
   * @param dir Common direction for all selected pins.
   * @return true on success; false if any I2C operation fails.
   */
  bool setMultipleDirections(uint16_t mask, GPIODir dir);

  /**
   * @brief Read the current logical level of a GPIO pin.
   *
   * @param pin Zero-based pin index (0-15).
   * @return true if pin is high; false if pin is low or on read error.
   */
  bool readPin(uint16_t pin);

  /**
   * @brief Write a logical level to a GPIO output pin.
   *
   * @param pin Zero-based pin index (0-15).
   * @param value true to drive high, false to drive low.
   * @return true if write succeeded; false on I2C failure.
   */
  bool writePin(uint16_t pin, bool value);

  /**
   * @brief Toggle the output state of a GPIO pin.
   *
   * @param pin Zero-based pin index (0-15).
   * @return true on success; false on I2C failure.
   */
  bool togglePin(uint16_t pin);

  /**
   * @brief Enable or disable the pull-up/pull-down resistor on a pin.
   *
   * @param pin Zero-based pin index (0-15).
   * @param enable true to enable; false to disable.
   * @return true on success; false on I2C failure.
   */
  bool setPullEnable(uint16_t pin, bool enable);
  /**
   * @brief Select internal pull-up or pull-down resistor direction.
   *
   * @param pin Zero-based pin index (0-15).
   * @param pullUp true for pull-up; false for pull-down.
   * @return true on success; false on I2C failure.
   */
  bool setPullDirection(uint16_t pin, bool pullUp);

  /**
   * @brief Configure the output drive strength for a GPIO pin.
   *
   * @param pin Zero-based pin index (0-15).
   * @param level Drive strength level (Level0..Level3).
   * @return true on success; false on I2C failure.
   */
  bool setDriveStrength(uint16_t pin, DriveStrength level);

  /**
   * @brief Enable or disable interrupts on multiple pins.
   *
   * @param mask Bitmask where 0 enables interrupt, 1 disables per pin.
   * @return true on success; false on I2C failure.
   */
  bool configureInterruptMask(uint16_t mask);
  /**
   * @brief Retrieve and clear the interrupt status.
   *
   * @return 16-bit mask indicating which pins triggered an interrupt.
   */
  uint16_t getInterruptStatus();

  /**
   * @brief Configure per-port output mode (push-pull or open-drain).
   *
   * @param port0OpenDrain true for open-drain on port 0.
   * @param port1OpenDrain true for open-drain on port 1.
   * @return true on success; false on I2C failure.
   */
  bool setOutputMode(bool port0OpenDrain, bool port1OpenDrain);

  /**
   * @brief Configure input polarity inversion for a single pin.
   *
   * @param pin Zero-based pin index (0-15).
   * @param polarity Polarity::Normal or Polarity::Inverted.
   * @return true on success; false on I2C failure.
   */
  bool setPinPolarity(uint16_t pin, Polarity polarity);

  /**
   * @brief Configure input polarity for multiple pins.
   *
   * @param mask Bitmask selecting pins to configure.
   * @param polarity Normal or Inverted for all selected pins.
   * @return true on success; false on any I2C failure.
   */
  bool setMultiplePolarities(uint16_t mask, Polarity polarity);

  /**
   * @brief Enable or disable the input latch for a single pin.
   *
   * @param pin Zero-based pin index (0-15).
   * @param enable true to enable latch; false to disable.
   * @return true on success; false on I2C failure.
   */
  bool enableInputLatch(uint16_t pin, bool enable);

  /**
   * @brief Enable or disable input latch for multiple pins.
   *
   * @param mask Bitmask selecting pins.
   * @param enable true to enable; false to disable.
   * @return true on success; false on any I2C failure.
   */
  bool enableMultipleInputLatches(uint16_t mask, bool enable);

  /**
   * @brief Register a callback to be invoked on GPIO interrupts.
   *
   * @param cb Function taking a 16-bit status mask for active interrupts.
   */
  void setInterruptCallback(std::function<void(uint16_t)> cb);
  /**
   * @brief Internal handler to process an interrupt event.
   *
   * @details Reads the interrupt status register and invokes the
   * previously registered callback if set.
   */
  void handleInterrupt();

protected:
  /**
   * @brief Read a device register with retry logic.
   *
   * @param reg Register address to read.
   * @param value Reference to store the read byte.
   * @return true if read succeeds; false on failure.
   */
  bool readRegister(uint8_t reg, uint8_t &value);
  /**
   * @brief Write a single byte to a device register with retry logic.
   *
   * @param reg Register address to write.
   * @param value Byte value to write.
   * @return true if write succeeds; false on failure.
   */
  bool writeRegister(uint8_t reg, uint8_t value);

  i2cBus *i2c_;
  uint8_t devAddr_;
  int retries_{1};
  uint16_t errorFlags_{0};
  std::function<void(uint16_t)> irqCallback_;

  void setError(Error e);
  void clearError(Error e);
};
