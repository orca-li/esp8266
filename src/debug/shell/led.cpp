#include "shell.h"

#define PUBLIC
#define SHELL

#define LED_PIN 2 // D4 (GPIO2)

typedef void (*led_handler)(void);
typedef enum
{
    LED_MODE_BLINK,
    LED_MODE_MANUAL,
    LED_MODE_DEFAULT = LED_MODE_BLINK,
} LED_MODES;
#define LED_MODE_INIT LED_MODE_DEFAULT
#define LED_DELAY_DEFAULT 1000
#define LED_POWER_ON 0
#define LED_POWER_OFF 1

static void _debug_led_init(void);

struct LEDCTL
{
    LED_MODES mode;
    int delay;
    int power;
    led_handler handler;
    int pin;
} ledctl = {
    .mode = LED_MODE_INIT,
    .delay = LED_DELAY_DEFAULT,
    .power = LED_POWER_ON,
    .handler = _debug_led_init,
    .pin = LED_PIN,
};

static void togglePin(int pin)
{
    digitalWrite(pin, !digitalRead(pin));
    ledctl.power = !ledctl.power;
}

static void led_mode_handler(void)
{
    if (ledctl.mode == LED_MODE_BLINK)
    {
        togglePin(ledctl.pin);
    }

    if (ledctl.mode == LED_MODE_MANUAL)
    {
        digitalWrite(ledctl.pin, ledctl.power);
    }
}

static void _debug_led(void)
{
    if_time_has_come(ledctl.delay, led_mode_handler());
}

static void _debug_led_init(void)
{
    digitalWrite(ledctl.pin, ledctl.power);
    ledctl.handler = _debug_led;
}

PUBLIC void debug_led(void)
{
    ledctl.handler();
}

void led_help_print(void)
{
    printf("Usage: led [options]\n");
    printf("Options:\n");
    printf("  -h, --help              Show this help message and exit\n");
    printf("  -m, --mode=MODE         Set the LED mode (blink, set, reset, default)\n");
    printf("  -d, --delay=MODE        Set the value (ms)\n");
}

int led_command(int argc, char **argv)
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

    int new_delay = LED_DELAY_DEFAULT;
    while ((opt = getopt_long(argc, argv, "hd:m:", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'h':
            led_help_print();
            return EXIT_SUCCESS;
        case 'd':
            char *endptr;
            new_delay = strtol(optarg, &endptr, 10);

            if (*endptr != '\0' || optarg == endptr)
            {
                printf("Error: '%s' is not number\n", optarg);
                printf("Set default delay %d\n", LED_DELAY_DEFAULT);
                ledctl.delay = LED_DELAY_DEFAULT;
                return EXIT_FAILURE;
            }

            ledctl.delay = new_delay;
            printf("Setting delay to %d\n", ledctl.delay);
            return EXIT_SUCCESS;
        case 'm':
            mode_flag = 1;
            if (strcmp(optarg, "blink") == 0)
            {
                printf("Setting LED mode to blink\n");
                ledctl.mode = LED_MODE_BLINK;
            }
            else if (strcmp(optarg, "set") == 0)
            {
                printf("Setting LED mode to set\n");
                ledctl.mode = LED_MODE_MANUAL;
                ledctl.power = LED_POWER_ON;
            }
            else if (strcmp(optarg, "reset") == 0)
            {
                printf("Setting LED mode to reset\n");
                ledctl.mode = LED_MODE_MANUAL;
                ledctl.power = LED_POWER_OFF;
            }
            else if (strcmp(optarg, "default") == 0)
            {
                printf("Setting LED mode to default\n");
                ledctl.mode = LED_MODE_DEFAULT;
                ledctl.delay = LED_DELAY_DEFAULT;
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

    if (ledctl.mode == LED_MODE_DEFAULT && ledctl.delay == LED_DELAY_DEFAULT)
    {
        printf("Led:\n\t[mode]\t%s\n", "DEFAULT");
        printf("\t[delay]\t%d\n", ledctl.delay);
        printf("\t[power]\t%s\n", (ledctl.power == LED_POWER_ON) ? "on" : "off");

        return EXIT_SUCCESS;
    }
    printf("Led:\n\t[mode]\t%s\n", (ledctl.mode == LED_MODE_BLINK) ? "BLINK" : "MANUAL");
    printf("\t[delay]\t%d\n", ledctl.delay);
    printf("\t[power]\t%s\n", (ledctl.power == LED_POWER_ON) ? "on" : "off");

    return EXIT_SUCCESS;
}

SHELL int shellmain_led(int argc, char **argv)
{
    if (argc < 1)
    {
        printf("Error: No command specified\n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[0], "led") == 0)
    {
        return led_command(argc, argv);
    }
    else
    {
        printf("Error: Unknown command '%s'\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
