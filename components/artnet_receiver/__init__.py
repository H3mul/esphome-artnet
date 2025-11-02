import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    UNIT_EMPTY,
    ICON_LIGHTBULB,
)

DEPENDENCIES = ["wifi"]
AUTO_LOAD = ["sensor"]

# Define the namespace for our component
artnet_receiver_ns = cg.esphome_ns.namespace("artnet_receiver")
ArtNetReceiver = artnet_receiver_ns.class_("ArtNetReceiver", cg.Component)
ArtNetChannel = artnet_receiver_ns.class_("ArtNetChannel", sensor.Sensor, cg.Component)

# Configuration keys
CONF_ARTNET_UNIVERSE = "artnet_universe"
CONF_CHANNELS = "channels"
CONF_CHANNEL_NUMBER = "channel_number"

# Channel configuration schema
CHANNEL_SCHEMA = sensor.sensor_schema(
    ArtNetChannel,
    unit_of_measurement=UNIT_EMPTY,
    icon=ICON_LIGHTBULB,
    accuracy_decimals=0,
).extend({
    cv.Required(CONF_CHANNEL_NUMBER): cv.int_range(min=1, max=512),
})

# Main component configuration schema
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ArtNetReceiver),
    cv.Optional(CONF_ARTNET_UNIVERSE, default=0): cv.int_range(min=0, max=15),
    cv.Optional(CONF_CHANNELS, default=[]): cv.ensure_list(CHANNEL_SCHEMA),
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    # Create the main ArtNet receiver component
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # Set the ArtNet universe
    cg.add(var.set_artnet_universe(config[CONF_ARTNET_UNIVERSE]))
    
    # Create and register channel components
    for channel_config in config[CONF_CHANNELS]:
        channel_var = cg.new_Pvariable(channel_config[CONF_ID])
        await cg.register_component(channel_var, channel_config)
        await sensor.register_sensor(channel_var, channel_config)
        
        # Set the channel number
        cg.add(channel_var.set_channel_number(channel_config[CONF_CHANNEL_NUMBER]))
        
        # Register the channel with the main receiver
        cg.add(var.add_channel(channel_var))
    
    # Add the external library dependency
    cg.add_library("rstephan/ArtnetWifi", "1.6.1")