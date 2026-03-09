idf_component_register(
    SRCS "button.c" "led_sm.c"
    INCLUDE_DIRS "."
    REQUIRES driver core_sm core_blinky
)
