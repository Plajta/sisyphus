/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <boards/pico.h>
#include <hardware/gpio.h>
#include <pico.h>
#include <pico/time.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "class/cdc/cdc_device.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include "protocol.h"
#include "audio.h"

#include "littlefs-pico.h"

#define BUTTON_PIN 14

lfs_t lfs;

volatile bool wakeup = false;
volatile uint8_t button_index = 0;

void button_interrupt(uint gpio, uint32_t events) {
    if(!stdio_usb_connected()){
        wakeup = true;
        button_index = 0; // Here as a preparation for implementing matrix controller
    }
}

int main() {
    stdio_init_all();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    int err = pico_lfs_init(&lfs);

    if (err) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(5000);
        return 0;
    }

    // Init audio before initializing the button interrupt
    init_audio();

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &button_interrupt);

    while (true) {
        // Check for USB-CDC connection
        if (stdio_usb_connected()) {
            protocol_loop(&lfs);
        }

        // If not connected and no button pressed, go to sleep
        while (!stdio_usb_connected() && !wakeup) {
            // LED only as an indicator for development
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            __wfi(); // Low-power wait for interrupt
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
        }

        if (wakeup) {
            wakeup = false;
            play_audio("audio.wav");
        }
    }


    return 0;
}
