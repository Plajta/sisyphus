#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    float t;
    float duration;
    bool reverse;
    uint32_t base_color;  // 24 bit RGB
    bool start_next_reversed;
} lightshow_quartic_fade_state_t;

void lightshow_fade_setup(lightshow_quartic_fade_state_t *initial_state);
