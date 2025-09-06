<img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/Hra_Zachran_Mesto/laskakit_mapa_litomerice.png" width="60%">

## Hra Zachraň město s vývojovou deskou Laskakit Interaktivní Mapa ČR

Tuto aplikaci jsme vytvořili jako zábavnou, interaktivní a naučnou hru pro použití s vývojovou deskou Laskakit Interaktivní Mapa ČR.

Hra začíná deseti městy, které jsou v plamenech a jen ty je můžeš zachránit! Ale pozor! Pokud odpovíš špatně, začne hořet další město!

### Využití
Využití této hry může být ve výuce na školách, kroužcích nebo třeba i mateřské škole, kde ovládat telefon bude učitel a odpovídat budou děti.
Položené otázky jsou jen ve vaší režii - jaké otázky tam zadáte, takové tam budete mít. Jediné, co je nutné, je dodržet formát otázek.
Chcete otázky ohledně matematiky pro první stupeň? Jasně! Zeměpis? Jasně! Čeština? Samozřejmě. 

Díky této interaktivní pomůcce může být i výuka zajímavější a zábavnější!

Tato aplikace využívá interaktivní mapu ČR od Laskakit.cz. Hráč na mobilním telefonu, tabletu nebo počítači musí skrze webový prohlížeč odpovídat na otázky, dokud neuhasí všechna města.

### Začátek hry

Po zapnutí vytvoří deska vlastní Access Point. Připoj se přes tvůj telefon k WiFi "LaskaKit_MapaCR", v prohlížeči zadej IP adresu 192.168.4.1. Tam můžeš zadat jméno a heslo k tvé WiFi.
Po restartu se ti v Serial Terminalu (Arduino, Yat, Putty; rychlost 115200 Bd) zobrazí přidělená IP adresa. V prohlížeči zadej tvou IP adresu a můžeš přidat otázky a začít hrát.

<img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/Hra_Zachran_Mesto/laskakit_mapa_wifi_otazky.png" width="60%">

Vývojová deska založená na ESP32 si otázky neukládá do trvalé paměti. Po každém restartu je nutné zadat sadu otázek. Čip si ale pamatuju zadané jméno a heslo k WiFi. 
Pokud se k ní nepřipojí, vytvoří si vlastní s názvem "Laskakit_MapaCR".

### Formát otázek
Číslo otázky; Otázka; Odpověď0; Odpověď1; Odpověď2; Odpověď3; Index správné odpovědi 0–3

<img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/Hra_Zachran_Mesto/laskakit_mapa_nahrani_otazek.png" width="60%">

Pokud chceš zadat vlastní otázky a nevyužiješ například AI (ChatGPT, Perplexity atp), otázky můžeš napsat v excelu a vyexportovat jako CSV s oddělovačem ";".

### Tip: Prompt (zadání) pro chatGPT, Perplexity atp
Příklad: Vytvoř 30 otázek. Otázky se musí týkat přírodovědy pro druhý stupeň základní školy. Vedle správné odpovědi na otázku přidej i tři nesprávné odpovědi. 
Správnou odpověď na jiné místo odpovědi.
Číslo otázky; Otázka; Odpověď0; Odpověď1; Odpověď2; Odpověď3; Číslo správné odpovědi 0–3

Prompt (zadání) si samozřejmě můžeš upravit dle vlastních potřeb.

<img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/Hra_Zachran_Mesto/laskakit_mapa_AI.png" width="60%">

Kód provádí základní kontrolu zadaných otázek - zda oddělovač je správně a počet oddělovačů odpovídá a také index správné odpovědi je v rozsahu.

<img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/Hra_Zachran_Mesto/laskakit_mapa_chyba1.png" width="40%"> <img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/Hra_Zachran_Mesto/laskakit_mapa_chyba2.png" width="40%">

### Potřebné knihovny
https://github.com/adafruit/Adafruit_NeoPixel

### Potřebujete koupit
https://www.laskakit.cz/laskakit-interaktivni-mapa-cr-ws2812b/

### Návod

Na začátku hry se vybere náhodně 10 měst

<img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/Hra_Zachran_Mesto/laskakit_mapa_zacatek.png" width="45%"> <img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/Hra_Zachran_Mesto/laskakit_mapa_10_mest_hori.png" width="45%">

Při správné odpovědi se uhasi jedno z měst o které se hraje (Litoměřice)

<img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/Hra_Zachran_Mesto/laskakit_mapa_spravna_odpoved.png" width="45%"> <img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/Hra_Zachran_Mesto/laskakit_mapa_litomerice.png" width="45%">

Při špatné odpovědi naopak začne jiné, náhodně vybrané město, hořet (Břeclav)

<img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/Hra_Zachran_Mesto/laskakit_mapa_spatna_odpoved.png" width="45%"> <img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/Hra_Zachran_Mesto/laskakit_mapa_hori_breclav.png" width="45%">

Při všech správně zodpovězených vyhraješ a můžeš začít hrát znovu a zadat nové otázky!

<img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/Hra_Zachran_Mesto/laskakit_mapa_uhasena_mesta_web.png" width="45%"> <img src="https://github.com/LaskaKit/LED_Czech_Map/blob/main/SW/Hra_Zachran_Mesto/laskakit_mapa_uhasena_mesta.png" width="45%">

