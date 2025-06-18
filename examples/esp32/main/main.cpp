#include "../../common/DummyBus.hpp"
#include "pcal95555.hpp"

extern "C" void app_main(void) {
  DummyBus bus;
  PCAL95555 gpio(&bus, 0x20);
  gpio.resetToDefault();
  gpio.setPinDirection(0, GPIODir::Output);
  gpio.writePin(0, true);
}
