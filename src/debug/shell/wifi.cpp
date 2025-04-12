#include "shell.h"

#define PUBLIC
#define SHELL

#define MAC_ADDRESS_SIZE 6
#define MAC_ADDRESS_F02hhx_SIZE (MAC_ADDRESS_SIZE * 2 + sizeof(":::::"))
#define SSID_SIZE 64
#define PASSWORD_SIZE 64
#define IPV4_OCTET_SIZE 3
#define IPV4_SIZE (IPV4_OCTET_SIZE * 4 + sizeof("..."))

#define WIFI_MAC_DEFAULT NULL
#define WIFI_IPV4_DEFAULT NULL
#define WIFI_SSID_DEFAULT NULL
#define WIFI_PASSWORD_DEFAULT NULL
#define WIFI_CONNECTION_DELAY_MAX_DEFAULT 2000
#define WIFI_STATUS_DEFAULT WIFI_STATUS_DISCONNECTED
#define WIFI_HIDDEN_PSWD_DEFAULT WIFI_HIDDENT_PSWD_ENABLE
#define WIFI_HIDDENT_PSWD_ENABLE true
#define WIFI_HIDDENT_PSWD_DISABLE false

typedef enum
{
    WIFI_MODE_MANUAL,
    WIFI_MODE_AUTO,
    WIFI_MODE_DEFAULT = WIFI_MODE_MANUAL,
} WIFI_MODE;

static char mac_address[MAC_ADDRESS_F02hhx_SIZE];
static char ip_address[IPV4_SIZE];
static char ssid[SSID_SIZE];
static char password[PASSWORD_SIZE];

struct WIFICTL
{
    char *ipv4;
    char *mac;
    char *ssid;
    char *password;
    int connection_delay_max;
    WIFI_MODE mode;
    bool status : 1;
    bool hidden_password : 1;
    bool connect_repeat : 1;
} wifictl{
    .ipv4 = WIFI_IPV4_DEFAULT,
    .mac = WIFI_MAC_DEFAULT,
    .ssid = WIFI_SSID_DEFAULT,
    .password = WIFI_PASSWORD_DEFAULT,
    .connection_delay_max = WIFI_CONNECTION_DELAY_MAX_DEFAULT,
    .mode = WIFI_MODE_DEFAULT,
    .status = WIFI_STATUS_DEFAULT,
    .hidden_password = WIFI_HIDDEN_PSWD_DEFAULT,
};

ESP8266WiFiMulti wifiMulti;

static char *get_ipv4(void)
{
    IPAddress ipv4 = WiFi.localIP();
    snprintf(ip_address, sizeof(ip_address), "%d.%d.%d.%d", ipv4[0], ipv4[1], ipv4[2], ipv4[3]);

    return ip_address;
}

PUBLIC bool GetWifiStatus(void)
{
    return wifictl.status;
}

static void check_connect(void)
{
    if (wifiMulti.run() != WL_CONNECTED)
        return;

    wifictl.status = WIFI_STATUS_CONNECTED;
    wifictl.ipv4 = get_ipv4();
}

static bool connect_config(void)
{
    if ((wifictl.ssid == NULL || wifictl.password == NULL) || wifictl.status == WIFI_STATUS_CONNECTED)
        return true;

    if (wifictl.connect_repeat == false)
    {
        wifiMulti.addAP(wifictl.ssid, wifictl.password);
        wifictl.connect_repeat = true;
    }

    return false;
}

static void _wifi_handler(void)
{
    if (connect_config())
        return;
    check_connect();
}

PUBLIC void wifi_handler(void)
{
    if_time_has_come(250, _wifi_handler());
}

PUBLIC void WiFiInit(void)
{
    printf("Getting MAC address...\n");
    uint8_t mac[6];
    wifi_get_macaddr(STATION_IF, mac);
    snprintf(mac_address, sizeof(mac_address), "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    wifictl.mac = mac_address;
    printf("WiFi driver Init...\n");
}

static char *print_password(void)
{
    static char hidden_password[PASSWORD_SIZE] = {0};

    if (wifictl.hidden_password == WIFI_HIDDENT_PSWD_ENABLE)
    {
        int pswdlen = strlen(password);
        int exitidx = 0;
        if (pswdlen == 0)
            return NULL;
        for (int i = 0; i < pswdlen;)
        {
            hidden_password[i] = '*';
            exitidx = ++i;
        }
        hidden_password[exitidx] = '\0';
        return hidden_password;
    }

    return wifictl.password;
}

static int connect_handler(int argc, char **argv)
{
    if (optind < argc)
    {
        wifictl.ssid = strdup(argv[optind]);
        strcpy(ssid, strdup(argv[optind]));
        ssid[strlen(argv[optind])] = '\0';
        optind++;
        if (optind < argc)
        {
            wifictl.password = strdup(argv[optind]);
            strcpy(password, strdup(argv[optind]));
            password[strlen(argv[optind])] = '\0';
            optind++;
        }
        else
        {
            printf("Error: Password required after SSID.\n");
            printf("Usage: wifi -c <ssid> <password>\n");
            free(wifictl.ssid);
            wifictl.ssid = NULL;
            return EXIT_FAILURE;
        }
    }

    printf("The installation of the ssid and password was successful!\n");

    return EXIT_SUCCESS;
}

static void wifi_help_print(void)
{
    printf("Usage: wifi [options]\n");
    printf("Options:\n");
    printf("  -h, --help                         Show this help message and exit\n");
    printf("  -c, --connect [ssid] [password]    Connect to the network\n");
    printf("  -H, --hidden-pswd=MODE             Hidden password set mode (yes, no)\n");
    printf("\nAlso see the configuration file in the following path:\n");
    printf("\t/home/wifi.cfg\n");
    printf("Fill in this file with the necessary wifi routers using the following\n");
    printf("template:\n");
    printf("\t<ssid> <password>\n");
    printf("\t<ssid> <password>\n");
    printf("\t<ssid> <password>\n");
}

static int wifi_command(int argc, char **argv)
{
    int opt;

    static option long_options[] = {
        {"help", NO_ARGUMENT, 0, 'h'},
        {"connect", NO_ARGUMENT, 0, 'c'},
        {"hidden-pswd", REQUIRED_ARGUMENT, 0, 'H'},
        {0, 0, 0, 0}};

    int option_index = 0;
    optind = 1;

    while ((opt = getopt_long(argc, argv, "hcH:", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'h':
            wifi_help_print();
            return EXIT_SUCCESS;
        case 'c':
            return connect_handler(argc, argv);
        case 'H':
            if (strcmp(optarg, "yes") == 0)
            {
                wifictl.hidden_password = WIFI_HIDDENT_PSWD_ENABLE;
            }
            else if (strcmp(optarg, "no") == 0)
            {
                wifictl.hidden_password = WIFI_HIDDENT_PSWD_DISABLE;
            }
            return EXIT_SUCCESS;
        case '?':
            printf("Error: Invalid option\n");
            return EXIT_FAILURE;
        default:
            return EXIT_FAILURE;
        }
    }

    if (argc == 1)
    {
        printf("Wifi:\n\t[mode]\t%s\n", (wifictl.mode == WIFI_MODE_MANUAL) ? "MANUAL" : "AUTO");
        printf("\t[mac address]: %s\n", (wifictl.mac == NULL) ? "disconnect" : wifictl.mac);
        printf("\t[ip address]: %s\n", (wifictl.ipv4 == NULL) ? "disconnect" : wifictl.ipv4);
        printf("\t[ssid]: %s\n", (wifictl.ssid == NULL) ? "disconnect" : wifictl.ssid);
        printf("\t[password]: %s\n", (wifictl.password == NULL) ? "disconnect" : print_password());
        printf("\t[connection delay]: %d\n", wifictl.connection_delay_max);
        printf("\t[connection status]: %s\n", (wifictl.status) ? "connected" : "disconnected");
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}

SHELL int shellmain_wifi(int argc, char **argv)
{
    if (argc < 1)
    {
        printf("Error: No command specified\n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[0], "wifi") == 0)
    {
        return wifi_command(argc, argv);
    }
    else
    {
        printf("Error: Unknown command '%s'\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
