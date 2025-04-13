#include "shell.h"

static void reboot_help_print(void)
{
    printf("Usage: reboot [options]\n");
    printf("Options:\n");
    printf("  -h, --help              Show this help message and exit\n");
}

static int reboot_command(int argc, char **argv)
{
    int opt;

    static option long_options[] = {
        {"help", NO_ARGUMENT, 0, 'h'},
        {0, 0, 0, 0}};

    int option_index = 0;
    optind = 1;

    while ((opt = getopt_long(argc, argv, "hl", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'h':
            reboot_help_print();
            return EXIT_SUCCESS;
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

    printf("Rebooting...\n");
    ESP.restart();

    return EXIT_SUCCESS;
}

int shellmain_reboot(int argc, char **argv)
{
    return reboot_command(argc, argv);
}
