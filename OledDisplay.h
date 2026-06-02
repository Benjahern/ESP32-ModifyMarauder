#pragma once
#ifndef OledDisplay_h
#define OledDisplay_h

// ============================================================
//  OledDisplay.h — Menú OLED jerárquico 4 niveles
//  ESP32 Marauder CyberDeck
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

// ── NIVELES DE MENÚ ─────────────────────────────────────────
#define MENU_MAIN     0
#define MENU_CATEGORY 1
#define MENU_SUBCAT   2
#define MENU_ACTION   3

// ── MENU NODE TYPES ──────────────────────────────────────────
#define NODE_SUBMENU  0   // Tiene hijos (submenú)
#define NODE_ACTION   1   // Ejecuta una acción
#define NODE_BACK     2   // Volver al nivel anterior

// ── TREES ───────────────────────────────────────────────────
#define TREE_MAIN   0
#define TREE_WIFI   1
#define TREE_BT     2
#define TREE_RFID   3
#define TREE_SYS    4

// Estructura para tarjetas RFID guardadas
struct SavedCard {
  String uid;
  String label;
};

// ── MENU NODE DEFINITION ─────────────────────────────────────
struct MenuNodeDef {
  const char* name;     // Nombre a mostrar
  uint8_t     type;     // NODE_SUBMENU, NODE_ACTION, NODE_BACK
  int         action;   // Código de acción (modo WiFiScan) o -1
  uint8_t     parent;   // Índice del padre (para nodos de submenú)
};

// ── WIFI MENU TREE ───────────────────────────────────────────
// Estructura: index, type, action, parent
// Parent 0 = nivel 1 (Sniffers, Attacks, General)
// Parent 1 = nivel 2 Sniffers (entrada es index 0)
// Parent 2 = nivel 2 Attacks (entrada es index 1)
// Parent 3 = nivel 2 General (entrada es index 2)
// Parent 4 = nivel 2 Evil Portal
const MenuNodeDef wifiMenuTree[] = {
  // Nivel 1 - WiFi (índice 0-5)
  { "Sniffers",     NODE_SUBMENU, -1, 0 },  // 0
  { "Attacks",      NODE_SUBMENU, -1, 0 },  // 1
  { "General",      NODE_SUBMENU, -1, 0 },  // 2
  { "Scan AP",      NODE_ACTION,  2,  0 },  // 3 - WIFI_SCAN_AP
  { "Evil Portal",  NODE_ACTION,  30, 0 },  // 4 - WIFI_SCAN_EVIL_PORTAL
  { "<- Volver",    NODE_BACK,    -1, 0 },  // 5

  // Sniffers (parent=0, índice base 6)
  { "Probe Sniff",       NODE_ACTION, 1,  0 },  // 6 - WIFI_SCAN_PROBE
  { "Beacon Sniff",      NODE_ACTION, 2,  0 },  // 7 - WIFI_SCAN_AP
  { "Deauth Sniff",      NODE_ACTION, 5,  0 },  // 8 - WIFI_SCAN_DEAUTH
  { "Packet Monitor",    NODE_ACTION, 7,  0 },  // 9 - WIFI_PACKET_MONITOR
  { "Channel Analyzer",  NODE_ACTION, 46, 0 },  // 10 - WIFI_SCAN_CHAN_ANALYZER
  { "PWN Scan",          NODE_ACTION, 3,  0 },  // 11 - WIFI_SCAN_PWN
  { "PineScan",          NODE_ACTION, 50, 0 },  // 12 - WIFI_SCAN_PINESCAN
  { "MultiSSID",         NODE_ACTION, 51, 0 },  // 13 - WIFI_SCAN_MULTISSID
  { "Raw Capture",       NODE_ACTION, 25, 0 },  // 14 - WIFI_SCAN_RAW_CAPTURE
  { "SAE Commit",        NODE_ACTION, 77, 0 },  // 15 - WIFI_SCAN_SAE_COMMIT
  { "<- Volver",         NODE_BACK,   -1, 0 },  // 16

  // Attacks (parent=1, índice base 17)
  { "Beacon Spam",       NODE_ACTION, 8,  1 },  // 17 - WIFI_ATTACK_BEACON_SPAM
  { "Beacon List",       NODE_ACTION, 15, 1 },  // 18 - WIFI_ATTACK_BEACON_LIST
  { "Rick Roll",         NODE_ACTION, 9,  1 },  // 19 - WIFI_ATTACK_RICK_ROLL
  { "Funny Beacon",      NODE_ACTION, 99, 1 },  // 20 - WIFI_ATTACK_FUNNY_BEACON
  { "Deauth",            NODE_ACTION, 20, 1 },  // 21 - WIFI_ATTACK_DEAUTH
  { "Deauth Target",     NODE_ACTION, 27, 1 },  // 22 - WIFI_ATTACK_DEAUTH_TARGETED
  { "AP Spam",           NODE_ACTION, 21, 1 },  // 23 - WIFI_ATTACK_AP_SPAM
  { "Auth Attack",       NODE_ACTION, 18, 1 },  // 24 - WIFI_ATTACK_AUTH
  { "Karma",             NODE_ACTION, 30, 1 },  // 25 - WIFI_SCAN_EVIL_PORTAL
  { "Bad Msg",           NODE_ACTION, 56, 1 },  // 26 - WIFI_ATTACK_BAD_MSG
  { "Assoc Sleep",       NODE_ACTION, 61, 1 },  // 27 - WIFI_ATTACK_SLEEP
  { "SAE Flood",         NODE_ACTION, 78, 1 },  // 28 - WIFI_ATTACK_SAE_COMMIT
  { "CSA",               NODE_ACTION, 79, 1 },  // 29 - WIFI_ATTACK_CSA
  { "Quiet",             NODE_ACTION, 80, 1 },  // 30 - WIFI_ATTACK_QUIET
  { "<- Volver",         NODE_BACK,   -1, 1 },  // 31

  // General (parent=2, índice base 32)
  { "Select AP",         NODE_ACTION, 83, 2 },  // 32 - WIFI_SCAN_DISPLAY_AP_INFO
  { "Select STA",        NODE_ACTION, 26, 2 },  // 33 - WIFI_SCAN_STATION
  { "View AP Info",      NODE_ACTION, 83, 2 },  // 34 - WIFI_SCAN_DISPLAY_AP_INFO
  { "Join WiFi",         NODE_ACTION, -2, 2 },  // 35 - serial join (no hay modo)
  { "Join Saved",        NODE_ACTION, -3, 2 },  // 36 - serial join saved (no hay modo)
  { "Start AP",          NODE_ACTION, -4, 2 },  // 37 - serial ap
  { "Set MACs",          NODE_ACTION, -5, 2 },  // 38 - serial setmac
  { "<- Volver",         NODE_BACK,   -1, 2 },  // 39
};
const int wifiMenuTreeLen = 40;

// ── BLUETOOTH MENU TREE ─────────────────────────────────────
const MenuNodeDef btMenuTree[] = {
  // Nivel 1 - Bluetooth (índice 0-5)
  { "BLE Scan",       NODE_ACTION,  10, 0 },  // 0 - BT_SCAN_ALL
  { "AirTag Scan",    NODE_ACTION,  43, 0 },  // 1 - BT_SCAN_AIRTAG
  { "AirTag Monitor", NODE_ACTION,  70, 0 },  // 2 - BT_SCAN_AIRTAG_MON
  { "Sniffers",       NODE_SUBMENU, -1,  0 },  // 3
  { "Attacks",        NODE_SUBMENU, -1,  0 },  // 4
  { "<- Volver",      NODE_BACK,    -1,  0 },  // 5

  // Sniffers (parent=3, índice base 6)
  { "Flipper Sniff",  NODE_ACTION, 45,  3 },  // 6 - BT_SCAN_FLIPPER
  { "Skimmers",      NODE_ACTION, 11,  3 },  // 7 - BT_SCAN_SKIMMERS
  { "Flock",          NODE_ACTION, 72,  3 },  // 8 - BT_SCAN_FLOCK
  { "Meta Detect",    NODE_ACTION, 81,  3 },  // 9 - BT_SCAN_RAYBAN
  { "Analyzer",       NODE_ACTION, 47,  3 },  // 10 - BT_SCAN_ANALYZER
  { "<- Volver",      NODE_BACK,    -1,  3 },  // 11

  // Attacks (parent=4, índice base 12)
  { "Sour Apple",     NODE_ACTION, 36,  4 },  // 12 - BT_ATTACK_SOUR_APPLE
  { "Apple Juice",    NODE_ACTION, 82,  4 },  // 13 - BT_ATTACK_APPLE_JUICE
  { "Swiftpair Spam", NODE_ACTION, 37,  4 },  // 14 - BT_ATTACK_SWIFTPAIR_SPAM
  { "Samsung Spam",   NODE_ACTION, 39,  4 },  // 15 - BT_ATTACK_SAMSUNG_SPAM
  { "Google Spam",    NODE_ACTION, 41,  4 },  // 16 - BT_ATTACK_GOOGLE_SPAM
  { "Flipper Spam",   NODE_ACTION, 42,  4 },  // 17 - BT_ATTACK_FLIPPER_SPAM
  { "Spam All",       NODE_ACTION, 38,  4 },  // 18 - BT_ATTACK_SPAM_ALL
  { "<- Volver",      NODE_BACK,    -1,  4 },  // 19
};
const int btMenuTreeLen = 20;

// ── RFID MENU TREE ──────────────────────────────────────────
const MenuNodeDef rfidMenuTree[] = {
  { "Leer UID",       NODE_ACTION, -10, 0 },  // 0 - acción local
  { "Leer Sector",    NODE_ACTION, -11, 0 },  // 1 - acción local
  { "Guardar Tarjeta",NODE_ACTION, -12, 0 },  // 2 - acción local
  { "Ver Guardadas",  NODE_ACTION, -13, 0 },  // 3 - acción local
  { "<- Volver",      NODE_BACK,   -1,  0 },  // 4
};
const int rfidMenuTreeLen = 5;

// ── SISTEMA MENU TREE ────────────────────────────────────────
const MenuNodeDef sysMenuTree[] = {
  { "Info Sistema",   NODE_ACTION, -20, 0 },  // 0 - acción local
  { "Stop",           NODE_ACTION, 0,   0 },  // 1 - WIFI_SCAN_OFF
  { "Shutdown",       NODE_ACTION, -21, 0 },  // 2 - acción local
  { "Settings",       NODE_ACTION, -22, 0 },  // 3 - serial cli
  { "<- Volver",       NODE_BACK,   -1,  0 },  // 4
};
const int sysMenuTreeLen = 5;

// ── MAIN MENU TREE ───────────────────────────────────────────
const MenuNodeDef mainMenuTree[] = {
  { "WiFi",       NODE_SUBMENU, -1, 0 },  // 0
  { "Bluetooth",  NODE_SUBMENU, -1, 0 },  // 1
  { "RFID",      NODE_SUBMENU, -1, 0 },  // 2
  { "Sistema",    NODE_SUBMENU, -1, 0 },  // 3
};
const int mainMenuTreeLen = 4;

class OledDisplay {
  public:
    OledDisplay();

    // Init
    void begin();

    // Render
    void showBoot();
    void showMessage(const char* title,
                     const char* line1 = "",
                     const char* line2 = "",
                     const char* line3 = "");
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

    // Estado menú jerárquico
    uint8_t  _menuLevel     = MENU_MAIN;   // 0=MAIN, 1=CATEGORY, 2=SUBCAT, 3=ACTION
    uint8_t  _menuIndex     = 0;           // Índice actual en nivel actual
    uint8_t  _currentTree   = TREE_MAIN;   // Árbol actual (TREE_*)
    uint8_t  _scrollOffset  = 0;          // Para menús largos

    // Camino de navegación (_path[0]=main idx, _path[1]=cat idx, etc.)
    uint8_t  _path[4] = {0, 0, 0, 0};

    // Acción solicitada a Marauder
    bool _actionRequested = false;
    int  _requestedAction = -1;

    // RFID storage
    SavedCard _savedCards[10];
    int       _savedCount = 0;

    // Debounce
    unsigned long _lastPress = 0;

    // Helpers
    void   _drawHeader(const char* title);
    bool   _btnPressed(int pin);
    bool   _btnHeld(int pin, int ms = 800);
    String _getUID();

    // Navegación jerárquica
    const MenuNodeDef* _getTree(uint8_t tree);
    uint8_t _getTreeSize(uint8_t tree);
    void   _drawCurrentMenu();
    void   _execAction(const MenuNodeDef* node);

    // Acciones locales (negative action codes)
    void   _doSysInfo();
    void   _doShutdown();
    void   _doBeaconSpam();
    void   _doShowAPList();
    void   _showSerialHelp(const char* title, const char* cmd);

    // Mostrar resultados
    void   _showAPResults();
};

#endif