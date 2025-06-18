#include "../../src/pcal95555.hpp"

class DummyBus : public PCAL95555::i2cBus {
public:
  bool write(uint8_t, uint8_t, const uint8_t *, size_t) override {
    return true;
  }
  bool read(uint8_t, uint8_t, uint8_t *, size_t) override {
    return true;
  }
};
