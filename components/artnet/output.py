import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_ID
from . import artnet_ns, ArtNet, CONF_ARTNET_ID

DEPENDENCIES = ["artnet"]

ArtNetOutput = artnet_ns.class_("ArtNetOutput", output.FloatOutput, cg.Component)

# Configuration keys
CONF_DESTINATION_ADDRESS = "destination_address"
CONF_UNIVERSE = "universe"
CONF_CHANNEL = "channel"

# Output configuration schema
CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(ArtNetOutput),
    cv.GenerateID(CONF_ARTNET_ID): cv.use_id(ArtNet),
    cv.Required(CONF_UNIVERSE): cv.int_range(min=0, max=15),
    cv.Required(CONF_CHANNEL): cv.int_range(min=1, max=512),
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    # Get the parent ArtNet component
    parent = await cg.get_variable(config[CONF_ARTNET_ID])
    
    # Create the output
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await output.register_output(var, config)
    
    # Set configuration
    cg.add(var.set_universe(config[CONF_UNIVERSE]))
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    
    # Register with parent
    cg.add(var.set_artnet_parent(parent))
