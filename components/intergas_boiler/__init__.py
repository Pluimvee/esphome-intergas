"""ESPHome component: Intergas Boiler Communicator."""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID, CONF_UART_ID

CODEOWNERS = ["@Pluimvee"]
AUTO_LOAD = ["sensor", "text_sensor", "binary_sensor", "uart"]

#allows multiple intergas_boiler components in one config
MULTI_CONF = True

ns = cg.global_ns
IntergasComponent = ns.class_("IntergasComponent", cg.PollingComponent, uart.UARTDevice)

CONF_INTERGAS  = "intergas_boiler"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(IntergasComponent),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
}).extend(cv.polling_component_schema("5s")).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
    uart_dev = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], uart_dev)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
