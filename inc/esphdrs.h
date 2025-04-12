#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
extern "C"
{
#include <user_interface.h>
}

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "core.h"
#include "getopt.h"

#define LED_PIN 2      // (GPIO2) D4
#define RELAY_PIN 4    // (GPIO4) D2
#define ONE_WIRE_BUS 0 // (GPIO0) D3
#define TERMO_SENSORS_COUNT 4
#define WIFI_STATUS_CONNECTED true
#define WIFI_STATUS_DISCONNECTED false

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
