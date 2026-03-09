idf_component_register(
    SRCS "button_idf.c" "led_sm_idf.c" "led_output_adapter_idf.c" "button_input_adapter_idf.c"
    INCLUDE_DIRS "."
    REQUIRES driver core_sm core_blinky
)
