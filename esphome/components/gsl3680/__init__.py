import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID

CODEOWNERS = ["@marcocastellani"]
DEPENDENCIES = ["i2c"]

gsl3680_ns = cg.esphome_ns.namespace("gsl3680")
GSL3680Component = gsl3680_ns.class_("GSL3680Component", cg.Component, i2c.I2CDevice)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(GSL3680Component),
}).extend(cv.COMPONENT_SCHEMA).extend(i2c.i2c_device_schema(0x40))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
