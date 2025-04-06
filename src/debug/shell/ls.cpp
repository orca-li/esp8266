#include "shell.h"

#define SHELL

void fslist(const char *dirname);

static void ls_help_print(void)
{
    printf("Usage: ls [options]\n");
    printf("Options:\n");
    printf("  -h, --help                        Show this help message and exit\n");
}

static int ls_command(int argc, char **argv)
{
    int opt;

    static option long_options[] = {
        {"help", NO_ARGUMENT, 0, 'h'},
        {0, 0, 0, 0}};

    int option_index = 0;
    optind = 1;

    while ((opt = getopt_long(argc, argv, "h", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'h':
            ls_help_print();
            return EXIT_SUCCESS;
        case '?':
            printf("Error: Invalid option\n");
            return EXIT_FAILURE;
        default:
            return EXIT_FAILURE;
        }
    }

    fslist(".");
    return EXIT_SUCCESS;
}

SHELL int shellmain_ls(int argc, char **argv)
{
    if (argc < 1)
    {
        printf("Error: No command specified\n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[0], "ls") == 0)
    {
        return ls_command(argc, argv);
    }
    else
    {
        printf("Error: Unknown command '%s'\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}