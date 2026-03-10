#include "led_config_idf.h"

#include "sdkconfig.h"

#define LED_GPIO ((gpio_num_t)CONFIG_BLINKY_LED_GPIO)
#define BTN_GPIO ((gpio_num_t)CONFIG_BLINKY_BTN_GPIO)

void idf_build_platform_config(led_platform_config_t *cfg)
{
    if (!cfg) {
        return;
    }

    button_pull_t pull = BUTTON_PULL_NONE;
#if CONFIG_BLINKY_BTN_PULL_UP
    pull = BUTTON_PULL_UP;
#elif CONFIG_BLINKY_BTN_PULL_DOWN
    pull = BUTTON_PULL_DOWN;
#endif
    bool log_intensity_enabled = false;
#ifdef CONFIG_BLINKY_LOG_INTENSITY
    log_intensity_enabled = CONFIG_BLINKY_LOG_INTENSITY;
#endif

    *cfg = (led_platform_config_t){
        .led_output =
            {
                .gpio = LED_GPIO,
                .pwm_freq_hz = CONFIG_BLINKY_PWM_FREQ_HZ,
                .active_low = true,
            },
        .button_gpio = BTN_GPIO,
        .button_active_low = CONFIG_BLINKY_BTN_ACTIVE_LOW,
        .button_pull = pull,
        .producer_poll_ms = CONFIG_BLINKY_PRODUCER_POLL_MS,
        .boot_pattern_ms = CONFIG_BLINKY_BOOT_PATTERN_MS,
        .boot_pattern_enabled = CONFIG_BLINKY_BOOT_PATTERN,
        .log_intensity_enabled = log_intensity_enabled,
    };
}

void idf_build_core_config(led_core_config_t *cfg)
{
    if (!cfg) {
        return;
    }

#if CONFIG_BLINKY_START_WAVE_SQUARE
    led_startup_selector_t startup_selector = LED_STARTUP_SELECT_SQUARE;
#elif CONFIG_BLINKY_START_WAVE_SAW_UP
    led_startup_selector_t startup_selector = LED_STARTUP_SELECT_SAW_UP;
#elif CONFIG_BLINKY_START_WAVE_SAW_DOWN
    led_startup_selector_t startup_selector = LED_STARTUP_SELECT_SAW_DOWN;
#elif CONFIG_BLINKY_START_WAVE_TRIANGLE
    led_startup_selector_t startup_selector = LED_STARTUP_SELECT_TRIANGLE;
#else
    led_startup_selector_t startup_selector = LED_STARTUP_SELECT_SINE;
#endif

    *cfg = (led_core_config_t){
        .model =
            {
                .wave_period_ms = CONFIG_BLINKY_WAVE_PERIOD_MS,
                .poll_ms = CONFIG_BLINKY_MODEL_POLL_MS,
                .sine_steps_max = CONFIG_BLINKY_SINE_STEPS_MAX,
                .saw_step_pct = CONFIG_BLINKY_SAW_STEP_PCT,
            },
        .startup =
            {
                .selector = startup_selector,
            },
        .button_timing = button_policy_timing_normalize((button_policy_timing_t){
            .debounce_count = CONFIG_BLINKY_DEBOUNCE_COUNT,
            .long_press_ms = CONFIG_BLINKY_LONG_PRESS_MS,
        }),
    };
}
