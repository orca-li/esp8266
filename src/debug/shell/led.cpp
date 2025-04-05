#include "shell.h"

int shellmain_led(int argc, char **argv)
{
    printf("led() called with %d arguments:\n", argc);
    for (int i = 0; i < argc; ++i)
    {
        printf("  argv[%d] = %s\n", i, argv[i]);
    }

    return 0;
}
