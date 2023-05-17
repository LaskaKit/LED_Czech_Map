/*
* Program pro zobrazeni srazek z radaru CHMI pro 
* interaktivni RGB mapu Ceske republiky - https://www.laskakit.cz/laskakit-interaktivni-mapa-cr-ws2812b/
* a 
* mapa  tvoje___mama - https://github.com/tomasbrincil/pcb_mapa_cr_1
*
* Puvodnim autorem kodu je Jakub Cizek (zive.cz) - https://twitter.com/pesvkloubouku
* https://github.com/jakubcizek/pojdmeprogramovatelektroniku/tree/master/SrazkovyRadar
* https://www.zive.cz/clanky/naprogramovali-jsme-radarovou-mapu-ceska-ukaze-kde-prave-prsi-a-muzete-si-ji-dat-i-na-zed/sc-3-a-222111/default.aspx
*
* Dale upraveno:
* Alan Hodinar - https://twitter.com/alistaarec
* https://github.com/alistaarec/pcb_mapa_cr_1/blob/main/src/src_chmi.cpp
* pridano: podpora ESP8266, mapaTvojiMamy (Tomas Brincil), TMEP json
*
* LaskaKit.cz - https://twitter.com/laska_kit
* pridano: moznost jednoducheho zobrazeni (pouze cervena, zelena, modra), 
* podpora animace (zobrazeni pred 30min, 20min, 10min a aktualni s volitelnym zpozdenim), volba mezi LaskaKit mapou a MapaTvojiMamy
*
*/

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#endif

#ifdef ESP32
#include <HTTPClient.h>
#include <WiFi.h>
#endif

/*-------------------------------------*/
/*------------- KONFIGURACE -----------*/
/*-------------------------------------*/
//SSID a Heslo k WiFi
const char *ssid = "SSID";
const char *heslo = "HESLO";

#ifdef ESP8266
#define LEDPin 5 // datovy pin LED pro ESP8266
#endif

#ifdef ESP32
#define LEDPin 25 // datovy pin LED pro ESP32, pouziva LaskaKit mapa
#endif

unsigned long lastmill = 0;   
unsigned long timeout = 60000;  //60000 - aktualizace každou minutu
int jas = 10;

// Vyber zdrojoveho JSON souboru, odkomentuj pouze jeden
//#define URL1
#define URL2

// Vyber mapy, odkomentuje pouze jeden
#define laskakit
//#define mapaTvojiMamy

// Animace radarovych srazek, podporovano pouze z TMEP.cz, odkomentuj pokud ma byt povoleno
#define animace
#define animaceZpozdeni 1500 //zpozdeni mezi jednotlivymi JSON soubory v ms

// Zobrazeni pouze cervena, modra, zelena, odkomentuj pokud ma byt povoleno
#define jednoducheZobrazeni

/*-------------------------------------*/
/*---------- KONEC KONFIGURACE --------*/
/*-------------------------------------*/

#ifdef URL1
const char *DataURL = "http://oracle-ams.kloboukuv.cloud/radarmapa/?chcu=posledni.json";
#endif
#ifdef URL2
const char *DataURL = "http://tmep.cz/app/export/okresy-srazky-laskakit.json";
#endif
// animace radaru po 10 minutach (pouze TMEP.cz)
#if defined(animace) && defined(URL2)
const char *DataURL30 = "http://tmep.cz/app/export/okresy-srazky-laskakit-30.json";
const char *DataURL20 = "http://tmep.cz/app/export/okresy-srazky-laskakit-20.json";
const char *DataURL10 = "http://tmep.cz/app/export/okresy-srazky-laskakit-10.json";
#endif

//Prevod ID pro mapu tvoje___mama
int MapaTvojiMamy[72] = {
9, 11, 12, 8, 10, 13, 6, 15, 7, 5, 3, 14, 16, 67, 66, 4, 2, 24, 17, 1,
68, 18, 65, 64 ,0, 25, 76, 20, 69, 19, 27, 23, 73, 70, 21, 29, 28, 59, 
22, 71, 61, 63, 30, 72, 31, 26, 48, 46, 33, 39, 58, 49, 51, 47, 57, 40, 
32, 35, 56 ,38, 55, 34, 45, 41, 50, 36, 54, 52 ,37, 44, 53, 43

};

Adafruit_NeoPixel mapLed(77, LEDPin, NEO_GRB + NEO_KHZ800);

StaticJsonDocument<10000> doc;

int jsonDecoder(String s, bool log) {
  DeserializationError e = deserializeJson(doc, s);
  int fid;
  int idMama;
  if (e) {
    if (e == DeserializationError::InvalidInput) {
      return -1;
    } else if (e == DeserializationError::NoMemory) {
      return -2;
    } else {
      return -3;
    }
  } else {
    mapLed.clear();
    JsonArray mesta = doc["seznam"].as<JsonArray>();
    for (JsonObject mesto : mesta) {
      int id = mesto["id"];
      int r = mesto["r"];
      int g = mesto["g"];
      int b = mesto["b"];

#ifdef laskakit
  fid = id;
#endif
#ifdef mapaTvojiMamy
  idMama = MapaTvojiMamy[id];
  fid = idMama;
#endif
       
      if (log) Serial.printf("Rozsvecuji mesto %d barvou R=%d G=%d B=%d", id, r, g, b);
      Serial.printf("  ---IDLaska %d --- IDMama %d --- FinalID %d\r\n", id, idMama, fid);

#ifdef jednoducheZobrazeni
      if((r > g) && (r > b))
        mapLed.setPixelColor(fid, mapLed.Color(r, 0, 0));
      else if((g > r) && (g > b))
        mapLed.setPixelColor(fid, mapLed.Color(0, g, 0));
      else if((b > r) && (b > g))
        mapLed.setPixelColor(fid, mapLed.Color(0, 0, b));
      
#ifdef mapaTvojiMamy
      // MapaTvojiMamy ma vice LED, fix rozdilu
      switch (id) {
        case 63: 
          if((r > g) && (r > b))
            mapLed.setPixelColor(42, mapLed.Color(r, 0, 0));
          else if((g > r) && (g > b))
            mapLed.setPixelColor(42, mapLed.Color(0, g, 0));
          else if((b > r) && (b > g))
            mapLed.setPixelColor(42, mapLed.Color(0, 0, b));
              break;
        case 26:
          if((r > g) && (r > b))
            mapLed.setPixelColor(74, mapLed.Color(r, 0, 0));
          else if((g > r) && (g > b))
            mapLed.setPixelColor(74, mapLed.Color(0, g, 0));
          else if((b > r) && (b > g))
            mapLed.setPixelColor(74, mapLed.Color(0, 0, b));

          if((r > g) && (r > b))
            mapLed.setPixelColor(75, mapLed.Color(r, 0, 0));
          else if((g > r) && (g > b))
            mapLed.setPixelColor(75, mapLed.Color(0, g, 0));
          else if((b > r) && (b > g))
            mapLed.setPixelColor(75, mapLed.Color(0, 0, b));
          break;
        case 40:
          if((r > g) && (r > b))
            mapLed.setPixelColor(60, mapLed.Color(r, 0, 0));
          else if((g > r) && (g > b))
            mapLed.setPixelColor(60, mapLed.Color(0, g, 0));
          else if((b > r) && (b > g))
            mapLed.setPixelColor(60, mapLed.Color(0, 0, b));
          if((r > g) && (r > b))
            mapLed.setPixelColor(62, mapLed.Color(r, 0, 0));
          else if((g > r) && (g > b))
            mapLed.setPixelColor(62, mapLed.Color(0, g, 0));
          else if((b > r) && (b > g))
            mapLed.setPixelColor(62, mapLed.Color(0, 0, b));
          break;
      }
#endif //mapaTvojiMamy
#else
      mapLed.setPixelColor(fid, mapLed.Color(r, g, b));
#ifdef mapaTvojiMamy
      // MapaTvojiMamy ma vice LED, fix rozdilu
      switch (id) {
        case 63: 
          mapLed.setPixelColor(42, mapLed.Color(r, g, b));
          break;
        case 26:
          mapLed.setPixelColor(74, mapLed.Color(r, g, b));
          mapLed.setPixelColor(75, mapLed.Color(r, g, b));
          break;
        case 40:
          mapLed.setPixelColor(60, mapLed.Color(r, g, b));
          mapLed.setPixelColor(62, mapLed.Color(r, g, b));
          break;
      }
#endif //mapaTvojiMamy
#endif //jednoducheZobrazeni
    }
    mapLed.show();
    return 0;
  }
}

// Stazeni radarovych dat z webu
void updateData(const char *URL) {
  #ifdef ESP8266
  WiFiClient client;
  HTTPClient http;
  http.begin(client, URL);
  #endif

  #ifdef ESP32
  HTTPClient http;
  http.begin(URL);
  #endif

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    int err = jsonDecoder(http.getString(), true);
    switch (err) {
      case 0:
        Serial.println("Hotovo!");
        break;
      case -1:
        Serial.println("Spatny format JSONu");
        break;
      case -2:
        Serial.println("Malo pameti, navys velikost StaticJsonDocument");
        break;
      case -3:
        Serial.println("Chyba pri parsovani JSONu");
        break;
    }
  }
  http.end();
}

void ledTest() {
  for (int l = 0; l < 77; l++) {
    mapLed.setPixelColor(l, mapLed.Color(255, 255, 255));
    mapLed.show();
    delay(50);
  }
  delay(1000);
}


void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, heslo);
  Serial.println("");
  Serial.printf("Pripojuji se k %s ", ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  Serial.println("");
  Serial.print(" IP: "); Serial.println(WiFi.localIP());

  mapLed.begin();
  mapLed.setBrightness(jas);
  mapLed.clear();
  mapLed.show();

  //Serial.println("Test LED");
  //ledTest();
  delay(20);
  updateData(DataURL);
}

void loop() {

  if (millis() - lastmill >= timeout) {
    lastmill = millis();
    #if defined(animace) && defined(URL2)
    updateData(DataURL30);
    delay(animaceZpozdeni);
    updateData(DataURL20);
    delay(animaceZpozdeni);
    updateData(DataURL10);
    delay(animaceZpozdeni);
    updateData(DataURL);
    #else
    updateData(DataURL);
    #endif
  }

}
