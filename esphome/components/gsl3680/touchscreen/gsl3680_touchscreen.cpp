#include "gsl3680_touchscreen.h"
#include "gsl3680_fw.h"
#include "gsl_point_id.h"

#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace gsl3680 {

static const char *const TAG = "gsl3680.touchscreen";
static const size_t MAX_BUTTONS = 2;  // max number of buttons scanned

unsigned int gsl_config_data_id[] = {
    0xccb69a,   0x200,      0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0x1cc86fd6,

    0x40000d00, 0xa,        0xe001a,    0xe001a,    0x3200500,  0,          0x5100,     0x8e00,     0,
    0x320014,   0,          0x14,       0,          0,          0,          0,          0x8,        0x4000,
    0x1000,     0x10170002, 0x10110000, 0,          0,          0x4040404,  0x1b6db688, 0x64,       0xb3000f,
    0xad0019,   0xa60023,   0xa0002d,   0xb3000f,   0xad0019,   0xa60023,   0xa0002d,   0xb3000f,   0xad0019,
    0xa60023,   0xa0002d,   0xb3000f,   0xad0019,   0xa60023,   0xa0002d,   0x804000,   0x90040,    0x90001,
    0,          0,          0,          0,          0,          0,          0x14012c,   0xa003c,    0xa0078,
    0x400,      0x1081,     0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,

    0,                                // key_map
    0x3200384,  0x64,       0x503e8,  // 0
    0,          0,          0,        // 1
    0,          0,          0,        // 2
    0,          0,          0,        // 3
    0,          0,          0,        // 4
    0,          0,          0,        // 5
    0,          0,          0,        // 6
    0,          0,          0,        // 7

    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,

    0x220,      0,          0,          0,          0,          0,          0,          0,          0,
    0x10203,    0x4050607,  0x8090a0b,  0xc0d0e0f,  0x10111213, 0x14151617, 0x18191a1b, 0x1c1d1e1f, 0x20212223,
    0x24252627, 0x28292a2b, 0x2c2d2e2f, 0x30313233, 0x34353637, 0x38393a3b, 0x3c3d3e3f, 0x10203,    0x4050607,
    0x8090a0b,  0xc0d0e0f,  0x10111213, 0x14151617, 0x18191a1b, 0x1c1d1e1f, 0x20212223, 0x24252627, 0x28292a2b,
    0x2c2d2e2f, 0x30313233, 0x34353637, 0x38393a3b, 0x3c3d3e3f,

    0x10203,    0x4050607,  0x8090a0b,  0xc0d0e0f,  0x10111213, 0x14151617, 0x18191a1b, 0x1c1d1e1f, 0x20212223,
    0x24252627, 0x28292a2b, 0x2c2d2e2f, 0x30313233, 0x34353637, 0x38393a3b, 0x3c3d3e3f,

    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,

    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,

    0x10203,    0x4050607,  0x8090a0b,  0xc0d0e0f,  0x10111213, 0x14151617, 0x18191a1b, 0x1c1d1e1f, 0x20212223,
    0x24252627, 0x28292a2b, 0x2c2d2e2f, 0x30313233, 0x34353637, 0x38393a3b, 0x3c3d3e3f,

    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,

    0x3,        0x101,      0,          0x100,      0,          0x20,       0x10,       0x8,        0x4,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,

    0x4,        0,          0,          0,          0,          0,          0,          0,          0x3800680,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,          0,          0,          0,          0,          0,          0,          0,          0,
    0,
};

void GSL3680Touchscreen::setup() {
  ESP_LOGCONFIG(TAG, "Setting up GSL3680 touchscreen...");

  this->display_height_ = this->display_->get_height();
  this->display_width_ = this->display_->get_width();
  this->x_raw_max_ = this->display_width_ - 1;
  this->y_raw_max_ = this->display_height_ - 1;
  if (this->swap_x_y_) {
    std::swap(this->x_raw_max_, this->y_raw_max_);
  }

  ESP_LOGI(TAG, "Display size: %dx%d", this->display_width_, this->display_height_);
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
  }

  if (this->interrupt_pin_ != nullptr) {
    this->interrupt_pin_->setup();
  }

  if (!this->reset_chip_()) {
    ESP_LOGE(TAG, "Failed to reset GSL3680 chip");
    return;
  }

  esp_lcd_touch_gsl3680_clear_reg();
  touch_gsl3680_reset();
  esp_lcd_touch_gsl3680_load_fw();
  esp_lcd_touch_gsl3680_startup_chip();
  touch_gsl3680_reset();
  esp_lcd_touch_gsl3680_startup_chip();

  this->setup_complete_ = true;
  this->setup_done_ = true;
  ESP_LOGCONFIG(TAG, "GSL3680 touchscreen setup completed successfully");
}

bool GSL3680Touchscreen::touch_gsl3680_reset() {
  ESP_LOGV(TAG, "touch_gsl3680_reset");

  unsigned char write_buf[4];
  uint8_t addr;

  reset_pin_->digital_write(0);
  delay(20);
  reset_pin_->digital_write(1);
  delay(20);

  addr = 0xe0;
  write_buf[0] = 0x88;
  touch_gsl3680_i2c_write(addr, write_buf, 1);
  delay(10);
  addr = 0xe4;
  write_buf[0] = 0x04;
  touch_gsl3680_i2c_write(addr, write_buf, 1);
  delay(10);

  write_buf[0] = 0x00;
  write_buf[1] = 0x00;
  write_buf[2] = 0x00;
  write_buf[3] = 0x00;
  touch_gsl3680_i2c_write(0xbc, write_buf, 4);

  delay(10);
  ESP_LOGV(TAG, "touch_gsl3680_reset done");
  return ESP_OK;
}

bool GSL3680Touchscreen::touch_gsl3680_i2c_write(uint8_t reg, const uint8_t *data, size_t len) {
  return this->write_register_(reg, data, len);
}

bool GSL3680Touchscreen::esp_lcd_touch_gsl3680_clear_reg() {
  uint8_t addr;
  uint8_t wrbuf[4];

  ESP_LOGI(TAG, "clear reg");
  addr = 0xe0;
  wrbuf[0] = 0x88;
  touch_gsl3680_i2c_write(addr, wrbuf, 1);
  delay(20);
  addr = 0x88;
  wrbuf[0] = 0x01;
  touch_gsl3680_i2c_write(addr, wrbuf, 1);
  delay(5);
  addr = 0xe4;
  wrbuf[0] = 0x04;
  touch_gsl3680_i2c_write(addr, wrbuf, 1);
  delay(5);
  addr = 0xe0;
  wrbuf[0] = 0x00;
  touch_gsl3680_i2c_write(addr, wrbuf, 1);
  delay(20);
  ESP_LOGI(TAG, "clear reg done");

  return ESP_OK;
}

bool GSL3680Touchscreen::esp_lcd_touch_gsl3680_load_fw() {
  ESP_LOGV(TAG, "start load fw");
  uint16_t addr;
  unsigned char wrbuf[4];
  uint16_t source_line = 0;
  uint16_t source_len = sizeof(GSLX680_FW) / sizeof(struct fw_data);

  for (source_line = 0; source_line < source_len; source_line++) {
    addr = GSLX680_FW[source_line].offset;
    wrbuf[0] = (uint8_t) (GSLX680_FW[source_line].val & 0x000000ff);
    wrbuf[1] = (uint8_t) ((GSLX680_FW[source_line].val & 0x0000ff00) >> 8);
    wrbuf[2] = (uint8_t) ((GSLX680_FW[source_line].val & 0x00ff0000) >> 16);
    wrbuf[3] = (uint8_t) ((GSLX680_FW[source_line].val & 0xff000000) >> 24);
    if (addr == 0xf0)
      touch_gsl3680_i2c_write(addr, wrbuf, 1);
    else
      touch_gsl3680_i2c_write(addr, wrbuf, 4);
  }
  ESP_LOGV(TAG, "load fw success");
  return ESP_OK;
};

bool GSL3680Touchscreen::esp_lcd_touch_gsl3680_startup_chip() {
  esp_err_t ret = ESP_OK;
  uint8_t write_buf[4];
  uint8_t addr = 0xe0;
  write_buf[0] = 0x00;
  ESP_LOGV(TAG, "enter");
  touch_gsl3680_i2c_write(addr, write_buf, 1);
  delay(10);

  gsl_DataInit(gsl_config_data_id);
  ESP_LOGV(TAG, "esp_lcd_touch_gsl3680_startup_chip success");

  return ret;
};

typedef struct _XY_DATA_T {
  unsigned short x_position, y_position;
  unsigned char finger_id;
} XY_DATA_T;
#define TG_UNKNOWN_STATE 3

#define TG_NO_DETECT 0
#define TG_ZOOM_IN 1
#define TG_ZOOM_OUT 2
#define MAX_FINGER_NUM 3
typedef unsigned char TG_STATE_E;

static uint8_t Finger_num = 0;
static XY_DATA_T XY_Coordinate[MAX_FINGER_NUM] = {0};
static TG_STATE_E tpc_gesture_id = TG_UNKNOWN_STATE;

static uint8_t pre_pen_flag = 0;
static uint32_t pre_distance = 0;
static uint32_t distance_flag = 0;
static uint16_t x_new = 0;
static uint16_t y_new = 0;
static uint16_t x_start = 0, y_start = 0;
static uint8_t zoomOutDebounce = 0;
static uint8_t zoomInDebounce = 0;
static portMUX_TYPE lock; /*!< Lock for read/write */

void GSL3680Touchscreen::update_touches() {
  this->skip_update_ = true;  // skip send touch events by default, set to false after successful error checks
  if (!this->setup_done_) {
    return;
  }

  struct gsl_touch_info cinfo = {0};
  unsigned int tmp1 = 0;
  uint8_t buf[4] = {0};

  esp_err_t err;
  uint8_t touch_data[24];
  uint8_t touch_cnt = 0;
  uint16_t x_poit, y_poit, x2_poit, y2_poit;
  uint16_t distance = 0, chazhi = 0;
  size_t i = 0;

  memset(XY_Coordinate, 0, sizeof(XY_Coordinate));

  err = this->read_register_(0x80 /*ESP_LCD_TOUCH_GSL3680_READ_XY_REG*/, touch_data, 24);

  Finger_num = touch_data[0];
  // ESP_LOGI(TAG,"0x80 = %d",touch_data[0]);

  x_poit = ((touch_data[7] & 0x0f) << 8) | touch_data[6];
  y_poit = (touch_data[5] << 8) | touch_data[4];
  x2_poit = ((touch_data[11] & 0x0f) << 8) | touch_data[10];
  y2_poit = (touch_data[9] << 8) | touch_data[8];

  // #ifdef USE_GSL_NOID_VERSION
  cinfo.finger_num = Finger_num;
  cinfo.x[0] = x_poit;
  cinfo.y[0] = y_poit;
  cinfo.id[0] = ((touch_data[7] & 0xf0) >> 4);
  cinfo.x[1] = x2_poit;
  cinfo.y[1] = y2_poit;
  cinfo.id[1] = ((touch_data[11] & 0xf0) >> 4);
  cinfo.finger_num = (touch_data[3] << 24) | (touch_data[2] << 16) | (touch_data[1] << 8) | touch_data[0];

  gsl_alg_id_main(&cinfo);
  tmp1 = gsl_mask_tiaoping();
  // SCI_TRACE_LOW("[tp-gsl] tmp1=%x\n", tmp1);
  if (tmp1 > 0 && tmp1 < 0xffffffff) {
    uint8_t addr = 0xf0;
    buf[0] = 0xa;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0;
    this->write_register_(addr, buf, 4);
    addr = 0x8;
    buf[0] = (uint8_t) (tmp1 & 0xff);
    buf[1] = (uint8_t) ((tmp1 >> 8) & 0xff);
    buf[2] = (uint8_t) ((tmp1 >> 16) & 0xff);
    buf[3] = (uint8_t) ((tmp1 >> 24) & 0xff);
    // SCI_TRACE_LOW("tmp1=%08x,buf[0]=%02x,buf[1]=%02x,buf[2]=%02x,buf[3]=%02x\n", tmp1,buf[0],buf[1],buf[2],buf[3]);
    this->write_register_(addr, buf, 4);
  }
  Finger_num = cinfo.finger_num;
  // #endif

  // #ifdef USE_GSL_NOID_VERSION
  XY_Coordinate[0].x_position = cinfo.x[0];
  XY_Coordinate[0].y_position = cinfo.y[0];
  XY_Coordinate[0].finger_id = cinfo.id[0];
  XY_Coordinate[1].x_position = cinfo.x[1];
  XY_Coordinate[1].y_position = cinfo.y[1];
  XY_Coordinate[1].finger_id = cinfo.id[1];

  this->skip_update_ = false;  // All error checks passed, send touch events
  for (uint8_t i = 0; i != Finger_num; i++) {
    uint16_t id = XY_Coordinate[i].finger_id;
    uint16_t x = XY_Coordinate[i].x_position;
    uint16_t y = XY_Coordinate[i].y_position;
    x = this->display_width_ - x;
    this->add_raw_touch_position_(id, x, y);
    ESP_LOGVV(TAG, "positions read x=%d,y=%d,id=%d", x, y, id);

    if (x > this->x_raw_max_)
      x = this->x_raw_max_;
    if (y > this->y_raw_max_)
      y = this->y_raw_max_;
  }

  auto keys = XY_Coordinate[0].finger_id & ((1 << MAX_BUTTONS) - 1);
  if (keys != this->button_state_) {
    this->button_state_ = keys;
    for (size_t i = 0; i != MAX_BUTTONS; i++) {
      for (auto *listener : this->button_listeners_)
        listener->update_button(i, (keys & (1 << i)) != 0);
    }
  }

  return;
}

void GSL3680Touchscreen::dump_config() {
  ESP_LOGCONFIG(TAG, "GSL3680 Touchscreen:");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
  ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->address_);
}

bool GSL3680Touchscreen::hardware_reset_() {
  if (this->reset_pin_ == nullptr) {
    ESP_LOGE(TAG, "Reset pin not configured");
    return false;
  }

  ESP_LOGD(TAG, "Performing hardware reset");
  this->reset_pin_->digital_write(0);
  this->interrupt_pin_->digital_write(0);
  delay(10);

  ESP_LOGD(TAG, "Performing hardware reset 2");
  this->interrupt_pin_->digital_write(0);
  delay(1);
  this->reset_pin_->digital_write(1);
  delay(10);
  delay(50);

  return true;
}

bool GSL3680Touchscreen::software_reset_() {
  ESP_LOGD(TAG, "Performing software reset");

  if (!this->write_register_(GSL3680_RESET_REG, GSL3680_RESET_VALUE)) {
    ESP_LOGE(TAG, "Failed to write reset register");
    return false;
  }

  delay(RESET_DELAY_MS);
  return true;
}

bool GSL3680Touchscreen::clear_register_() {
  ESP_LOGD(TAG, "Clearing control register");

  if (!this->write_register_(GSL3680_RESET_REG, GSL3680_CLEAR_VALUE)) {
    ESP_LOGE(TAG, "Failed to clear control register");
    return false;
  }

  delay(10);
  return true;
}

bool GSL3680Touchscreen::reset_chip_() {
  ESP_LOGD(TAG, "Resetting GSL3680 chip");

  if (!this->hardware_reset_()) {
    return false;
  }
  uint8_t buf[4];
  uint8_t write[4];
  uint8_t i2c_buffer_read = 0;
  uint8_t i2c_buffer_write = 0x12;
  esp_err_t ret = ESP_OK;

  write[0] = 0x12;
  write[1] = 0x34;
  write[2] = 0x56;

  this->read_register_(0xf0, (uint8_t *) &buf, 4);  // Dummy read to stabilize I2C bus
  ESP_LOGVV(TAG, "gsl3680 read reg 0xf0 before is %x %x %x %x", buf[0], buf[1], buf[2], buf[3]);
  delay(20);

  ESP_LOGVV(TAG, "gsl3680 writing 0xf0 0x12");
  write_register_(0xf0, write, 4);
  delay(20);

  this->read_register_(0xf0, (uint8_t *) &buf, 4);  // Dummy read to stabilize I2C bus
  ESP_LOGVV(TAG, "gsl3680 read reg 0xf0 after is %x %x %x %x", buf[0], buf[1], buf[2], buf[3]);

  if (i2c_buffer_read == i2c_buffer_write) {
    ESP_LOGV(TAG, "read cfg success");
    return true;
  }

  ESP_LOGD(TAG, "Chip reset completed");
  return true;
}

bool GSL3680Touchscreen::startup_chip_() {
  ESP_LOGD(TAG, "Starting GSL3680 chip");

  if (!this->write_register_(GSL3680_STARTUP_REG, GSL3680_STARTUP_VALUE)) {
    ESP_LOGE(TAG, "Failed to write startup register");
    return false;
  }

  delay(STARTUP_DELAY_MS);
  ESP_LOGD(TAG, "Chip startup completed");
  return true;
}

bool GSL3680Touchscreen::write_register_(uint8_t reg, uint8_t value) { return this->write_register_(reg, &value, 1); }

bool GSL3680Touchscreen::write_register_(uint8_t reg, const uint8_t *data, size_t len) {
  auto err = this->write_register(reg, data, len);
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "I2C write failed for register 0x%02X: %s", reg,
             err == i2c::ERROR_TIMEOUT ? "timeout" : "unknown error");
    return false;
  }
  return true;
}

bool GSL3680Touchscreen::read_register_(uint8_t reg, uint8_t *data, size_t len) {
  auto err = this->read_register(reg, data, len);
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "I2C read failed for register 0x%02X: %s", reg,
             err == i2c::ERROR_TIMEOUT ? "timeout" : "unknown error");
    return false;
  }
  return true;
}

}  // namespace gsl3680
}  // namespace esphome
