#include "pacl95555.hpp"
#include "driver/i2c.h"
#include <cstring>

// Implementation of the PACL95555::i2cBus interface using ESP-IDF APIs
class ESP32I2CBus : public PACL95555::i2cBus {
public:
    bool write(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) override {
        uint8_t buf[1 + len];
        buf[0] = reg;
        memcpy(&buf[1], data, len);
        return i2c_master_write_to_device(I2C_NUM_0, addr, buf, len + 1, pdMS_TO_TICKS(100)) == ESP_OK;
    }

    bool read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) override {
        return i2c_master_write_read_device(I2C_NUM_0, addr, &reg, 1, data, len, pdMS_TO_TICKS(100)) == ESP_OK;
    }
};

static ESP32I2CBus bus;

static void init_i2c()
{
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = GPIO_NUM_21;
    conf.scl_io_num = GPIO_NUM_22;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
}

extern "C" void app_main()
{
    init_i2c();
    PACL95555 gpio(&bus, 0x20);
    gpio.resetToDefault();
    gpio.setPinDirection(0, GPIODir::Output);

    while (true) {
        gpio.togglePin(0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
