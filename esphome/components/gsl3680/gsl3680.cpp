#include "gsl3680.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "gsl3680_fw.h"

namespace esphome {
namespace gsl3680 {

static const char *const TAG = "gsl3680";

void GSL3680Touchscreen::setup() {
  ESP_LOGCONFIG(TAG, "Setting up GSL3680 touchscreen...");
  
  // First configure pins for initialization sequence
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->reset_pin_->pin_mode(gpio::FLAG_OUTPUT);
    ESP_LOGD(TAG, "Reset pin configured");
  }
  
  if (this->interrupt_pin_ != nullptr) {
    this->interrupt_pin_->setup();
    // During initialization, INT pin must be OUTPUT (as per manufacturer code)
    this->interrupt_pin_->pin_mode(gpio::FLAG_OUTPUT);
    ESP_LOGD(TAG, "Interrupt pin configured as OUTPUT for initialization");
  }
  
  // Perform hardware reset with proper pin sequence
  this->hardware_reset_sequence_();
  delay(100);  // Give more time after reset
  
  if (!this->init_chip_()) {
    ESP_LOGE(TAG, "Failed to initialize GSL3680");
    this->mark_failed();
    return;
  }
  
  // After initialization, configure interrupt pin as INPUT
  if (this->interrupt_pin_ != nullptr) {
    this->interrupt_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
    ESP_LOGD(TAG, "Interrupt pin reconfigured as INPUT");
  }
  
  ESP_LOGCONFIG(TAG, "GSL3680 setup complete");
}

void GSL3680Touchscreen::hardware_reset_sequence_() {
  ESP_LOGD(TAG, "Performing hardware reset sequence...");
  
  if (this->reset_pin_ != nullptr && this->interrupt_pin_ != nullptr) {
    // Set reset low and interrupt low
    this->reset_pin_->digital_write(false);
    this->interrupt_pin_->digital_write(false);
    delay(10);
    
    // Set interrupt to select I2C address (0 for 0x40)
    this->interrupt_pin_->digital_write(false);  // 0x40 address
    delay(1);
    
    // Release reset
    this->reset_pin_->digital_write(true);
    delay(10);
    
    delay(50);  // Wait for chip to be ready
    ESP_LOGD(TAG, "Hardware reset sequence completed");
  } else if (this->reset_pin_ != nullptr) {
    // Simple reset if no interrupt pin
    this->reset_pin_->digital_write(false);
    delay(20);
    this->reset_pin_->digital_write(true);
    delay(50);
    ESP_LOGD(TAG, "Simple hardware reset completed");
  } else {
    ESP_LOGD(TAG, "No reset pin configured, will try software reset only");
  }
}

bool GSL3680Touchscreen::clear_registers_() {
  ESP_LOGD(TAG, "Clearing registers...");
  
  uint8_t wrbuf[4];
  
  // Step 1: Write 0x88 to register 0xe0
  wrbuf[0] = 0x88;
  if (!this->write_register(0xe0, wrbuf, 1)) {
    ESP_LOGW(TAG, "Failed to write 0x88 to register 0xe0");
    return false;
  }
  delay(20);
  
  // Step 2: Write 0x01 to register 0x88
  wrbuf[0] = 0x01;
  if (!this->write_register(0x88, wrbuf, 1)) {
    ESP_LOGW(TAG, "Failed to write 0x01 to register 0x88");
    return false;
  }
  delay(5);
  
  // Step 3: Write 0x04 to register 0xe4
  wrbuf[0] = 0x04;
  if (!this->write_register(0xe4, wrbuf, 1)) {
    ESP_LOGW(TAG, "Failed to write 0x04 to register 0xe4");
    return false;
  }
  delay(5);
  
  // Step 4: Write 0x00 to register 0xe0
  wrbuf[0] = 0x00;
  if (!this->write_register(0xe0, wrbuf, 1)) {
    ESP_LOGW(TAG, "Failed to write 0x00 to register 0xe0");
    return false;
  }
  delay(20);
  
  ESP_LOGD(TAG, "Registers cleared successfully");
  return true;
}

void GSL3680Touchscreen::reset_chip_() {
  ESP_LOGD(TAG, "Performing chip reset...");
  
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->digital_write(false);
    delay(20);
    this->reset_pin_->digital_write(true);
    delay(20);
  }
  
  // Additional software reset steps
  uint8_t wrbuf[4];
  
  wrbuf[0] = 0x88;
  this->write_register(0xe0, wrbuf, 1);
  delay(10);
  
  wrbuf[0] = 0x04;
  this->write_register(0xe4, wrbuf, 1);
  delay(10);
  
  wrbuf[0] = 0x00; wrbuf[1] = 0x00; wrbuf[2] = 0x00; wrbuf[3] = 0x00;
  this->write_register(0xbc, wrbuf, 4);
  delay(10);
  
  ESP_LOGD(TAG, "Chip reset completed");
}

bool GSL3680Touchscreen::startup_chip_() {
  ESP_LOGD(TAG, "Starting up chip...");
  
  uint8_t wrbuf[1];
  wrbuf[0] = 0x00;
  
  if (!this->write_register(0xe0, wrbuf, 1)) {
    ESP_LOGE(TAG, "Failed to startup chip");
    return false;
  }
  delay(10);
  
  ESP_LOGD(TAG, "Chip startup completed");
  return true;
}

bool GSL3680Touchscreen::init_chip_() {
  ESP_LOGD(TAG, "Initializing GSL3680 chip following manufacturer sequence...");
  
  // Step 1: Clear registers
  if (!this->clear_registers_()) {
    ESP_LOGE(TAG, "Failed to clear registers");
    return false;
  }
  
  // Step 2: Reset chip
  this->reset_chip_();
  
  // Step 3: Load firmware
  if (!this->load_firmware_()) {
    ESP_LOGE(TAG, "Failed to load firmware");
    return false;
  }
  
  // Step 4: Startup chip
  if (!this->startup_chip_()) {
    ESP_LOGE(TAG, "Failed to startup chip");
    return false;
  }
  
  // Step 5: Reset chip again
  this->reset_chip_();
  
  // Step 6: Startup chip again
  if (!this->startup_chip_()) {
    ESP_LOGE(TAG, "Failed to startup chip (second time)");
    return false;
  }
  
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
    if (i % 200 == 0) {
      ESP_LOGV(TAG, "Firmware loading progress: %zu/%zu", i, sizeof(GSLX680_FW) / sizeof(GSLX680_FW[0]));
    }
  }
  
  ESP_LOGD(TAG, "Firmware loaded successfully");
  return true;
}

void GSL3680Touchscreen::loop() {
  // Check interrupt pin
  if (this->interrupt_pin_ != nullptr && this->interrupt_pin_->digital_read()) {
    return;  // No touch event (interrupt is active low)
  }
  
  this->update_touches();
}

void GSL3680Touchscreen::read_touches_() {
  uint8_t touch_data[24];
  
  if (!this->read_register(0x80, touch_data, 24)) {
    ESP_LOGV(TAG, "Failed to read touch data");
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
  
  ESP_LOGD(TAG, "Touch detected: x=%d, y=%d, id=%d, fingers=%d", x, y, id, finger_num);
  
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
