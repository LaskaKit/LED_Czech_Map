/*
 * Interaktivni mapa CR s webovym portalem pro snadne nastaveni Wi-Fi site 
 * a zobrazeni ruznych dat - teplota, vlhkost, tlak, prasnost a meteoradar na mape.
 *
 * Po prvnim zapnuti ESP32 vytvori Access Point s SSID Laskakit-MapaCR, pripojte se na tuto WiFi a ve
 * vasem webovem prohlizeci zadejte IP adresu 192.168.4.1, tam nastavte SSID a heslo vasi domaci Wi-Fi site,
 * Po restartu a pripojeni do vasi Wi-Fi site si mapa nacte vybrana data a zobrazi. Upravu zobrazeni dat, jasu nebo
 * intervalu aktualizace muzete provest pres webovy prohlizec kam napisete IP adresu zarizeni.
 * Tu zjistite napriklad z vaseho routeru (jakou IP adresu router mape pridelil) nebo z Terminalu (Putty, YAT, Arduino)
 * rychlost 115 200 Bd
 *
 *  Knihovny:
 * https://arduinojson.org/
 * https://github.com/adafruit/Adafruit_NeoPixel
 *
 * Kompilace
 * Tools -> Board -> ESP32 Dev Module
 *
 * Hardware: https://www.laskakit.cz/laskakit-interaktivni-mapa-cr-ws2812b/
 *
 * Laskakit (2025)
 */


#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>

#define LEDS_COUNT 72     // počet LED diod na mapě
#define LEDS_PIN 25       // pin pro připojení LED pásku

Preferences preferences;  // pro ukládání nastavení
WebServer server(80);    // webový server na portu 80

Adafruit_NeoPixel strip(LEDS_COUNT, LEDS_PIN, NEO_GRB + NEO_KHZ800); // LED pás konfigurace

// Klíče pro uložení konfigurace do Preferences
const char* KEY_SSID = "ssid";
const char* KEY_PASS = "password";
const char* KEY_PARAM = "param";
const char* KEY_BRIGHT = "brightness";
const char* KEY_UPDATE = "update";

// URL pro získání JSON dat
const char* JSON_URL_MAIN = "http://cdn.tmep.cz/app/export/okresy-cr-vse.json";
const char* JSON_URL_METEORADAR = "http://tmep.cz/app/export/okresy-srazky-laskakit.json";

// Proměnné pro uložené hodnoty
char storedSSID[32] = "";
char storedPassword[64] = "";
char selectedParameter[16] = "h1";  // výchozí parametr - teplota
int brightness = 50;                // jas LED
unsigned long timerDelay = 60000;  // interval aktualizace v ms

float TMEPDistrictValues[77];       // pole hodnot pro jednotlivé okresy

// Mapování indexů okresů na pozice LED diod
int TMEPDistrictPosition[72] = {
  9, 11, 12, 8, 10, 13, 6, 15, 7, 5, 3, 14, 16, 67, 66, 4, 2, 24, 17, 1, 68, 18, 65, 64, 0,
  25, 76, 20, 69, 19, 27, 23, 73, 70, 21, 29, 28, 59, 22, 71, 61, 63, 30, 72, 31, 26, 48,
  46, 33, 39, 58, 49, 51, 47, 57, 40, 32, 35, 56, 38, 55, 34, 45, 41, 50, 36, 54, 52, 37,
  44, 53, 43
};

unsigned long lastTime = 0;  // čas poslední aktualizace
bool wifiConnected = false;  // připojeno k WiFi
int color;                   // proměnná pro barvu LED
int first_time = 0;          // příznak pro první spuštění

// Struktura pro statistiky hodnot a odpovídající barvy LED
struct StatColor {
  float minValue;
  float avgValue;
  float maxValue;
  uint32_t minColor;
  uint32_t avgColor;
  uint32_t maxColor;
};

// Pomocná funkce mapující hodnotu na barvu dle odstínu (Hue) LED
int mapValueToHue(float val, float minVal, float maxVal) {
  if (maxVal == minVal) return 85;  // střední barva, když všechny hodnoty stejné
  int color = map(val, minVal, maxVal, 170, 0);
  if (color < 0) color = 0;
  if (color > 170) color = 170;
  return color;
}

// Převod 32bitové barvy RGB na hex string pro HTML stylování
String colorToHex(uint32_t c) {
  uint8_t r = (c >> 16) & 0xFF;
  uint8_t g = (c >> 8) & 0xFF;
  uint8_t b = c & 0xFF;
  char buf[8];
  snprintf(buf, sizeof(buf), "#%02X%02X%02X", r, g, b);
  return String(buf);
}

// Vypočítá min, průměr a max hodnotu z pole JSON dat a příslušné barvy LED
StatColor calculateStatsAndColors(JsonArray data, const char* param) {
  float minV = 1e6;
  float maxV = -1e6;
  float sumV = 0;
  int count = 0;

  // Průchod pro zjištění min a max hodnoty
  for (JsonObject obj : data) {
    if (obj.containsKey(param)) {
      float val = obj[param];
      if (val < minV) minV = val;
      if (val > maxV) maxV = val;
    }
  }

  // Výpočet součtu a počtu hodnot pro průměr
  for (JsonObject obj : data) {
    if (obj.containsKey(param)) {
      float val = obj[param];
      sumV += val;
      count++;
    }
  }
  float avgV = (count > 0) ? (sumV / count) : 0.0;

  // Převedení hodnot na barvu ve škále LED (HSV)
  uint32_t minCol = strip.ColorHSV(mapValueToHue(minV, minV, maxV) * 256);
  uint32_t avgCol = strip.ColorHSV(mapValueToHue(avgV, minV, maxV) * 256);
  uint32_t maxCol = strip.ColorHSV(mapValueToHue(maxV, minV, maxV) * 256);

  return StatColor{minV, avgV, maxV, minCol, avgCol, maxCol};
}

void setup() {
  Serial.begin(115200);
  delay(10);

  // Inicializace preferencí pro uložení nastavení
  preferences.begin("LaskaKit", false);

  // Načtení uložených hodnot z paměti
  preferences.getString(KEY_SSID, "").toCharArray(storedSSID, sizeof(storedSSID));
  preferences.getString(KEY_PASS, "").toCharArray(storedPassword, sizeof(storedPassword));
  String param = preferences.getString(KEY_PARAM, "h1");
  param.toCharArray(selectedParameter, sizeof(selectedParameter));
  brightness = preferences.getInt(KEY_BRIGHT, 50);
  timerDelay = preferences.getInt(KEY_UPDATE, 60) * 1000UL;

  // Inicializace LED pásku
  strip.begin();
  strip.setBrightness(brightness);
  strip.show();

  // Připojení k WiFi nebo spuštění AP podle uložených dat
  if (strlen(storedSSID) > 0) {
    setupWiFi();
  } else {
    startAP();
  }
}

void loop() {
  // Webový server musí zpracovávat požadavky stále
  server.handleClient();

  // Pravidelné aktualizace dat a zobrazení, popř. při prvním spuštění
  if (wifiConnected) {
    if ((millis() - lastTime) > timerDelay || first_time == 0) {
      fetchAndDisplayData();
      lastTime = millis();
      first_time = 1;
    }
  }
}

void setupWiFi() {
  Serial.printf("Připojuji se k WiFi: '%s'\n", storedSSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(storedSSID, storedPassword);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Připojeno! IP adresa: %s\n", WiFi.localIP().toString().c_str());
    wifiConnected = true;
  } else {
    Serial.println("Nepodařilo se připojit! Spouštím AP...");
    wifiConnected = false;
    startAP();
  }

  // Nastavení obsluhy HTTP cest
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound(handleNotFound);
  server.begin();
}

void startAP() {
  // Spustí WiFi AP (Access Point) s názvem LaskaKit-Config pro konfiguraci zařízení
  WiFi.mode(WIFI_AP);
  WiFi.softAP("LaskaKit-MapaCR");
  Serial.print("IP adresa AP: ");
  Serial.println(WiFi.softAPIP());

  wifiConnected = false;

  // HTTP cesty i v AP režimu
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound(handleNotFound);
  server.begin();

  // Zhasnutí LED při konfiguraci
  strip.clear();
  strip.show();
}

void handleRoot() {
  // Funkce generující hlavní stránku s formulářem i s výpisem statistik

  WiFiClient client;
  HTTPClient http;
  // URL se liší podle typu zobrazeného parametru (meteoradar má vlastní data)
  const char* currentURL = (strcmp(selectedParameter, "meteoradar") == 0) ? JSON_URL_METEORADAR : JSON_URL_MAIN;
  http.begin(client, currentURL);
  int httpCode = http.GET();

  String statHTML = "";

  // Statistiky se zobrazují pouze pokud není vybrán meteoradar
  if (strcmp(selectedParameter, "meteoradar") != 0 && httpCode > 0) {
    String payload = http.getString();
    DynamicJsonDocument doc(8192);
    DeserializationError err = deserializeJson(doc, payload);
    if (!err) {
      JsonArray arr = doc.as<JsonArray>();
      const char* paramToUse = "h1";      // výchozí parametr je teplota
      String paramLabel = "teplotu";      // popisek pro statistiku

      // Nastavení podle vybraného parametru
      if (strcmp(selectedParameter, "h2") == 0) {
        paramToUse = "h2";
        paramLabel = "vlhkost";
      } else if (strcmp(selectedParameter, "h3") == 0) {
        paramToUse = "h3";
        paramLabel = "tlak";
      } else if (strcmp(selectedParameter, "h4") == 0) {
        paramToUse = "h4";
        paramLabel = "prašnost";
      }

      // Výpočet statistik a odpovídajících barev
      StatColor sc = calculateStatsAndColors(arr, paramToUse);

      // HTML s barevně zvýrazněnými statistikami a jiným fontem
      statHTML = "<div style='max-width:380px; margin:20px auto; font-family: Verdana, Geneva, Tahoma, sans-serif;'>";
      statHTML += "<h3>Statistiky pro " + paramLabel + "</h3>";
      statHTML += "<p style='color:#333; font-weight: normal;'>";
      statHTML += "Minimální hodnota: <span style='color:" + colorToHex(sc.minColor) + "; font-weight: bold;'>" + String(sc.minValue, 2) + "</span><br>";
      statHTML += "Průměrná hodnota: <span style='color:" + colorToHex(sc.avgColor) + "; font-weight: bold;'>" + String(sc.avgValue, 2) + "</span><br>";
      statHTML += "Maximální hodnota: <span style='color:" + colorToHex(sc.maxColor) + "; font-weight: bold;'>" + String(sc.maxValue, 2) + "</span>";
      statHTML += "</p></div>";
    }
  }
  http.end();

  // HTML stránka s formulářem a vloženými statistikami (pokud jsou)
/* Formulář pro nastavení WiFi, parametru, jasu, intervalu apod. */
  String page = R"rawliteral(
<!DOCTYPE html>
<html lang="cs">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>LaskaKit LED Config</title>
<style>
  body { font-family: 'Segoe UI', 'Arial', sans-serif; background: #f0f4f8; margin:0; padding:30px; }
  h2 { color: #0073e6; text-align: center; margin-bottom:25px; }
  form { max-width: 380px; margin: auto; background: #fff; border-radius:10px; padding:24px; box-shadow: 0 4px 36px rgba(0,45,100,0.10);}
  label { font-weight:600; display:block; margin-top:13px; }
  input, select { width:100%; padding:11px; margin-top:6px; border-radius:6px; border:1px solid #aaa; font-size:17px; }
  input[type=submit] { background: #0073e6; color: #fff; border:none; margin-top:26px; font-size:18px; cursor:pointer; transition:background 0.2s; }
  input[type=submit]:hover { background: #005bb5;}
  .footer { text-align: center; margin-top: 40px; color: #888; font-size:14px;}
  @media (max-width:470px) { form { padding:12px;} body { padding:8px;} }
</style>
</head>
<body>
  <h2>Nastavení LaskaKit LED Mapy ČR</h2>
  <form action="/save" method="post">
    <label for="ssid">WiFi SSID:</label>
    <input type="text" id="ssid" name="ssid" value="%SSID%" required />
    <label for="password">WiFi heslo:</label>
    <input type="password" id="password" name="password" placeholder="Nezadávej pokud nechceš měnit" value="" autocomplete="new-password" />
    <label for="param">Zobrazovaný parametr:</label>
    <select id="param" name="param">
      <option value="h1" %H1_SELECTED%>Teplota (Temperature)</option>
      <option value="h2" %H2_SELECTED%>Vlhkost (Humidity)</option>
      <option value="h3" %H3_SELECTED%>Tlak (Pressure)</option>
      <option value="h4" %H4_SELECTED%>Prášnost (Dustiness)</option>
      <option value="meteoradar" %METEORADAR_SELECTED%>Meteoradar (Srážky)</option>
    </select>
    <label for="brightness">Jas LED (0-255):</label>
    <input type="number" id="brightness" name="brightness" min="0" max="255" value="%BRIGHTNESS%" required />
    <label for="update">Interval aktualizace (sec):</label>
    <input type="number" id="update" name="update" min="10" max="3600" value="%UPDATE%" required />
    <input type="submit" value="Uložit a restartovat" />
  </form>
)rawliteral";

  // Nahrazení zástupných hodnot aktuálními daty
  page.replace("%SSID%", String(storedSSID));
  page.replace("%BRIGHTNESS%", String(brightness));
  page.replace("%UPDATE%", String(timerDelay / 1000));
  page.replace("%H1_SELECTED%", (strcmp(selectedParameter, "h1") == 0) ? "selected" : "");
  page.replace("%H2_SELECTED%", (strcmp(selectedParameter, "h2") == 0) ? "selected" : "");
  page.replace("%H3_SELECTED%", (strcmp(selectedParameter, "h3") == 0) ? "selected" : "");
  page.replace("%H4_SELECTED%", (strcmp(selectedParameter, "h4") == 0) ? "selected" : "");
  page.replace("%METEORADAR_SELECTED%", (strcmp(selectedParameter, "meteoradar") == 0) ? "selected" : "");

  // Přidání části se statistikami (pokud nejsou meteoradar)
  page += statHTML;

  page += R"rawliteral(
  <div class="footer">LaskaKit &copy; 2025 — WiFi konfigurace je dostupná vždy na této adrese zařízení.</div>
</body>
</html>
)rawliteral";

  // Odeslání výsledné webové stránky
  server.send(200, "text/html; charset=UTF-8", page);
}

void handleSave() {
  // Uložení nastavení z webového formuláře
  if (server.hasArg("ssid") && server.hasArg("param") && server.hasArg("brightness") && server.hasArg("update")) {
    String newSSID = server.arg("ssid");
    String newPassword = server.arg("password");
    String newParam = server.arg("param");
    int newBrightness = server.arg("brightness").toInt();
    int newUpdate = server.arg("update").toInt();

    bool restartNeeded = false;

    // Restart je potřeba pokud se změní SSID nebo heslo WiFi
    if (newSSID != String(storedSSID)) {
      restartNeeded = true;
      preferences.putString(KEY_SSID, newSSID);
      newSSID.toCharArray(storedSSID, sizeof(storedSSID));
    }

    if (newPassword.length() > 0) {
      restartNeeded = true;
      preferences.putString(KEY_PASS, newPassword);
      newPassword.toCharArray(storedPassword, sizeof(storedPassword));
    }

    // Pokud se změní parametr, uloží se a aktualizují se LED ihned
    if (newParam != String(selectedParameter)) {
      preferences.putString(KEY_PARAM, newParam);
      newParam.toCharArray(selectedParameter, sizeof(selectedParameter));
      fetchAndDisplayData();
    }

    // Změna jasu LED bez restartu
    if (newBrightness != brightness) {
      preferences.putInt(KEY_BRIGHT, newBrightness);
      brightness = newBrightness;
      strip.setBrightness(brightness);
      strip.show();
    }

    // Změna intervalu aktualizace bez restartu
    if (newUpdate != (timerDelay / 1000)) {
      preferences.putInt(KEY_UPDATE, newUpdate);
      timerDelay = newUpdate * 1000UL;
    }

    // Odeslání potvrzení, pokud je potřeba restart, za 2 sekundy restart
    if (restartNeeded) {
      String message =
        "<!DOCTYPE html>"
        "<html lang='cs'>"
        "<head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<title>Nastavení uloženo</title>"
        "<style>"
        "body { font-family: 'Segoe UI', Arial, sans-serif; background: #f0f4f8; text-align: center; padding: 32px; }"
        "h2 { color: #0073e6; }"
        "</style>"
        "</head>"
        "<body>"
        "<h2>Nastavení uloženo. Restartuje se zařízení...</h2>"
        "</body></html>";

      server.send(200, "text/html; charset=UTF-8", message);
      delay(2000);
      ESP.restart();
    } else {
      // Odeslání potvrzení bez restartu s možností návratu na konfiguraci
      String message =
        "<!DOCTYPE html>"
        "<html lang='cs'>"
        "<head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<title>Nastavení uloženo</title>"
        "<style>"
        "body { font-family: 'Segoe UI', Arial, sans-serif; background: #f0f4f8; text-align: center; padding: 32px; }"
        "h2 { color: #0073e6; }"
        ".back-btn { margin-top:24px; padding:12px 36px; font-size:17px; border-radius:7px; background:#0073e6; color:#fff; border:none; cursor:pointer; transition:background 0.2s; }"
        ".back-btn:hover { background:#005bb5; }"
        "</style>"
        "</head>"
        "<body>"
        "<h2>Nastavení uloženo.</h2>"
        "<form action='/' method='get'>"
        "<button class='back-btn' type='submit'>Zpět na konfiguraci</button>"
        "</form>"
        "</body></html>";

      server.send(200, "text/html; charset=UTF-8", message);
    }
  } else {
    server.send(400, "text/plain; charset=UTF-8", "Chybějící parametry.");
  }
}

void handleNotFound() {
  // Vypsání chyby při požadavku na neznámou stránku
  server.send(404, "text/plain; charset=UTF-8", "Stránka nenalezena");
}

void fetchAndDisplayData() {
  // Nastavení jasu LED pásku dle uložené hodnoty
  strip.setBrightness(brightness);

  // URL vybraných dat podle parametru
  String currentURL = JSON_URL_MAIN;
  if (strcmp(selectedParameter, "meteoradar") == 0) {
    currentURL = JSON_URL_METEORADAR;
  }

  WiFiClient client;
  HTTPClient http;
  http.begin(client, currentURL);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String payload = http.getString();

    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print(F("Chyba parsování JSON: "));
      Serial.println(error.f_str());
      return;
    }

    // Speciální zpracování pro meteoradar, kde barvy LED definují srážky
    if (strcmp(selectedParameter, "meteoradar") == 0) {
      Serial.println("Zobrazuji meteoradar");
      strip.clear();
      JsonArray mesta = doc["seznam"].as<JsonArray>();
      for (JsonObject mesto : mesta) {
        int id = mesto["id"];
        int r = mesto["r"];
        int g = mesto["g"];
        int b = mesto["b"];

#ifdef jednoducheZobrazeni
        if ((r > g) && (r > b))
          strip.setPixelColor(id, strip.Color(r, 0, 0));
        else if ((g > r) && (g > b))
          strip.setPixelColor(id, strip.Color(0, g, 0));
        else if ((b > r) && (b > g))
          strip.setPixelColor(id, strip.Color(0, 0, b));
#else
        strip.setPixelColor(id, strip.Color(r, g, b));
#endif
      }
      strip.show();
    } else {
      // Standardní zpracování JSON dat pro teplotu, vlhkost, tlak, prašnost
      float maxValue = -999999;
      float minValue = 999999;

      for (JsonObject item : doc.as<JsonArray>()) {
        int districtIndex = item["id"];
        districtIndex -= 1;
        if (item.containsKey(selectedParameter)) {
          float val = item[selectedParameter];
          TMEPDistrictValues[districtIndex] = val;
          if (val < minValue) minValue = val;
          if (val > maxValue) maxValue = val;
        } else {
          TMEPDistrictValues[districtIndex] = 0.0;
        }
      }

      // Nastavení barvy LED podle hodnoty daného okresu převedené ze škály HSV
      for (int LED = 0; LED < LEDS_COUNT; LED++) {
        int districtIdx = TMEPDistrictPosition[LED];
        float val = TMEPDistrictValues[districtIdx];
        color = map(val, minValue, maxValue, 170, 0);
        strip.setPixelColor(LED, strip.ColorHSV(color * 256));
      }
      strip.show();
    }
  } else {
    Serial.print("HTTP GET selhal, kód: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}
