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

void core_init(void)
{
    SerialInit();
    GPIOInit();
    Serial.println("Core init finish...");
}
