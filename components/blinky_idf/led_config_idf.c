#include "led_config_idf.h"

#include "sdkconfig.h"

#define LED_GPIO ((gpio_num_t)CONFIG_BLINKY_LED_GPIO)
#define BTN_GPIO ((gpio_num_t)CONFIG_BLINKY_BTN_GPIO)

#define BLINKY_PRODUCER_POLL_MS_MIN ((blinky_time_ms_t)1U)
#define BLINKY_PRODUCER_POLL_MS_MAX ((blinky_time_ms_t)1000U)
#define BLINKY_BOOT_PATTERN_MS_MIN  ((blinky_time_ms_t)1U)
#define BLINKY_BOOT_PATTERN_MS_MAX  ((blinky_time_ms_t)2000U)

/* Keep these bounds aligned with Kconfig ranges:
 * - BLINKY_PRODUCER_POLL_MS range lower-bound
 * - BLINKY_PRODUCER_POLL_MS range upper-bound
 * - BLINKY_BOOT_PATTERN_MS range lower-bound
 * - BLINKY_BOOT_PATTERN_MS range upper-bound
 */
static blinky_time_ms_t clamp_ms_u32(int value,
                                     blinky_time_ms_t min_inclusive,
                                     blinky_time_ms_t max_inclusive)
{
    if (value <= 0) {
        return min_inclusive;
    }
    blinky_time_ms_t normalized = (blinky_time_ms_t)value;
    if (normalized < min_inclusive) {
        return min_inclusive;
    }
    if (normalized > max_inclusive) {
        return max_inclusive;
    }
    return normalized;
}

blinky_time_ms_t led_config_idf_clamp_producer_poll_ms(int value)
{
    return clamp_ms_u32(value, BLINKY_PRODUCER_POLL_MS_MIN, BLINKY_PRODUCER_POLL_MS_MAX);
}

int led_config_idf_clamp_boot_pattern_ms(int value)
{
    return (int)clamp_ms_u32(value, BLINKY_BOOT_PATTERN_MS_MIN, BLINKY_BOOT_PATTERN_MS_MAX);
}

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

    blinky_log_level_t log_min_level = BLINKY_LOG_LEVEL_INFO;
#if CONFIG_BLINKY_LOG_MIN_LEVEL_ERROR
    log_min_level = BLINKY_LOG_LEVEL_ERROR;
#elif CONFIG_BLINKY_LOG_MIN_LEVEL_WARN
    log_min_level = BLINKY_LOG_LEVEL_WARN;
#elif CONFIG_BLINKY_LOG_MIN_LEVEL_DEBUG
    log_min_level = BLINKY_LOG_LEVEL_DEBUG;
#else
    log_min_level = BLINKY_LOG_LEVEL_INFO;
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
        .producer_poll_ms = led_config_idf_clamp_producer_poll_ms(CONFIG_BLINKY_PRODUCER_POLL_MS),
        .boot_pattern_ms = led_config_idf_clamp_boot_pattern_ms(CONFIG_BLINKY_BOOT_PATTERN_MS),
        .boot_pattern_enabled = CONFIG_BLINKY_BOOT_PATTERN,
        .log_intensity_enabled = log_intensity_enabled,
        .log_min_level = log_min_level,
    };
}

void idf_build_core_config(led_core_config_t *cfg)
{
    if (!cfg) {
        return;
    }

    led_startup_selector_t startup_selector = LED_STARTUP_SELECT_DEFAULT;

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
