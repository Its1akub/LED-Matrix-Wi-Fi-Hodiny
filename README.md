# LED-Matrix-Wi-Fi-Hodiny

Tento projekt je jednoduché zařízení založené na desce **Arduino UNO R4 WiFi (ESP32)**, které zobrazuje aktuální čas a odpočítává časový úsek s vizuálním upozorněním na zvolený milník. Vše se zobrazuje na **dvouřádkovém LED maticovém displeji (64x16)** a ovládá se přes webové rozhraní dostupné v lokální síti.

## 🚀 Funkce

- Zobrazení aktuálního času synchronizovaného přes NTP
- Webové rozhraní pro nastavení délky odpočtu a milníku
- Zobrazení odpočtu na displeji s animacemi
- Upozornění při dosažení milníku
- Výstupní zpráva „Time’s Up“ po skončení odpočtu
- Rozdělení displeje do dvou zón – horní a dolní řádek

## 🧰 Použité technologie

- **Arduino UNO R4 WiFi (ESP32)**
- **LED Matrix 64x16** (2 řádky po 8 modulech 8x8)
- **MD_Parola** a **MD_MAX72XX** knihovny pro práci s displejem
- **ezTime** knihovna pro synchronizaci a práci s časem
- **WiFiServer** pro jednoduchý HTTP server

## 🌐 Webové rozhraní

Přístupné na IP adrese zařízení v místní síti. Obsahuje formulář pro zadání délky odpočtu (v sekundách) a milníku. Po odeslání dat se hodnoty použijí a odpočet začne automaticky.

## 📷 Ukázka použití

![ukázka zařízení](https://github.com/user-attachments/assets/51a9fedd-a833-41a3-ad67-ddd47bb9da58)

## 📝 Instalace a spuštění

1. Nahrajte kód do Arduino IDE
2. Zajistěte, že máte knihovny:
   - MD_Parola
   - MD_MAX72XX
   - ezTime
3. Vyplňte soubor `arduino_secrets.h`:

```cpp
#define SECRET_SSID "váš_wifi_ssid"
#define SECRET_PASS "vaše_wifi_heslo"
