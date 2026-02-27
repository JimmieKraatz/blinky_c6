idf_component_register(
    SRCS "button.c" "led_model.c" "led_sm.c" "menu_logic.c"
    INCLUDE_DIRS "."
    REQUIRES driver
)
