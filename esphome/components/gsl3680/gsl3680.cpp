#include "gsl3680.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "gsl3680_fw.h"  // Firmware data

namespace esphome {
namespace gsl3680 {

static const char *const TAG = "gsl3680";

void GSL3680Touchscreen::setup() {
  ESP_LOGCONFIG(TAG, "Setting up GSL3680 touchscreen...");
  
  // Configure pins
  if (this->interrupt_pin_ != nullptr) {
    this->interrupt_pin_->setup();
    this->interrupt_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
  }
  
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->reset_pin_->pin_mode(gpio::FLAG_OUTPUT);
  }
  
  // Reset and initialize chip
  this->reset_chip_();
  delay(50);
  
  if (!this->init_chip_()) {
    ESP_LOGE(TAG, "Failed to initialize GSL3680");
    this->mark_failed();
    return;
  }
  
  ESP_LOGCONFIG(TAG, "GSL3680 setup complete");
}

void GSL3680Touchscreen::reset_chip_() {
  if (this->reset_pin_ == nullptr) return;
  
  this->reset_pin_->digital_write(false);
  delay(20);
  this->reset_pin_->digital_write(true);
  delay(20);
}

bool GSL3680Touchscreen::init_chip_() {
  // Clear registers
  uint8_t clear_reg_data[] = {0x88};
  if (!this->write_register(0xe0, clear_reg_data, 1)) return false;
  delay(20);
  
  uint8_t data1[] = {0x01};
  if (!this->write_register(0x88, data1, 1)) return false;
  delay(5);
  
  uint8_t data2[] = {0x04};
  if (!this->write_register(0xe4, data2, 1)) return false;
  delay(5);
  
  uint8_t data3[] = {0x00};
  if (!this->write_register(0xe0, data3, 1)) return false;
  delay(20);
  
  // Load firmware
  if (!this->load_firmware_()) return false;
  
  // Startup chip
  uint8_t startup_data[] = {0x00};
  if (!this->write_register(0xe0, startup_data, 1)) return false;
  delay(10);
  
  return true;
}

bool GSL3680Touchscreen::load_firmware_() {
  ESP_LOGD(TAG, "Loading GSL3680 firmware...");
  
  // Load firmware from gsl3680_fw.h
  for (size_t i = 0; i < sizeof(GSLX680_FW) / sizeof(GSLX680_FW[0]); i++) {
    uint8_t wrbuf[4];
    uint16_t addr = GSLX680_FW[i].offset;
    uint32_t val = GSLX680_FW[i].val;
    
    wrbuf[0] = (uint8_t)(val & 0xFF);
    wrbuf[1] = (uint8_t)((val >> 8) & 0xFF);
    wrbuf[2] = (uint8_t)((val >> 16) & 0xFF);
    wrbuf[3] = (uint8_t)((val >> 24) & 0xFF);
    
    if (addr == 0xf0) {
      if (!this->write_register(addr, wrbuf, 1)) return false;
    } else {
      if (!this->write_register(addr, wrbuf, 4)) return false;
    }
  }
  
  ESP_LOGD(TAG, "Firmware loaded successfully");
  return true;
}

void GSL3680Touchscreen::loop() {
  // Check interrupt pin
  if (this->interrupt_pin_ != nullptr && this->interrupt_pin_->digital_read()) {
    return;  // No touch event
  }
  
  this->read_touches_();
}

void GSL3680Touchscreen::read_touches_() {
  uint8_t touch_data[24];
  
  if (!this->read_register(0x80, touch_data, 24)) {
    ESP_LOGW(TAG, "Failed to read touch data");
    return;
  }
  
  uint8_t finger_num = touch_data[0];
  
  if (finger_num == 0) {
    // No touch
    return;
  }
  
  // Process first touch point
  uint16_t x = ((touch_data[7] & 0x0F) << 8) | touch_data[6];
  uint16_t y = (touch_data[5] << 8) | touch_data[4];
  
  ESP_LOGV(TAG, "Touch detected: x=%d, y=%d, fingers=%d", x, y, finger_num);
  
  touchscreen::TouchPoint touch_point;
  touch_point.x = x;
  touch_point.y = y;
  touch_point.id = (touch_data[7] & 0xF0) >> 4;
  touch_point.state = touchscreen::TOUCH_STATE_PRESSED;
  
  this->defer([this, touch_point]() { this->send_touch_(touch_point); });
}

void GSL3680Touchscreen::dump_config() {
  ESP_LOGCONFIG(TAG, "GSL3680 Touchscreen:");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
}

}  // namespace gsl3680
}  // namespace esphome
