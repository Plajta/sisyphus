#pragma once

#include <stdint.h>
#include <string.h>

struct color_entry {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    char color_name;
};

struct color_match_array {
    struct color_entry *data;
    size_t len;
};

typedef struct {
    uint8_t clear;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} color_measurement;

#define LUT_LINE_SIZE 9

int color_init();
int color_read_sensor(color_measurement *color);
