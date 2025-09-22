import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_INDEX

from .. import gsl3680_ns
from ..touchscreen import GSL3680ButtonListener, GSL3680Touchscreen

CONF_GSL3680_ID = "gsl3680_id"

GSL3680Button = gsl3680_ns.class_(
    "GSL3680Button",
    binary_sensor.BinarySensor,
    cg.Component,
    GSL3680ButtonListener,
    cg.Parented.template(GSL3680Touchscreen),
)

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(GSL3680Button).extend(
    {
        cv.GenerateID(CONF_GSL3680_ID): cv.use_id(GSL3680Touchscreen),
        cv.Optional(CONF_INDEX, default=0): cv.int_range(min=0, max=3),
    }
)


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_GSL3680_ID])
    cg.add(var.set_index(config[CONF_INDEX]))
