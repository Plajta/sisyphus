#pragma once
#include <stdbool.h>
#include "pico/time.h"

#define POWER_OF_4(base) (base * base * base * base)

bool lightshow_poweron_timer_cb(repeating_timer_t *rt);
void lightshow_bootup_sequence();
