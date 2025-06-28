from esphome.components import sensor
from esphome import config_validation as cv
import esphome.codegen as cg
from esphome.const import (
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    ICON_THERMOMETER,
)
from . import IntergasComponent, CONF_INTERGAS

SENSOR_DEFINITIONS = {
    # generic
    "fault_code":   sensor.sensor_schema(icon="mdi:code-tags"),
    "last_fault":   sensor.sensor_schema(icon="mdi:code-tags"),

    # temperature readings
    "heater": sensor.sensor_schema(icon="mdi:gas-burner", unit_of_measurement=UNIT_CELSIUS, 
                                   device_class=DEVICE_CLASS_TEMPERATURE, state_class=STATE_CLASS_MEASUREMENT, 
                                   accuracy_decimals=1
    ),
    "inlet": sensor.sensor_schema(icon="mdi:gas-burner", unit_of_measurement=UNIT_CELSIUS, 
                                   device_class=DEVICE_CLASS_TEMPERATURE, state_class=STATE_CLASS_MEASUREMENT, 
                                   accuracy_decimals=1
    ),
    "outlet": sensor.sensor_schema(icon="mdi:gas-burner", unit_of_measurement=UNIT_CELSIUS, 
                                   device_class=DEVICE_CLASS_TEMPERATURE, state_class=STATE_CLASS_MEASUREMENT, 
                                   accuracy_decimals=1
    ),
    "dhw": sensor.sensor_schema(icon="mdi:thermometer-water", unit_of_measurement=UNIT_CELSIUS, 
                                   device_class=DEVICE_CLASS_TEMPERATURE, state_class=STATE_CLASS_MEASUREMENT, 
                                   accuracy_decimals=1
    ),
    "set_point": sensor.sensor_schema(icon="mdi:thermostat-box", unit_of_measurement=UNIT_CELSIUS, 
                                   device_class=DEVICE_CLASS_TEMPERATURE, state_class=STATE_CLASS_MEASUREMENT, 
                                   accuracy_decimals=1
    ),
    "room_set": sensor.sensor_schema(icon="mdi:thermostat", unit_of_measurement=UNIT_CELSIUS, 
                                   device_class=DEVICE_CLASS_TEMPERATURE, state_class=STATE_CLASS_MEASUREMENT, 
                                   accuracy_decimals=1
    ),
    "room": sensor.sensor_schema(icon="mdi:home-thermometer", unit_of_measurement=UNIT_CELSIUS, 
                                   device_class=DEVICE_CLASS_TEMPERATURE, state_class=STATE_CLASS_MEASUREMENT, 
                                   accuracy_decimals=1
    ),
    # operations
    "pressure": sensor.sensor_schema(icon="mdi:gauge", unit_of_measurement="bar",
                                   device_class="pressure",  state_class=STATE_CLASS_MEASUREMENT, 
                                   accuracy_decimals=1
    ),
    "fan_rpm": sensor.sensor_schema(icon="mdi:fan", unit_of_measurement="rpm",
                                   device_class="speed",  state_class=STATE_CLASS_MEASUREMENT, 
                                   accuracy_decimals=1
    ),
    "fan": sensor.sensor_schema(icon="mdi:fan", unit_of_measurement="%",
                                   device_class="power_factor",  state_class=STATE_CLASS_MEASUREMENT, 
                                   accuracy_decimals=1
    ),
    "fan_set": sensor.sensor_schema(icon="mdi:fan", unit_of_measurement="rpm",
                                   device_class="speed",  state_class=STATE_CLASS_MEASUREMENT, 
                                   accuracy_decimals=1
    ),
    "pump": sensor.sensor_schema(icon="mdi:shower", unit_of_measurement="%",
                                   device_class="power_factor",  state_class=STATE_CLASS_MEASUREMENT, 
                                   accuracy_decimals=1
    ),
    "flow": sensor.sensor_schema(icon="mdi:shower", unit_of_measurement="l/min",
                                   device_class="water",  state_class=STATE_CLASS_MEASUREMENT, 
                                   accuracy_decimals=1
    ),
    # energy usage
    "power": sensor.sensor_schema(icon="mdi:meter-gas", unit_of_measurement="kW",
                                   device_class="water",  state_class=STATE_CLASS_MEASUREMENT, 
                                   accuracy_decimals=1
    ),
    "energy_ch": sensor.sensor_schema(icon="mdi:meter-gas", unit_of_measurement="m³",
                                   device_class="gas",  state_class="total_increasing", 
                                   accuracy_decimals=1
    ),
    "energy_dhw": sensor.sensor_schema(icon="mdi:meter-gas", unit_of_measurement="m³",
                                   device_class="gas",  state_class="total_increasing",
                                   accuracy_decimals=1
    ),
}

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_INTERGAS): cv.use_id(IntergasComponent),
    **{
        cv.Optional(key): schema for key, schema in SENSOR_DEFINITIONS.items()
    }
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_INTERGAS])
    for key in SENSOR_DEFINITIONS:
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(parent.register_value_sensor(key, sens))
