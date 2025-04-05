#pragma once

#define NO_ARGUMENT 0
#define REQUIRED_ARGUMENT 1
#define OPTIONAL_ARGUMENT 2

typedef struct GETOPT_OPTION
{
    const char *name;
    int has_arg;
    int *flag;
    int val;
} option;

extern int optind;
extern char *optarg;

int getopt_long(int argc, char *const argv[], const char *optstring,
                const option *longopts, int *longindex);