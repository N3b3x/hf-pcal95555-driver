#include "pacl95555.hpp"
#include "stm32f1xx_hal.h" // adjust to your STM32 series

extern I2C_HandleTypeDef hi2c1; // provided by CubeMX-generated code

// Implementation of the PACL95555::i2cBus interface using STM32 HAL
class STM32I2CBus : public PACL95555::i2cBus {
  public:
    bool write(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) override {
        return HAL_I2C_Mem_Write(&hi2c1, addr << 1, reg, 1, (uint8_t *)data, len, HAL_MAX_DELAY) ==
               HAL_OK;
    }

    bool read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) override {
        return HAL_I2C_Mem_Read(&hi2c1, addr << 1, reg, 1, data, len, HAL_MAX_DELAY) == HAL_OK;
    }
};

STM32I2CBus bus;

int main() {
    HAL_Init();
    SystemClock_Config(); // generated elsewhere
    MX_I2C1_Init();       // generated elsewhere

    PACL95555 gpio(&bus, 0x20);
    gpio.resetToDefault();
    gpio.setPinDirection(0, GPIODir::Output);

    while (1) {
        gpio.togglePin(0);
        HAL_Delay(1000);
    }
}
