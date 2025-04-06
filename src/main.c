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
#include "pico/audio.h"
#include "pico/stdlib.h"
#include "pico/audio_i2s.h"
#include "pico/binary_info.h"

#include "littlefs-pico.h"

#include "wav_samples.h"

bi_decl(bi_3pins_with_names(PICO_AUDIO_I2S_DATA_PIN, "I2S DIN", PICO_AUDIO_I2S_CLOCK_PIN_BASE, "I2S BCK", PICO_AUDIO_I2S_CLOCK_PIN_BASE+1, "I2S LRCK"));

#define SAMPLES_PER_BUFFER 256

#define BUTTON_PIN 14

bool running = false;
lfs_t lfs;

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

int main() {
    #ifdef SISYPHUS_DEBUG
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    stdio_init_all();

    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    printf("Connected\n");
    #endif

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    int err = pico_lfs_init(&lfs);

    if (err) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(5000);
        return 0;
    }

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

    struct audio_buffer_pool *ap = init_audio();

    uint32_t sample_index = 0;

    while (true) {
        while (gpio_get(BUTTON_PIN) && (sample_index == 0)) {
            sleep_ms(10);
        }
        if (!running){
            running = true;
        }

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
    return 0;
}
