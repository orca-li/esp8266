#include "shell.h"

#define PUBLIC
#define SHELL

typedef void (*relay_handler_t)(void);
typedef enum
{
    RELAY_MODE_BLINK,
    RELAY_MODE_MANUAL,
    RELAY_MODE_TERMO,
    RELAY_MODE_DEFAULT = RELAY_MODE_TERMO,
} RELAY_MODES;
#define RELAY_MODE_INIT RELAY_MODE_DEFAULT
#define RELAY_DELAY_DEFAULT 1000
#define RELAY_POWER_ON 1
#define RELAY_POWER_OFF 0

static void _relay_init(void);

struct RELAYCTL
{
    RELAY_MODES mode;
    int delay;
    int power;
    relay_handler_t handler;
    int pin;
} relayctl = {
    .mode = RELAY_MODE_INIT,
    .delay = RELAY_DELAY_DEFAULT,
    .power = RELAY_POWER_OFF,
    .handler = _relay_init,
    .pin = RELAY_PIN,
};

static void togglePin(int pin)
{
    digitalWrite(pin, !digitalRead(pin));
    relayctl.power = !relayctl.power;
}

static void relay_mode_handler(void)
{
    if (relayctl.mode == RELAY_MODE_BLINK)
    {
        togglePin(relayctl.pin);
        return;
    }

    digitalWrite(relayctl.pin, relayctl.power);
}

static void _relay(void)
{
    if_time_has_come(relayctl.delay, relay_mode_handler());
}

static void _relay_init(void)
{
    digitalWrite(relayctl.pin, relayctl.power);
    relayctl.handler = _relay;
}

PUBLIC void relay_handler(void)
{
    relayctl.handler();
}

void relay_help_print(void)
{
    printf("Usage: relay [options]\n");
    printf("Options:\n");
    printf("  -h, --help              Show this help message and exit\n");
    printf("  -m, --mode=MODE         Set the RELAY mode (termo, blink, set, reset, default)\n");
    printf("  -d, --delay=MODE        Set the value (ms)\n");
}

int relay_command(int argc, char **argv)
{
    int opt;
    int mode_flag = 0;
    char *mode_value = nullptr;

    static option long_options[] = {
        {"help", NO_ARGUMENT, 0, 'h'},
        {"mode", REQUIRED_ARGUMENT, 0, 'm'},
        {"delay", REQUIRED_ARGUMENT, 0, 'd'},
        {0, 0, 0, 0}};

    int option_index = 0;
    optind = 1;

    int new_delay = RELAY_DELAY_DEFAULT;
    while ((opt = getopt_long(argc, argv, "hd:m:", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'h':
            relay_help_print();
            return EXIT_SUCCESS;
        case 'd':
            char *endptr;
            new_delay = strtol(optarg, &endptr, 10);

            if (*endptr != '\0' || optarg == endptr)
            {
                printf("Error: '%s' is not number\n", optarg);
                printf("Set default delay %d\n", RELAY_DELAY_DEFAULT);
                relayctl.delay = RELAY_DELAY_DEFAULT;
                return EXIT_FAILURE;
            }

            relayctl.delay = new_delay;
            printf("Setting delay to %d\n", relayctl.delay);
            return EXIT_SUCCESS;
        case 'm':
            mode_flag = 1;
            if (strcmp(optarg, "blink") == 0)
            {
                printf("Setting RELAY mode to blink\n");
                relayctl.mode = RELAY_MODE_BLINK;
            }
            else if (strcmp(optarg, "set") == 0)
            {
                printf("Setting RELAY mode to set\n");
                relayctl.mode = RELAY_MODE_MANUAL;
                relayctl.power = RELAY_POWER_ON;
            }
            else if (strcmp(optarg, "reset") == 0)
            {
                printf("Setting RELAY mode to reset\n");
                relayctl.mode = RELAY_MODE_MANUAL;
                relayctl.power = RELAY_POWER_OFF;
            }
            else if (strcmp(optarg, "termo") == 0)
            {
                printf("Setting RELAY mode to termo\n");
                relayctl.mode = RELAY_MODE_TERMO;
                relayctl.delay = 0;
            }
            else if (strcmp(optarg, "default") == 0)
            {
                printf("Setting RELAY mode to default\n");
                relayctl.mode = RELAY_MODE_DEFAULT;
                relayctl.delay = RELAY_DELAY_DEFAULT;
            }
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

    if (relayctl.mode == RELAY_MODE_TERMO)
    {
        printf("Relay:\n\t[mode]\t%s\n", "TERMO");
    }
    else if (relayctl.mode == RELAY_MODE_MANUAL)
    {
        printf("Relay:\n\t[mode]\t%s\n", "MANUAL");
    }
    else if (relayctl.mode == RELAY_MODE_BLINK)
    {
        printf("Relay:\n\t[mode]\t%s\n", "BLINK");
    }
    else
    {
        printf("Relay:\n\t[mode]\t%s\n", "ERROR");
    }
    printf("\t[delay]\t%d\n", relayctl.delay);
    printf("\t[power]\t%s\n", (relayctl.power == RELAY_POWER_ON) ? "on" : "off");

    return EXIT_SUCCESS;
}

PUBLIC void PublicRelaySet(void)
{
    if ((relayctl.mode == RELAY_MODE_MANUAL) || (relayctl.mode == RELAY_MODE_BLINK))
        return;
    relayctl.power = RELAY_POWER_ON;
}

PUBLIC void PublicRelayReset(void)
{
    if ((relayctl.mode == RELAY_MODE_MANUAL) || (relayctl.mode == RELAY_MODE_BLINK))
        return;
    relayctl.power = RELAY_POWER_OFF;
}

SHELL int shellmain_relay(int argc, char **argv)
{
    if (argc < 1)
    {
        printf("Error: No command specified\n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[0], "relay") == 0)
    {
        return relay_command(argc, argv);
    }
    else
    {
        printf("Error: Unknown command '%s'\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
