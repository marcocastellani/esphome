#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace gsl3680 {

// GSL3680 Register definitions
static const uint8_t GSL3680_DATA_REG = 0x80;
static const uint8_t GSL3680_STATUS_REG = 0xE0;
static const uint8_t GSL3680_PAGE_REG = 0xF0;
static const uint8_t GSL3680_RESET_REG = 0xE4;
static const uint8_t GSL3680_STARTUP_REG = 0xE0;

// Control values
static const uint8_t GSL3680_RESET_VALUE = 0x88;
static const uint8_t GSL3680_CLEAR_VALUE = 0x03;
static const uint8_t GSL3680_STARTUP_VALUE = 0x00;

class GSL3680ButtonListener {
 public:
  virtual void update_button(uint8_t index, bool state) = 0;
};

class GSL3680Touchscreen : public touchscreen::Touchscreen, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  bool can_proceed() override { return this->setup_done_; }

  void set_interrupt_pin(InternalGPIOPin *pin) { this->interrupt_pin_ = pin; }
  void set_reset_pin(GPIOPin *pin) { this->reset_pin_ = pin; }

 protected:
  void update_touches() override;

  /// @brief Perform the internal setup routine for the GT911 touchscreen.
  ///
  /// This function checks the I2C address, configures the interrupt pin (if available),
  /// reads the touchscreen mode from the controller, and attempts to read calibration
  /// data (maximum X and Y values) if not already set.
  ///
  /// On success, sets @ref setup_done_ to true.
  /// On failure, calls @ref mark_failed() with an appropriate error message.
  void setup_internal_();
  /// @brief True if the touchscreen setup has completed successfully.
  bool setup_done_{false};

  InternalGPIOPin *interrupt_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  std::vector<GSL3680ButtonListener *> button_listeners_;
  uint8_t button_state_{0xFF};  // last button state. Initial FF guarantees first update.

  bool reset_chip_();
  bool hardware_reset_();
  bool software_reset_();
  bool clear_register_();
  bool startup_chip_();

  bool write_register_(uint8_t reg, uint8_t value);
  bool write_register_(uint8_t reg, const uint8_t *data, size_t len);
  bool touch_gsl3680_i2c_write(uint8_t reg, const uint8_t *data, size_t len);
  bool read_register_(uint8_t reg, uint8_t *data, size_t len);

  bool setup_complete_{false};

  static const uint32_t STARTUP_DELAY_MS = 20;
  static const uint32_t RESET_DELAY_MS = 20;

  bool esp_lcd_touch_gsl3680_clear_reg();
  bool touch_gsl3680_reset();
  bool esp_lcd_touch_gsl3680_load_fw();
  bool esp_lcd_touch_gsl3680_startup_chip();
};

}  // namespace gsl3680
}  // namespace esphome
