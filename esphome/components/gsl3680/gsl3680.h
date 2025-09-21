#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace gsl3680 {

class GSL3680Touchscreen : public touchscreen::Touchscreen, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  void loop() override;
  void update_touches() override;
  
  void set_interrupt_pin(GPIOPin *pin) { this->interrupt_pin_ = pin; }
  void set_reset_pin(GPIOPin *pin) { this->reset_pin_ = pin; }

 protected:
  GPIOPin *interrupt_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  
  bool init_chip_();
  bool load_firmware_();
  void reset_chip_();
  void read_touches_();
};

}  // namespace gsl3680
}  // namespace esphome
