#include "shell.h"
#include <string.h>

#define MAX_LINE_LENGTH 100
#define MQTT_CONFIG_FILE "mqtt.cfg"

char mqttIp[32];
char mqttPort[16];
char mqttUser[64];
char mqttPassword[32];

const char *mqtt_server = NULL;
int mqtt_port = -1;
const char *mqtt_user = NULL;
const char *mqtt_pass = NULL;
const char *topic_manual_temp = "esp8266/manual_temp";
const char *topic_auto_mode = "esp8266/auto_mode";
const char *topic_sensor_temp = "esp8266/sensor_temp";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

#define MQTT_STARTED true
#define MQTT_NOT_STARTED false

struct MQTTCTL
{
    bool status : 1;
} mqttctl = {
    .status = MQTT_NOT_STARTED,
};
static char *WifiCfgGetLineFromFile(int lineNumber);
static char *getWordFromLine(const char *line, int wordNumber);
static int ConnectViaConfig(char *line)
{
    char *ip = getWordFromLine(line, 1);
    char *port = getWordFromLine(line, 2);
    char *user = getWordFromLine(line, 3);
    char *password = getWordFromLine(line, 4);

    strcpy(mqttIp, ip);
    strcpy(mqttPort, port);
    strcpy(mqttUser, user);
    strcpy(mqttPassword, password);
    mqtt_port = atoi(mqttPort);
    mqtt_server = mqttIp;
    mqtt_user = mqttUser;
    mqtt_pass = mqttPassword;

    free(ip);
    free(port);
    free(user);
    free(password);

    return 0;
}

void TryConnectViaConfig(void)
{
    printf("Connection attempt via config: %s\n", MQTT_CONFIG_FILE);

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
        ConnectViaConfig(credential);
        free(credential);
        free(word);
    }

cannot_connect:
    printf("Cann't connect.\n");
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

static void WifiCfgWriteLineToFile(File &file, const char *line)
{
    file.println(line);
}

static char *WifiCfgGetLineFromFile(int lineNumber)
{
    File configFile = LittleFS.open(MQTT_CONFIG_FILE, "r");
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


static void MqttCfgAppendWiFiConfig(const char *ip, const char *port, const char *login, const char *password)
{
    File configFile = LittleFS.open(MQTT_CONFIG_FILE, "a");
    if (!configFile)
    {
        Serial.print("Error: Could not open ");
        Serial.print(MQTT_CONFIG_FILE);
        Serial.println(" for appending.");
        return;
    }

    configFile.printf("%s %s %s %s\n", ip, port, login, password);

    configFile.close();
    Serial.print("Mqtt configuration added to ");
    Serial.println(MQTT_CONFIG_FILE);
}

static void WifiCfgCreateCommentedFile(void)
{
    File configFile = LittleFS.open(MQTT_CONFIG_FILE, "w");
    if (!configFile)
    {
        Serial.print("Error: Could not create or open ");
        Serial.print(MQTT_CONFIG_FILE);
        Serial.println(" for writing.");
        return;
    }

    WifiCfgWriteLineToFile(configFile, "# mqtt -h");
    WifiCfgWriteLineToFile(configFile, "# ip port login password");

    configFile.close();
    Serial.print(MQTT_CONFIG_FILE);
    Serial.println(" created with comments.");
}

void mqttPublishTemperaturechar(char *temp)
{
    mqttClient.publish(topic_sensor_temp, temp);
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message received! Topic: ");
    Serial.println(topic);

    char message[length + 1];
    for (int i = 0; i < length; i++)
    {
        message[i] = (char)payload[i];
    }
    message[length] = '\0';

    if (strcmp(topic, topic_manual_temp) == 0)
    {
        float manualTemperature = atof(message);
        SetNewManualTemp(manualTemperature);
    }
    else if (strcmp(topic, topic_auto_mode) == 0)
    {
        SetAutoSensorTemp();
    }
}

void reconnect(void)
{
    while (!mqttClient.connected())
    { // Add while loop
        Serial.println("\nAttempting MQTT connection...");
        printf("Mqtt Client: %s %s\n", mqtt_user, mqtt_pass);
        if (mqttClient.connect("ESP8266Client", mqtt_user, mqtt_pass))
        {
            Serial.println("MQTT connected!");
            mqttctl.status = MQTT_STARTED;
            mqttClient.subscribe(topic_manual_temp);
            mqttClient.subscribe(topic_auto_mode);
        }
        else
        {
            mqttctl.status = MQTT_NOT_STARTED;
            Serial.print("MQTT connection failed, rc=");
            Serial.println(mqttClient.state());
        }
    }
}

void MqttInit(void)
{
    TryConnectViaConfig();
    printf("Mqtt broker: %s %d\n", mqtt_server, mqtt_port);
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(callback);
    printf("Mqtt protocol init...\n");
}

extern bool GetWifiStatus(void);

void mqtt_handler(void)
{
    if (GetWifiStatus() != WIFI_STATUS_CONNECTED || !(mqtt_server != NULL && mqtt_port != -1 && mqtt_user != NULL && mqtt_pass != NULL))
        return;
    if (!mqttClient.connected())
        if_time_has_come(5000, reconnect());
    mqttClient.loop();
    SendCurrTempMqtt();
}

static void mqtt_help_print(void)
{
    printf("Usage: mqtt [options]\n");
    printf("Options:\n");
    printf("  -h, --help                                      Show this help message and exit\n");
    printf("  -a, --add-mqtt [ip] [port] [user] [password]    Add a wifi network\n");
    printf("  -d, --delete-config                             Remove all brokers from mqtt.cfg\n");
}

static int mqtt_command(int argc, char **argv)
{
    int opt;

    static option long_options[] = {
        {"help", NO_ARGUMENT, 0, 'h'},
        {"start", NO_ARGUMENT, 0, 's'},
        {"stop", NO_ARGUMENT, 0, 'S'},
        {0, 0, 0, 0}};

    int option_index = 0;
    optind = 1;

    while ((opt = getopt_long(argc, argv, "hlsSa:d", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'h':
            mqtt_help_print();
            return EXIT_SUCCESS;
        case 's':
            mqttctl.status = MQTT_STARTED;
            return EXIT_SUCCESS;
        case 'S':
            mqttctl.status = MQTT_NOT_STARTED;
            return EXIT_SUCCESS;
        case 'a':
            MqttCfgAppendWiFiConfig(argv[2], argv[3], argv[4], argv[5]);
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

    if (optind < argc)
    {
        printf("Error: unexpected argument: %s\n", argv[optind]);
        return EXIT_FAILURE;
    }

    if (argc == 1)
    {
        printf("MQTT:\n");
        printf("\t[ip]: %s\n", (mqtt_server == NULL) ? "(null)" : mqtt_server);
        printf("\t[port]: %d\n", mqtt_port);
        printf("\t[user]: %s\n", (mqtt_user == NULL) ? "(null)" : mqtt_user);
        printf("\t[password]: %s\n", (mqtt_pass == NULL) ? "(null)" : mqtt_pass);
        printf("\t[connection status]: %s\n", (mqttctl.status == MQTT_STARTED) ? "connected" : "disconnected");
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}

int shellmain_mqtt(int argc, char **argv)
{
    return mqtt_command(argc, argv);
}
