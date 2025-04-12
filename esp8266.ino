#include "inc/esphdrs.h"

void setup(void)
{
    core_init();
}

void loop(void)
{
    debug_led();
    relay_handler();
    termo_handler();
    wifi_handler();
    server_handler();
    shell();
}
