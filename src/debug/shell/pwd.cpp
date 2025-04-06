#include "shell.h"

extern const char *fscurrdir(void);

#define SHELL

static void pwd_help_print(void)
{
    printf("Usage: pwd [options]\n");
    printf("Options:\n");
    printf("  -h, --help                        Show this help message and exit\n");
}

static int pwd_command(int argc, char **argv)
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
            pwd_help_print();
            return EXIT_SUCCESS;
        case '?':
            printf("Error: Invalid option\n");
            return EXIT_FAILURE;
        default:
            return EXIT_FAILURE;
        }
    }

    printf("%s\n", fscurrdir());
    return EXIT_SUCCESS;
}

SHELL int shellmain_pwd(int argc, char **argv)
{
    if (argc < 1)
    {
        printf("Error: No command specified\n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[0], "pwd") == 0)
    {
        return pwd_command(argc, argv);
    }
    else
    {
        printf("Error: Unknown command '%s'\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}