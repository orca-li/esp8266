#include "shell.h"

extern bool fschdir(const char *newDir);

#define SHELL

static void cd_help_print(void)
{
    printf("Usage: cd [directory] [options]\n");
    printf("Options:\n");
    printf("  -h, --help                        Show this help message and exit\n");
}

static int cd_command(int argc, char **argv)
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
            cd_help_print();
            return EXIT_SUCCESS;
        case '?':
            printf("Error: Invalid option\n");
            return EXIT_FAILURE;
        default:
            return EXIT_FAILURE;
        }
    }

    if (argc != 2)
    {
        cd_help_print();
        return EXIT_FAILURE;
    }

    if (fschdir(argv[1]) == false)
    {
        printf("(cd): Couldn't change folder\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

SHELL int shellmain_cd(int argc, char **argv)
{
    if (argc < 1)
    {
        printf("Error: No command specified\n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[0], "cd") == 0)
    {
        return cd_command(argc, argv);
    }
    else
    {
        printf("Error: Unknown command '%s'\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}