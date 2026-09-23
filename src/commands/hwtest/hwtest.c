/**
 * commands_hwtest.c — Hardware test commands for the Sisyphus protocol.
 *
 * Compiled only when SISYPHUS_HWTEST is defined (-DSISYPHUS_HWTEST=ON).
 * Python frontend (hwtest.py) drives the interactive session; this file
 * provides the backend that executes hardware actions and returns data.
 *
 */

#include "commands.h"
#include "protocol_common.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pico/time.h"

/* ── Conditional driver includes ─────────────────────────────────────────── */

#ifdef PICO_DEFAULT_WS2812_PIN
#include "ws2812.h"
#endif

#ifdef SISYFOSS_HAS_KEYBOARD_CONTROLLER
#include "tca8418.h"
extern i2c_inst_t *sisyfoss_i2c_inst;

/* Ring buffer filled by sisyphus_keyboard_hook() (called from main.c IRQ). */
#define HWTEST_KBD_BUF_SIZE 32
typedef struct { uint8_t key; bool pressed; } hwtest_kbd_event_t;
static hwtest_kbd_event_t hwtest_kbd_buf[HWTEST_KBD_BUF_SIZE];
static volatile int hwtest_kbd_head = 0; /* write index */
static volatile int hwtest_kbd_tail = 0; /* read  index */

void sisyphus_keyboard_hook(uint8_t key, bool pressed) {
    int next = (hwtest_kbd_head + 1) % HWTEST_KBD_BUF_SIZE;
    if (next != hwtest_kbd_tail) { /* drop if full */
        hwtest_kbd_buf[hwtest_kbd_head].key     = key;
        hwtest_kbd_buf[hwtest_kbd_head].pressed = pressed;
        hwtest_kbd_head = next;
    }
}
#endif

#ifdef SISYFOSS_COLOR_VEML3328
#include "color.h"
#endif

#ifdef SISYFOSS_HAS_CHARGER
#include "bq25619.h"
extern i2c_inst_t *sisyfoss_i2c_inst;
#endif

#ifdef SISYFOSS_LID_DETECT
#include "hardware/gpio.h"
#endif

#ifdef SISYFOSS_AUX_SENSOR_LED
#include "hardware/gpio.h"
#endif

/* ── hwtest_led <r> <g> <b> ──────────────────────────────────────────────── */
/* Set the WS2812 LED to an arbitrary RGB colour (values 0-255). */
void do_hwtest_led(int argc, char **argv) {
#ifdef PICO_DEFAULT_WS2812_PIN
    uint8_t r = (uint8_t)atoi(argv[1]);
    uint8_t g = (uint8_t)atoi(argv[2]);
    uint8_t b = (uint8_t)atoi(argv[3]);
    uint32_t color = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    ws2812_put_color(color);
    print_newline("ack");
#else
    print_newline("err not available");
#endif
}

/* ── hwtest_led_off ───────────────────────────────────────────────────────── */
/* Turn the WS2812 LED off. */
void do_hwtest_led_off(int argc, char **argv) {
#ifdef PICO_DEFAULT_WS2812_PIN
    ws2812_put_color(0);
    print_newline("ack");
#else
    print_newline("err not available");
#endif
}

/* ── hwtest_audio ────────────────────────────────────────────────────────── */
/* Play volume_sample.wav at maximum gain. */
void do_hwtest_audio(int argc, char **argv) {
    extern void play_audio(char *filename, uint16_t gain);
    play_audio("volume_sample.wav", 0x7FFF);
    print_newline("ack");
}

/* ── hwtest_sensor_led <0|1> ─────────────────────────────────────────────── */
/* Turn the aux sensor illumination LED on (1) or off (0). */
void do_hwtest_sensor_led(int argc, char **argv) {
#ifdef SISYFOSS_AUX_SENSOR_LED
    uint8_t state = (uint8_t)atoi(argv[1]);
    gpio_init(SISYFOSS_AUX_SENSOR_LED);
    gpio_set_dir(SISYFOSS_AUX_SENSOR_LED, GPIO_OUT);
    gpio_put(SISYFOSS_AUX_SENSOR_LED, state ? 1 : 0);
    print_newline("ack");
#else
    print_newline("err not available");
#endif
}

/* ── hwtest_sensor ───────────────────────────────────────────────────────── */
/* Read the color sensor. Reply: "ack <hue> <sat> <val> <clear>" */
void do_hwtest_sensor(int argc, char **argv) {
#ifdef SISYFOSS_COLOR_VEML3328
    color_measurement m;
    int err = color_read_sensor(&m);
    if (err != 0) {
        print_newline("err sensor read failed");
        return;
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "ack %d %d %d %d",
             m.hue, m.saturation, m.value, m.clear);
    print_newline(buf);
#else
    print_newline("err not available");
#endif
}

/* ── hwtest_keyboard_poll ────────────────────────────────────────────────── */
/* Drain the hwtest ring buffer (filled by the IRQ hook in main.c) and report
 * all pending events since the last call.
 * Reply: "ack <count> [<key> <pressed|released> ...]"
 * e.g.  "ack 2 11 pressed 11 released"
 * Python calls this repeatedly, accumulating which keys have been pressed. */
void do_hwtest_keyboard_poll(int argc, char **argv) {
#ifdef SISYFOSS_HAS_KEYBOARD_CONTROLLER
    static bool hook_bound = false;
    if (!hook_bound) {
        tca8418_set_key_callback(sisyphus_keyboard_hook);
        hook_bound = true;
    }
    char buf[256];
    int  pos   = 0;
    int  count = 0;

    /* Snapshot head so we don't race with the IRQ writing new events */
    int head = hwtest_kbd_head;

    /* Drain ring buffer into reply string */
    pos = snprintf(buf, sizeof(buf), "ack 0"); /* placeholder count */

    while (hwtest_kbd_tail != head && count < 10) {
        hwtest_kbd_event_t *ev = &hwtest_kbd_buf[hwtest_kbd_tail];
        hwtest_kbd_tail = (hwtest_kbd_tail + 1) % HWTEST_KBD_BUF_SIZE;

        pos += snprintf(buf + pos, sizeof(buf) - pos,
                        " %d %s",
                        ev->key,
                        ev->pressed ? "pressed" : "released");
        count++;
    }

    /* Patch the count back in (overwrite the "0" placeholder) */
    buf[4] = '0' + (count % 10); /* works for count 0-9, safe for our buffer */

    print_newline(buf);
#else
    print_newline("err not available");
#endif
}


/* ── hwtest_battery ──────────────────────────────────────────────────────── */
/* Read BQ25619 status registers.
 * Reply: "ack <vbus_status> <charge_status> <power_good> <vin_dpm> <thermal>" */
void do_hwtest_battery(int argc, char **argv) {
#ifdef SISYFOSS_HAS_CHARGER
    bq25619_status st;
    if (bq25619_read_status(sisyfoss_i2c_inst, &st) != 0) {
        print_newline("err i2c read failed");
        return;
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "ack %d %d %d %d %d",
             st.vbus_status,
             st.charge_status,
             (int)st.power_good,
             (int)st.vin_dpm_mode,
             (int)st.termal_regulated);
    print_newline(buf);
#else
    print_newline("err not available");
#endif
}

/* ── hwtest_battery_faults ───────────────────────────────────────────────── */
/* Read BQ25619 fault register.
 * Reply: "ack <charge_fault> <bat_ovp> <boost_fault> <watchdog> <ntc_temp>" */
void do_hwtest_battery_faults(int argc, char **argv) {
#ifdef SISYFOSS_HAS_CHARGER
    bq25619_fault_status fs;
    if (bq25619_read_fault_status(sisyfoss_i2c_inst, &fs) != 0) {
        print_newline("err i2c read failed");
        return;
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "ack %d %d %d %d %d",
             fs.charge_fault,
             (int)fs.battery_overvoltage,
             (int)fs.boost_fault,
             (int)fs.watchdog_expired,
             fs.ntc_temp);
    print_newline(buf);
#else
    print_newline("err not available");
#endif
}

/* ── hwtest_lid ──────────────────────────────────────────────────────────── */
/* Read the lid detection GPIO.
 * Reply: "ack open" or "ack closed" */
void do_hwtest_lid(int argc, char **argv) {
#ifdef SISYFOSS_LID_DETECT
    /* Active-low: 0 = closed, 1 = open */
    bool closed = !gpio_get(SISYFOSS_LID_DETECT);
    print_newline(closed ? "ack closed" : "ack open");
#else
    print_newline("err not available");
#endif
}

/* ── Command Table ───────────────────────────────────────────────────────── */
static const command_entry_t hwtest_command_table[] = {
    {"hwtest_led",            4, do_hwtest_led},
    {"hwtest_led_off",        1, do_hwtest_led_off},
    {"hwtest_audio",          1, do_hwtest_audio},
    {"hwtest_sensor_led",     2, do_hwtest_sensor_led},
    {"hwtest_sensor",         1, do_hwtest_sensor},
    {"hwtest_keyboard_poll",  1, do_hwtest_keyboard_poll},
    {"hwtest_battery",        1, do_hwtest_battery},
    {"hwtest_battery_faults", 1, do_hwtest_battery_faults},
    {"hwtest_lid",            1, do_hwtest_lid},
};

#define HWTEST_COMMAND_COUNT (sizeof(hwtest_command_table) / sizeof(hwtest_command_table[0]))

const command_entry_t* hwtest_find_command(const char *name) {
    for (size_t i = 0; i < HWTEST_COMMAND_COUNT; i++) {
        if (strcmp(name, hwtest_command_table[i].name) == 0) {
            return &hwtest_command_table[i];
        }
    }
    return NULL;
}
