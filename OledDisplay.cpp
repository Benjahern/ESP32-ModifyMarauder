// ============================================================
//  OledDisplay.cpp — Menú OLED jerárquico 4 niveles
//  ESP32 Marauder CyberDeck
// ============================================================

#include "OledDisplay.h"
#include <WiFi.h>

// ── CONSTRUCTOR ───────────────────────────────────────────────
OledDisplay::OledDisplay()
  : _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
    _rfid(RC522_SS, RC522_RST) {}

// ── INIT ─────────────────────────────────────────────────────
void OledDisplay::begin() {
  // Botones
  pinMode(BTN_UP,   INPUT);
  pinMode(BTN_DOWN, INPUT);
  pinMode(BTN_OK,   INPUT_PULLUP);
  pinMode(WAKE_BTN, INPUT_PULLUP);

  // OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("[OLED] ERROR: pantalla no encontrada en 0x3C"));
    return;
  }
  Serial.println(F("[OLED] OK"));

  // RC522
  SPI.begin();
  _rfid.PCD_Init();
  Serial.println(F("[RFID] RC522 iniciado"));

  showBoot();
  _drawCurrentMenu();
}

// ── HELPERS DISPLAY ──────────────────────────────────────────
void OledDisplay::_drawHeader(const char* title) {
  _display.fillRect(0, 0, SCREEN_WIDTH, 12, SSD1306_WHITE);
  _display.setTextColor(SSD1306_BLACK);
  _display.setTextSize(1);
  _display.setCursor(2, 2);
  _display.print(title);
  _display.setTextColor(SSD1306_WHITE);
}

// ── TREE HELPERS ─────────────────────────────────────────────
const MenuNodeDef* OledDisplay::_getTree(uint8_t tree) {
  switch (tree) {
    case TREE_MAIN: return mainMenuTree;
    case TREE_WIFI: return wifiMenuTree;
    case TREE_BT:   return btMenuTree;
    case TREE_RFID: return rfidMenuTree;
    case TREE_SYS:  return sysMenuTree;
    default: return nullptr;
  }
}

uint8_t OledDisplay::_getTreeSize(uint8_t tree) {
  switch (tree) {
    case TREE_MAIN: return mainMenuTreeLen;
    case TREE_WIFI: return wifiMenuTreeLen;
    case TREE_BT:   return btMenuTreeLen;
    case TREE_RFID: return rfidMenuTreeLen;
    case TREE_SYS:  return sysMenuTreeLen;
    default: return 0;
  }
}

// ── DRAW CURRENT MENU ────────────────────────────────────────
void OledDisplay::_drawCurrentMenu() {
  const MenuNodeDef* tree = _getTree(_currentTree);
  uint8_t count = _getTreeSize(_currentTree);

  if (!tree) return;

  // Título del header
  const char* title;
  switch (_currentTree) {
    case TREE_MAIN: title = "CYBERWAWALDO"; break;
    case TREE_WIFI: title = "WiFi"; break;
    case TREE_BT:   title = "Bluetooth"; break;
    case TREE_RFID: title = "RFID"; break;
    case TREE_SYS:  title = "Sistema"; break;
    default: title = "";
  }

  _display.clearDisplay();
  _drawHeader(title);

  // Calcular scroll si hay más de 4 items
  if (_menuIndex >= _scrollOffset + 4) {
    _scrollOffset = (_menuIndex >= 4) ? _menuIndex - 3 : 0;
  }
  if (_menuIndex < _scrollOffset && _menuIndex >= 4) {
    _scrollOffset = _menuIndex;
  }

  // Mostrar 4 items visibles
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t idx = _scrollOffset + i;
    if (idx >= count) break;

    int y = 14 + (i * 12);

    // Selección
    if (idx == _menuIndex) {
      _display.fillRect(0, y - 1, SCREEN_WIDTH, 12, SSD1306_WHITE);
      _display.setTextColor(SSD1306_BLACK);
    } else {
      _display.setTextColor(SSD1306_WHITE);
    }

    _display.setTextSize(1);
    _display.setCursor(4, y);

    // Mostrar nombre (truncar si muy largo)
    const char* name = tree[idx].name;
    if (strlen(name) > 16) {
      char tmp[17];
      strncpy(tmp, name, 14);
      tmp[14] = '\0';
      _display.print(tmp);
      _display.print("..");
    } else {
      _display.print(name);
    }
  }

  // Indicador de scroll si hay más items
  if (count > 4) {
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(100, 56);
    _display.print(_menuIndex + 1);
    _display.print("/");
    _display.print(count);
  }

  _display.display();
}

// ── BOTONES ──────────────────────────────────────────────────
bool OledDisplay::_btnPressed(int pin) {
  if (digitalRead(pin) == LOW && (millis() - _lastPress > DEBOUNCE_MS)) {
    _lastPress = millis();
    return true;
  }
  return false;
}

bool OledDisplay::_btnHeld(int pin, int ms) {
  if (digitalRead(pin) == LOW) {
    unsigned long t = millis();
    while (digitalRead(pin) == LOW) {
      if (millis() - t > (unsigned long)ms) return true;
      delay(10);
    }
  }
  return false;
}

// ── NAVEGACIÓN ───────────────────────────────────────────────
void OledDisplay::handleUp() {
  const MenuNodeDef* tree = _getTree(_currentTree);
  uint8_t count = _getTreeSize(_currentTree);

  if (_menuIndex == 0) {
    _menuIndex = count - 1;
  } else {
    _menuIndex--;
  }

  // Scroll
  if (_menuIndex < _scrollOffset) {
    _scrollOffset = (_menuIndex >= 4) ? _menuIndex - 3 : 0;
  }

  _drawCurrentMenu();
}

void OledDisplay::handleDown() {
  const MenuNodeDef* tree = _getTree(_currentTree);
  uint8_t count = _getTreeSize(_currentTree);

  _menuIndex = (_menuIndex + 1) % count;

  // Scroll
  if (_menuIndex >= _scrollOffset + 4) {
    _scrollOffset = _menuIndex - 3;
  }

  _drawCurrentMenu();
}

void OledDisplay::handleOK() {
  const MenuNodeDef* tree = _getTree(_currentTree);
  MenuNodeDef node = tree[_menuIndex];

  switch (node.type) {
    case NODE_SUBMENU:
      // Guardar posición actual en path
      _path[_menuLevel] = _menuIndex;
      _menuLevel = MENU_SUBCAT;
      _menuIndex = 0;
      _scrollOffset = 0;

      // Cambiar al subtree del nodo seleccionado
      switch (_currentTree) {
        case TREE_MAIN:
          if (_menuIndex == 0) _currentTree = TREE_WIFI;
          else if (_menuIndex == 1) _currentTree = TREE_BT;
          else if (_menuIndex == 2) _currentTree = TREE_RFID;
          else if (_menuIndex == 3) _currentTree = TREE_SYS;
          break;
      }
      _drawCurrentMenu();
      break;

    case NODE_ACTION:
      _path[_menuLevel] = _menuIndex;
      _execAction(&node);
      break;

    case NODE_BACK:
      handleBack();
      break;
  }
}

void OledDisplay::handleBack() {
  if (_menuLevel == MENU_MAIN) return;

  _menuLevel--;
  _menuIndex = _path[_menuLevel];
  _scrollOffset = 0;

  // Volver al tree del nivel correspondiente
  if (_menuLevel == MENU_MAIN) {
    _currentTree = TREE_MAIN;
  } else if (_menuLevel == MENU_CATEGORY) {
    // Volvemos al main, ajustar el index para saber cuál category
    _menuIndex = _path[0];
    if (_currentTree == TREE_WIFI || _currentTree == TREE_BT ||
        _currentTree == TREE_RFID || _currentTree == TREE_SYS) {
      _currentTree = TREE_MAIN;
    }
  }

  _drawCurrentMenu();
}

// ── EJECUTAR ACCIÓN ──────────────────────────────────────────
void OledDisplay::_execAction(const MenuNodeDef* node) {
  int action = node->action;

  // ══ ACCIONES LOCALES (códigos negativos) ══════════════════

  // RFID acciones
  if (action == -10) { // Leer UID
    doRFIDRead();
    _drawCurrentMenu();
    return;
  }
  if (action == -11) { // Leer Sector
    showMessage("Sector Read", "No implementada", "Mifare Classic", "OK=volver");
    while (!_btnPressed(BTN_OK)) delay(50);
    _drawCurrentMenu();
    return;
  }
  if (action == -12) { // Guardar Tarjeta
    doRFIDRead();
    _drawCurrentMenu();
    return;
  }
  if (action == -13) { // Ver Guardadas
    doRFIDList();
    _drawCurrentMenu();
    return;
  }

  // Sistema acciones
  if (action == -20) { // Info Sistema
    _doSysInfo();
    _drawCurrentMenu();
    return;
  }
  if (action == -21) { // Shutdown
    _doShutdown();
    return;
  }
  if (action == -22) { // Settings (serial)
    _showSerialHelp("Settings", "Use serial/cli");
    return;
  }

  // Acciones que requieren serial (códigos -2 a -5)
  if (action == -2) { // Join WiFi
    _showSerialHelp("Join WiFi", "join <ssid> <pass>");
    return;
  }
  if (action == -3) { // Join Saved
    _showSerialHelp("Join Saved", "loadwifi / joinwifi");
    return;
  }
  if (action == -4) { // Start AP
    _showSerialHelp("Start AP", "ap <ssid> <pass>");
    return;
  }
  if (action == -5) { // Set MACs
    _showSerialHelp("Set MACs", "setmac <ap|sta> <mac>");
    return;
  }

  // ══ ACCIONES WIFI/BT/GPS (códigos positivos) ════════════════

  // Para ataques持久, mostrar feedback y retornar
  // El loop principal se encargará de iniciar el scan

  const char* title = node->name;
  const char* msg1 = "Corriendo...";
  const char* msg2 = "OK para menu";

  switch (action) {
    // WiFi Sniffers
    case 1:  msg1 = "Sniffing probes..."; break;    // WIFI_SCAN_PROBE
    case 2:  msg1 = "Escaneando APs..."; break;     // WIFI_SCAN_AP
    case 3:  msg1 = "PWN mode..."; break;           // WIFI_SCAN_PWN
    case 5:  msg1 = "Sniffing deauths..."; break;   // WIFI_SCAN_DEAUTH
    case 7:  msg1 = "Monitor packets..."; break;    // WIFI_PACKET_MONITOR
    case 25: msg1 = "Raw capture..."; break;        // WIFI_SCAN_RAW_CAPTURE
    case 46: msg1 = "Channel analyze..."; break;    // WIFI_SCAN_CHAN_ANALYZER
    case 50: msg1 = "PineAP scan..."; break;       // WIFI_SCAN_PINESCAN
    case 51: msg1 = "MultiSSID scan..."; break;     // WIFI_SCAN_MULTISSID
    case 77: msg1 = "SAE commit..."; break;          // WIFI_SCAN_SAE_COMMIT

    // WiFi Attacks
    case 8:  msg1 = "Enviando beacons..."; break;   // WIFI_ATTACK_BEACON_SPAM
    case 9:  msg1 = "Rick Roll!"; break;             // WIFI_ATTACK_RICK_ROLL
    case 15: msg1 = "Beacon list..."; break;         // WIFI_ATTACK_BEACON_LIST
    case 18: msg1 = "Auth attack..."; break;         // WIFI_ATTACK_AUTH
    case 20: msg1 = "Deauth broadcast..."; break;    // WIFI_ATTACK_DEAUTH
    case 21: msg1 = "AP spam..."; break;            // WIFI_ATTACK_AP_SPAM
    case 27: msg1 = "Deauth target..."; break;      // WIFI_ATTACK_DEAUTH_TARGETED
    case 30: msg1 = "Karma/EvilPortal..."; break;   // WIFI_SCAN_EVIL_PORTAL
    case 56: msg1 = "Bad msg attack..."; break;     // WIFI_ATTACK_BAD_MSG
    case 61: msg1 = "Assoc sleep..."; break;        // WIFI_ATTACK_SLEEP
    case 78: msg1 = "SAE flood..."; break;          // WIFI_ATTACK_SAE_COMMIT
    case 79: msg1 = "Channel switch..."; break;     // WIFI_ATTACK_CSA
    case 80: msg1 = "Quiet mode..."; break;         // WIFI_ATTACK_QUIET
    case 99: msg1 = "Funny beacons..."; break;      // WIFI_ATTACK_FUNNY_BEACON

    // WiFi General
    case 26: msg1 = "STA select..."; break;         // WIFI_SCAN_STATION
    case 83: msg1 = "AP info..."; break;            // WIFI_SCAN_DISPLAY_AP_INFO

    // Bluetooth
    case 10: msg1 = "BLE scan..."; break;           // BT_SCAN_ALL
    case 11: msg1 = "CC skimmers..."; break;        // BT_SCAN_SKIMMERS
    case 36: msg1 = "Sour Apple..."; break;         // BT_ATTACK_SOUR_APPLE
    case 37: msg1 = "Swiftpair spam..."; break;     // BT_ATTACK_SWIFTPAIR_SPAM
    case 38: msg1 = "Spam all BLE..."; break;       // BT_ATTACK_SPAM_ALL
    case 39: msg1 = "Samsung spam..."; break;       // BT_ATTACK_SAMSUNG_SPAM
    case 41: msg1 = "Google spam..."; break;        // BT_ATTACK_GOOGLE_SPAM
    case 42: msg1 = "Flipper spam..."; break;       // BT_ATTACK_FLIPPER_SPAM
    case 43: msg1 = "AirTag scan..."; break;        // BT_SCAN_AIRTAG
    case 45: msg1 = "Flipper sniff..."; break;      // BT_SCAN_FLIPPER
    case 47: msg1 = "BT analyzer..."; break;         // BT_SCAN_ANALYZER
    case 70: msg1 = "AirTag monitor..."; break;     // BT_SCAN_AIRTAG_MON
    case 72: msg1 = "Flock detect..."; break;        // BT_SCAN_FLOCK
    case 81: msg1 = "Meta detect..."; break;         // BT_SCAN_RAYBAN
    case 82: msg1 = "Apple Juice..."; break;        // BT_ATTACK_APPLE_JUICE

    // GPS
    case 31: msg1 = "GPS data..."; break;           // WIFI_SCAN_GPS_DATA
    case 40: msg1 = "NMEA stream..."; break;         // WIFI_SCAN_GPS_NMEA
    case 55: msg1 = "GPS tracking..."; break;         // GPS_TRACKER
    case 63: msg1 = "POI mode..."; break;           // GPS_POI

    // Stop
    case 0:  msg1 = "Detenido"; msg2 = "OK"; break; // WIFI_SCAN_OFF
  }

  showMessage(title, msg1, msg2);
  delay(1000);

  // Solicitar acción al loop principal
  _actionRequested = true;
  _requestedAction = action;
}

// ── MOSTRAR MENSAJE CON ESPERA ───────────────────────────────
void OledDisplay::_showSerialHelp(const char* title, const char* cmd) {
  showMessage(title, cmd, "OK=volver", "Ver serial");
  while (!_btnPressed(BTN_OK)) delay(50);
  _drawCurrentMenu();
}

// ── ACCIONES LOCALES ─────────────────────────────────────────
void OledDisplay::_doSysInfo() {
  while (true) {
    _display.clearDisplay();
    _drawHeader("Sistema");
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);

    _display.setCursor(0, 14);
    _display.print("Heap: ");
    _display.print(ESP.getFreeHeap() / 1024);
    _display.println(" KB libre");

    _display.setCursor(0, 24);
    _display.print("Flash: ");
    _display.print(ESP.getFlashChipSize() / 1024 / 1024);
    _display.println(" MB");

    _display.setCursor(0, 34);
    _display.print("CPU: ");
    _display.print(ESP.getCpuFreqMHz());
    _display.println(" MHz");

    _display.setCursor(0, 44);
    String mac = WiFi.macAddress();
    _display.print("MAC: "); _display.println(mac.substring(9));

    _display.setCursor(0, 55);
    _display.println("OK para volver");
    _display.display();

    if (_btnPressed(BTN_OK)) break;
    delay(200);
  }
}

void OledDisplay::_doShutdown() {
  _display.clearDisplay();
  _drawHeader("Shutdown");
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);
  _display.setCursor(0, 20);
  _display.println("Apagando...");
  _display.setCursor(0, 32);
  _display.println("Desconectar USB");
  _display.setCursor(0, 44);
  _display.println("OK = Confirmar");
  _display.display();
  delay(2000);

  while (true) {
    if (_btnPressed(BTN_OK)) {
      _display.ssd1306_command(SSD1306_DISPLAYOFF);
      delay(100);
      ESP.deepSleep(0);
    }
    if (_btnPressed(BTN_DOWN)) {
      _drawCurrentMenu();
      return;
    }
    delay(50);
  }
}

// ── RFID ─────────────────────────────────────────────────────
String OledDisplay::_getUID() {
  String uid = "";
  for (byte i = 0; i < _rfid.uid.size; i++) {
    if (_rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(_rfid.uid.uidByte[i], HEX);
    if (i < _rfid.uid.size - 1) uid += ":";
  }
  uid.toUpperCase();
  return uid;
}

void OledDisplay::doRFIDRead() {
  showMessage("RFID", "Acerca tarjeta", "al lector...", "OK=cancelar");

  unsigned long timeout = millis() + 15000;

  while (millis() < timeout) {
    if (_btnPressed(BTN_OK)) return;

    if (!_rfid.PICC_IsNewCardPresent()) { delay(100); continue; }
    if (!_rfid.PICC_ReadCardSerial())   { delay(100); continue; }

    String uid = _getUID();
    MFRC522::PICC_Type t = _rfid.PICC_GetType(_rfid.uid.sak);
    String typeName = String(_rfid.PICC_GetTypeName(t));

    Serial.println("[RFID] UID: " + uid + " Tipo: " + typeName);

    _display.clearDisplay();
    _drawHeader("RFID - Leido");
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 15); _display.print("UID: "); _display.println(uid);
    _display.setCursor(0, 27); _display.print("Tipo: "); _display.println(typeName);
    _display.setCursor(0, 39); _display.print("SAK: 0x"); _display.println(_rfid.uid.sak, HEX);
    _display.setCursor(0, 51); _display.println("UP=Guardar OK=Sal");
    _display.display();

    _rfid.PICC_HaltA();
    _rfid.PCD_StopCrypto1();

    while (true) {
      if (_btnPressed(BTN_UP)) {
        if (_savedCount < 10) {
          _savedCards[_savedCount].uid   = uid;
          _savedCards[_savedCount].label = "Card_" + String(_savedCount + 1);
          _savedCount++;
          showMessage("RFID", "Guardada!", uid.c_str(), "OK para continuar");
          while (!_btnPressed(BTN_OK)) delay(50);
        } else {
          showMessage("RFID", "Memoria llena", "Max 10 tarjetas");
          delay(1500);
        }
        return;
      }
      if (_btnPressed(BTN_OK)) return;
      delay(50);
    }
  }

  showMessage("RFID", "Timeout", "Sin tarjeta");
  delay(1500);
}

void OledDisplay::doRFIDList() {
  if (_savedCount == 0) {
    showMessage("Guardadas", "Sin tarjetas", "Lee una primero");
    delay(2000);
    return;
  }

  int idx = 0;
  while (true) {
    _display.clearDisplay();
    _drawHeader("Tarjetas");
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 15);
    _display.print(idx + 1); _display.print("/"); _display.print(_savedCount);
    _display.setCursor(0, 27); _display.println(_savedCards[idx].label);
    _display.setCursor(0, 39); _display.println(_savedCards[idx].uid);
    _display.setCursor(0, 51); _display.println("U/D=Nav OK=Salir");
    _display.display();

    if (_btnPressed(BTN_UP)   && idx > 0)              idx--;
    if (_btnPressed(BTN_DOWN) && idx < _savedCount - 1) idx++;
    if (_btnPressed(BTN_OK))                            return;
    delay(50);
  }
}

// ── MOSTRAR RESULTADOS ────────────────────────────────────────
void OledDisplay::_showAPResults() {
  if (apCount == 0) {
    showMessage("WiFi Scan", "Sin resultados", "OK=volver");
    while (!_btnPressed(BTN_OK)) delay(50);
    return;
  }

  int scroll = 0;
  while (true) {
    _display.clearDisplay();
    _drawHeader("WiFi Scan");

    for (int i = 0; i < 3; i++) {
      int idx = scroll + i;
      if (idx >= apCount) break;
      int y = 14 + (i * 17);

      String ssid = apResults[idx].ssid;
      if (ssid.length() == 0) ssid = "(oculto)";
      if (ssid.length() > 15) ssid = ssid.substring(0, 13) + "..";

      _display.setTextSize(1);
      _display.setTextColor(SSD1306_WHITE);
      _display.setCursor(0, y);
      _display.print(ssid);

      _display.setCursor(0, y + 8);
      _display.print(apResults[idx].rssi);
      _display.print("dBm ");
      _display.print(apResults[idx].enc);
    }

    _display.setCursor(100, 56);
    _display.print(scroll + 1);
    _display.print("/");
    _display.print(apCount);
    _display.display();

    if (_btnPressed(BTN_UP) && scroll > 0) scroll--;
    if (_btnPressed(BTN_DOWN) && scroll < apCount - 3) scroll++;
    if (_btnPressed(BTN_OK)) return;
    delay(50);
  }
}

// ── SHOW MESSAGE ──────────────────────────────────────────────
void OledDisplay::showMessage(const char* title,
                               const char* line1,
                               const char* line2,
                               const char* line3) {
  _display.clearDisplay();
  _drawHeader(title);
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);
  _display.setCursor(0, 15); _display.println(line1);
  _display.setCursor(0, 27); _display.println(line2);
  _display.setCursor(0, 39); _display.println(line3);
  _display.display();
}

void OledDisplay::showStatus(const char* msg) {
  _display.fillRect(0, 55, SCREEN_WIDTH, 9, SSD1306_BLACK);
  _display.setTextColor(SSD1306_WHITE);
  _display.setTextSize(1);
  _display.setCursor(0, 56);
  _display.print(msg);
  _display.display();
}

// ── BOOT ─────────────────────────────────────────────────────
void OledDisplay::showBoot() {
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);
  _display.setCursor(15, 4);  _display.println("ESP32 CYBERWAWALDO");
  _display.setCursor(40, 16); _display.println("v2.0.0");
  _display.setCursor(5, 28);  _display.println("WiFi | BT | RFID");
  _display.setCursor(5, 42);  _display.println("Hold OK = Volver");
  _display.display();
  delay(2000);
}

// ── TICK — llamar en loop() ──────────────────────────────────
void OledDisplay::tick() {
  // Leer botones
  if (_btnPressed(BTN_UP))   handleUp();
  if (_btnPressed(BTN_DOWN)) handleDown();

  // WAKE button: presionar 2s = shutdown directo
  if (digitalRead(WAKE_BTN) == LOW) {
    unsigned long t = millis();
    while (digitalRead(WAKE_BTN) == LOW) {
      if (millis() - t > 2000) {
        _display.ssd1306_command(SSD1306_DISPLAYOFF);
        delay(100);
        ESP.deepSleep(0);
      }
      delay(10);
    }
  }

  // OK: corto = seleccionar, hold = volver
  if (digitalRead(BTN_OK) == LOW) {
    unsigned long t = millis();
    delay(40);
    if (digitalRead(BTN_OK) == LOW) {
      while (digitalRead(BTN_OK) == LOW) {
        if (millis() - t > 800) {
          handleBack();
          _lastPress = millis();
          while (digitalRead(BTN_OK) == LOW) delay(10);
          return;
        }
      }
      if (millis() - _lastPress > DEBOUNCE_MS) {
        _lastPress = millis();
        handleOK();
      }
    }
  }
}