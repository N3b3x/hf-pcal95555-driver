#include "../inc/pcal95555.hpp"
#include <cassert>
#include <iostream>
#include <unordered_map>

// Mock I2C bus that simulates the PCAL9555A device registers and behavior
class MockI2C : public pcal95555::I2cInterface<MockI2C> {
public:
  MockI2C() {
    // Initialize all registers to power-on default values:
    registers[PCAL95555_REG::INPUT_PORT_0] = 0xFF;
    registers[PCAL95555_REG::INPUT_PORT_1] = 0xFF;
    registers[PCAL95555_REG::OUTPUT_PORT_0] = 0xFF;
    registers[PCAL95555_REG::OUTPUT_PORT_1] = 0xFF;
    registers[PCAL95555_REG::POLARITY_INV_0] = 0x00;
    registers[PCAL95555_REG::POLARITY_INV_1] = 0x00;
    registers[PCAL95555_REG::CONFIG_PORT_0] = 0xFF;
    registers[PCAL95555_REG::CONFIG_PORT_1] = 0xFF;
    registers[PCAL95555_REG::DRIVE_STRENGTH_0] = 0xFF;
    registers[PCAL95555_REG::DRIVE_STRENGTH_1] = 0xFF;
    registers[PCAL95555_REG::DRIVE_STRENGTH_2] = 0xFF;
    registers[PCAL95555_REG::DRIVE_STRENGTH_3] = 0xFF;
    registers[PCAL95555_REG::INPUT_LATCH_0] = 0x00;
    registers[PCAL95555_REG::INPUT_LATCH_1] = 0x00;
    registers[PCAL95555_REG::PULL_ENABLE_0] = 0xFF;
    registers[PCAL95555_REG::PULL_ENABLE_1] = 0xFF;
    registers[PCAL95555_REG::PULL_SELECT_0] = 0xFF;
    registers[PCAL95555_REG::PULL_SELECT_1] = 0xFF;
    registers[PCAL95555_REG::INT_MASK_0] = 0xFF;
    registers[PCAL95555_REG::INT_MASK_1] = 0xFF;
    registers[PCAL95555_REG::INT_STATUS_0] = 0x00;
    registers[PCAL95555_REG::INT_STATUS_1] = 0x00;
    registers[PCAL95555_REG::OUTPUT_CONF] = 0x00;
  }

  bool write(uint8_t addr, uint8_t reg, const uint8_t* data, size_t len) override {
    if (failNextWriteCount > 0) {
      failNextWriteCount--;
      return false;
    }
    for (size_t i = 0; i < len; ++i) {
      uint8_t regAddr = reg + i;
      registers[regAddr] = data[i];
      // Simulate output->input reflection for push-pull/open-drain
      if (regAddr == PCAL95555_REG::OUTPUT_PORT_0 || regAddr == PCAL95555_REG::OUTPUT_PORT_1) {
        bool isPort0 = (regAddr == PCAL95555_REG::OUTPUT_PORT_0);
        uint8_t portBit = isPort0 ? 0 : 1;
        uint8_t confVal =
            registers[isPort0 ? PCAL95555_REG::CONFIG_PORT_0 : PCAL95555_REG::CONFIG_PORT_1];
        uint8_t outVal = registers[regAddr];
        uint8_t modeVal = registers[PCAL95555_REG::OUTPUT_CONF];
        bool openDrain = (modeVal >> portBit) & 0x1;
        uint8_t inputReg = isPort0 ? PCAL95555_REG::INPUT_PORT_0 : PCAL95555_REG::INPUT_PORT_1;
        uint8_t newInputVal = registers[inputReg];
        for (int bit = 0; bit < 8; ++bit) {
          bool isOutputPin = ((confVal >> bit) & 0x1) == 0;
          bool outputLevel = (outVal >> bit) & 0x1;
          if (isOutputPin) {
            if (openDrain) {
              if (outputLevel)
                newInputVal |= (1 << bit);
              else
                newInputVal &= ~(1 << bit);
            } else {
              if (outputLevel)
                newInputVal |= (1 << bit);
              else
                newInputVal &= ~(1 << bit);
            }
          }
        }
        registers[inputReg] = newInputVal;
      }
    }
    return true;
  }

  bool read(uint8_t addr, uint8_t reg, uint8_t* data, size_t len) override {
    if (failNextReadCount > 0) {
      failNextReadCount--;
      return false;
    }
    for (size_t i = 0; i < len; ++i) {
      uint8_t regAddr = reg + i;
      data[i] = registers.count(regAddr) ? registers[regAddr] : 0;
      if (regAddr == PCAL95555_REG::INT_STATUS_0 || regAddr == PCAL95555_REG::INT_STATUS_1)
        registers[regAddr] = 0x00;
    }
    return true;
  }

  void setNextWriteNack(int count) {
    failNextWriteCount = count;
  }
  void setNextReadNack(int count) {
    failNextReadCount = count;
  }
  uint8_t getReg(uint8_t regAddr) const {
    auto it = registers.find(regAddr);
    return (it != registers.end()) ? it->second : 0;
  }

private:
  std::unordered_map<uint8_t, uint8_t> registers;
  int failNextWriteCount = 0;
  int failNextReadCount = 0;
};

int main() {
  MockI2C mock;
  PCAL95555 dev(&mock, 0x20);

  // Test ResetToDefault()
  uint8_t tmp = 0x00;
  mock.write(0x20, PCAL95555_REG::CONFIG_PORT_0, &tmp, 1);
  dev.ResetToDefault();
  assert(mock.getReg(PCAL95555_REG::CONFIG_PORT_0) == 0xFF);

  // Test pin direction
  assert(dev.SetPinDirection(3, GPIODir::Output));
  assert((mock.getReg(PCAL95555_REG::CONFIG_PORT_0) & (1 << 3)) == 0);

  // Simulate I2C write failure
  mock.setNextWriteNack(2); // fail both attempts
  assert(!dev.SetPinDirection(1, GPIODir::Input));
  assert(dev.GetErrorFlags() & static_cast<uint16_t>(Error::I2CWriteFail));
  // Successful call should clear the error
  assert(dev.SetPinDirection(1, GPIODir::Input));
  assert((dev.GetErrorFlags() & static_cast<uint16_t>(Error::I2CWriteFail)) == 0);

  // Invalid pin test
  assert(!dev.WritePin(20, true));
  assert(dev.GetErrorFlags() & static_cast<uint16_t>(Error::InvalidPin));
  dev.ClearErrorFlags();
  assert(dev.GetErrorFlags() == 0);

  // ...additional tests as detailed earlier...

  std::cout << "All tests passed." << std::endl;
  return 0;
}
