from esphome import codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from . import IntergasComponent, CONF_INTERGAS

TEXT_SENSOR_DEFINITIONS = {
    "mode": text_sensor.text_sensor_schema(
        icon="mdi:state-machine"
    ),
}

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_INTERGAS): cv.use_id(IntergasComponent),
    **{
        cv.Optional(key): schema for key, schema in TEXT_SENSOR_DEFINITIONS.items()
    }
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_INTERGAS])
    for key in TEXT_SENSOR_DEFINITIONS:
        if key in config:
            sens = await text_sensor.new_text_sensor(config[key])
            cg.add(parent.register_text_sensor(key, sens))
