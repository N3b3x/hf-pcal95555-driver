#ifndef PCAL95555_IMPL
#define PCAL95555_IMPL

// When included from header, use relative path; when compiled directly, use standard include
#ifdef PCAL95555_HEADER_INCLUDED
#include "../inc/pcal95555.hpp"
#else
#include "../inc/pcal95555.hpp"
#endif

// Constructor: store I2C bus and pin levels (lazy initialization)
template <typename I2cType>
pcal95555::PCAL95555<I2cType>::PCAL95555(I2cType* bus, bool a0_level, bool a1_level, bool a2_level)
    : i2c_(bus), previous_pin_states_(0), initialized_(false),
      a0_level_(a0_level), a1_level_(a1_level), a2_level_(a2_level) {
  // Calculate address bits from pin levels (A0=bit0, A1=bit1, A2=bit2)
  address_bits_ = (a0_level ? 1 : 0) | ((a1_level ? 1 : 0) << 1) | ((a2_level ? 1 : 0) << 2);
  dev_addr_ = CalculateAddress(address_bits_);

  // Initialize pin callbacks array
  for (auto& callback : pin_callbacks_) {
    callback.registered = false;
    callback.edge = InterruptEdge::Both;
  }

  // No initialization here - use EnsureInitialized() when ready
}

// Constructor: store I2C bus and calculate pin levels from address (lazy initialization)
template <typename I2cType>
pcal95555::PCAL95555<I2cType>::PCAL95555(I2cType* bus, uint8_t address)
    : i2c_(bus), previous_pin_states_(0), initialized_(false) {
  // Validate address range (0x20 to 0x27)
  constexpr uint8_t BASE_ADDRESS = 0x20;
  constexpr uint8_t MAX_ADDRESS = 0x27;
  constexpr uint8_t MAX_BITS = 0x07;

  if (address < BASE_ADDRESS || address > MAX_ADDRESS) {
    // Address out of range - clamp to valid range
    if (address < BASE_ADDRESS) {
      address = BASE_ADDRESS;
    } else {
      address = MAX_ADDRESS;
    }
    // Note: We don't set an error flag here as the address is still valid, just clamped
  }

  // Calculate address bits from address: address_bits = address - BASE_ADDRESS
  address_bits_ = (address - BASE_ADDRESS) & MAX_BITS;
  dev_addr_ = CalculateAddress(address_bits_);

  // Calculate pin levels from address bits and store for lazy initialization
  a0_level_ = (address_bits_ & 0x01) != 0;
  a1_level_ = (address_bits_ & 0x02) != 0;
  a2_level_ = (address_bits_ & 0x04) != 0;

  // Initialize pin callbacks array
  for (auto& callback : pin_callbacks_) {
    callback.registered = false;
    callback.edge = InterruptEdge::Both;
  }

  // No initialization here - use EnsureInitialized() when ready
}

// Ensure initialization (lazy initialization)
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::EnsureInitialized() noexcept {
  if (initialized_) {
    return true;  // Already initialized
  }
  return Initialize();
}

// Perform actual initialization
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::Initialize() noexcept {
  // Ensure I2C bus is initialized and ready
  if (!i2c_->EnsureInitialized()) {
    setError(Error::I2CReadFail);
    initialized_ = false;
    return false;
  }

  // Attempt to set address pins via I2C interface (if supported)
  // This will return false if GPIO control is not implemented (hardwired pins)
  i2c_->SetAddressPins(a0_level_, a1_level_, a2_level_);

  // Verify communication by reading a register (INPUT_PORT_0)
  // This ensures the device is accessible at the calculated address
  uint8_t test_value = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_PORT_0), test_value)) {
    // Communication failed - this is expected if address pins are hardwired differently
    // or device is not connected. We still store the address for potential retry.
    setError(Error::I2CReadFail);
    initialized_ = false;
    return false;
  }

  // Communication successful - clear any previous errors
  clearError(Error::I2CReadFail);
  
  // Initialize previous pin states for edge detection
  previous_pin_states_ = ReadPinStates();
  
  // Mark as initialized
  initialized_ = true;
  return true;
}

// Configure retry count
template <typename I2cType>
void pcal95555::PCAL95555<I2cType>::SetRetries(int retries) noexcept {
  retries_ = retries;
}

// Low-level write with retries
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::writeRegister(uint8_t reg, uint8_t value) noexcept {
  for (int attempt = 0; attempt <= retries_; ++attempt) {
    if (i2c_->write(dev_addr_, reg, &value, 1)) {
      clearError(Error::I2CWriteFail);
      return true;
    }
  }
  setError(Error::I2CWriteFail);
  return false;
}
// Low-level read with retries
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::readRegister(uint8_t reg, uint8_t& value) noexcept {
  for (int attempt = 0; attempt <= retries_; ++attempt) {
    if (i2c_->read(dev_addr_, reg, &value, 1)) {
      clearError(Error::I2CReadFail);
      return true;
    }
  }
  setError(Error::I2CReadFail);
  return false;
}

// Reset all registers to defaults as per datasheet.
template <typename I2cType>
void pcal95555::PCAL95555<I2cType>::ResetToDefault() noexcept {
  if (!EnsureInitialized()) {
    return;
  }
  // All defaults taken from datasheet tables.
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::OUTPUT_PORT_0), 0xFF);
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::OUTPUT_PORT_1), 0xFF);
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::POLARITY_INV_0), 0x00);
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::POLARITY_INV_1), 0x00);
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::CONFIG_PORT_0), 0xFF);
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::CONFIG_PORT_1), 0xFF);
  // Drive strength = full (all bits 1)
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::DRIVE_STRENGTH_0), 0xFF);
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::DRIVE_STRENGTH_1), 0xFF);
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::DRIVE_STRENGTH_2), 0xFF);
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::DRIVE_STRENGTH_3), 0xFF);
  // Latch disabled (default 0x00)
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_LATCH_0), 0x00);
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_LATCH_1), 0x00);
  // Pull-up/down enabled (1), pull-up selected (1), interrupt masked (1)
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_ENABLE_0), 0xFF);
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_ENABLE_1), 0xFF);
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_SELECT_0), 0xFF);
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_SELECT_1), 0xFF);
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::INT_MASK_0), 0xFF);
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::INT_MASK_1), 0xFF);
  // Output mode = push-pull (0)
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::OUTPUT_CONF), 0x00);
}

// Initialize using compile-time configuration
template <typename I2cType>
void pcal95555::PCAL95555<I2cType>::InitFromConfig() noexcept {
  if (!EnsureInitialized()) {
    return;
  }
#if CONFIG_PCAL95555_INIT_FROM_KCONFIG
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::OUTPUT_PORT_0), uint8_t(CONFIG_PCAL95555_INIT_OUTPUT & 0xFF));
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::OUTPUT_PORT_1), uint8_t((CONFIG_PCAL95555_INIT_OUTPUT >> 8) & 0xFF));

  writeRegister(static_cast<uint8_t>(PCAL95555_REG::CONFIG_PORT_0), uint8_t(CONFIG_PCAL95555_INIT_DIRECTION & 0xFF));
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::CONFIG_PORT_1),
                uint8_t((CONFIG_PCAL95555_INIT_DIRECTION >> 8) & 0xFF));

  writeRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_ENABLE_0), uint8_t(CONFIG_PCAL95555_INIT_PULL_ENABLE & 0xFF));
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_ENABLE_1),
                uint8_t((CONFIG_PCAL95555_INIT_PULL_ENABLE >> 8) & 0xFF));

  writeRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_SELECT_0), uint8_t(CONFIG_PCAL95555_INIT_PULL_UP & 0xFF));
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_SELECT_1), uint8_t((CONFIG_PCAL95555_INIT_PULL_UP >> 8) & 0xFF));

  uint8_t open_drain_config =
      (CONFIG_PCAL95555_INIT_OD_PORT1 ? 1 : 0) << 1 | (CONFIG_PCAL95555_INIT_OD_PORT0 ? 1 : 0);
  writeRegister(static_cast<uint8_t>(PCAL95555_REG::OUTPUT_CONF), open_drain_config);
#endif
}

// Set or clear a bit in a register byte
static uint8_t updateBit(uint8_t regVal, uint8_t bit, bool set) {
  if (set) {
    return regVal | (1 << bit);
  }
  return regVal & ~(1 << bit);
}

// Direction configuration
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::SetPinDirection(uint16_t pin, GPIODir dir) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? static_cast<uint8_t>(PCAL95555_REG::CONFIG_PORT_0) : static_cast<uint8_t>(PCAL95555_REG::CONFIG_PORT_1);
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val)) {
    return false;
  }
  val = updateBit(val, bit, (dir == GPIODir::Input));
  return writeRegister(reg, val);
}
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::SetMultipleDirections(uint16_t mask, GPIODir dir) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  clearError(Error::InvalidMask);
  uint8_t val0 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::CONFIG_PORT_0), val0)) {
    return false;
  }
  for (int bit = 0; bit < 8; ++bit) {
    if ((mask & (1U << bit)) != 0U) {
      val0 = updateBit(val0, bit, (dir == GPIODir::Input));
    }
  }
  if (!writeRegister(static_cast<uint8_t>(PCAL95555_REG::CONFIG_PORT_0), val0)) {
    return false;
  }

  uint8_t val1 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::CONFIG_PORT_1), val1)) {
    return false;
  }
  for (int bit = 0; bit < 8; ++bit) {
    if ((mask & (1U << (bit + 8))) != 0U) {
      val1 = updateBit(val1, bit, (dir == GPIODir::Input));
    }
  }
  return writeRegister(static_cast<uint8_t>(PCAL95555_REG::CONFIG_PORT_1), val1);
}

// Configure direction for multiple pins with individual settings
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::SetDirections(std::initializer_list<std::pair<uint16_t, GPIODir>> configs) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  // Read current config registers
  uint8_t port0 = 0;
  uint8_t port1 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::CONFIG_PORT_0), port0)) {
    return false;
  }
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::CONFIG_PORT_1), port1)) {
    return false;
  }

  // Update values for each pin
  for (const auto& config : configs) {
    uint16_t pin = config.first;
    GPIODir dir = config.second;

    if (pin >= 16) {
      setError(Error::InvalidPin);
      return false;
    }

    uint8_t bit = pin % 8;
    if (pin < 8) {
      port0 = updateBit(port0, bit, (dir == GPIODir::Input));
    } else {
      port1 = updateBit(port1, bit, (dir == GPIODir::Input));
    }
  }

  clearError(Error::InvalidPin);

  // Write back both registers
  if (!writeRegister(static_cast<uint8_t>(PCAL95555_REG::CONFIG_PORT_0), port0)) {
    return false;
  }
  return writeRegister(static_cast<uint8_t>(PCAL95555_REG::CONFIG_PORT_1), port1);
}

// Read input port registers and return bit
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::ReadPin(uint16_t pin) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? static_cast<uint8_t>(PCAL95555_REG::INPUT_PORT_0) : static_cast<uint8_t>(PCAL95555_REG::INPUT_PORT_1);
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val)) {
    return false;
  }
  return (val & (1 << bit)) != 0;
}

// Write output port registers
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::WritePin(uint16_t pin, bool value) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? static_cast<uint8_t>(PCAL95555_REG::OUTPUT_PORT_0) : static_cast<uint8_t>(PCAL95555_REG::OUTPUT_PORT_1);
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val)) {
    return false;
  }
  val = updateBit(val, bit, value);
  return writeRegister(reg, val);
}

template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::TogglePin(uint16_t pin) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? static_cast<uint8_t>(PCAL95555_REG::OUTPUT_PORT_0) : static_cast<uint8_t>(PCAL95555_REG::OUTPUT_PORT_1);
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val)) {
    return false;
  }
  val ^= (1 << bit);
  return writeRegister(reg, val);
}

// Write multiple pins
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::WritePins(std::initializer_list<std::pair<uint16_t, bool>> configs) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  // Read current output registers
  uint8_t port0 = 0;
  uint8_t port1 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::OUTPUT_PORT_0), port0)) {
    return false;
  }
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::OUTPUT_PORT_1), port1)) {
    return false;
  }

  // Update values for each pin
  for (const auto& config : configs) {
    uint16_t pin = config.first;
    bool value = config.second;

    if (pin >= 16) {
      setError(Error::InvalidPin);
      return false;
    }

    uint8_t bit = pin % 8;
    if (pin < 8) {
      port0 = updateBit(port0, bit, value);
    } else {
      port1 = updateBit(port1, bit, value);
    }
  }

  clearError(Error::InvalidPin);

  // Write back both registers
  if (!writeRegister(static_cast<uint8_t>(PCAL95555_REG::OUTPUT_PORT_0), port0)) {
    return false;
  }
  return writeRegister(static_cast<uint8_t>(PCAL95555_REG::OUTPUT_PORT_1), port1);
}

// Read multiple pins
template <typename I2cType>
std::vector<std::pair<uint16_t, bool>> pcal95555::PCAL95555<I2cType>::ReadPins(std::initializer_list<uint16_t> pins) noexcept {
  std::vector<std::pair<uint16_t, bool>> results;
  if (!EnsureInitialized()) {
    return results;  // Return empty results if not initialized
  }

  // Read both input port registers once
  uint8_t port0 = 0;
  uint8_t port1 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_PORT_0), port0)) {
    // On failure, return empty results
    return results;
  }
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_PORT_1), port1)) {
    return results;
  }

  // Extract values for each requested pin
  for (uint16_t pin : pins) {
    if (pin >= 16) {
      // Invalid pin - add with false value
      results.emplace_back(pin, false);
      continue;
    }

    uint8_t bit = pin % 8;
    bool value = false;
    if (pin < 8) {
      value = (port0 & (1 << bit)) != 0;
    } else {
      value = (port1 & (1 << bit)) != 0;
    }
    results.emplace_back(pin, value);
  }

  return results;
}

// Pull-up/down control
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::SetPullEnable(uint16_t pin, bool enable) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? static_cast<uint8_t>(PCAL95555_REG::PULL_ENABLE_0) : static_cast<uint8_t>(PCAL95555_REG::PULL_ENABLE_1);
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val)) {
    return false;
  }
  val = updateBit(val, bit, enable);
  return writeRegister(reg, val);
}
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::SetPullDirection(uint16_t pin, bool pull_up) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? static_cast<uint8_t>(PCAL95555_REG::PULL_SELECT_0) : static_cast<uint8_t>(PCAL95555_REG::PULL_SELECT_1);
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val)) {
    return false;
  }
  val = updateBit(val, bit, pull_up);
  return writeRegister(reg, val);
}

// Configure pull enable for multiple pins
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::SetPullEnables(std::initializer_list<std::pair<uint16_t, bool>> configs) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  // Read current pull enable registers
  uint8_t port0 = 0;
  uint8_t port1 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_ENABLE_0), port0)) {
    return false;
  }
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_ENABLE_1), port1)) {
    return false;
  }

  // Update values for each pin
  for (const auto& config : configs) {
    uint16_t pin = config.first;
    bool enable = config.second;

    if (pin >= 16) {
      setError(Error::InvalidPin);
      return false;
    }

    uint8_t bit = pin % 8;
    if (pin < 8) {
      port0 = updateBit(port0, bit, enable);
    } else {
      port1 = updateBit(port1, bit, enable);
    }
  }

  clearError(Error::InvalidPin);

  // Write back both registers
  if (!writeRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_ENABLE_0), port0)) {
    return false;
  }
  return writeRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_ENABLE_1), port1);
}

// Configure pull direction for multiple pins
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::SetPullDirections(std::initializer_list<std::pair<uint16_t, bool>> configs) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  // Read current pull select registers
  uint8_t port0 = 0;
  uint8_t port1 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_SELECT_0), port0)) {
    return false;
  }
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_SELECT_1), port1)) {
    return false;
  }

  // Update values for each pin
  for (const auto& config : configs) {
    uint16_t pin = config.first;
    bool pull_up = config.second;

    if (pin >= 16) {
      setError(Error::InvalidPin);
      return false;
    }

    uint8_t bit = pin % 8;
    if (pin < 8) {
      port0 = updateBit(port0, bit, pull_up);
    } else {
      port1 = updateBit(port1, bit, pull_up);
    }
  }

  clearError(Error::InvalidPin);

  // Write back both registers
  if (!writeRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_SELECT_0), port0)) {
    return false;
  }
  return writeRegister(static_cast<uint8_t>(PCAL95555_REG::PULL_SELECT_1), port1);
}

// Drive strength (2 bits per pin)
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::SetDriveStrength(uint16_t pin, DriveStrength level) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t base = (pin < 8) ? static_cast<uint8_t>(PCAL95555_REG::DRIVE_STRENGTH_0) : static_cast<uint8_t>(PCAL95555_REG::DRIVE_STRENGTH_2);
  uint8_t index = pin % 8;
  uint8_t reg = base + ((index >= 4) ? 1 : 0);
  uint8_t bit = (index % 4) * 2; // each pin uses 2 bits
  uint8_t val = 0;
  if (!readRegister(reg, val)) {
    return false;
  }
  // Clear the two bits and set the new level
  val &= ~(0x3 << bit);
  val |= (uint8_t(level) << bit);
  return writeRegister(reg, val);
}

// Configure drive strength for multiple pins
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::SetDriveStrengths(std::initializer_list<std::pair<uint16_t, DriveStrength>> configs) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  // Read all drive strength registers (4 registers total)
  uint8_t ds0 = 0, ds1 = 0, ds2 = 0, ds3 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::DRIVE_STRENGTH_0), ds0)) {
    return false;
  }
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::DRIVE_STRENGTH_1), ds1)) {
    return false;
  }
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::DRIVE_STRENGTH_2), ds2)) {
    return false;
  }
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::DRIVE_STRENGTH_3), ds3)) {
    return false;
  }

  // Update values for each pin
  for (const auto& config : configs) {
    uint16_t pin = config.first;
    DriveStrength level = config.second;

    if (pin >= 16) {
      setError(Error::InvalidPin);
      return false;
    }

    uint8_t index = pin % 8;
    uint8_t reg_offset = (index >= 4) ? 1 : 0;
    uint8_t bit = (index % 4) * 2; // each pin uses 2 bits

    uint8_t* reg_ptr = nullptr;
    if (pin < 8) {
      reg_ptr = (reg_offset == 0) ? &ds0 : &ds1;
    } else {
      reg_ptr = (reg_offset == 0) ? &ds2 : &ds3;
    }

    // Clear the two bits and set the new level
    *reg_ptr &= ~(0x3 << bit);
    *reg_ptr |= (uint8_t(level) << bit);
  }

  clearError(Error::InvalidPin);

  // Write back all registers
  if (!writeRegister(static_cast<uint8_t>(PCAL95555_REG::DRIVE_STRENGTH_0), ds0)) {
    return false;
  }
  if (!writeRegister(static_cast<uint8_t>(PCAL95555_REG::DRIVE_STRENGTH_1), ds1)) {
    return false;
  }
  if (!writeRegister(static_cast<uint8_t>(PCAL95555_REG::DRIVE_STRENGTH_2), ds2)) {
    return false;
  }
  return writeRegister(static_cast<uint8_t>(PCAL95555_REG::DRIVE_STRENGTH_3), ds3);
}

// Configure interrupt for a single pin
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::ConfigureInterrupt(uint16_t pin, InterruptState state) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);

  // Read current mask
  uint8_t mask0 = 0;
  uint8_t mask1 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::INT_MASK_0), mask0)) {
    return false;
  }
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::INT_MASK_1), mask1)) {
    return false;
  }

  // Combine into 16-bit mask
  uint16_t mask = (uint16_t(mask1) << 8) | mask0;

  // Update the bit for this pin (0 = enabled, 1 = disabled)
  if (state == InterruptState::Enabled) {
    mask &= ~(1U << pin);  // Clear bit (enable interrupt)
  } else {
    mask |= (1U << pin);   // Set bit (disable interrupt)
  }

  // Write back
  return ConfigureInterruptMask(mask);
}

// Configure interrupts for multiple pins
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::ConfigureInterrupts(std::initializer_list<std::pair<uint16_t, InterruptState>> configs) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  // Read current mask
  uint8_t mask0 = 0;
  uint8_t mask1 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::INT_MASK_0), mask0)) {
    return false;
  }
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::INT_MASK_1), mask1)) {
    return false;
  }

  // Combine into 16-bit mask
  uint16_t mask = (uint16_t(mask1) << 8) | mask0;

  // Update mask for each pin configuration
  for (const auto& config : configs) {
    uint16_t pin = config.first;
    InterruptState state = config.second;

    if (pin >= 16) {
      setError(Error::InvalidPin);
      return false;
    }

    // Update the bit for this pin (0 = enabled, 1 = disabled)
    if (state == InterruptState::Enabled) {
      mask &= ~(1U << pin);  // Clear bit (enable interrupt)
    } else {
      mask |= (1U << pin);   // Set bit (disable interrupt)
    }
  }

  clearError(Error::InvalidPin);

  // Write back once for all pins
  return ConfigureInterruptMask(mask);
}

// Interrupt mask (low-level method)
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::ConfigureInterruptMask(uint16_t mask) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  if (!writeRegister(static_cast<uint8_t>(PCAL95555_REG::INT_MASK_0), uint8_t(mask & 0xFF))) {
    return false;
  }
  return writeRegister(static_cast<uint8_t>(PCAL95555_REG::INT_MASK_1), uint8_t((mask >> 8) & 0xFF));
}

// Read interrupt status (and clear)
template <typename I2cType>
uint16_t pcal95555::PCAL95555<I2cType>::GetInterruptStatus() noexcept {
  if (!EnsureInitialized()) {
    return 0;
  }
  uint8_t low_byte = 0;
  uint8_t high_byte = 0;
  readRegister(static_cast<uint8_t>(PCAL95555_REG::INT_STATUS_0), low_byte);
  readRegister(static_cast<uint8_t>(PCAL95555_REG::INT_STATUS_1), high_byte);
  return uint16_t(high_byte) << 8 | low_byte;
}

// Output mode configuration (ODEN bits)
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::SetOutputMode(bool port_0_open_drain, bool port_1_open_drain) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  uint8_t val = (port_1_open_drain ? 1 : 0) << 1 | (port_0_open_drain ? 1 : 0);
  return writeRegister(static_cast<uint8_t>(PCAL95555_REG::OUTPUT_CONF), val);
}

// Register pin interrupt callback
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::RegisterPinInterrupt(uint16_t pin, InterruptEdge edge,
                                                         std::function<void(uint16_t, bool)> callback) {
  if (!EnsureInitialized()) {
    return false;
  }
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);

  if (!callback) {
    return false;  // Invalid callback
  }

  pin_callbacks_[pin].callback = callback;
  pin_callbacks_[pin].edge = edge;
  pin_callbacks_[pin].registered = true;

  // Read current pin state for edge detection
  uint16_t current_states = ReadPinStates();
  previous_pin_states_ = current_states;

  return true;
}

// Unregister pin interrupt callback
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::UnregisterPinInterrupt(uint16_t pin) noexcept {
  // No I2C communication needed, but check initialization for consistency
  if (!EnsureInitialized()) {
    return false;
  }
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);

  if (!pin_callbacks_[pin].registered) {
    return false;  // No callback registered
  }

  pin_callbacks_[pin].registered = false;
  pin_callbacks_[pin].callback = nullptr;
  return true;
}

// Set global interrupt callback
template <typename I2cType>
void pcal95555::PCAL95555<I2cType>::SetInterruptCallback(const std::function<void(uint16_t)>& callback) noexcept {
  irq_callback_ = callback;
}

// Register interrupt handler with I2C interface
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::RegisterInterruptHandler() noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  // Register this driver's HandleInterrupt method with the I2C interface
  return i2c_->RegisterInterruptHandler([this]() {
    HandleInterrupt();
  });
}

// Read current pin states
template <typename I2cType>
uint16_t pcal95555::PCAL95555<I2cType>::ReadPinStates() noexcept {
  uint8_t port0 = 0;
  uint8_t port1 = 0;
  readRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_PORT_0), port0);
  readRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_PORT_1), port1);
  return (uint16_t(port1) << 8) | port0;
}

// Handle interrupt - read status, check conditions, call callbacks
template <typename I2cType>
void pcal95555::PCAL95555<I2cType>::HandleInterrupt() noexcept {
  if (!EnsureInitialized()) {
    return;
  }
  // Read interrupt status registers (which pins triggered interrupts)
  uint16_t interrupt_status = GetInterruptStatus();

  // Read current pin states
  uint16_t current_states = ReadPinStates();

  // Call global callback if registered
  if (irq_callback_) {
    irq_callback_(interrupt_status);
  }

  // Process each pin that triggered an interrupt
  for (uint16_t pin = 0; pin < 16; ++pin) {
    // Check if this pin triggered an interrupt
    if ((interrupt_status & (1U << pin)) == 0) {
      continue;  // Pin didn't trigger interrupt
    }

    // Check if callback is registered for this pin
    if (!pin_callbacks_[pin].registered || !pin_callbacks_[pin].callback) {
      continue;  // No callback registered
    }

    // Determine edge type
    bool previous_state = (previous_pin_states_ & (1U << pin)) != 0;
    bool current_state = (current_states & (1U << pin)) != 0;
    bool rising_edge = !previous_state && current_state;
    bool falling_edge = previous_state && !current_state;

    // Check if edge matches registered condition
    bool should_call = false;
    switch (pin_callbacks_[pin].edge) {
      case InterruptEdge::Rising:
        should_call = rising_edge;
        break;
      case InterruptEdge::Falling:
        should_call = falling_edge;
        break;
      case InterruptEdge::Both:
        should_call = rising_edge || falling_edge;
        break;
    }

    // Call callback if condition matches
    if (should_call) {
      pin_callbacks_[pin].callback(pin, current_state);
    }
  }

  // Update previous states for next interrupt
  previous_pin_states_ = current_states;
}

template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::SetPinPolarity(uint16_t pin, Polarity polarity) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? static_cast<uint8_t>(PCAL95555_REG::POLARITY_INV_0) : static_cast<uint8_t>(PCAL95555_REG::POLARITY_INV_1);
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val)) {
    return false;
  }
  val = updateBit(val, bit, (polarity == Polarity::Inverted));
  return writeRegister(reg, val);
}
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::SetMultiplePolarities(uint16_t mask, Polarity polarity) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  uint8_t val0 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::POLARITY_INV_0), val0)) {
    return false;
  }
  for (int bit = 0; bit < 8; ++bit) {
    if ((mask & (1U << bit)) != 0U) {
      val0 = updateBit(val0, bit, (polarity == Polarity::Inverted));
    }
  }
  if (!writeRegister(static_cast<uint8_t>(PCAL95555_REG::POLARITY_INV_0), val0)) {
    return false;
  }
  uint8_t val1 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::POLARITY_INV_1), val1)) {
    return false;
  }
  for (int bit = 0; bit < 8; ++bit) {
    if ((mask & (1U << (bit + 8))) != 0U) {
      val1 = updateBit(val1, bit, (polarity == Polarity::Inverted));
    }
  }
  return writeRegister(static_cast<uint8_t>(PCAL95555_REG::POLARITY_INV_1), val1);
}

// Configure polarity for multiple pins with individual settings
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::SetPolarities(std::initializer_list<std::pair<uint16_t, Polarity>> configs) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  // Read current polarity registers
  uint8_t port0 = 0;
  uint8_t port1 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::POLARITY_INV_0), port0)) {
    return false;
  }
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::POLARITY_INV_1), port1)) {
    return false;
  }

  // Update values for each pin
  for (const auto& config : configs) {
    uint16_t pin = config.first;
    Polarity polarity = config.second;

    if (pin >= 16) {
      setError(Error::InvalidPin);
      return false;
    }

    uint8_t bit = pin % 8;
    if (pin < 8) {
      port0 = updateBit(port0, bit, (polarity == Polarity::Inverted));
    } else {
      port1 = updateBit(port1, bit, (polarity == Polarity::Inverted));
    }
  }

  clearError(Error::InvalidPin);

  // Write back both registers
  if (!writeRegister(static_cast<uint8_t>(PCAL95555_REG::POLARITY_INV_0), port0)) {
    return false;
  }
  return writeRegister(static_cast<uint8_t>(PCAL95555_REG::POLARITY_INV_1), port1);
}

template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::EnableInputLatch(uint16_t pin, bool enable) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  if (pin >= 16) {
    setError(Error::InvalidPin);
    return false;
  }
  clearError(Error::InvalidPin);
  uint8_t reg = (pin < 8) ? static_cast<uint8_t>(PCAL95555_REG::INPUT_LATCH_0) : static_cast<uint8_t>(PCAL95555_REG::INPUT_LATCH_1);
  uint8_t bit = pin % 8;
  uint8_t val = 0;
  if (!readRegister(reg, val)) {
    return false;
  }
  val = updateBit(val, bit, enable);
  return writeRegister(reg, val);
}
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::EnableMultipleInputLatches(uint16_t mask, bool enable) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  clearError(Error::InvalidMask);
  uint8_t val0 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_LATCH_0), val0)) {
    return false;
  }
  for (int bit = 0; bit < 8; ++bit) {
    if ((mask & (1U << bit)) != 0U) {
      val0 = updateBit(val0, bit, enable);
    }
  }
  if (!writeRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_LATCH_0), val0)) {
    return false;
  }
  uint8_t val1 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_LATCH_1), val1)) {
    return false;
  }
  for (int bit = 0; bit < 8; ++bit) {
    if ((mask & (1U << (bit + 8))) != 0U) {
      val1 = updateBit(val1, bit, enable);
    }
  }
  return writeRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_LATCH_1), val1);
}

// Configure input latch for multiple pins with individual settings
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::EnableInputLatches(std::initializer_list<std::pair<uint16_t, bool>> configs) noexcept {
  if (!EnsureInitialized()) {
    return false;
  }
  // Read current latch registers
  uint8_t port0 = 0;
  uint8_t port1 = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_LATCH_0), port0)) {
    return false;
  }
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_LATCH_1), port1)) {
    return false;
  }

  // Update values for each pin
  for (const auto& config : configs) {
    uint16_t pin = config.first;
    bool enable = config.second;

    if (pin >= 16) {
      setError(Error::InvalidPin);
      return false;
    }

    uint8_t bit = pin % 8;
    if (pin < 8) {
      port0 = updateBit(port0, bit, enable);
    } else {
      port1 = updateBit(port1, bit, enable);
    }
  }

  clearError(Error::InvalidPin);

  // Write back both registers
  if (!writeRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_LATCH_0), port0)) {
    return false;
  }
  return writeRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_LATCH_1), port1);
}

template <typename I2cType>
uint16_t pcal95555::PCAL95555<I2cType>::GetErrorFlags() const noexcept {
  return error_flags_;
}

template <typename I2cType>
void pcal95555::PCAL95555<I2cType>::ClearErrorFlags(uint16_t mask) noexcept {
  error_flags_ &= ~mask;
}

template <typename I2cType>
void pcal95555::PCAL95555<I2cType>::setError(Error error_code) noexcept {
  error_flags_ |= static_cast<uint16_t>(error_code);
}

template <typename I2cType>
void pcal95555::PCAL95555<I2cType>::clearError(Error error_code) noexcept {
  error_flags_ &= ~static_cast<uint16_t>(error_code);
}

// Get current I2C address
template <typename I2cType>
uint8_t pcal95555::PCAL95555<I2cType>::GetAddress() const noexcept {
  return dev_addr_;
}

// Get current A2-A0 address bits
template <typename I2cType>
uint8_t pcal95555::PCAL95555<I2cType>::GetAddressBits() const noexcept {
  return address_bits_;
}

// Change address by setting A2-A0 pins via GPIO
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::ChangeAddress(bool a0_level, bool a1_level, bool a2_level) noexcept {
  // Note: ChangeAddress doesn't require initialization - it can be called before init
  // However, we should ensure I2C bus is ready if it needs initialization
  // Calculate new address bits
  uint8_t new_bits = (a0_level ? 1 : 0) | ((a1_level ? 1 : 0) << 1) | ((a2_level ? 1 : 0) << 2);
  uint8_t new_addr = CalculateAddress(new_bits);

  // Try to set GPIO pins via I2C interface
  // This will return false if GPIO control is not implemented (hardwired pins)
  bool gpio_set = i2c_->SetAddressPins(a0_level, a1_level, a2_level);

  // Update internal state
  address_bits_ = new_bits;
  dev_addr_ = new_addr;
  a0_level_ = a0_level;
  a1_level_ = a1_level;
  a2_level_ = a2_level;
  
  // Reset initialization flag since address changed
  initialized_ = false;

  // Verify communication at new address
  uint8_t test_value = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_PORT_0), test_value)) {
    // Communication failed - restore previous address if GPIO was set
    if (gpio_set) {
      // GPIO was set but communication failed - this is an error
      setError(Error::I2CReadFail);
      return false;
    }
    // GPIO not set (hardwired) but communication failed - address mismatch
    setError(Error::I2CReadFail);
    return false;
  }

  // Communication successful - mark as initialized
  clearError(Error::I2CReadFail);
  initialized_ = true;
  return true;
}

// Change address using address value directly
template <typename I2cType>
bool pcal95555::PCAL95555<I2cType>::ChangeAddress(uint8_t address) noexcept {
  // Note: ChangeAddress doesn't require initialization - it can be called before init
  // Validate address range (0x20 to 0x27)
  constexpr uint8_t BASE_ADDRESS = 0x20;
  constexpr uint8_t MAX_ADDRESS = 0x27;
  constexpr uint8_t MAX_BITS = 0x07;

  if (address < BASE_ADDRESS || address > MAX_ADDRESS) {
    // Address out of range
    setError(Error::InvalidPin); // Reuse error flag for invalid address
    return false;
  }

  // Calculate address bits from address: address_bits = address - BASE_ADDRESS
  uint8_t new_bits = (address - BASE_ADDRESS) & MAX_BITS;
  uint8_t new_addr = CalculateAddress(new_bits);

  // Calculate pin levels from address bits
  bool a0_level = (new_bits & 0x01) != 0;
  bool a1_level = (new_bits & 0x02) != 0;
  bool a2_level = (new_bits & 0x04) != 0;

  // Try to set GPIO pins via I2C interface
  // This will return false if GPIO control is not implemented (hardwired pins)
  bool gpio_set = i2c_->SetAddressPins(a0_level, a1_level, a2_level);

  // Update internal state
  address_bits_ = new_bits;
  dev_addr_ = new_addr;
  a0_level_ = a0_level;
  a1_level_ = a1_level;
  a2_level_ = a2_level;
  
  // Reset initialization flag since address changed
  initialized_ = false;

  // Verify communication at new address
  uint8_t test_value = 0;
  if (!readRegister(static_cast<uint8_t>(PCAL95555_REG::INPUT_PORT_0), test_value)) {
    // Communication failed
    if (gpio_set) {
      // GPIO was set but communication failed - this is an error
      setError(Error::I2CReadFail);
      return false;
    }
    // GPIO not set (hardwired) but communication failed - address mismatch
    setError(Error::I2CReadFail);
    return false;
  }

  // Communication successful - mark as initialized
  clearError(Error::I2CReadFail);
  clearError(Error::InvalidPin);
  initialized_ = true;
  return true;
}

#endif // PCAL95555_IMPL
