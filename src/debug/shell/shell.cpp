#include "shell.h"

extern shelllist *commands;
extern void shell_command_list(void);

static void shell_help_print(void)
{
    printf("Usage: shell [options]\n");
    printf("Options:\n");
    printf("  -h, --help              Show this help message and exit\n");
    printf("  -l, --list              Print all supported commands by the shell\n");
}

static int server_command(int argc, char **argv)
{
    int opt;

    static option long_options[] = {
        {"help", NO_ARGUMENT, 0, 'h'},
        {"list", NO_ARGUMENT, 0, 'l'},
        {0, 0, 0, 0}};

    int option_index = 0;
    optind = 1;

    while ((opt = getopt_long(argc, argv, "hl", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'h':
            shell_help_print();
            return EXIT_SUCCESS;
        case 'l':
            printf("Shell command list:\n");
            shell_command_list();
            return EXIT_SUCCESS;
        case '?':
            printf("Error: Invalid option\n");
            return EXIT_FAILURE;
        default:
            return EXIT_FAILURE;
        }
    }

    if (optind < argc)
    {
        printf("Error: unexpected argument: %s\n", argv[optind]);
        return EXIT_FAILURE;
    }

    shell_help_print();

    return EXIT_SUCCESS;
}

int shellmain_shell(int argc, char **argv)
{
    if (argc < 1)
    {
        printf("Error: No command specified\n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[0], "shell") == 0)
    {
        return server_command(argc, argv);
    }
    else
    {
        printf("Error: Unknown command '%s'\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

    return 0;
}
