#pragma once

typedef char charshell_t;

typedef int (*shellmain)(int, char **);
typedef struct SHELL_LIST
{
    const char *name;
    shellmain handler;
} shelllist;

void relay_handler(void);
void termo_handler(void);
void shell(void);
void debug_led(void);
void PublicRelaySet(void);
void PublicRelayReset(void);