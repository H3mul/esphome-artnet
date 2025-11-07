import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["wifi"]
AUTO_LOAD = ["sensor", "output"]

# Define the namespace for our component
artnet_ns = cg.esphome_ns.namespace("artnet")
ArtNet = artnet_ns.class_("ArtNet", cg.Component)

# Get reference to DMX component namespace - using use_id requires the component to be available
dmx_ns = cg.esphome_ns.namespace("dmx")
DMXComponent = dmx_ns.class_("DMXComponent")
DMXMode = dmx_ns.enum("DMXMode")

# Configuration keys
CONF_ARTNET_ID = "artnet_id"
CONF_NAME_SHORT = "name_short"
CONF_NAME_LONG = "name_long"
CONF_NET = "net"
CONF_SUBNET = "subnet"
CONF_OUTPUT = "output"
CONF_OUTPUT_ADDRESS = "address"
CONF_FLUSH_PERIOD = "flush_period"
CONF_CONTINUOUS_OUTPUT = "continuous_output"
CONF_ROUTE = "route"
CONF_ARTNET_TO_DMX = "artnet_to_dmx"
CONF_DMX_TO_ARTNET = "dmx_to_artnet"
CONF_DMX_ID = "dmx_id"
CONF_UNIVERSE = "universe"

# Configuration schema for the global artnet component
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ArtNet),
    cv.Optional(CONF_NAME_SHORT, default=""): cv.string,
    cv.Optional(CONF_NAME_LONG, default=""): cv.string,
    cv.Optional(CONF_NET, default=0): cv.int_range(min=0, max=127),
    cv.Optional(CONF_SUBNET, default=0): cv.int_range(min=0, max=15),
    cv.Optional(CONF_OUTPUT): cv.Schema({
        cv.Optional(CONF_OUTPUT_ADDRESS): cv.ipv4address,
        cv.Optional(CONF_NET, default=0): cv.int_range(min=0, max=127),
        cv.Optional(CONF_SUBNET, default=0): cv.int_range(min=0, max=15),
        cv.Optional(CONF_FLUSH_PERIOD, default="10ms"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_CONTINUOUS_OUTPUT, default=False): cv.boolean,
    }),
    cv.Optional(CONF_ROUTE): cv.Schema({
        cv.Optional(CONF_ARTNET_TO_DMX): cv.All(cv.ensure_list(cv.Schema({
            cv.Required(CONF_DMX_ID): cv.use_id(DMXComponent),
            cv.Required(CONF_UNIVERSE): cv.int_range(min=0, max=15),
        }))),
        cv.Optional(CONF_DMX_TO_ARTNET): cv.All(cv.ensure_list(cv.Schema({
            cv.Required(CONF_DMX_ID): cv.use_id(DMXComponent),
            cv.Required(CONF_UNIVERSE): cv.int_range(min=0, max=15),
        }))),
    }),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    # Create the global ArtNet component
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # Set name_short if present
    if CONF_NAME_SHORT in config:
        cg.add(var.set_name_short(config[CONF_NAME_SHORT]))
    
    # Set name_long if present
    if CONF_NAME_LONG in config:
        cg.add(var.set_name_long(config[CONF_NAME_LONG]))
    
    # Set incoming frame filtering (net and subnet)
    if CONF_NET in config:
        cg.add(var.set_net(config[CONF_NET]))
    if CONF_SUBNET in config:
        cg.add(var.set_subnet(config[CONF_SUBNET]))
    
    # Set output configuration if present
    if CONF_OUTPUT in config:
        output_config = config[CONF_OUTPUT]
        if CONF_OUTPUT_ADDRESS in output_config:
            cg.add(var.set_output_address(str(output_config[CONF_OUTPUT_ADDRESS])))
        if CONF_NET in output_config:
            cg.add(var.set_output_net(output_config[CONF_NET]))
        if CONF_SUBNET in output_config:
            cg.add(var.set_output_subnet(output_config[CONF_SUBNET]))
        if CONF_FLUSH_PERIOD in output_config:
            cg.add(var.set_flush_period(output_config[CONF_FLUSH_PERIOD]))
        if CONF_CONTINUOUS_OUTPUT in output_config:
            cg.add(var.set_continuous_output(output_config[CONF_CONTINUOUS_OUTPUT]))
    
    # Set routing configuration if present
    if CONF_ROUTE in config:
        route_config = config[CONF_ROUTE]

        include_dmx = False
        
        # Configure ArtNet to DMX routing
        if CONF_ARTNET_TO_DMX in route_config:
            include_dmx = len(route_config[CONF_ARTNET_TO_DMX]) > 0
            for route in route_config[CONF_ARTNET_TO_DMX]:
                dmx_component = await cg.get_variable(route[CONF_DMX_ID])
                universe = route[CONF_UNIVERSE]
                # Pass the resolved DMX component pointer
                cg.add(var.add_artnet_to_dmx_route(dmx_component, universe))
            
        # Configure DMX to ArtNet routing
        if CONF_DMX_TO_ARTNET in route_config:
            include_dmx = include_dmx or len(route_config[CONF_DMX_TO_ARTNET]) > 0
            for route in route_config[CONF_DMX_TO_ARTNET]:
                dmx_component = await cg.get_variable(route[CONF_DMX_ID])
                universe = route[CONF_UNIVERSE]
                # Pass the resolved DMX component pointer
                cg.add(var.add_dmx_to_artnet_route(dmx_component, universe))

        if include_dmx:
            cg.add_build_flag("-DUSE_DMX_COMPONENT")
    
    # Add the external library dependency
    cg.add_library("rstephan/ArtnetWifi", "1.6.1")
