## Hra Zachraň město s vývojovou deskou Laskakit Interaktivní Mapa ČR

Tuto aplikaci jsme vytvořili jako zábavnou, interaktivní a naučnou hru pro použití s vývojovou deskou Laskakit Interaktivní Mapa ČR.
Hra začíná deseti městy, které jsou v plamenech, a jen ty je můžeš zachránit! Ale pozor! Pokud odpovíš špatně, začne hořet další město!

### Využití
Využití této hry může být ve výuce na školách, kroužcích nebo třeba i mateřské škole, kde ovládat telefon bude učitel a odpovídat budou děti.
Položené otázky jsou jen ve vaší režii - jaké otázky tam zadáte, takové tam budete mít. 
Chcete otázky ohledně matematiky pro první stupeň? Jasně! Zeměpis? Jasně! Čeština? Samozřejmě. 

Tato aplikace využívá interaktivní mapu ČR od Laskakit.cz. Hráč na mobilním telefonu, tabletu nebo počítači musí skrze webový prohlížeč odpovídat na otázky, dokud neuhasí všechna města.

### Začátek hry

Po zapnutí vytvoří deska vlastní Access Point. Připoj se přes tvůj telefon k WiFi "LaskaKit_MapaCR", v prohlížeči zadej IP adresu 192.168.4.1. Tam můžeš zadat jméno a heslo k tvé WiFi.
Po restartu se ti v Serial Terminalu (Arduino, Yat, Putty; rychlost 115200 Bd) zobrazí přidělená IP adresa. V prohlížeči zadej tvou IP adresu a můžeš přidat otázky.

Vývojová deska založená na ESP32 si otázky neukládá do trvalé paměti. Po každém restartu je nutné zadat sadu otázek. Čip si ale pamatuju zadané jméno a heslo k WiFi. 
Pokud se k ní nepřipojí, vytvoří si vlastní s názvem "Laskakit_MapaCR".

### Formát otázek
Číslo otázky; Otázka; Odpověď0; Odpověď1; Odpověď2; Odpověď3; Číslo správné odpovědi 0–3

### Tip: Prompt (zadání) pro chatGPT, Perplexity atp
Příklad: Vytvoř 30 otázek. Otázky se musí týkat přírodovědy pro druhý stupeň základní školy. Vedle správné odpovědi na otázku přidej i tři nesprávné odpovědi. 
Správnou odpověď na jiné místo odpovědi.
Číslo otázky; Otázka; Odpověď0; Odpověď1; Odpověď2; Odpověď3; Číslo správné odpovědi 0–3

Prompt (zadání) si samozřejmě můžete upravit dle vlastních potřeb.

### Potřebné knihovny
https://github.com/adafruit/Adafruit_NeoPixel

### Potřebujete koupit
https://www.laskakit.cz/laskakit-interaktivni-mapa-cr-ws2812b/
