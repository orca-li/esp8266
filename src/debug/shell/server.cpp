#include "shell.h"

#define PUBLIC

#define SERVER_STATUS_CONNECTED 1
#define SERVER_STATUS_DISCONNECTED 0
#define SERVER_MODE_AUTO 1
#define SERVER_MODE_MANUAL 0

ESP8266WebServer server(80);

struct SERVERCTL
{
    bool status : 1;
    bool mode : 1;
} serverctl = {
    .status = SERVER_STATUS_DISCONNECTED,
    .mode = SERVER_MODE_AUTO,
};

PUBLIC void server_handler(void)
{
    if (GetWifiStatus() == WIFI_STATUS_DISCONNECTED)
        serverctl.status = SERVER_STATUS_DISCONNECTED;
    else
        serverctl.status = SERVER_STATUS_CONNECTED;
    server.handleClient();
}

PUBLIC void ServerSendJson(String jsondata)
{
    server.send(200, "application/json", jsondata);
}

static void ServerSendFile(const char *path, const char *contentType)
{
    File file = LittleFS.open(path, "r");
    if (!file)
        return;
    server.streamFile(file, contentType);
    file.close();
}

static void JavaScriptPathHandle(void)
{
    ServerSendFile("/www/script.js", "application/javascript");
}

static void HandleDNSInit(void)
{
    if (MDNS.begin("esp8266"))
        Serial.println("mDNS responder started");
    else
        Serial.println("Error setting up MDNS responder!");
}

static void RootPathHandle(void)
{
    ServerSendFile("/www/index.html", "text/html");
}

static void NotFoundPathHandle(void)
{
    server.send(404, "text/plain", "404: Not found");
}

static void SetupRoutesInit(void)
{
    server.on("/", RootPathHandle);
    server.onNotFound(NotFoundPathHandle);
}

static void StartServerInit(void)
{
    server.begin();
    server.on("/www/script.js", JavaScriptPathHandle);
    Serial.println("HTTP server started");
}

PUBLIC void ServerInit(void)
{
    printf("Server Init...\n");
    HandleDNSInit();
    SetupRoutesInit();
    StartServerInit();
}

static void server_help_print(void)
{
    printf("Usage: server [options]\n");
    printf("Options:\n");
    printf("  -h, --help              Show this help message and exit\n");
}

static int server_command(int argc, char **argv)
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
            server_help_print();
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
        printf("Server:\n\t[mode]\t%s\n", (serverctl.mode == SERVER_MODE_MANUAL) ? "MANUAL" : "AUTO");
        printf("\t[status]: %s\n", (serverctl.status == SERVER_STATUS_DISCONNECTED) ? "disconnect" : "connected");
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}

int shellmain_server(int argc, char **argv)
{
    if (argc < 1)
    {
        printf("Error: No command specified\n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[0], "server") == 0)
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
