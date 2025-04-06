#include "shell.h"

extern size_t fsread(char* filename);

#define SHELL

static void cat_help_print(void)
{
    printf("Usage: cat [options]\n");
    printf("Options:\n");
    printf("  -h, --help                        Show this help message and exit\n");
}

static int cat_command(int argc, char **argv)
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
            cat_help_print();
            return EXIT_SUCCESS;
        case '?':
            printf("Error: Invalid option\n");
            return EXIT_FAILURE;
        default:
            return EXIT_FAILURE;
        }
    }

    if (0 == fsread(argv[1]))
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

SHELL int shellmain_cat(int argc, char **argv)
{
    if (argc < 1)
    {
        printf("Error: No command specified\n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[0], "cat") == 0)
    {
        return cat_command(argc, argv);
    }
    else
    {
        printf("Error: Unknown command '%s'\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}