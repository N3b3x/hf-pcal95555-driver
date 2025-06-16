#include "../../src/pacl95555.hpp"

class DummyBus : public PACL95555::i2cBus {
public:
  bool write(uint8_t, uint8_t, const uint8_t *, size_t) override {
    return true;
  }
  bool read(uint8_t, uint8_t, uint8_t *, size_t) override { return true; }
};
