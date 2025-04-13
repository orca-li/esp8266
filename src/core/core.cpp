#include "../../inc/esphdrs.h"

static void SerialInit(void)
{
    Serial.begin(115200);
	delay(10);
	Serial.println('\n');
    Serial.println("Serial Init...");
}

static void GPIOInit(void)
{
    pinMode(LED_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    Serial.println("GPIO Init...");
}

extern void SensorsInit(void);
extern void FsInit(void);
extern void WiFiInit(void);
extern void ServerInit(void);
extern void TryConnect(void);

void core_init(void)
{
    SerialInit();
    GPIOInit();
    SensorsInit();
    FsInit();
    WiFiInit();
    TryConnect();
    ServerInit();
    Serial.println("Core init finish...");
}
