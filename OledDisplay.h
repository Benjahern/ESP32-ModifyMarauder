#pragma once
#ifndef OledDisplay_h
#define OledDisplay_h

// ============================================================
//  OledDisplay.h — Integración OLED SSD1306 con Marauder
//  Pines: SDA=26, SCL=22
//  Botones: UP=34, DOWN=35, OK=32
// ============================================================

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <MFRC522.h>

// ── OLED ─────────────────────────────────────────────────────
#define OLED_SDA      26
#define OLED_SCL      22
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define OLED_ADDR      0x3C

// ── BOTONES ──────────────────────────────────────────────────
#define BTN_UP    34
#define BTN_DOWN  35
#define BTN_OK    32
#define DEBOUNCE_MS 200

// ── RC522 ────────────────────────────────────────────────────
#define RC522_SS   5
#define RC522_RST  21

// ── WAKE BUTTON (deep sleep wake-up) ────────────────────────
#define WAKE_BTN   16

// ── MENÚ ─────────────────────────────────────────────────────
#define MENU_MAIN   0
#define MENU_SUB    1
#define MENU_ACTION 2

// Estructura para tarjetas RFID guardadas
struct SavedCard {
  String uid;
  String label;
};

class OledDisplay {
  public:
    OledDisplay();

    // Init
    void begin();

    // Render
    void showBoot();
    void showMainMenu();
    void showSubMenu();
    void showMessage(const char* title,
                     const char* line1,
                     const char* line2 = "",
                     const char* line3 = "");
    void showAPList(int scrollOffset);
    void showStatus(const char* msg);

    // Navegación
    void handleUp();
    void handleDown();
    void handleOK();
    void handleBack();
    void tick(); // llamar en loop()

    // RFID
    void doRFIDRead();
    void doRFIDList();

    // Getters para integración con Marauder
    bool isActionRequested()    { return _actionRequested; }
    int  getRequestedAction()   { return _requestedAction; }
    void clearActionRequest()   { _actionRequested = false; }

    // AP results — Marauder los llena desde WiFiScan
    struct APEntry {
      String ssid;
      int    rssi;
      String enc;
    };
    APEntry apResults[30];
    int     apCount = 0;

  private:
    Adafruit_SSD1306 _display;
    MFRC522          _rfid;

    // Estado menú
    int  _menuLevel  = MENU_MAIN;
    int  _menuIndex  = 0;
    int  _subIndex   = 0;
    int  _parentIdx  = 0;
    int  _apScroll   = 0;

    // Acción solicitada a Marauder
    bool _actionRequested  = false;
    int  _requestedAction  = -1;

    // RFID storage
    SavedCard _savedCards[10];
    int       _savedCount = 0;

    // Debounce
    unsigned long _lastPress = 0;

    // Menús
    static const char* _mainMenu[];
    static const int   _mainMenuLen;
    static const char* _wifiMenu[];
    static const int   _wifiMenuLen;
    static const char* _btMenu[];
    static const int   _btMenuLen;
    static const char* _rfidMenu[];
    static const int   _rfidMenuLen;
    static const char* _sysMenu[];
    static const int   _sysMenuLen;

    // Helpers
    void   _drawHeader(const char* title);
    void   _drawMenu(const char** items, int len, int selected, const char* title);
    bool   _btnPressed(int pin);
    bool   _btnHeld(int pin, int ms = 800);
    String _getUID();
    void   _execAction();
    void   _doBeaconSpam();
    void   _doSysInfo();
    void   _doShutdown();
    void   _showAPResults();
};

// Acciones que OledDisplay puede solicitar a Marauder
#define ACTION_WIFI_SCAN          10
#define ACTION_DEAUTH             11
#define ACTION_DEAUTH_TARGETED    12
#define ACTION_BEACON_SPAM        13
#define ACTION_EVIL_TWIN          14
#define ACTION_PROBE_SNIFF        15
#define ACTION_PACKET_MONITOR     16
#define ACTION_BLE_SCAN           20
#define ACTION_AIRTAG_SCAN        21
#define ACTION_TRACKER            22
#define ACTION_STOP               99

#endif
