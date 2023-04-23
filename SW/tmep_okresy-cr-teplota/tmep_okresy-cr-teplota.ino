/* 
 * LaskaKit LED Czech Map, TMEP temperature.
 * Get Temperature from TMEP, find minimum and maximum of temperature,
 * and show on the map. 
 * For settings see config.h
 * 
 * Email:podpora@laskakit.cz
 * Web:laskakit.cz
 * 
 * How TO: https://wiki.tmep.cz/doku.php?id=ruzne:led_mapa_okresu_cr
 * Inspired by "Your Mothers Map" (Mapa Tvoji Mamy): https://github.com/tomasbrincil/pcb_mapa_cr_1
 * Modification of color layout based on min and max temperature in Czechia from https://github.com/rpsh1919/LED-mapa/blob/main/LED-mspa.src
 */

#include "config.h"            // - >>>>> change to config.h and fill the file.

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>          // https://github.com/bblanchon/ArduinoJson v6+
#include "Freenove_WS2812_Lib_for_ESP32.h"
#define LEDS_COUNT  72
#define LEDS_PIN	25
#define CHANNEL		0

// URL with JSON data
const char* json_url = "http://cdn.tmep.cz/app/export/okresy-cr-teplota.json";
unsigned long lastTime = 0;
unsigned long timerDelay = 60000;
int lastid, value, color, first_time;
Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);
String sensorReadings;
float sensorReadingsArr[3];

// Initialize array with number of districts readed from TMEP, 
// which we will later populate with values from JSON
float TMEPDistrictTemperatures[77];

// We need to map our LED order with district order from TMEP,
// so here we have array with positions of districts on TMEP that we get from JSON
// starting from zero, so: index 0 (first LED) is district with ID 9 and so on
// until we have district for every from our 72 LEDs
int TMEPDistrictPosition[72] = {
 9, 11, 12, 8, 10, 13, 6, 15, 7, 5, 3, 14, 16, 67, 66, 4, 2, 24, 17, 1, 68, 18, 65, 64, 0, 
 25, 76, 20, 69, 19, 27, 23, 73, 70, 21, 29, 28, 59, 22, 71, 61, 63, 30, 72, 31, 26, 48, 
 46, 33, 39, 58, 49, 51, 47, 57, 40, 32, 35, 56, 38, 55, 34, 45, 41, 50, 36, 54, 52, 37, 
 44, 53, 43
};

void setup()
{
    Serial.begin(115200);
    delay(10);
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    strip.begin();
    strip.setBrightness(10); 
}

void loop()
{
    // Did we wait long enough or was it just powered on?
    if ((millis() - lastTime) > timerDelay || first_time == 0) {
        if(WiFi.status()== WL_CONNECTED){
            first_time = 1;
            sensorReadings = httpGETRequest(json_url);
            DynamicJsonDocument doc(6144);
            DeserializationError error = deserializeJson(doc, sensorReadings);
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }

            String tmp;
            float maxTemp = -99;
            float minTemp =  99;
            // Read all TMEP districts with their indexes
            for (JsonObject item : doc.as<JsonArray>()) {
                int TMEPdistrictIndex = item["id"];
                // Substract 1, so index will start from 0
                TMEPdistrictIndex -= 1;
                double h = item["h"];
                TMEPDistrictTemperatures[TMEPdistrictIndex] = h;

                // find the min and max temperature for adjusting of color layout
                tmp = TMEPDistrictTemperatures[TMEPdistrictIndex];
                if (tmp.toFloat() < minTemp) minTemp = tmp.toFloat();
                if (tmp.toFloat() > maxTemp) maxTemp = tmp.toFloat();
            }

            // Now go through our LEDs and we will set their colors
            for (int LED = 0; LED <= LEDS_COUNT - 1; LED++) 
            {
              // Get color for correct district and recalculate based on the min and max of temperature in Czechia - variable color layout
              //color = map(TMEPDistrictTemperatures[TMEPDistrictPosition[LED]], minTemp, maxTemp, 170, 0);
              // Get color for correct district LED - fixed color layout (min -15; max 40 °C)
              color = map(TMEPDistrictTemperatures[TMEPDistrictPosition[LED]], -15, 40, 170, 0);
              strip.setLedColorData(LED, strip.Wheel(color));
            }

            strip.show();
        }
        else {
            Serial.println("WiFi Disconnected");
        }
        lastTime = millis();
    }
}

String httpGETRequest(const char* serverName) {
    WiFiClient client;
    HTTPClient http;
    http.begin(client, serverName);
    int httpResponseCode = http.GET();
    String payload = "{}"; 
    if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        payload = http.getString();
    }
    else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    http.end();
    return payload;
}
