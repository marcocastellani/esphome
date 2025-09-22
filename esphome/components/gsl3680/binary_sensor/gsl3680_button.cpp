#include "gsl3680_button.h"
#include "esphome/core/log.h"

namespace esphome {
namespace gsl3680 {

static const char *const TAG = "GSL3680.binary_sensor";

void GSL3680Button::setup() {
  this->parent_->register_button_listener(this);
  this->publish_initial_state(false);
}

void GSL3680Button::dump_config() {
  LOG_BINARY_SENSOR("", "GSL3680 Button", this);
  ESP_LOGCONFIG(TAG, "  Index: %u", this->index_);
}

void GSL3680Button::update_button(uint8_t index, bool state) {
  if (index != this->index_)
    return;

  this->publish_state(state);
}

}  // namespace gsl3680
}  // namespace esphome
