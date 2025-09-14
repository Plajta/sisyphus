#pragma once

#include <stdint.h>
#include <string.h>

struct color_entry {
    uint16_t hue;
    uint8_t saturation;
    uint8_t value;
    char color_name;
};

struct color_match_array {
    struct color_entry *data;
    size_t len;
};

typedef struct {
    uint16_t clear;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
} raw_color_measurement;

typedef struct {
    uint16_t hue;
    uint8_t saturation;
    uint8_t value;
    uint8_t clear;
} color_measurement;

#define MAX_LUT_LINE_SIZE 14

void color_rgb_to_hsv(float r, float g, float b, color_measurement *output_hsv);
int color_init();
int color_read_sensor(color_measurement *color);
char color_lut_get_code(color_measurement *color, int max_dist, uint8_t min_clear);

// Debug code
#ifdef SISYPHUS_DEBUG
struct color_entry* get_color_lut_entry(uint8_t index);
#endif
