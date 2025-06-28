from esphome import codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from . import IntergasComponent, CONF_INTERGAS

BIN_SENSOR_DEFINITIONS = {
    "alarm": binary_sensor.binary_sensor_schema(icon="mdi:alarm-light", device_class="problem"),
    "pump_on": binary_sensor.binary_sensor_schema(icon="mdi:pump", device_class="running"),
    "tap_switch": binary_sensor.binary_sensor_schema(icon="mdi:shower", device_class="running"),
}

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_INTERGAS): cv.use_id(IntergasComponent),
    **{
        cv.Optional(key): schema for key, schema in BIN_SENSOR_DEFINITIONS.items()
    }
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_INTERGAS])
    for key in BIN_SENSOR_DEFINITIONS:
        if key in config:
            sens = await binary_sensor.new_binary_sensor(config[key])
            cg.add(parent.register_binary_sensor(key, sens))
