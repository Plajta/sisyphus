#pragma once
#include <stdbool.h>
#include "pico/time.h"

bool lightshow_poweron_timer_cb(repeating_timer_t *rt);
void lightshow_bootup_sequence();
