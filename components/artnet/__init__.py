import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["wifi"]
AUTO_LOAD = ["sensor", "output"]

# Define the namespace for our component
artnet_ns = cg.esphome_ns.namespace("artnet")
ArtNet = artnet_ns.class_("ArtNet", cg.Component)

# Configuration key for parent reference
CONF_ARTNET_ID = "artnet_id"

# Configuration schema for the global artnet component
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ArtNet),
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    # Create the global ArtNet component
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # Add the external library dependency
    cg.add_library("rstephan/ArtnetWifi", "1.6.1")
