#include "lightshow.h"
#include "color.h"
#include "ws2812.h"

volatile float t = 0.0f;

volatile bool led_light_state = false;

extern struct color_matched_entry matched_color;

volatile uint blink_cycles = 0;
#define LIGHTSHOW_POST_FADEON_BLINK_CYCLES 3
#define LIGHTSHOW_POST_FADEON_BLINK_CYCLE_DELAY_MS 250

int64_t lightshow_blink_alarm_callback(alarm_id_t id, __unused void *user_data) {
    if (led_light_state){
        led_light_state = false;
        ws2812_put_color(0x000000);
        if (blink_cycles >= LIGHTSHOW_POST_FADEON_BLINK_CYCLES){
            return 0;
        }
    }
    else {
        blink_cycles++;
        led_light_state = true;
        ws2812_put_color(matched_color.led_color_representation);
    }

    return LIGHTSHOW_POST_FADEON_BLINK_CYCLE_DELAY_MS * 1000; // It's in us
}

#define LIGHTSHOW_POWERON_TIME_DELTA_MS 10
#define LIGHTSHOW_POWERON_TIME_DELTA ((1.0f/1000.0f)*LIGHTSHOW_POWERON_TIME_DELTA_MS)

bool lightshow_poweron_timer_cb(repeating_timer_t *rt) {
    t += LIGHTSHOW_POWERON_TIME_DELTA;

    uint32_t color = (uint32_t)(t*t*t*t*200) << 8; // green channel
    ws2812_put_color(color);

    if (t < 1.0f){
        return true;
    }
    else {
        extern bool matched_color_valid;
        if (matched_color_valid || matched_color.led_color_representation == 0)
            add_alarm_in_ms(LIGHTSHOW_POST_FADEON_BLINK_CYCLE_DELAY_MS, lightshow_blink_alarm_callback, NULL, true);
        return false;
    }
}

void lightshow_bootup_sequence(int8_t *status_code){
    static struct repeating_timer timer;
    led_light_state = true;
    add_repeating_timer_ms(-LIGHTSHOW_POWERON_TIME_DELTA_MS, lightshow_poweron_timer_cb, NULL, &timer); // Negative because that counts the time differently
}
