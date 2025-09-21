#include "gsl3680.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "gsl3680_fw.h"

namespace esphome {
namespace gsl3680 {

static const char *const TAG = "gsl3680";

void GSL3680Touchscreen::setup() {
  ESP_LOGCONFIG(TAG, "Setting up GSL3680 touchscreen...");
  
  // Configure pins
  if (this->interrupt_pin_ != nullptr) {
    this->interrupt_pin_->setup();
    this->interrupt_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
    ESP_LOGD(TAG, "Interrupt pin configured");
  }
  
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->reset_pin_->pin_mode(gpio::FLAG_OUTPUT);
    ESP_LOGD(TAG, "Reset pin configured");
  }
  
  // Test I2C communication first
  ESP_LOGD(TAG, "Testing I2C communication...");
  uint8_t test_data[4];
  if (!this->read_register(0xf0, test_data, 4)) {
    ESP_LOGE(TAG, "I2C communication test failed");
    this->mark_failed();
    return;
  }
  ESP_LOGD(TAG, "I2C communication OK, read: %02X %02X %02X %02X", test_data[0], test_data[1], test_data[2], test_data[3]);
  
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
  ESP_LOGD(TAG, "Resetting GSL3680 chip...");
  
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->digital_write(false);
    delay(20);
    this->reset_pin_->digital_write(true);
    delay(20);
    ESP_LOGD(TAG, "Hardware reset completed");
  }
  
  // Software reset sequence
  uint8_t reset_data[] = {0x88};
  if (!this->write_register(0xe0, reset_data, 1)) {
    ESP_LOGW(TAG, "Software reset failed");
    return;
  }
  delay(10);
  
  uint8_t clear_data[] = {0x04};
  if (!this->write_register(0xe4, clear_data, 1)) {
    ESP_LOGW(TAG, "Clear register failed");
    return;
  }
  delay(10);
  
  uint8_t zero_data[] = {0x00, 0x00, 0x00, 0x00};
  if (!this->write_register(0xbc, zero_data, 4)) {
    ESP_LOGW(TAG, "Clear BC register failed");
    return;
  }
  delay(10);
  
  ESP_LOGD(TAG, "Software reset completed");
}

bool GSL3680Touchscreen::init_chip_() {
  ESP_LOGD(TAG, "Initializing GSL3680 chip...");
  
  // Clear registers
  uint8_t clear_reg_data[] = {0x88};
  if (!this->write_register(0xe0, clear_reg_data, 1)) {
    ESP_LOGE(TAG, "Failed to write to register 0xe0");
    return false;
  }
  delay(20);
  
  uint8_t data1[] = {0x01};
  if (!this->write_register(0x88, data1, 1)) {
    ESP_LOGE(TAG, "Failed to write to register 0x88");
    return false;
  }
  delay(5);
  
  uint8_t data2[] = {0x04};
  if (!this->write_register(0xe4, data2, 1)) {
    ESP_LOGE(TAG, "Failed to write to register 0xe4");
    return false;
  }
  delay(5);
  
  uint8_t data3[] = {0x00};
  if (!this->write_register(0xe0, data3, 1)) {
    ESP_LOGE(TAG, "Failed to clear register 0xe0");
    return false;
  }
  delay(20);
  
  // Load firmware
  if (!this->load_firmware_()) {
    ESP_LOGE(TAG, "Failed to load firmware");
    return false;
  }
  
  // Startup chip
  uint8_t startup_data[] = {0x00};
  if (!this->write_register(0xe0, startup_data, 1)) {
    ESP_LOGE(TAG, "Failed to startup chip");
    return false;
  }
  delay(10);
  
  // Check if chip is responding
  delay(30);
  uint8_t check_data[4];
  if (this->read_register(0xb0, check_data, 4)) {
    ESP_LOGD(TAG, "Chip check: %02X %02X %02X %02X", check_data[0], check_data[1], check_data[2], check_data[3]);
    if (check_data[3] == 0x5a && check_data[2] == 0x5a && check_data[1] == 0x5a && check_data[0] == 0x5a) {
      ESP_LOGD(TAG, "Chip initialization successful");
      return true;
    }
  }
  
  ESP_LOGW(TAG, "Chip check failed, but continuing...");
  return true;
}

bool GSL3680Touchscreen::load_firmware_() {
  ESP_LOGD(TAG, "Loading GSL3680 firmware (%zu entries)...", sizeof(GSLX680_FW) / sizeof(GSLX680_FW[0]));
  
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
      if (!this->write_register(addr, wrbuf, 1)) {
        ESP_LOGE(TAG, "Failed to write firmware at index %zu (addr 0x%02X)", i, addr);
        return false;
      }
    } else {
      if (!this->write_register(addr, wrbuf, 4)) {
        ESP_LOGE(TAG, "Failed to write firmware at index %zu (addr 0x%02X)", i, addr);
        return false;
      }
    }
    
    // Progress indicator
    if (i % 100 == 0) {
      ESP_LOGV(TAG, "Firmware loading progress: %zu/%zu", i, sizeof(GSLX680_FW) / sizeof(GSLX680_FW[0]));
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
  
  this->update_touches();
}

void GSL3680Touchscreen::read_touches_() {
  uint8_t touch_data[24];
  
  if (!this->read_register(0x80, touch_data, 24)) {
    ESP_LOGW(TAG, "Failed to read touch data");
    return;
  }
  
  uint8_t finger_num = touch_data[0];
  
  if (finger_num == 0) {
    // No touch - clear any existing touches
    return;
  }
  
  if (finger_num > 10) {
    ESP_LOGW(TAG, "Invalid finger count: %d", finger_num);
    return;
  }
  
  // Process first touch point
  uint16_t x = ((touch_data[7] & 0x0F) << 8) | touch_data[6];
  uint16_t y = (touch_data[5] << 8) | touch_data[4];
  uint8_t id = (touch_data[7] & 0xF0) >> 4;
  
  ESP_LOGV(TAG, "Touch detected: x=%d, y=%d, id=%d, fingers=%d", x, y, id, finger_num);
  
  // Add the touch point using the standard touchscreen API
  this->add_raw_touch_position_(id, x, y);
}

void GSL3680Touchscreen::update_touches() {
  this->read_touches_();
}

void GSL3680Touchscreen::dump_config() {
  ESP_LOGCONFIG(TAG, "GSL3680 Touchscreen:");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  ESP_LOGCONFIG(TAG, "  Firmware entries: %zu", sizeof(GSLX680_FW) / sizeof(GSLX680_FW[0]));
}

}  // namespace gsl3680
}  // namespace esphome
