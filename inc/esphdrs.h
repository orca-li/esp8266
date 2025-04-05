#pragma once

#include <Arduino.h>
#include <stdint.h>
#include "debug.h"
#include "core.h"

#define DEBUG_LED_PIN 2 // GPIO2 D4

#define if_time_has_come(_delay, _continue)    \
    do                                         \
    {                                          \
        static uintmax_t prevMillis = 0;       \
        uintmax_t currMillis = millis();       \
                                               \
        if (currMillis - prevMillis >= _delay) \
        {                                      \
            prevMillis = currMillis;           \
            _continue;                         \
        }                                      \
    } while (0)
