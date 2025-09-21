from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import touchscreen, i2c
from esphome.const import (
    CONF_ID,
    CONF_INTERRUPT_PIN,
    CONF_RESET_PIN,
)

CODEOWNERS = ["@marcocastellani"]
DEPENDENCIES = ["i2c"]

gsl3680_ns = cg.esphome_ns.namespace("gsl3680")
GSL3680Touchscreen = gsl3680_ns.class_(
    "GSL3680Touchscreen", touchscreen.Touchscreen, cg.Component, i2c.I2CDevice
)

CONFIG_SCHEMA = touchscreen.TOUCHSCREEN_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(GSL3680Touchscreen),
        cv.Optional(CONF_INTERRUPT_PIN): pins.internal_gpio_input_pin_schema,
        cv.Optional(CONF_RESET_PIN): pins.gpio_output_pin_schema,
    }
).extend(i2c.i2c_device_schema(0x40))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await touchscreen.register_touchscreen(var, config)
    await i2c.register_i2c_device(var, config)
    
    if interrupt_pin := config.get(CONF_INTERRUPT_PIN):
        cg.add(var.set_interrupt_pin(await cg.gpio_pin_expression(interrupt_pin)))
    if reset_pin := config.get(CONF_RESET_PIN):
        cg.add(var.set_reset_pin(await cg.gpio_pin_expression(reset_pin)))
