#include "shell.h"

#define PUBLIC
#define SHELL

typedef void (*termo_handler_t)(void);

static void _termo_init(void);
static float getAverageTemperature(void);

typedef enum
{
    TERMO_MODE_MANUAL,
    TERMO_MODE_AUTO,
    TERMO_MODE_DEFAULT = TERMO_MODE_AUTO,
} TERMO_MODES;

#define TERMO_MODE_INIT TERMO_MODE_DEFAULT
#define TERMO_DELAY_DEFAULT 1000
#define TERMO_TEMPERATURE_DEFAULT 0.00
#define TERMO_SHOW_LOGS_DEFAULT false

struct TERMOCTL
{
    TERMO_MODES mode;
    int delay;
    termo_handler_t handler;
    float temperature;
    float tempmanual;
    bool show_logs : 1;
} termoctl = {
    .mode = TERMO_MODE_INIT,
    .delay = TERMO_DELAY_DEFAULT,
    .handler = _termo_init,
    .temperature = TERMO_TEMPERATURE_DEFAULT,
    .tempmanual = TERMO_TEMPERATURE_DEFAULT,
    .show_logs = TERMO_SHOW_LOGS_DEFAULT,
};

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress temperatureSensors[TERMO_SENSORS_COUNT];
uint8_t deviceCount = 0;

static void printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        printf("%02hhx", deviceAddress[i]);
    }
}

static void GetCurrTemp(void)
{
    sensors.requestTemperatures();

    termoctl.temperature = getAverageTemperature();
}

static float getAverageTemperature(void)
{
    float averageTempC = 0;

    sensors.requestTemperatures();
    for (int i = 0; i < deviceCount; i++)
    {
        averageTempC += sensors.getTempC(temperatureSensors[i]);
    }

    averageTempC /= deviceCount;

    return averageTempC;
}

PUBLIC void SensorsInit(void)
{
    Serial.println("Sensors Init...");
    sensors.begin();
    deviceCount = sensors.getDeviceCount();

    for (uint8_t i = 0; i < deviceCount; i++)
        sensors.getAddress(temperatureSensors[i], i);
}

static void termo_mode_handler(void)
{
    if (termoctl.mode == TERMO_MODE_AUTO)
    {
        GetCurrTemp();
    }
}

static void _termo(void)
{
    if_time_has_come(termoctl.delay, termo_mode_handler());
}

static void _termo_init(void)
{
    termoctl.handler = _termo;
}

PUBLIC void termo_handler(void)
{
    termoctl.handler();
}

void termo_help_print(void)
{
    printf("Usage: termo [options]\n");
    printf("Options:\n");
    printf("  -h, --help                        Show this help message and exit\n");
    printf("  -m, --mode=MODE                   Set the TERMO mode (manual, auto, default)\n");
    printf("  -d, --delay=MODE                  Set the value (ms)\n");
    printf("  -t, --set-temperature=MODE        Set the temperature (*C)\n");
}

int termo_command(int argc, char **argv)
{
    int opt;
    int mode_flag = 0;
    char *mode_value = nullptr;

    static option long_options[] = {
        {"help", NO_ARGUMENT, 0, 'h'},
        {"mode", REQUIRED_ARGUMENT, 0, 'm'},
        {"delay", REQUIRED_ARGUMENT, 0, 'd'},
        {"set-temperature", REQUIRED_ARGUMENT, 0, 't'},
        {0, 0, 0, 0}};

    int option_index = 0;
    optind = 1;

    int new_delay = TERMO_DELAY_DEFAULT;
    float new_temperature = TERMO_TEMPERATURE_DEFAULT;
    char *endptr = NULL;
    while ((opt = getopt_long(argc, argv, "hd:m:t:", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'h':
            termo_help_print();
            return EXIT_SUCCESS;

        case 't':
            termoctl.mode = TERMO_MODE_MANUAL;
            new_temperature = strtof(optarg, &endptr);

            if (*endptr != '\0' || optarg == endptr)
            {
                printf("Error: '%s' is not number\n", optarg);
                printf("Set default temperature %d\n", TERMO_TEMPERATURE_DEFAULT);
                termoctl.tempmanual = TERMO_TEMPERATURE_DEFAULT;
                return EXIT_FAILURE;
            }

            termoctl.tempmanual = new_temperature;
            termoctl.temperature = termoctl.tempmanual;
            printf("Setting temperature to %d\n", termoctl.tempmanual);
            return EXIT_SUCCESS;

        case 'd':
            new_delay = strtol(optarg, &endptr, 10);

            if (*endptr != '\0' || optarg == endptr)
            {
                printf("Error: '%s' is not number\n", optarg);
                printf("Set default delay %d\n", TERMO_DELAY_DEFAULT);
                termoctl.delay = TERMO_DELAY_DEFAULT;
                return EXIT_FAILURE;
            }

            termoctl.delay = new_delay;
            printf("Setting delay to %d\n", termoctl.delay);
            return EXIT_SUCCESS;

        case 'm':
            mode_flag = 1;
            if (strcmp(optarg, "auto") == 0)
            {
                printf("Setting TERMO mode to auto\n");
                termoctl.mode = TERMO_MODE_AUTO;
            }
            else if (strcmp(optarg, "manual") == 0)
            {
                printf("Setting TERMO mode to manual\n");
                termoctl.mode = TERMO_MODE_MANUAL;
                termoctl.temperature = termoctl.tempmanual;
            }
            else if (strcmp(optarg, "default") == 0)
            {
                printf("Setting TERMO mode to default\n");
                termoctl.mode = TERMO_MODE_DEFAULT;
                termoctl.delay = TERMO_DELAY_DEFAULT;
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

    printf("Termo:\n\t[mode]\t%s\n", (termoctl.mode = TERMO_MODE_MANUAL) ? "MANUAL" : "AUTO");
    printf("\t[delay]\t%d\n", termoctl.delay);
    printf("\t[devices]:\n");
    for (int i = 0; i < deviceCount; ++i)
    {
        printf("\t  sensor[%d] ", i);
        printAddress(temperatureSensors[i]);
        printf("(temperature: %.2f)\n", sensors.getTempC(temperatureSensors[i]));
    }
    printf("\t[temperature *C]: %.2f\n", termoctl.temperature);

    return EXIT_SUCCESS;
}

SHELL int shellmain_termo(int argc, char **argv)
{
    if (argc < 1)
    {
        printf("Error: No command specified\n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[0], "termo") == 0)
    {
        return termo_command(argc, argv);
    }
    else
    {
        printf("Error: Unknown command '%s'\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
