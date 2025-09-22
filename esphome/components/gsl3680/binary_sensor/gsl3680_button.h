#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/gsl3680/touchscreen/gsl3680_touchscreen.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace gsl3680 {

class GSL3680Button : public binary_sensor::BinarySensor,
                      public Component,
                      public GSL3680ButtonListener,
                      public Parented<GSL3680Touchscreen> {
 public:
  void setup() override;
  void dump_config() override;

  void set_index(uint8_t index) { this->index_ = index; }

  void update_button(uint8_t index, bool state) override;

 protected:
  uint8_t index_;
};

}  // namespace gsl3680
}  // namespace esphome
