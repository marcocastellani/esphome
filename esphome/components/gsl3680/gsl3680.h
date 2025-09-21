#pragma once

#include "esphome/core/component.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/gpio.h"

namespace esphome {
namespace gsl3680 {

class GSL3680Touchscreen : public touchscreen::Touchscreen, public i2c::I2CDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  
  void set_reset_pin(GPIOPin *reset_pin) { reset_pin_ = reset_pin; }
  void set_interrupt_pin(GPIOPin *interrupt_pin) { interrupt_pin_ = interrupt_pin; }

 protected:
  void update_touches() override;
  void read_touches_();
  
  void hardware_reset_sequence_();
  bool init_chip_();
  bool clear_registers_();
  void reset_chip_();
  bool startup_chip_();
  bool load_firmware_();
  
  GPIOPin *reset_pin_{nullptr};
  GPIOPin *interrupt_pin_{nullptr};
};

}  // namespace gsl3680
}  // namespace esphome
