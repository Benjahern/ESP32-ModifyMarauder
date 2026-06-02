// ============================================================
//  OledDisplay.cpp — Integración OLED SSD1306 con Marauder
//  Benjamin — ESP32 WROOM-32 Cyberdeck
// ============================================================

#include "OledDisplay.h"
#include <WiFi.h>

// ── MENÚS ────────────────────────────────────────────────────
const char* OledDisplay::_mainMenu[] = { "WiFi", "Bluetooth", "RFID", "Sistema" };
const int   OledDisplay::_mainMenuLen = 4;

const char* OledDisplay::_wifiMenu[] = {
  "Scan Redes", "Deauth General", "Deauth Dirigido",
  "Probe Sniff", "Beacon Spam", "Packet Monitor",
  "Evil Portal", "< Volver"
};
const int OledDisplay::_wifiMenuLen = 8;

const char* OledDisplay::_btMenu[] = {
  "BLE Scan", "AirTag Scan", "Tracker", "< Volver"
};
const int OledDisplay::_btMenuLen = 4;

const char* OledDisplay::_rfidMenu[] = {
  "Leer UID", "Leer Sector", "Guardar Tarjeta",
  "Ver Guardadas", "< Volver"
};
const int OledDisplay::_rfidMenuLen = 5;

const char* OledDisplay::_sysMenu[] = {
  "Info Sistema", "Stop Ataque", "Shutdown", "< Volver"
};
const int OledDisplay::_sysMenuLen = 4;

// ── CONSTRUCTOR ───────────────────────────────────────────────
OledDisplay::OledDisplay()
  : _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
    _rfid(RC522_SS, RC522_RST) {}

// ── INIT ─────────────────────────────────────────────────────
void OledDisplay::begin() {
  // Botones
  pinMode(BTN_UP,   INPUT);          // GPIO 34 — input only, pull-up externo 10kΩ
  pinMode(BTN_DOWN, INPUT);          // GPIO 35 — input only, pull-up externo 10kΩ
  pinMode(BTN_OK,   INPUT_PULLUP);   // GPIO 32 — pull-up interno
  pinMode(WAKE_BTN, INPUT_PULLUP);   // GPIO 16 — wake-up desde deep sleep

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
  showMainMenu();
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

void OledDisplay::_drawMenu(const char** items, int len, int selected, const char* title) {
  _display.clearDisplay();
  _drawHeader(title);

  int visibleStart = 0;
  if (selected >= 4) visibleStart = selected - 3;

  for (int i = 0; i < 4; i++) {
    int idx = visibleStart + i;
    if (idx >= len) break;
    int y = 14 + (i * 12);

    if (idx == selected) {
      _display.fillRect(0, y - 1, SCREEN_WIDTH, 12, SSD1306_WHITE);
      _display.setTextColor(SSD1306_BLACK);
    } else {
      _display.setTextColor(SSD1306_WHITE);
    }
    _display.setTextSize(1);
    _display.setCursor(4, y);
    _display.print(items[idx]);
  }

  _display.setTextColor(SSD1306_WHITE);
  _display.display();
}

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

void OledDisplay::showBoot() {
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);
  _display.setCursor(15, 4);  _display.println("ESP32 CYBERWAWALDO");
  _display.setCursor(40, 16); _display.println("v1.0.0");
  _display.setCursor(5, 28);  _display.println("WiFi | BT | RFID");
  _display.setCursor(5, 42);  _display.println("Hold OK = Volver");
  _display.display();
  delay(2000);
}

void OledDisplay::showMainMenu() {
  _drawMenu(_mainMenu, _mainMenuLen, _menuIndex, "CYBERWAWALDO");
}

void OledDisplay::showSubMenu() {
  switch (_parentIdx) {
    case 0: _drawMenu(_wifiMenu, _wifiMenuLen, _subIndex, "WiFi");      break;
    case 1: _drawMenu(_btMenu,   _btMenuLen,   _subIndex, "Bluetooth"); break;
    case 2: _drawMenu(_rfidMenu, _rfidMenuLen, _subIndex, "RFID");      break;
    case 3: _drawMenu(_sysMenu,  _sysMenuLen,  _subIndex, "Sistema");   break;
  }
}

void OledDisplay::showAPList(int scrollOffset) {
  _apScroll = scrollOffset;
  _showAPResults();
}

void OledDisplay::_showAPResults() {
  _display.clearDisplay();
  _drawHeader("WiFi Scan");

  if (apCount == 0) {
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 20);
    _display.println("Sin resultados");
    _display.display();
    return;
  }

  for (int i = 0; i < 3; i++) {
    int idx = _apScroll + i;
    if (idx >= apCount) break;
    int y = 14 + (i * 17);

    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);

    String ssid = apResults[idx].ssid;
    if (ssid.length() == 0) ssid = "(oculto)";
    if (ssid.length() > 15) ssid = ssid.substring(0, 13) + "..";

    _display.setCursor(0, y);
    _display.print(ssid);

    _display.setCursor(0, y + 8);
    _display.print(apResults[idx].rssi);
    _display.print("dBm ");
    _display.print(apResults[idx].enc);
  }

  // Indicador de scroll
  _display.setCursor(100, 56);
  _display.print(_apScroll + 1);
  _display.print("/");
  _display.print(apCount);
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
  if (_menuLevel == MENU_MAIN) {
    _menuIndex = (_menuIndex > 0) ? _menuIndex - 1 : _mainMenuLen - 1;
    showMainMenu();
  } else {
    int len = 0;
    switch (_parentIdx) {
      case 0: len = _wifiMenuLen; break;
      case 1: len = _btMenuLen;   break;
      case 2: len = _rfidMenuLen; break;
      case 3: len = _sysMenuLen;  break;
    }
    _subIndex = (_subIndex > 0) ? _subIndex - 1 : len - 1;
    showSubMenu();
  }
}

void OledDisplay::handleDown() {
  if (_menuLevel == MENU_MAIN) {
    _menuIndex = (_menuIndex < _mainMenuLen - 1) ? _menuIndex + 1 : 0;
    showMainMenu();
  } else {
    int len = 0;
    switch (_parentIdx) {
      case 0: len = _wifiMenuLen; break;
      case 1: len = _btMenuLen;   break;
      case 2: len = _rfidMenuLen; break;
      case 3: len = _sysMenuLen;  break;
    }
    _subIndex = (_subIndex < len - 1) ? _subIndex + 1 : 0;
    showSubMenu();
  }
}

void OledDisplay::handleBack() {
  if (_menuLevel != MENU_MAIN) {
    _menuLevel = MENU_MAIN;
    showMainMenu();
  }
}

void OledDisplay::handleOK() {
  if (_menuLevel == MENU_MAIN) {
    _parentIdx = _menuIndex;
    _subIndex  = 0;
    _menuLevel = MENU_SUB;
    showSubMenu();
    return;
  }
  _execAction();
}

// ── EJECUTAR ACCIÓN ──────────────────────────────────────────
void OledDisplay::_execAction() {
  // Detectar "< Volver" en cualquier submenú
  bool isBack = false;
  switch (_parentIdx) {
    case 0: isBack = (_subIndex == _wifiMenuLen - 1); break;
    case 1: isBack = (_subIndex == _btMenuLen   - 1); break;
    case 2: isBack = (_subIndex == _rfidMenuLen - 1); break;
    case 3: isBack = (_subIndex == _sysMenuLen  - 1); break;
  }
  if (isBack) { handleBack(); return; }

  switch (_parentIdx) {
    // ── WIFI ─────────────────────────────────────────────────
    case 0:
      switch (_subIndex) {
        case 0: // Scan Redes
          _actionRequested = true;
          _requestedAction = ACTION_WIFI_SCAN;
          break;
        case 1: // Deauth General
          _actionRequested = true;
          _requestedAction = ACTION_DEAUTH;
          break;
        case 2: // Deauth Dirigido
          _actionRequested = true;
          _requestedAction = ACTION_DEAUTH_TARGETED;
          break;
        case 3: // Probe Sniff
          _actionRequested = true;
          _requestedAction = ACTION_PROBE_SNIFF;
          break;
        case 4: // Beacon Spam
          _actionRequested = true;
          _requestedAction = ACTION_BEACON_SPAM;
          break;
        case 5: // Packet Monitor
          _actionRequested = true;
          _requestedAction = ACTION_PACKET_MONITOR;
          break;
        case 6: // Evil Portal
          showMessage("Evil Portal", "Usa serial:", "evilportal", "OK=volver");
          while (!_btnPressed(BTN_OK)) delay(50);
          showSubMenu();
          break;
      }
      break;

    // ── BLUETOOTH ────────────────────────────────────────────
    case 1:
      switch (_subIndex) {
        case 0: // BLE Scan
          _actionRequested = true;
          _requestedAction = ACTION_BLE_SCAN;
          break;
        case 1: // AirTag Scan
          _actionRequested = true;
          _requestedAction = ACTION_AIRTAG_SCAN;
          break;
        case 2: // Tracker
          _actionRequested = true;
          _requestedAction = ACTION_TRACKER;
          break;
      }
      break;

    // ── RFID ─────────────────────────────────────────────────
    case 2:
      switch (_subIndex) {
        case 0: doRFIDRead();   showSubMenu(); break;
        case 1: // Leer Sector
          showMessage("Sector Read", "Acerca tarjeta", "Mifare Classic");
          doRFIDRead(); // reutilizamos con sector
          showSubMenu();
          break;
        case 2: // Guardar
          showMessage("Guardar", "Acerca tarjeta", "para guardar");
          doRFIDRead();
          showSubMenu();
          break;
        case 3: doRFIDList(); showSubMenu(); break;
      }
      break;

    // ── SISTEMA ──────────────────────────────────────────────
    case 3:
      switch (_subIndex) {
        case 0: _doSysInfo(); showSubMenu(); break;
        case 1: // Stop
          _actionRequested = true;
          _requestedAction = ACTION_STOP;
          showMessage("Sistema", "Ataque detenido", "OK para volver");
          delay(1500);
          showSubMenu();
          break;
        case 2: // Shutdown
          _doShutdown();
          break;
      }
      break;
  }
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

  // Esperar confirmación con OK
  while (true) {
    if (_btnPressed(BTN_OK)) {
      // Apagar pantalla primero
      _display.ssd1306_command(SSD1306_DISPLAYOFF);
      delay(100);
      // Deep sleep - despierta con RESET o GPIO16
      ESP.deepSleep(0);
    }
    if (_btnPressed(BTN_DOWN)) {
      showSubMenu();
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

    // Mostrar en Serial también
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

// ── TICK — llamar en loop() ───────────────────────────────────
void OledDisplay::tick() {
  // Leer botones
  if (_btnPressed(BTN_UP))   handleUp();
  if (_btnPressed(BTN_DOWN)) handleDown();

  // WAKE button: presionar 2s = shutdown directo
  if (digitalRead(WAKE_BTN) == LOW) {
    unsigned long t = millis();
    while (digitalRead(WAKE_BTN) == LOW) {
      if (millis() - t > 2000) {
        // Apagar pantalla y deep sleep
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

  // Si hay resultado de WiFi scan, mostrar en pantalla
  if (_requestedAction == ACTION_WIFI_SCAN && !_actionRequested && apCount > 0) {
    _showAPResults();
  }
}
