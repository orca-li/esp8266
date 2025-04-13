#include "shell.h"

#define PUBLIC
#define SHELL

#define MAC_ADDRESS_SIZE 6
#define MAC_ADDRESS_F02hhx_SIZE (MAC_ADDRESS_SIZE * 2 + sizeof(":::::"))
#define SSID_SIZE 64
#define PASSWORD_SIZE 64
#define MAX_LINE_LENGTH (SSID_SIZE + sizeof(' ') + PASSWORD_SIZE + sizeof('\n'))
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
#define WIFI_CONFIG_FILE "/home/wifi.cfg"

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

static void WifiCfgWriteLineToFile(File &file, const char *line)
{
    file.println(line);
}

char *WifiCfgGetLineFromFile(int lineNumber)
{
    File configFile = LittleFS.open(WIFI_CONFIG_FILE, "r");
    if (!configFile)
    {
        return NULL;
    }

    char *line = (char *)malloc(MAX_LINE_LENGTH * sizeof(char));
    if (line == NULL)
    {
        configFile.close();
        return NULL;
    }

    int currentLine = 0;
    while (configFile.available())
    {
        int bytesRead = configFile.readBytesUntil('\n', line, MAX_LINE_LENGTH - 1);
        if (bytesRead > 0)
        {
            line[bytesRead] = '\0';
            currentLine++;

            if (currentLine == lineNumber)
            {
                configFile.close();
                size_t len = strlen(line);
                if (len > 0)
                {
                    if (line[len - 1] == '\n' || line[len - 1] == '\r')
                    {
                        line[len - 1] = '\0';
                        len--;
                    }
                    if (len > 0 && line[len - 1] == '\r')
                        line[len - 1] = '\0';
                }
                return line;
            }
        }
        else
            break;
    }

    configFile.close();
    free(line);
    return NULL;
}

static void WifiCfgAppendWiFiConfig(const char *ssid, const char *password)
{
    File configFile = LittleFS.open(WIFI_CONFIG_FILE, "a");
    if (!configFile)
    {
        Serial.print("Error: Could not open ");
        Serial.print(WIFI_CONFIG_FILE);
        Serial.println(" for appending.");
        return;
    }

    configFile.printf("%s %s\n", ssid, password);

    configFile.close();
    Serial.print("WiFi configuration added to ");
    Serial.println(WIFI_CONFIG_FILE);
}

static void WifiCfgCreateCommentedFile(void)
{
    File configFile = LittleFS.open(WIFI_CONFIG_FILE, "w");
    if (!configFile)
    {
        Serial.print("Error: Could not create or open ");
        Serial.print(WIFI_CONFIG_FILE);
        Serial.println(" for writing.");
        return;
    }

    WifiCfgWriteLineToFile(configFile, "# wifi -h");
    WifiCfgWriteLineToFile(configFile, "# ssid password");

    configFile.close();
    Serial.print(WIFI_CONFIG_FILE);
    Serial.println(" created with comments.");
}

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

static char *getWordFromLine(const char *line, int wordNumber)
{
    if (line == NULL || wordNumber <= 0)
        return NULL;

    char *lineCopy = strdup(line);
    if (lineCopy == NULL)
        return NULL;

    char *token;
    int currentWord = 0;

    token = strtok(lineCopy, " \t\n");

    while (token != NULL)
    {
        currentWord++;
        if (currentWord == wordNumber)
        {
            char *result = strdup(token);
            free(lineCopy);
            return result;
        }
        token = strtok(NULL, " \t\n");
    }

    free(lineCopy);
    return NULL;
}

static void ShowWifiList(void)
{
    WiFi.mode(WIFI_STA);
    int n = WiFi.scanNetworks();

    Serial.print("Found ");
    Serial.print(n);
    Serial.println(" networks");

    if (n == 0)
        Serial.println("No networks found :(");
    else
    {
        Serial.println("Networks found:");
        for (int i = 0; i < n; ++i)
        {
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print("dBm)");
            Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " Unsecured" : "");
            delay(10);
        }
    }
}

static int TryConnectSSID(char *line)
{
    char *_ssid = getWordFromLine(line, 1);
    char *_password = getWordFromLine(line, 2);

    strcpy(ssid, _ssid);
    strcpy(password, _password);
    wifictl.ssid = ssid;
    wifictl.password = password;
    free(_ssid);
    free(_password);

    WiFi.begin(wifictl.ssid, wifictl.password);
    check_connect();
    if (wifictl.status == WIFI_STATUS_CONNECTED)
        return EXIT_SUCCESS;

    return EXIT_FAILURE;
}

PUBLIC void TryConnect(void)
{
    printf("Connection attempt via config: %s\n", WIFI_CONFIG_FILE);

    int line = 1;
    char *credential = NULL;
    char *word = NULL;
    while (1)
    {
        credential = WifiCfgGetLineFromFile(line++);
        if (!credential)
            goto cannot_connect;
        if (credential[0] == '#')
        {
            free(credential);
            continue;
        }

        word = getWordFromLine(credential, 1);
        printf("Try connect to %s...\n", word);
        if (TryConnectSSID(credential) != EXIT_FAILURE)
        {
            free(credential);
            printf("%s connected!\n", word);
            free(word);
            return;
        }
        free(credential);
        free(word);
    }

cannot_connect:
    printf("Cann't connect. Show ssid list...\n");
    ShowWifiList();
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

static int add_ssid_password(int argc, char **argv)
{

    if ((argv[2] == NULL || argv[3] == NULL) && argc < 4)
        goto bad_usage;

    WifiCfgAppendWiFiConfig(argv[2], argv[3]);
    printf("The installation of the ssid and password was successful!\n");
    return EXIT_SUCCESS;

bad_usage:
    printf("Error: Password required after SSID.\n");
    printf("Usage: wifi -a <ssid> <password>\n");
    return EXIT_FAILURE;
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
    printf("  -h, --help                          Show this help message and exit\n");
    printf("  -c, --connect  [ssid] [password]    Connect to the network\n");
    printf("  -H, --hidden-pswd=MODE              Hidden password set mode (yes, no)\n");
    printf("  -a, --add-wifi [ssid] [password]    Add a wifi network\n");
    printf("  -d, --delete-config                 Remove all networks from wifi.cfg\n");
    printf("  -s, --show                          Show ssid list\n");
    printf("  -r, --reconnect                     Reconnect\n");
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
        {"add-wifi", REQUIRED_ARGUMENT, 0, 'a'},
        {"delete-config", NO_ARGUMENT, 0, 'd'},
        {"show", NO_ARGUMENT, 0, 's'},
        {"reconnect", NO_ARGUMENT, 0, 'r'},
        {0, 0, 0, 0}};

    int option_index = 0;
    optind = 1;

    while ((opt = getopt_long(argc, argv, "hcH:a:dsr", long_options, &option_index)) != -1)
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
        case 'a':
            return add_ssid_password(argc, argv);
        case 's':
            ShowWifiList();
            return EXIT_SUCCESS;
        case 'd':
            WifiCfgCreateCommentedFile();
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
