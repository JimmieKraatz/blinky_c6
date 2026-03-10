idf_component_register(
    SRCS
        "led_model.c"
        "menu_logic.c"
        "led_policy.c"
        "led_runtime.c"
        "button_logic.c"
        "led_event_consumer.c"
        "led_event_factory.c"
        "led_event_map.c"
    INCLUDE_DIRS "."
    REQUIRES blinky_interfaces
)
