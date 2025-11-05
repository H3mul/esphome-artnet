import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["wifi"]
AUTO_LOAD = ["sensor", "output"]

# Define the namespace for our component
artnet_ns = cg.esphome_ns.namespace("artnet")
ArtNet = artnet_ns.class_("ArtNet", cg.Component)

# Configuration keys
CONF_ARTNET_ID = "artnet_id"
CONF_OUTPUT = "output"
CONF_OUTPUT_ADDRESS = "address"
CONF_FLUSH_PERIOD = "flush_period"

# Configuration schema for the global artnet component
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ArtNet),
    cv.Optional(CONF_OUTPUT): cv.Schema({
        cv.Optional(CONF_OUTPUT_ADDRESS): cv.ipv4address,
        cv.Optional(CONF_FLUSH_PERIOD, default="100ms"): cv.positive_time_period_milliseconds,
    }),
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    # Create the global ArtNet component
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # Set output configuration if present
    if CONF_OUTPUT in config:
        output_config = config[CONF_OUTPUT]
        if CONF_OUTPUT_ADDRESS in output_config:
            cg.add(var.set_output_address(str(output_config[CONF_OUTPUT_ADDRESS])))
        if CONF_FLUSH_PERIOD in output_config:
            cg.add(var.set_flush_period(output_config[CONF_FLUSH_PERIOD]))
    
    # Add the external library dependency
    cg.add_library("rstephan/ArtnetWifi", "1.6.1")
