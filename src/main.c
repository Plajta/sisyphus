/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <hardware/gpio.h>
#include <pico/time.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <bsp/board_api.h>
#include "bq25619.h"
#include "class/cdc/cdc_device.h"
#include "pico/binary_info.h"
#include <tusb.h>
#include "protocol.h"
#include "hardware/i2c.h"
#include "audio.h"
#include "color.h"
#include "tca8418.h"

#include "littlefs-pico.h"

#define BUTTON_PIN 14

#ifdef SISYFOSS_I2S_ENABLE
bi_decl(bi_1pin_with_name(SISYFOSS_I2S_ENABLE, "I2S ENABLE"));
#endif

#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
#error Sisyphus requires a board with I2C pins
#endif

lfs_t lfs;

volatile bool wakeup = false;
volatile uint8_t button_index = 0;

volatile uint8_t interrupts = 0;

static repeating_timer_t usb_timer;

i2c_inst_t *sisyfoss_i2c_inst = i2c_default;

static bool usb_background_task(repeating_timer_t *rt) {
    (void) rt;
    tud_task();  // TinyUSB background task
    return true; // Keep repeating
}

#ifndef SISYFOSS_HAS_KEYBORD_CONTROLLER
void button_interrupt(uint gpio, uint32_t events) {
    if(!tud_cdc_connected()){
        wakeup = true;
        button_index = 0;
    }
}
#else
void keyboard_interrupt(uint gpio, uint32_t events) {
    if(!tca8418_k_int_available(sisyfoss_i2c_inst)){
        return;
    }

    uint8_t kbd_events = tca8418_num_events(sisyfoss_i2c_inst);

    // Re-check INT_STAT here (important per documentation)
    if (!tca8418_k_int_available(sisyfoss_i2c_inst)) {
        return;
    }

    uint8_t value;
    bool pressed;

    while (tca8418_num_events(sisyfoss_i2c_inst) > 0) {
        tca8418_get_key_from_fifo(sisyfoss_i2c_inst, &value, &pressed);
        if (pressed && !tud_cdc_connected()){
            wakeup = true;
            button_index = value;
        }
    }

    // Clear the KE_INT interrupt bit by writing 1
    tca8418_k_int_reset(sisyfoss_i2c_inst);
}
#endif

int main() {
    i2c_init(sisyfoss_i2c_inst, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
    bq25619_init(sisyfoss_i2c_inst); // init charger before all due safety reasons

    tusb_init();
    // Run tud_task every millisecond just like pico_stdio would do
    add_repeating_timer_ms(1, usb_background_task, NULL, &usb_timer);

    #ifdef SISYFOSS_I2S_ENABLE
    gpio_init(SISYFOSS_I2S_ENABLE);
    gpio_set_dir(SISYFOSS_I2S_ENABLE, GPIO_OUT);
    gpio_put(SISYFOSS_I2S_ENABLE, 1);
    #endif

    int err = pico_lfs_init(&lfs);

    if (err) {
        sleep_ms(5000);
        return 0; // Todo add a color indicated error
    }

    init_audio();
    err = color_init();

    #ifndef SISYFOSS_HAS_KEYBORD_CONTROLLER
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &button_interrupt);
    #else
    tca8418_init(sisyfoss_i2c_inst);
    tca8418_setup_keyboard(sisyfoss_i2c_inst, 0b1111, 0b1111);
    tca8418_setup_interrupt(&keyboard_interrupt);
    #endif

    while (true) {
        // Check for USB-CDC connection
        if (tud_cdc_connected()) {
            protocol_loop();
        }

        // If not connected and no button pressed, go to sleep
        while (!tud_cdc_connected() && !wakeup) {
            __wfi(); // Low-power wait for interrupt
        }

        if (wakeup) {
            wakeup = false;
            play_audio("audio.wav");
        }
    }


    return 0;
}
