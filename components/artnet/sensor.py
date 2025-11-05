import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    UNIT_EMPTY,
    ICON_LIGHTBULB,
)
from . import artnet_ns, ArtNet, CONF_ARTNET_ID

DEPENDENCIES = ["artnet"]

ArtNetSensor = artnet_ns.class_("ArtNetSensor", sensor.Sensor, cg.Component)

# Configuration keys
CONF_UNIVERSE = "universe"
CONF_CHANNEL = "channel"

# Sensor configuration schema
CONFIG_SCHEMA = sensor.sensor_schema(
    ArtNetSensor,
    unit_of_measurement=UNIT_EMPTY,
    icon=ICON_LIGHTBULB,
    accuracy_decimals=0,
).extend({
    cv.GenerateID(CONF_ARTNET_ID): cv.use_id(ArtNet),
    cv.Required(CONF_UNIVERSE): cv.int_range(min=0, max=15),
    cv.Required(CONF_CHANNEL): cv.int_range(min=1, max=512),
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    # Get the parent ArtNet component
    parent = await cg.get_variable(config[CONF_ARTNET_ID])
    
    # Create the sensor
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    
    # Set configuration
    cg.add(var.set_universe(config[CONF_UNIVERSE]))
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    
    # Register with parent
    cg.add(var.set_artnet_parent(parent))
