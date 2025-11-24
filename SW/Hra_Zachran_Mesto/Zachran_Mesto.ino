/*
* Hra Zachran Mesto s vývojovou deskou Laskakit Interaktivni Mapa CR 
* https://www.laskakit.cz/laskakit-interaktivni-mapa-cr-ws2812b/
*
* Tato aplikace vyuziva interaktivni mapu CR od Laskakit.cz 
* Hrac na mobilnim telefonu, tabletu nebo pocitaci musi skrze webovy prohlizec
* odpovedet na otazky dokud neuhasi vsechna mesta. Otazky zadavas ty.
* Hra zacina deseti mesty, ktere jsou v plamenech a jen ty je muzes zachranit! 
* Ale pozor! Pokud odpovis spatne, zacne horet dalsi mesto!
* 
* Po zapnuti vytvori deska vlastni Access Point, pripoj se pres tvuj telefon k WiFi "LaskaKit_MapaCR"
* v prohlizeci zadej IP adresu 192.168.4.1. Tam muzes zadat jmeno a heslo k tve WiFi.
* Po restartu se ti v Serial Terminal (Arduino, Yat, Putty, rychlost 115200 Bd) zobrazi pridelena IP adresa.
* V prohlizeci zadej tvou IP adresu a muzes pridat otazky.
* Format otazek
* Cislo otazky;Otazka;Odpoved0;Odpoved1;Odpoved2;Odpoved3;Cislo Spravne Odpovedi 0-3
*
* Knihovny:
* https://github.com/adafruit/Adafruit_NeoPixel
* Hardware: 
* https://www.laskakit.cz/laskakit-interaktivni-mapa-cr-ws2812b/
*
* Laskakit.cz (2025)
*/

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Adafruit_NeoPixel.h>
#include <vector>

#define LED_PIN    25
#define NUM_LEDS   72
#define BRIGHTNESS 50

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
WebServer server(80);
Preferences prefs;

enum CityState { OFF = 0, BURNING = 1, EXTINGUISHED = 2 };
CityState cityState[NUM_LEDS];

// Správné pořadí měst podle LaskaKit ID (1–72)
String cityNames[NUM_LEDS] = {
  "Děčín","Liberec","Jablonec nad Nisou","Ústí nad Labem","Česká Lípa","Semily",
  "Teplice","Trutnov","Litoměřice","Most","Chomutov","Jičín","Náchod","Mladá Boleslav",
  "Mělník","Louny","Karlovy Vary","Jeseník","Hradec Králové","Sokolov","Nymburk",
  "Rychnov nad Kněžnou","Kladno","Rakovník","Cheb","Bruntál","Praha","Pardubice",
  "Kolín","Ústí nad Orlicí","Opava","Šumperk","Beroun","Kutná Hora","Chrudim",
  "Karviná","Ostrava-město","Tachov","Svitavy","Benešov","Plzeň-město","Rokycany",
  "Frýdek-Místek","Příbram","Nový Jičín","Olomouc","Havlíčkův Brod","Žďár nad Sázavou",
  "Prostějov","Přerov","Domažlice","Pelhřimov","Tábor","Jihlava","Klatovy","Blansko",
  "Vsetín","Kroměříž","Písek","Vyškov","Strakonice","Zlín","Třebíč","Brno-město",
  "Jindřichův Hradec","Uherské Hradiště","Prachatice","České Budějovice","Hodonín",
  "Znojmo","Český Krumlov","Břeclav"
};

// --- Otázky ---
struct Question {
  String q;
  String opts[4];
  uint8_t correct;
};

std::vector<Question> questionsDynamic; // dynamické otázky

// Paměť otázek pro každé město
int currentQuestion[NUM_LEDS]; // -1 = žádná

// --- Utility: JSON-escape String (upraví " \ a newline) ---
String jsonEscape(const String &s){
  String out;
  out.reserve(s.length()*2);
  for (size_t i=0;i<s.length();++i){
    char c = s.charAt(i);
    if(c=='\\'){ out += "\\\\"; }
    else if(c=='"'){ out += "\\\""; }
    else if(c=='\n'){ out += "\\n"; }
    else if(c=='\r'){ /* ignore CR */ }
    else out += c;
  }
  return out;
}

// --- LED funkce ---
void setLEDColor(int idx, uint8_t r, uint8_t g, uint8_t b) {
  if (idx < 0 || idx >= NUM_LEDS) return;
  strip.setPixelColor(idx, strip.Color(r,g,b));
}

void updateStripFromState() {
  for (int i=0;i<NUM_LEDS;i++){
    if (cityState[i] == BURNING) setLEDColor(i,255,50,0);   // základní oranžová
    else if (cityState[i] == EXTINGUISHED) setLEDColor(i,0,0,255);
    else setLEDColor(i,0,0,0);
  }
  strip.setBrightness(BRIGHTNESS);
  strip.show();
}

int pickRandomUnburned() {
  int candidates[NUM_LEDS]; int cnt=0;
  for (int i=0;i<NUM_LEDS;i++) if (cityState[i] == OFF) candidates[cnt++] = i;
  if (cnt==0) return -1;
  return candidates[random(cnt)];
}

// --- Hra ---
void startGame() {
  // Reset všech měst
  for (int i=0; i<NUM_LEDS; i++) {
    cityState[i] = OFF;
    currentQuestion[i] = -1;
  }

  // Zapálíme 10 náhodných měst
  int count = 0;
  while (count < 10) {
    int idx = pickRandomUnburned();
    if (idx >= 0) {
      cityState[idx] = BURNING;
      count++;
    } else {
      break; // už nejsou žádná OFF města
    }
  }

  updateStripFromState();
}

// --- Parsování otázek ze stringu ---
// Očekává řádek: číslo otázky;otázka;odp0;odp1;odp2;odp3;correctIdx
bool parseQuestionLine(const String &lineRaw, Question &q, String &errorMsg){
  String line = lineRaw;
  line.trim();
  if(line.length()==0){ errorMsg = "Prázdný řádek"; return false; }

  // Najdeme středníky. Potřebujeme 6 středníků (7 polí)
  int positions[6];
  int found = 0;
  int pos = 0;
  while(found < 6){
    int p = line.indexOf(';', pos);
    if(p < 0) break;
    positions[found++] = p;
    pos = p + 1;
  }
  if(found < 6){
    errorMsg = "Nedostatek středníků (očekáváno 6 oddělovačů ';').";
    return false;
  }

  String numStr    = line.substring(0, positions[0]); numStr.trim();
  String qtext     = line.substring(positions[0]+1, positions[1]); qtext.trim();
  String o0        = line.substring(positions[1]+1, positions[2]); o0.trim();
  String o1        = line.substring(positions[2]+1, positions[3]); o1.trim();
  String o2        = line.substring(positions[3]+1, positions[4]); o2.trim();
  String o3        = line.substring(positions[4]+1, positions[5]); o3.trim();
  String correctS  = line.substring(positions[5]+1); correctS.trim();

  // Validace polí
  if(qtext.length()==0){ errorMsg = "Prázdný text otázky."; return false; }
  if(o0.length()==0 || o1.length()==0 || o2.length()==0 || o3.length()==0){
    errorMsg = "Některá odpověď je prázdná."; return false;
  }
  if(correctS.length()==0){ errorMsg = "Chybí index správné odpovědi."; return false; }
  // correctS musí být číslo 0-3
  bool allDigits = true;
  for (size_t i=0;i<correctS.length();++i){
    char c = correctS.charAt(i);
    if(!(c>='0' && c<='9')){ allDigits = false; break; }
  }
  if(!allDigits){ errorMsg = "Index správné odpovědi není číslo."; return false; }
  int correct = correctS.toInt();
  if(correct < 0 || correct > 3){ errorMsg = "Index správné odpovědi mimo rozsah 0-3 (" + correctS + ")"; return false; }

  // Naplníme struct
  q.q = qtext;
  q.opts[0] = o0;
  q.opts[1] = o1;
  q.opts[2] = o2;
  q.opts[3] = o3;
  q.correct = (uint8_t)correct;
  return true;
}

// Načte více řádků (text může obsahovat CRLF nebo LF)
void loadQuestionsFromText(const String &text, String &result){
  questionsDynamic.clear();
  int start = 0;
  int lineNo = 1;
  while(start < text.length()){
    int end = text.indexOf('\n', start);
    if(end < 0) end = text.length();
    String line = text.substring(start, end);
    // odstraň CR pokud existuje
    if(line.endsWith("\r")) line = line.substring(0, line.length()-1);
    line.trim();
    if(line.length() > 0){
      Question qq;
      String err;
      if(!parseQuestionLine(line, qq, err)){
        result = "Chyba na řádku " + String(lineNo) + ": " + err;
        return;
      } else {
        questionsDynamic.push_back(qq);
      }
    }
    start = end + 1;
    lineNo++;
  }
  result = "OK, načteno " + String(questionsDynamic.size()) + " otázek.";
}

// --- Web stránky ---
String webPageRoot() {
  String s = R"rawliteral(
  <!doctype html>
  <html lang="cs">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>Zachraň město</title>
    <style>
      body{font-family:Arial,Helvetica,sans-serif;background:#f5f7fb;color:#222;margin:10px}
      h1{font-size:1.6rem;margin:0 0 12px 0}
      .card{background:#fff;border-radius:12px;padding:12px;box-shadow:0 4px 10px rgba(0,0,0,0.06);margin-bottom:12px}
      .city{display:flex;justify-content:space-between;padding:6px;border-bottom:1px solid #eee}
      .statusOff{color:#999} .statusBurn{color:#c0392b;font-weight:700} .statusExt{color:#2980b9;font-weight:700}
      button{display:block;width:100%;padding:10px 12px;margin:6px 0;border-radius:8px;border:0;background:#2d9cdb;color:white;cursor:pointer}
      .question{margin:10px 0;font-weight:600}
      form input, form textarea{width:100%;padding:8px;margin:6px 0;border-radius:8px;border:1px solid #ddd;box-sizing:border-box}
      textarea{font-family:monospace}
    </style>
  </head>
  <body>
    <h1>Zachraň město</h1>

    <div class="card">
      <h3>Otázka</h3>
      <div id="questionBox">Klikni <b>Spustit hru</b>.</div>
    </div>

    <div class="card">
      <h3>Hořící města</h3>
      <div id="burningList"></div>
    </div>

    <div class="card">
      <h3>Uhašená města</h3>
      <div id="extList"></div>
    </div>

    <div class="card">
      <h3>Bezpečná města</h3>
      <div id="safeList"></div>
    </div>

    <div class="card">
      <h3>Nastavení Wi-Fi</h3>
      <form onsubmit="saveWifi();return false;">
        <input id="ssid" placeholder="SSID" required>
        <input id="pass" placeholder="Heslo" type="password" required>
        <button type="submit">Uložit Wi-Fi</button>
      </form>
    </div>

    <div class="card">
      <h3>Nahrát otázky</h3>
      <div style="font-size:0.9rem;color:#666;margin-bottom:6px">
        Každý řádek: <code>číslo otázky;otázka;odp0;odp1;odp2;odp3;index_správné_odpovědi</code><br>
        Index správné odpovědi je 0–3.
      </div>
      <form onsubmit="uploadQuestions();return false;">
        <textarea id="questionsText" rows="8" placeholder="1;Hlavní město ČR?;Praha;Brno;Ostrava;Plzeň;0"></textarea>
        <button type="submit">Nahrát otázky</button>
      </form>
    </div>

    <div style="margin:12px 0">
      <button onclick="startGame()">Spustit hru</button>
      <button onclick="fetchStatus()">Aktualizovat</button>
    </div>

    <script>
      let currentCity = -1;
      async function fetchStatus(){
        const r = await fetch('/status');
        const j = await r.json();
        const burn=document.getElementById('burningList'); burn.innerHTML='';
        const ext=document.getElementById('extList'); ext.innerHTML='';
        const safe=document.getElementById('safeList'); safe.innerHTML='';
        j.cities.forEach((c,i)=>{
          const div=document.createElement('div');div.className='city';
          if(c.state==1){div.innerHTML='<div>'+c.name+'</div><div class=statusBurn>Hoří</div>';burn.appendChild(div);}
          else if(c.state==2){div.innerHTML='<div>'+c.name+'</div><div class=statusExt>Uhašeno</div>';ext.appendChild(div);}
          else {div.innerHTML='<div>'+c.name+'</div><div class=statusOff>OK</div>';safe.appendChild(div);}
        });
        let burning=j.cities.filter(c=>c.state==1);
        if(burning.length>0){currentCity=burning[0].idx;loadQuestion(currentCity);} 
        else {document.getElementById('questionBox').innerHTML='<b>Všechna města uhašena!</b>';}
      }

      async function loadQuestion(cityIdx){
        const r=await fetch('/question?city='+cityIdx); 
        if(!r.ok){
          const txt = await r.text();
          document.getElementById('questionBox').innerHTML='<span style="color:red">Chyba: '+txt+'</span>';
          return;
        }
        const j=await r.json();
        const qb=document.getElementById('questionBox'); qb.innerHTML='';
        qb.innerHTML='<b>Město: '+j.city+'</b><div class=question>'+j.q.text+'</div>';
        j.q.opts.forEach((o,i)=>{
          const b=document.createElement('button'); b.textContent=o;
          b.onclick=()=>submitAnswer(cityIdx,i); qb.appendChild(b);
        });
      }

      async function submitAnswer(cityIdx,optIdx){
        const r=await fetch('/answer',{
          method:'POST',
          headers:{'Content-Type':'application/json'},
          body:JSON.stringify({city:cityIdx,answer:optIdx})
        });
        const j=await r.json();
        if(j.error){ alert("Chyba: "+j.error); return; }
        alert(j.correct ? 'Správně! Město uhašeno.' : 'Špatně! Další město hoří.');
        fetchStatus();
      }

      function startGame(){fetch('/start').then(()=>fetchStatus());}
      async function saveWifi(){
        const ssid=document.getElementById('ssid').value;
        const pass=document.getElementById('pass').value;
        await fetch('/wifi',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid:ssid,pass:pass})});
        alert("Wi-Fi uloženo, restartujte zařízení.");
      }

      async function uploadQuestions(){
        const txt=document.getElementById('questionsText').value;
        if(!txt.trim()){alert("Pole je prázdné!");return;}
        const r=await fetch('/uploadQuestions',{
          method:'POST',
          headers:{'Content-Type':'text/plain'},
          body:txt
        });
        const j = await r.json();
        if(j.error){ alert("Chyba: " + j.error); }
        else { alert(j.ok); document.getElementById('questionsText').value=''; fetchStatus(); }
      }

      fetchStatus();
    </script>
  </body>
  </html>
  )rawliteral";
  return s;
}

// --- API ---
String handleStatus() {
  String s = "{\"cities\":[";
  for(int i=0;i<NUM_LEDS;i++){
    s += "{\"idx\":" + String(i) + ",\"name\":\"" + jsonEscape(cityNames[i]) + "\",\"state\":" + String((int)cityState[i]) + "}";
    if(i<NUM_LEDS-1) s += ",";
  }
  s += "]}"; return s;
}

String handleQuestionJSON(int idx) {
  if(idx<0 || idx>=NUM_LEDS) return String();
  if(questionsDynamic.empty()){
    String s = "{\"city\":\""+jsonEscape(cityNames[idx])+"\",\"q\":{\"text\":\"(Žádné otázky)\",\"opts\":[\"-\",\"-\",\"-\",\"-\"]}}";
    return s;
  }
  if(currentQuestion[idx] < 0 || currentQuestion[idx] >= (int)questionsDynamic.size()){
    currentQuestion[idx] = random(questionsDynamic.size());
  }
  Question &q = questionsDynamic[currentQuestion[idx]];
  String s = "{\"city\":\""+jsonEscape(cityNames[idx])+"\",\"q\":{\"text\":\""+jsonEscape(q.q)+"\",\"opts\":[";
  for(int i=0;i<4;i++){
    s += "\"" + jsonEscape(q.opts[i]) + "\"";
    if(i<3) s += ",";
  }
  s += "]}}";
  return s;
}

void handleRoot(){ server.send(200,"text/html; charset=utf-8",webPageRoot()); }
void handleStatusAPI(){ server.send(200,"application/json; charset=utf-8",handleStatus()); }

void handleQuestionAPI(){
  if(!server.hasArg("city")){ server.send(400,"application/json","{\"error\":\"missing city\"}"); return; }
  int idx = server.arg("city").toInt();
  if(idx<0||idx>=NUM_LEDS){ server.send(400,"application/json","{\"error\":\"bad city\"}"); return; }
  String payload = handleQuestionJSON(idx);
  if(payload.length()==0) server.send(500,"application/json","{\"error\":\"internal error\"}");
  else server.send(200,"application/json; charset=utf-8", payload);
}

void handleStart(){ startGame(); server.send(200,"application/json","{\"ok\":true}"); }

void handleAnswer(){
  if(server.method()!=HTTP_POST){ server.send(405); return; }
  String body = server.arg("plain");
  if(body.length()==0){ server.send(400,"application/json","{\"error\":\"empty body\"}"); return; }

  // jednoduché parsování JSON { "city":N, "answer":M }
  int city=-1, answer=-1;
  int pCity = body.indexOf("city");
  if(pCity>=0){
    int c = body.indexOf(':', pCity);
    if(c>=0) city = body.substring(c+1).toInt();
  }
  int pAns = body.indexOf("answer");
  if(pAns>=0){
    int c = body.indexOf(':', pAns);
    if(c>=0) answer = body.substring(c+1).toInt();
  }
  if(city<0 || city>=NUM_LEDS || answer<0){ server.send(400,"application/json","{\"error\":\"bad payload\"}"); return; }
  if(questionsDynamic.empty()){ server.send(400,"application/json","{\"error\":\"no questions loaded\"}"); return; }

  int qidx = currentQuestion[city];
  if(qidx < 0 || qidx >= (int)questionsDynamic.size()) qidx = random(questionsDynamic.size());
  bool correct = (answer == questionsDynamic[qidx].correct);
  if(correct){
    cityState[city] = EXTINGUISHED;
  } else {
    int nxt = pickRandomUnburned();
    if(nxt >= 0) cityState[nxt] = BURNING;
  }
  currentQuestion[city] = -1;
  updateStripFromState();
  server.send(200,"application/json","{\"correct\":" + String(correct ? "true" : "false") + "}");
}

// Wi-Fi uložení
void handleWifi(){
  if(server.method()!=HTTP_POST){ server.send(405); return; }
  String body = server.arg("plain");
  if(body.length()==0){ server.send(400,"application/json","{\"error\":\"empty body\"}"); return; }
  // očekáváme JSON {"ssid":"...","pass":"..."}
  int ps = body.indexOf("ssid");
  int pp = body.indexOf("pass");
  if(ps<0||pp<0){ server.send(400,"application/json","{\"error\":\"bad payload\"}"); return; }
  int cs = body.indexOf(':', ps); int ce = body.indexOf(',', cs); if(ce<0) ce = body.length();
  int cp = body.indexOf(':', pp); int pe = body.indexOf('}', cp); if(pe<0) pe = body.length();
  String ssid = body.substring(cs+1, ce);
  String pass = body.substring(cp+1, pe);
  ssid.trim(); pass.trim();
  // oříznout uvozovky pokud jsou
  if(ssid.startsWith("\"") && ssid.endsWith("\"")) ssid = ssid.substring(1, ssid.length()-1);
  if(pass.startsWith("\"") && pass.endsWith("\"")) pass = pass.substring(1, pass.length()-1);
  prefs.putString("ssid", ssid);
  prefs.putString("pass", pass);
  server.send(200,"application/json","{\"ok\":true}");
}

// Endpoint pro upload otázek (text/plain)
void handleUploadQuestions(){
  if(server.method()!=HTTP_POST){ server.send(405); return; }
  String body = server.arg("plain");
  if(body.length()==0){ server.send(400,"application/json","{\"error\":\"empty body\"}"); return; }
  String result;
  loadQuestionsFromText(body, result);
  if(result.startsWith("Chyba")){
    // vrátíme chybovou zprávu přesně
    String payload = "{\"error\":\"" + jsonEscape(result) + "\"}";
    server.send(400,"application/json", payload);
  } else {
    String payload = "{\"ok\":\"" + jsonEscape(result) + "\"}";
    server.send(200,"application/json", payload);
  }
}

void setupRoutes(){
  server.on("/",HTTP_GET,handleRoot);
  server.on("/status",HTTP_GET,handleStatusAPI);
  server.on("/question",HTTP_GET,handleQuestionAPI);
  server.on("/start",HTTP_GET,handleStart);
  server.on("/answer",HTTP_POST,handleAnswer);
  server.on("/wifi",HTTP_POST,handleWifi);
  server.on("/uploadQuestions",HTTP_POST,handleUploadQuestions);
  server.begin();
  Serial.println("HTTP server běží.");
}

// --- WiFi helpers ---
void setupAPFallback(){
  WiFi.softAP("LaskaKit_MapaCR"); Serial.println("AP IP: "+WiFi.softAPIP().toString());
}
void tryConnectSaved(){
  String ssid=prefs.getString("ssid","");
  String pass=prefs.getString("pass","");
  if(ssid.length()==0){ setupAPFallback(); return; }
  WiFi.begin(ssid.c_str(), pass.c_str());
  unsigned long st = millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-st<10000) delay(200);
  if(WiFi.status()==WL_CONNECTED) Serial.println("WiFi IP: "+WiFi.localIP().toString());
  else setupAPFallback();
}

void setup(){
  Serial.begin(115200);
  randomSeed(analogRead(0));
  strip.begin(); strip.show(); strip.setBrightness(BRIGHTNESS);
  prefs.begin("zachran",false);
  for(int i=0;i<NUM_LEDS;i++){ cityState[i]=OFF; currentQuestion[i]=-1; }
  tryConnectSaved();
  setupRoutes();
  startGame();
}

void loop(){
  server.handleClient();

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 120) {
    lastUpdate = millis();

    for (int i = 0; i < NUM_LEDS; i++) {
      if (cityState[i] == BURNING) {
        // náhodná fluktuace hořící barvy
        uint8_t r = 200 + random(0, 56);  // červená 200-255
        uint8_t g = random(0, 200);       // zelená 0-199
        uint8_t b = 0;                     // modrá vždy 0
        strip.setPixelColor(i, strip.Color(r, g, b));
      }
      else if (cityState[i] == EXTINGUISHED) {
        // stabilní modrá, žádné blikání
        strip.setPixelColor(i, strip.Color(0, 0, 150));
      }
      else {
        // zhasnuté město
        strip.setPixelColor(i, 0);
      }
    }

    strip.setBrightness(BRIGHTNESS); // jas celého pásu
    strip.show();
  }
}
