#include <Wire.h>
#include "pcal95555.hpp"

// Implementation of the PCAL95555::i2cBus interface using the Arduino Wire library
class ArduinoI2CBus : public PCAL95555::i2cBus {
public:
    bool write(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) override {
        Wire.beginTransmission(addr);
        Wire.write(reg);
        Wire.write(data, len);
        return Wire.endTransmission() == 0;
    }

    bool read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) override {
        Wire.beginTransmission(addr);
        Wire.write(reg);
        if (Wire.endTransmission(false) != 0) return false;
        Wire.requestFrom(addr, len);
        for (size_t i = 0; i < len; ++i) {
            data[i] = Wire.read();
        }
        return true;
    }
};

ArduinoI2CBus bus;
PCAL95555 gpio(&bus, 0x20); // Default I2C address

void setup() {
    Wire.begin();
    gpio.resetToDefault();
    gpio.setPinDirection(0, GPIODir::Output); // configure pin 0 as output
}

void loop() {
    gpio.togglePin(0); // toggle pin 0 every second
    delay(1000);
}
