# Platform Integration 🧩

`PCAL95555` abstracts the I²C operations behind the `I2cInterface` interface.
To use the library on a new platform you must implement this interface.

Each platform shown below demonstrates the minimum required methods. You can
adapt the approach to any framework as long as your implementation performs the
read and write transactions expected by the driver.

## ESP32 (ESP-IDF)

```cpp
class ESP32I2CBus : public pcal95555::I2cInterface<ESP32I2CBus> {
    bool write(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) override {
        return i2c_master_write_to_device(I2C_NUM_0, addr, &reg, 1, 50/portTICK_PERIOD_MS) == ESP_OK &&
               i2c_master_write_to_device(I2C_NUM_0, addr, data, len, 50/portTICK_PERIOD_MS) == ESP_OK;
    }
    bool read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) override {
        return i2c_master_write_read_device(I2C_NUM_0, addr, &reg, 1, data, len, 50/portTICK_PERIOD_MS) == ESP_OK;
    }
};
```

## Arduino (Wire)

```cpp
class ArduinoI2CBus : public pcal95555::I2cInterface<ArduinoI2CBus> {
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
        for (size_t i = 0; i < len; ++i) data[i] = Wire.read();
        return true;
    }
};
```

## STM32 (HAL)

```cpp
class STM32I2CBus : public pcal95555::I2cInterface<STM32I2CBus> {
    bool write(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) override {
        return HAL_I2C_Mem_Write(&hi2c1, addr<<1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t*)data, len,
HAL_MAX_DELAY) == HAL_OK;
    }
    bool read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) override {
        return HAL_I2C_Mem_Read(&hi2c1, addr<<1, reg, I2C_MEMADD_SIZE_8BIT, data, len, HAL_MAX_DELAY) == HAL_OK;
    }
};
```

Use the interface that matches your platform or adapt one of these examples.

---

**Navigation**
⬅️ [API Reference](./api_reference.md) • [Back to Index](./index.md) • ➡️ [Examples](./examples.md)
