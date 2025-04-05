#include "../../inc/esphdrs.h"

static void togglePin(int pin)
{
    digitalWrite(pin, !digitalRead(pin));
}

void debug_led(uintmax_t delay)
{
    if_time_has_come(delay, togglePin(DEBUG_LED_PIN));
}
