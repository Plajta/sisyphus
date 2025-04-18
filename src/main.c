/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <boards/pico.h>
#include <hardware/gpio.h>
#include <pico/time.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pico/audio.h"
#include "pico/stdlib.h"
#include "pico/audio_i2s.h"
#include "pico/binary_info.h"
#include "tusb.h"

#include "littlefs-pico.h"

#include "wav_samples.h"

bi_decl(bi_3pins_with_names(PICO_AUDIO_I2S_DATA_PIN, "I2S DIN", PICO_AUDIO_I2S_CLOCK_PIN_BASE, "I2S BCK", PICO_AUDIO_I2S_CLOCK_PIN_BASE+1, "I2S LRCK"));

#define SAMPLES_PER_BUFFER 256

#define BUTTON_PIN 14

lfs_t lfs;

static struct audio_buffer_pool *ap;

volatile bool wakeup = false;
static uint32_t sample_index = 0;


struct audio_buffer_pool *init_audio() {

    static audio_format_t audio_format = {
        .format = AUDIO_BUFFER_FORMAT_PCM_S16,
        .sample_freq = 44100,
        .channel_count = 1,
    };

    static struct audio_buffer_format producer_format = {
        .format = &audio_format,
        .sample_stride = 2
    };

    struct audio_buffer_pool *producer_pool = audio_new_producer_pool(&producer_format, 3,
                                                                      SAMPLES_PER_BUFFER); // todo correct size
    bool ok;
    const struct audio_format *output_format;

    struct audio_i2s_config config = {
        .data_pin = PICO_AUDIO_I2S_DATA_PIN,
        .clock_pin_base = PICO_AUDIO_I2S_CLOCK_PIN_BASE,
        .dma_channel = 0,
        .pio_sm = 0,
    };

    output_format = audio_i2s_setup(&audio_format, &config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }

    ok = audio_i2s_connect(producer_pool);
    assert(ok);
    audio_i2s_set_enabled(true);
    return producer_pool;
}

void button_interrupt(uint gpio, uint32_t events) {
    wakeup = true;
    sample_index = 0;
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
    ap = init_audio();

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &button_interrupt);

    // ONLY HERE AS A DEMONSTRATION
    // ONLY HERE AS A DEMONSTRATION
    // ONLY HERE AS A DEMONSTRATION
    lfs_file_t file;

    // read current count
    uint32_t boot_count = 0;
    lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&lfs, &file);

    // release any resources we were using
    lfs_unmount(&lfs);

    printf("boot_count: %d\n", boot_count);
    // ONLY HERE AS A DEMONSTRATION
    // ONLY HERE AS A DEMONSTRATION
    // ONLY HERE AS A DEMONSTRATION

    while (true) {
        // Check for USB-CDC connection
        if (stdio_usb_connected()) {
            printf("USB connected!\n");

            while (stdio_usb_connected()) {
                if (tud_cdc_available()) {
                    char buf[64];
                    uint32_t count = tud_cdc_read(buf, sizeof(buf));
                    tud_cdc_write(buf, count);
                    tud_cdc_write_flush();
                }
                sleep_ms(10);
            }
        }

        // If not connected and no button pressed, go to sleep
        wakeup = false;
        while (!stdio_usb_connected() && !wakeup) {
            // LED only as an indicator for development
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            __wfi(); // Low-power wait for interrupt
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
        }

        if (wakeup) {
            bool running = true;

            while (running) {
                struct audio_buffer *buffer = take_audio_buffer(ap, true);
                int16_t *samples = (int16_t *) buffer->buffer->bytes;

                for (uint i = 0; i < buffer->max_sample_count; i++) {
                    samples[i] = wav_data[sample_index++];
                    if (sample_index >= WAV_SAMPLE_COUNT) {
                        sample_index = 0;
                        running = false;
                    }
                }

                buffer->sample_count = buffer->max_sample_count;
                give_audio_buffer(ap, buffer);
            }
        }
    }


    return 0;
}
