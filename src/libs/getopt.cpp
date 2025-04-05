#include "../../inc/esphdrs.h"

extern int optind;
extern char *optarg;

int getopt_long(int argc, char *const argv[], const char *optstring,
                const option *longopts, int *longindex)
{
    static int nextchar = 0;
    optarg = NULL;

    if (optind >= argc || argv[optind][0] != '-')
    {
        return -1;
    }

    if (argv[optind][1] == '-')
    {
        const char *name = argv[optind] + 2;
        int i = 0;
        bool found = false;

        if (longopts == NULL)
            return '?';

        for (i = 0; longopts[i].name != NULL; i++)
        {
            if (strcmp(name, longopts[i].name) == 0)
            {
                found = true;
                if (longindex != NULL)
                {
                    *longindex = i;
                }

                if (longopts[i].has_arg == REQUIRED_ARGUMENT)
                {
                    optind++;
                    if (optind < argc)
                    {
                        optarg = argv[optind];
                        optind++;
                    }
                    else
                    {
                        return '?';
                    }
                }
                else if (longopts[i].has_arg == OPTIONAL_ARGUMENT)
                {
                    optind++;
                    if (optind < argc && argv[optind][0] != '-')
                    {
                        optarg = argv[optind];
                        optind++;
                    }
                    else
                    {
                        optarg = NULL;
                    }
                }
                else
                {
                    optind++;
                }
                return longopts[i].val;
            }
        }
        if (!found)
        {
            return '?';
        }
    }
    else
    {
        char optchar = argv[optind][1];
        const char *p;

        for (p = optstring; *p != '\0'; p++)
        {
            if (*p == optchar)
            {
                if (p[1] == ':')
                {
                    if (argv[optind][2] != '\0') {
                        optarg = (char*)argv[optind] + 2;
                        optind++;
                    } else {
                        optind++;
                        if (optind < argc)
                        {
                            optarg = argv[optind];
                            optind++;
                        }
                        else
                        {
                            return '?';
                        }
                    }
                }
                else
                {
                    optind++;
                }
                return optchar;
            }
        }
        return '?';
    }
    return '?';
}

int optind = 1;
char *optarg = NULL;