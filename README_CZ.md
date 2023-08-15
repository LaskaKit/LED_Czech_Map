# Interaktivní LaskaKit LED Mapa ČR s adresovatelnými RGB LED

Domů, do zasedačky nebo do kanceláře, teplota nebo srážky, interaktivní mapa ČR s adresovatelnými RGB LED určitě překážky klást nebude. Měj přehled o aktuální situaci ať už je to teplota napříč Českou republikou nebo přicházející srážky, které pohodlně zobrazíš na této mapě. 

![Mapa](https://github.com/LaskaKit/LED_Czech_Map/blob/main/img/10.jpg)

Na plošném spoji ve tvaru České republiky o velikosti cca 265x150mm najdeš desítky adresovatelných RGB LED, každá představuje jedno město v ČR.

Seznam měst a pořadí RGB LED najdeš níže 
| LaskaKit ID | TMEP ID | Okres |
|:--:|:-------:|:------|
| 25	| 1	| Cheb |
| 20	| 2	| Sokolov |
| 17	| 3	| Karlovy Vary |
| 11	| 4	| Chomutov |
| 16	| 5	| Louny |
| 10	| 6	| Most |
| 7	| 7 | Teplice |
| 9	| 8 | Litoměřice |
| 4	| 9 | Ústí nad Labem |
| 1 | 10 | Děčín |
| 5 | 11 | Česká Lípa |
| 2 | 12 | Liberec |
| 3 | 13 | Jablonec nad Nisou |
| 6 | 14 | Semily |
| 12 | 15 | Jičín |
| 8 | 16 | Trutnov |
| 13 | 17 | Náchod |
| 19 | 18 | Hradec Králové |
| 22 | 19 | Rychnov nad Kněžnou |
| 30 | 20 | Ústí nad Orlicí |
| 28 | 21 | Pardubice |
| 35 | 22 | Chrudim |
| 39 | 23 | Svitavy |
| 32 | 24 | Šumperk |
| 18 | 25 | Jeseník |
| 26 | 26 | Bruntál |
| 46 | 27 | Olomouc |
| 31 | 28 | Opava |
| 37 | 29 | Ostrava-město |
| 36 | 30 | Karviná |
| 43 | 31 | Frýdek-Místek |
| 45 | 32 | Nový Jičín |
| 57 | 33 | Vsetín |
| 49 | 34 | Přerov |
| 62 | 35 | Zlín |
| 58 | 36 | Kroměříž |
| 66 | 37 | Uherské Hradiště |
| 69 | 38 | Hodonín |
| 60 | 39 | Vyškov |
| 50 | 40 | Prostějov |
| 56 | 41 | Blansko |
| 64 | 42 | Brno-město |
| xx | 43 | Brno-venkov |
| 72 | 44 | Břeclav |
| 70 | 45 | Znojmo |
| 63 | 46 | Třebíč |
| 48 | 47 | Žďár nad Sázavou |
| 54 | 48 | Jihlava |
| 47 | 49 | Havlíčkův Brod |
| 52 | 50 | Pelhřimov |
| 65 | 51 | Jindřichův Hradec |
| 53 | 52 | Tábor |
| 68 | 53 | České Budějovice |
| 71 | 54 | Český Krumlov |
| 67 | 55 | Prachatice |
| 61 | 56 | Strakonice |
| 59 | 57 | Písek |
| 55 | 58 | Klatovy |
| 51 | 59 | Domažlice |
| 38 | 60 | Tachov |
| xx | 61 | Plzeň-sever |
| 41 | 62 | Plzeň-město |
| xx | 63 | Plzeň-jih |
| 42 | 64 | Rokycany |
| 24 | 65 | Rakovník |
| 23 | 66 | Kladno |
| 15 | 67 | Mělník |
| 14 | 68 | Mladá Boleslav |
| 21 | 69 | Nymburk |
| 29 | 70 | Kolín |
| 34 | 71 | Kutná Hora |
| 40 | 72 | Benešov |
| 44 | 73 | Příbram |
| 33 | 74 | Beroun |
| xx | 75 | Praha-západ |
| xx | 76 | Praha-východ |
| 27 | 77 | Praha |

Deska na které je osazen Wi-Fi a Bluetooth modul ESP32 obsahuje navíc i [uŠup konektor](https://blog.laskakit.cz/predstavujeme-univerzalni-konektor-pro-propojeni-modulu-a-cidel-%CE%BCsup/)https://blog.laskakit.cz/predstavujeme-univerzalni-konektor-pro-propojeni-modulu-a-cidel-%CE%BCsup/ pro připojení nejrůznějších čidel.

A jaké moduly to mohou být? Třeba čidlo [SCD41 pro měření CO2, teploty a vlhkosti vzduchu](https://www.laskakit.cz/laskakit-scd41-senzor-co2--teploty-a-vlhkosti-vzduchu/), čidla SHT41 [teploty a vlhkosti vzduchu](https://www.laskakit.cz/laskakit-sht40-senzor-teploty-a-vlhkosti-vzduchu/) a [čidla tlaku, teploty a vlhkosti vzduchu BME280](https://www.laskakit.cz/arduino-senzor-tlaku--teploty-a-vlhkosti-bme280/), [RTC Hodiny reálného času](https://www.laskakit.cz/laskakit-ds3231-orig--rtc-hodiny-realneho-casu/) s DS3231 nebo [OLED displej o velikosti 1.3" a rozlišení 128x64px](https://www.laskakit.cz/laskakit-oled-displej-128x64-1-3--i2c/?variantId=11903).

Deska LaskaKit LED Mapa ČR má osazen i vlastní programátor a není tak třeba mačkat tlačítka proto, aby se ESP32 naprogramovalo. Prostě jen připojíš USB-C kabel s počítačem a klikneš na "Nahrát".</br>
GPIO pin, který ovládá adresovatelné RGB LED je číslo 25 na ESP32. Adresovatelné RGB LED jsou zapojeny za sebou a díky už napsaným knihovnám je ovládání LEDek naprostá hračka. </br>

Mapa má i čtyři upevňovací otvory a navíc i rozšiřující konektor pro připojení třeba OLED displeje nebo čehokoliv jiného.

Díky popularitě této mapy vznikly nejrůznější zajímavé projekty - zobrazení teplot ze služby [TMEP.cz](https://tmep.cz/) ([vzorový kód najdeš tady](https://github.com/LaskaKit/LED_Czech_Map/tree/main/SW/tmep_okresy-cr-teplota)) nebo zobrazení srážek na základě dat z radaru.</br>
Kód je opět [na našem github](https://github.com/LaskaKit/LED_Czech_Map/tree/main/SW/CHMI_srazky). Kód má navíc možnosti nastavení zobrazení. 
Jednoduché zobrazení (vybereme pouze dominantní barva) nebo možnost přidání animace, zobrazení srážek před 30 minutami, 20, 10 a aktuální radarový obraz.

Máš jiný nápad? Sem s ním!

Mapu můžeš zakoupit na https://www.laskakit.cz/laskakit-interaktivni-mapa-cr-ws2812b/
