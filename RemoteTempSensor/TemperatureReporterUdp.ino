// Remote wireless temperature sensor
// HW: NodeMCU + DHT22 + 12 V battery

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>


 // this is my local file with following definitions:
 //const char* ssid     = "********";
 //const char* password = "********";
 #include "wifi_passcode.h"

// Uncomment for debug logs on serial console
#define DEBUG 1

#ifdef DEBUG
#define LOG Serial.print
#define LOGLN Serial.println
#else
#define LOG
#define LOGLN
#endif

#define DHTPIN      14      // 14 = D5 on nodeMCU
#define DHTTYPE     DHT22   // DHT 22 (AM2302)

IPAddress remote_ip(192, 168, 0, 4);
int remote_port = 10001;
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

// Battery voltage monitoring
// Uses 1.5k/10k voltage divider
float ReadVoltage(unsigned int numIterations)
{
    int a2 = analogRead (A0);
    long sum = a2;
    for (unsigned int i = 0; i < numIterations; i++)
    {
        sum += analogRead (A0);
    }
    a2 = sum / (numIterations + 1);
    int R1 = 9900; // measured with multimeter
    int R2 = 1475;
    // Calibration:
    R2 = 1440;
    float value = float(a2) / 1023.0;
    float scale = float(R1 + R2) / R2;
    float voltage = 3.3 * value;
    return voltage * scale;
}

void ReadData()
{
    float voltage = ReadVoltage(100);
    float temperature = 0.0f;
    float humidity = 0.0f;
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
        LOGLN(F("Error reading temperature!"));
    }
    else {
        LOG(F("Temperature: "));
        temperature = event.temperature;
        LOG(temperature);
        dht.humidity().getEvent(&event);
        if (isnan(event.relative_humidity)) {
            LOGLN(F(". Error reading humidity!"));
        }
        else
        {
            LOG(F("°C, Humidity: "));
            humidity = event.relative_humidity;
            LOG(humidity);
            LOG(F("%, Voltage: "));
        }
    }
    LOG(voltage);
    LOGLN(F("V"));

    String replyPacket1 = String("TEMP: ") +
        temperature + String("C, ") +
        humidity + String("%h, ") +
        voltage + String("V");
    WiFiUDP client;
    client.beginPacket(remote_ip, remote_port );
    client.write(replyPacket1.c_str(), replyPacket1.length());
    client.endPacket();
    yield();
}

void setup()
{
    float voltage = ReadVoltage(1);
    if( voltage > 1.0 && voltage < 9.6)
    {
        Serial.begin(74880);
        delay(10);
        LOGLN("Battery too low - going into deep sleep mode for 600 seconds");
        LOGLN("To flash the new program please disconnect the battery from A0 (or charge it)");
        yield();
        delay(500);
        ESP.deepSleep(600e6);
    }

#ifdef DEBUG
    Serial.begin(74880);
    delay(10);
    LOG("System up...");
#endif
    int inputVal = analogRead (A0);
    LOG("Analog input: ");
    LOGLN(inputVal);
    dht.begin();
    LOGLN("");
    LOG("Connecting to ");
    LOGLN(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        LOG(".");
    }
    LOGLN("");
    LOGLN("WiFi connected");
    LOGLN("IP address: ");
    LOGLN(WiFi.localIP());

    ReadData();
    yield();
    LOGLN("I'm awake, but I'm going into deep sleep mode for 60 seconds (1 min)");
    yield();
    delay(500);
    ESP.deepSleep(60e6);
}

void loop()
{
    // If deep sleep is enabled we'll never get here
    ReadData();
    yield();
    delay(60000);
}
