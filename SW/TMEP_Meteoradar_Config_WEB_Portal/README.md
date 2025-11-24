<img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/TMEP_Meteoradar_Config_WEB_Portal/srazky_meteoradar.jpeg" width="40%">
Zobrazeni Meteoradaru

Tento vzorový kód představuje slouží k vizualizaci environmentálních dat pomocí interaktivní LED mapy České republiky.
LaskaKit Interaktivní Mapa ČR nabízí uživatelům pohodlnou webovou konfiguraci přes WiFi. 

## Hlavní funkce a schopnosti
### Zobrazení dat na LED mapě
Na mapě je 72 adresovatelných LED diod reprezentující vybraná města v ČR. LED diody indikují hodnoty různých environmentálních parametrů — teplotu, vlhkost, tlak či prašnost — pomocí barevného kódování. Barvy se dynamicky mění podle aktuálních hodnot, což umožňuje rychlou vizuální orientaci v lokalizovaných podmínkách.
Vstupní data jsou použita z projektu [TMEP.cz](https://tmep.cz/).

### Podpora Meteoradaru
Kromě základních meteorologických veličin lze pro dané oblasti zobrazit i speciální datovou vrstvu „meteoradar“, která ukazuje srážky. LED diody v tomto režimu mění barvu podle dat získaných z meteoradaru a nepoužívají standardní škálu parametrů, což umožňuje uživatelům detailnější pohled na aktuální počasí.
Původní projekt i s popisem funkce najdete na [zive.cz](https://www.zive.cz/clanky/naprogramovali-jsme-radarovou-mapu-ceska-ukaze-kde-prave-prsi-a-muzete-si-ji-dat-i-na-zed/sc-3-a-222111/default.aspx)

### Webová konfigurační stránka přes WiFi
Zařízení vytváří vlastní WiFi síť, do které se uživatel připojí a přes webový prohlížeč nastaví parametry jako SSID a heslo WiFi sítě, vybraný meteorologický parametr pro vizualizaci, jas LED, interval aktualizací dat a jiné. Tato konfigurace je přístupná z libovolného zařízení schopného připojit se k WiFi.
Pokud se ESP32 nedokáže připojit k Wi-Fi (první zapnutí či výpadek klientské Wi-Fi), ESP32 vytvoří Access Point s názvem Wi-Fi sítě "Laskakit-MapaCR". Skrze (například) mobilní telefon se připojíte k této Wi-Fi síti a ve webovém prohlížeči (například google chrome) zadáte adresu 192.168.4.1.
Na této stránce můžete upravit výše zmíněné údaje - SSID a heslo vaší Wi-Fi sítě, zobrazený parametr atp. Po restartu se Interaktivní mapa připojí do vaší Wi-Fi sítě. Zjistíte si (například z routeru, Terminálu (Putty, YAT atp, rychlost 115 200Bd) ) IP adresu tohoto zařízení a opět si můžete upravovat konfiguraci ve webovém prohlížeči - stačí zadat IP adresu zařízení (například 192.168.10.242 - vy budete mít jinou)

### Okamžité aplikování změn bez nutnosti restartu
Změny parametrů se většinou projeví ihned, například úprava jasu LED nebo změna sledovaného parametru bez nutnosti restartu zařízení. Restart je nutný pouze při změně WiFi SSID nebo hesla, kdy je potřeba připojení znovu inicializovat.

### Statistické přehledy parametrů přímo na webu
Webová stránka zobrazuje u aktuálně zvoleného parametru statistiky: minimální, průměrnou a maximální hodnotu. Tyto hodnoty jsou doplněny barevným značením shodným s barvou LED indikující hodnoty na mapě, což zvyšuje přehlednost a vizuální korelaci s naměřenými daty.
Toto zobrazení přehledu není v režimu meteoradaru dostupné.

### Podpora automatických aktualizací dat
Data jsou pravidelně stahována z veřejných JSON zdrojů s intervalem nastavitelným uživatelem (uvedené jsou defaultně 60 sekund). Díky tomu je mapa vždy aktuální a odráží změny počasí či kvality ovzduší v reálném čase.

## Možné využití
### Domácí či komunitní monitorování počasí a kvality ovzduší
Uživatelé mohou snadno sledovat meteorologické podmínky ve svém okolí nebo ve vybraných částech ČR díky vizuální LED indikaci a přehledným statistikám.

### Vzdělávací účely a demonstrace datové vizualizace
Projekt může sloužit jako praktický nástroj pro výuku práce s IoT zařízeními, senzory, zpracováním dat a webovým ovládáním embedded systémů.

### Podpora environmentálních projektů a komunitních senzorových sítí
Kód umožňuje jednoduchou integraci s volně dostupnými daty a škálovatelnost pro zobrazování více typů měřených veličin, což ocení i projekty zaměřené na sledování kvality ovzduší nebo klimatu.

### Shrnutí
Celkově jde o sofistikované zařízení postavené na platformě ESP32, které vkusně kombinuje vizuální LED mapu s robustní webovou konfigurací a četnými užitečnými funkcemi. Díky tomu nabízí uživatelům snadné a flexibilní prostředky pro monitorování a zobrazení environmentálních dat, a to jak pro osobní použití, tak i pro širší komunitní projekty.

## Interaktivní mapu můžete objednat na [https://www.laskakit.cz/laskakit-interaktivni-mapa-cr-ws2812b/](https://www.laskakit.cz/laskakit-interaktivni-mapa-cr-ws2812b/)

Webstránka a první zapnutí (nebo nedostupnost klientské Wi-Fi sítě. SSID Laskakit-MapaCR, IP adresa 192.168.4.1
<img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/TMEP_Meteoradar_Config_WEB_Portal/web_access_point.jpeg" width="50%">
Webstránka - první nastavení

<img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/TMEP_Meteoradar_Config_WEB_Portal/web_teplota.jpeg" width="50%">
Webstránka teplota

<img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/TMEP_Meteoradar_Config_WEB_Portal/web_tlak.jpeg" width="50%">
Webstránka tlak

<img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/TMEP_Meteoradar_Config_WEB_Portal/web_prasnost.jpeg" width="50%">
Webstránka prasnost

<img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/TMEP_Meteoradar_Config_WEB_Portal/web_meteoradar.jpeg" width="50%">
Webstránka meteoradar
