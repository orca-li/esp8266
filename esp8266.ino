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
    shell();
}
