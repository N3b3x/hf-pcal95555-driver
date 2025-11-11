#include "pcal95555.hpp"
#include <cstring>

// Constructor: store I2C bus and device address (7-bit)
PCAL95555::PCAL95555(PCAL95555::i2cBus *bus, uint8_t address) : i2c_(bus), devAddr_(address) {}

// Configure retry count
void PCAL95555::setRetries(int r) {
  retries_ = r;
}

// Low-level write with retries
bool PCAL95555::writeRegister(uint8_t reg, uint8_t value) {
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
bool PCAL95555::readRegister(uint8_t reg, uint8_t &value) {
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
void PCAL95555::resetToDefault() {
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
void PCAL95555::initFromConfig() {
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
bool PCAL95555::setPinDirection(uint16_t pin, GPIODir dir) {
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
bool PCAL95555::setMultipleDirections(uint16_t mask, GPIODir dir) {
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
bool PCAL95555::readPin(uint16_t pin) {
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
bool PCAL95555::writePin(uint16_t pin, bool value) {
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

bool PCAL95555::togglePin(uint16_t pin) {
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
bool PCAL95555::setPullEnable(uint16_t pin, bool enable) {
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
bool PCAL95555::setPullDirection(uint16_t pin, bool pullUp) {
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
bool PCAL95555::setDriveStrength(uint16_t pin, DriveStrength level) {
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
bool PCAL95555::configureInterruptMask(uint16_t mask) {
  if (!writeRegister(PCAL95555_REG::INT_MASK_0, uint8_t(mask & 0xFF)))
    return false;
  return writeRegister(PCAL95555_REG::INT_MASK_1, uint8_t((mask >> 8) & 0xFF));
}

// Read interrupt status (and clear)
uint16_t PCAL95555::getInterruptStatus() {
  uint8_t lo = 0, hi = 0;
  readRegister(PCAL95555_REG::INT_STATUS_0, lo);
  readRegister(PCAL95555_REG::INT_STATUS_1, hi);
  return uint16_t(hi) << 8 | lo;
}

// Output mode configuration (ODEN bits)
bool PCAL95555::setOutputMode(bool od0, bool od1) {
  uint8_t val = (od1 ? 1 : 0) << 1 | (od0 ? 1 : 0);
  return writeRegister(PCAL95555_REG::OUTPUT_CONF, val);
}

// Interrupt callback
void PCAL95555::setInterruptCallback(std::function<void(uint16_t)> cb) {
  irqCallback_ = cb;
}

// Call this when an INT occurs; read status and invoke callback.
void PCAL95555::handleInterrupt() {
  if (irqCallback_) {
    uint16_t status = getInterruptStatus();
    irqCallback_(status);
  }
}

bool PCAL95555::setPinPolarity(uint16_t pin, Polarity polarity) {
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
bool PCAL95555::setMultiplePolarities(uint16_t mask, Polarity polarity) {
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

bool PCAL95555::enableInputLatch(uint16_t pin, bool enable) {
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
bool PCAL95555::enableMultipleInputLatches(uint16_t mask, bool enable) {
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

uint16_t PCAL95555::getErrorFlags() const {
  return errorFlags_;
}

void PCAL95555::clearErrorFlags(uint16_t mask) {
  errorFlags_ &= ~mask;
}

void PCAL95555::setError(Error e) {
  errorFlags_ |= static_cast<uint16_t>(e);
}

void PCAL95555::clearError(Error e) {
  errorFlags_ &= ~static_cast<uint16_t>(e);
}
