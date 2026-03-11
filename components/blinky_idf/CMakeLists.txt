idf_component_register(
    SRCS
        "led_config_idf.c"
        "button_idf.c"
        "led_sm_idf.c"
        "led_sm_producer_idf.c"
        "led_sm_consumer_idf.c"
        "led_output_adapter_idf.c"
        "button_input_adapter_idf.c"
        "blinky_log_adapter_idf.c"
        "app_event_queue.c"
    INCLUDE_DIRS "."
    REQUIRES driver core_sm core_blinky blinky_interfaces
)
