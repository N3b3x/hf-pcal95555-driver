#ifndef PCAL95555_IMPL
#define PCAL95555_IMPL

// When included from header, use relative path; when compiled directly, use standard include
#ifdef PCAL95555_HEADER_INCLUDED
#include "../inc/pcal95555.hpp"
#else
#include "../inc/pcal95555.hpp"
#endif

// Constructor: store I2C bus and device address (7-bit)
template <typename I2cType>
PCAL95555<I2cType>::PCAL95555(I2cType* bus, uint8_t address) : i2c_(bus), devAddr_(address) {}

// Configure retry count
template <typename I2cType>
void PCAL95555<I2cType>::setRetries(int r) {
  retries_ = r;
}

// Low-level write with retries
template <typename I2cType>
bool PCAL95555<I2cType>::writeRegister(uint8_t reg, uint8_t value) {
  for (int attempt = 0; attempt <= retries_; ++attempt) {
    if (i2c_->write(devAddr_, reg, &value, 1)) {
      clearError(Error::I2CWriteFail);
      return true;
    }
  }
  setError(Error::I2CWriteFail);
  return false;
}
// Low-level read with retries
template <typename I2cType>
bool PCAL95555<I2cType>::readRegister(uint8_t reg, uint8_t& value) {
  for (int attempt = 0; attempt <= retries_; ++attempt) {
    if (i2c_->read(devAddr_, reg, &value, 1)) {
      clearError(Error::I2CReadFail);
      return true;
    }
  }
  setError(Error::I2CReadFail);
  return false;
}

// Reset all registers to defaults as per datasheet.
template <typename I2cType>
void PCAL95555<I2cType>::resetToDefault() {
  // All defaults taken from datasheet tables.
  writeRegister(PCAL95555_REG::OUTPUT_PORT_0, 0xFF);
  writeRegister(PCAL95555_REG::OUTPUT_PORT_1, 0xFF);
  writeRegister(PCAL95555_REG::POLARITY_INV_0, 0x00);
  writeRegister(PCAL95555_REG::POLARITY_INV_1, 0x00);
  writeRegister(PCAL95555_REG::CONFIG_PORT_0, 0xFF);
  writeRegister(PCAL95555_REG::CONFIG_PORT_1, 0xFF);
  // Drive strength = full (all bits 1)
  writeRegister(PCAL95555_REG::DRIVE_STRENGTH_0, 0xFF);
  writeRegister(PCAL95555_REG::DRIVE_STRENGTH_1, 0xFF);
  writeRegister(PCAL95555_REG::DRIVE_STRENGTH_2, 0xFF);
  writeRegister(PCAL95555_REG::DRIVE_STRENGTH_3, 0xFF);
  // Latch disabled (default 0x00)
  writeRegister(PCAL95555_REG::INPUT_LATCH_0, 0x00);
  writeRegister(PCAL95555_REG::INPUT_LATCH_1, 0x00);
  // Pull-up/down enabled (1), pull-up selected (1), interrupt masked (1)
  writeRegister(PCAL95555_REG::PULL_ENABLE_0, 0xFF);
  writeRegister(PCAL95555_REG::PULL_ENABLE_1, 0xFF);
  writeRegister(PCAL95555_REG::PULL_SELECT_0, 0xFF);
  writeRegister(PCAL95555_REG::PULL_SELECT_1, 0xFF);
  writeRegister(PCAL95555_REG::INT_MASK_0, 0xFF);
  writeRegister(PCAL95555_REG::INT_MASK_1, 0xFF);
  // Output mode = push-pull (0)
  writeRegister(PCAL95555_REG::OUTPUT_CONF, 0x00);
}

// Initialize using compile-time configuration
template <typename I2cType>
void PCAL95555<I2cType>::initFromConfig() {
#if CONFIG_PCAL95555_INIT_FROM_KCONFIG
  writeRegister(PCAL95555_REG::OUTPUT_PORT_0, uint8_t(CONFIG_PCAL95555_INIT_OUTPUT & 0xFF));
  writeRegister(PCAL95555_REG::OUTPUT_PORT_1, uint8_t((CONFIG_PCAL95555_INIT_OUTPUT >> 8) & 0xFF));

  writeRegister(PCAL95555_REG::CONFIG_PORT_0, uint8_t(CONFIG_PCAL95555_INIT_DIRECTION & 0xFF));
  writeRegister(PCAL95555_REG::CONFIG_PORT_1,
                uint8_t((CONFIG_PCAL95555_INIT_DIRECTION >> 8) & 0xFF));

  writeRegister(PCAL95555_REG::PULL_ENABLE_0, uint8_t(CONFIG_PCAL95555_INIT_PULL_ENABLE & 0xFF));
  writeRegister(PCAL95555_REG::PULL_ENABLE_1,
                uint8_t((CONFIG_PCAL95555_INIT_PULL_ENABLE >> 8) & 0xFF));

  writeRegister(PCAL95555_REG::PULL_SELECT_0, uint8_t(CONFIG_PCAL95555_INIT_PULL_UP & 0xFF));
  writeRegister(PCAL95555_REG::PULL_SELECT_1, uint8_t((CONFIG_PCAL95555_INIT_PULL_UP >> 8) & 0xFF));

  uint8_t od =
      (CONFIG_PCAL95555_INIT_OD_PORT1 ? 1 : 0) << 1 | (CONFIG_PCAL95555_INIT_OD_PORT0 ? 1 : 0);
  writeRegister(PCAL95555_REG::OUTPUT_CONF, od);
#endif
}

// Set or clear a bit in a register byte
static uint8_t updateBit(uint8_t regVal, uint8_t bit, bool set) {
  if (set)
    return regVal | (1 << bit);
  else
    return regVal & ~(1 << bit);
}

// Direction configuration
template <typename I2cType>
bool PCAL95555<I2cType>::setPinDirection(uint16_t pin, GPIODir dir) {
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? PCAL95555_REG::CONFIG_PORT_0 : PCAL95555_REG::CONFIG_PORT_1;
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val))
    return false;
  val = updateBit(val, bit, (dir == GPIODir::Input));
  return writeRegister(reg, val);
}
template <typename I2cType>
bool PCAL95555<I2cType>::setMultipleDirections(uint16_t mask, GPIODir dir) {
  clearError(Error::InvalidMask);
  uint8_t val0 = 0;
  if (!readRegister(PCAL95555_REG::CONFIG_PORT_0, val0))
    return false;
  for (int bit = 0; bit < 8; ++bit) {
    if (mask & (1u << bit))
      val0 = updateBit(val0, bit, (dir == GPIODir::Input));
  }
  if (!writeRegister(PCAL95555_REG::CONFIG_PORT_0, val0))
    return false;

  uint8_t val1 = 0;
  if (!readRegister(PCAL95555_REG::CONFIG_PORT_1, val1))
    return false;
  for (int bit = 0; bit < 8; ++bit) {
    if (mask & (1u << (bit + 8)))
      val1 = updateBit(val1, bit, (dir == GPIODir::Input));
  }
  return writeRegister(PCAL95555_REG::CONFIG_PORT_1, val1);
}

// Read input port registers and return bit
template <typename I2cType>
bool PCAL95555<I2cType>::readPin(uint16_t pin) {
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? PCAL95555_REG::INPUT_PORT_0 : PCAL95555_REG::INPUT_PORT_1;
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val))
    return false;
  return (val & (1 << bit)) != 0;
}

// Write output port registers
template <typename I2cType>
bool PCAL95555<I2cType>::writePin(uint16_t pin, bool value) {
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? PCAL95555_REG::OUTPUT_PORT_0 : PCAL95555_REG::OUTPUT_PORT_1;
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val))
    return false;
  val = updateBit(val, bit, value);
  return writeRegister(reg, val);
}

template <typename I2cType>
bool PCAL95555<I2cType>::togglePin(uint16_t pin) {
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? PCAL95555_REG::OUTPUT_PORT_0 : PCAL95555_REG::OUTPUT_PORT_1;
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val))
    return false;
  val ^= (1 << bit);
  return writeRegister(reg, val);
}

// Pull-up/down control
template <typename I2cType>
bool PCAL95555<I2cType>::setPullEnable(uint16_t pin, bool enable) {
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? PCAL95555_REG::PULL_ENABLE_0 : PCAL95555_REG::PULL_ENABLE_1;
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val))
    return false;
  val = updateBit(val, bit, enable);
  return writeRegister(reg, val);
}
template <typename I2cType>
bool PCAL95555<I2cType>::setPullDirection(uint16_t pin, bool pullUp) {
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? PCAL95555_REG::PULL_SELECT_0 : PCAL95555_REG::PULL_SELECT_1;
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val))
    return false;
  val = updateBit(val, bit, pullUp);
  return writeRegister(reg, val);
}

// Drive strength (2 bits per pin)
template <typename I2cType>
bool PCAL95555<I2cType>::setDriveStrength(uint16_t pin, DriveStrength level) {
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t base = (pin < 8) ? PCAL95555_REG::DRIVE_STRENGTH_0 : PCAL95555_REG::DRIVE_STRENGTH_2;
  uint8_t index = pin % 8;
  uint8_t reg = base + ((index >= 4) ? 1 : 0);
  uint8_t bit = (index % 4) * 2; // each pin uses 2 bits
  uint8_t val = 0;
  if (!readRegister(reg, val))
    return false;
  // Clear the two bits and set the new level
  val &= ~(0x3 << bit);
  val |= (uint8_t(level) << bit);
  return writeRegister(reg, val);
}

// Interrupt mask
template <typename I2cType>
bool PCAL95555<I2cType>::configureInterruptMask(uint16_t mask) {
  if (!writeRegister(PCAL95555_REG::INT_MASK_0, uint8_t(mask & 0xFF)))
    return false;
  return writeRegister(PCAL95555_REG::INT_MASK_1, uint8_t((mask >> 8) & 0xFF));
}

// Read interrupt status (and clear)
template <typename I2cType>
uint16_t PCAL95555<I2cType>::getInterruptStatus() {
  uint8_t lo = 0, hi = 0;
  readRegister(PCAL95555_REG::INT_STATUS_0, lo);
  readRegister(PCAL95555_REG::INT_STATUS_1, hi);
  return uint16_t(hi) << 8 | lo;
}

// Output mode configuration (ODEN bits)
template <typename I2cType>
bool PCAL95555<I2cType>::setOutputMode(bool od0, bool od1) {
  uint8_t val = (od1 ? 1 : 0) << 1 | (od0 ? 1 : 0);
  return writeRegister(PCAL95555_REG::OUTPUT_CONF, val);
}

// Interrupt callback
template <typename I2cType>
void PCAL95555<I2cType>::setInterruptCallback(std::function<void(uint16_t)> cb) {
  irqCallback_ = cb;
}

// Call this when an INT occurs; read status and invoke callback.
template <typename I2cType>
void PCAL95555<I2cType>::handleInterrupt() {
  if (irqCallback_) {
    uint16_t status = getInterruptStatus();
    irqCallback_(status);
  }
}

template <typename I2cType>
bool PCAL95555<I2cType>::setPinPolarity(uint16_t pin, Polarity polarity) {
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? PCAL95555_REG::POLARITY_INV_0 : PCAL95555_REG::POLARITY_INV_1;
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val))
    return false;
  val = updateBit(val, bit, uint8_t(polarity));
  return writeRegister(reg, val);
}
template <typename I2cType>
bool PCAL95555<I2cType>::setMultiplePolarities(uint16_t mask, Polarity polarity) {
  uint8_t val0 = 0;
  if (!readRegister(PCAL95555_REG::POLARITY_INV_0, val0))
    return false;
  for (int bit = 0; bit < 8; ++bit) {
    if (mask & (1u << bit))
      val0 = updateBit(val0, bit, uint8_t(polarity));
  }
  if (!writeRegister(PCAL95555_REG::POLARITY_INV_0, val0))
    return false;
  uint8_t val1 = 0;
  if (!readRegister(PCAL95555_REG::POLARITY_INV_1, val1))
    return false;
  for (int bit = 0; bit < 8; ++bit) {
    if (mask & (1u << (bit + 8)))
      val1 = updateBit(val1, bit, uint8_t(polarity));
  }
  return writeRegister(PCAL95555_REG::POLARITY_INV_1, val1);
}

template <typename I2cType>
bool PCAL95555<I2cType>::enableInputLatch(uint16_t pin, bool enable) {
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? PCAL95555_REG::INPUT_LATCH_0 : PCAL95555_REG::INPUT_LATCH_1;
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val))
    return false;
  val = updateBit(val, bit, enable);
  return writeRegister(reg, val);
}
template <typename I2cType>
bool PCAL95555<I2cType>::enableMultipleInputLatches(uint16_t mask, bool enable) {
  clearError(Error::InvalidMask);
  uint8_t val0 = 0;
  if (!readRegister(PCAL95555_REG::INPUT_LATCH_0, val0))
    return false;
  for (int bit = 0; bit < 8; ++bit) {
    if (mask & (1u << bit))
      val0 = updateBit(val0, bit, enable);
  }
  if (!writeRegister(PCAL95555_REG::INPUT_LATCH_0, val0))
    return false;
  uint8_t val1 = 0;
  if (!readRegister(PCAL95555_REG::INPUT_LATCH_1, val1))
    return false;
  for (int bit = 0; bit < 8; ++bit) {
    if (mask & (1u << (bit + 8)))
      val1 = updateBit(val1, bit, enable);
  }
  return writeRegister(PCAL95555_REG::INPUT_LATCH_1, val1);
}

template <typename I2cType>
uint16_t PCAL95555<I2cType>::getErrorFlags() const {
  return errorFlags_;
}

template <typename I2cType>
void PCAL95555<I2cType>::clearErrorFlags(uint16_t mask) {
  errorFlags_ &= ~mask;
}

template <typename I2cType>
void PCAL95555<I2cType>::setError(Error e) {
  errorFlags_ |= static_cast<uint16_t>(e);
}

template <typename I2cType>
void PCAL95555<I2cType>::clearError(Error e) {
  errorFlags_ &= ~static_cast<uint16_t>(e);
}

#endif  // PCAL95555_IMPL
