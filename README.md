# ESP32 modificaciones del marauder
La idea del proyecto era tener un marauder propio y modficado, le añadi cosas propias como un display propio, y funciones extras como el RFID, para clonar o conseguir tarjetas nfc

La idea salio del flipper zero, queria algo del estilo pero enfocado en ciberseguridad.

## Menu
El menu que cree es simple con 4 botones en OledDisplay, si tiene una pantalla del estilo le servira, para este codigo se utilizo una de 128x64 pixeles
El menu funciona en base a 4 botones, arriba, abajo, aceptar, y el boton de encendido y apagado. 

## Funcionalidades
### Wifi
  **Escaneo**:
 Son modos que escuchan sin transmitir:

  Función: Probe Sniff
  Qué hace realmente: Captura requests que tu dispositivo envía buscando redes conocidas
  Por qué sirve: Identifica qué redes busca un dispositivo (敏感信息)
  ────────────────────────────────────────
  Función: Beacon Sniff
  Qué hace realmente: Captura beacons que los APs emiten constantemente
  Por qué sirve: Descubrir redes, SSIDs ocultos, canales
  ────────────────────────────────────────
  Función: Deauth Sniff
  Qué hace realmente: Detecta paquetes de deautenticación en el aire
  Por qué sirve: Ver si alguien está haciendo deauth attacks
  ────────────────────────────────────────
  Función: Packet Monitor
  Qué hace realmente: Captura todo el tráfico 802.11 raw
  Por qué sirve: Análisis profundo de red
  ────────────────────────────────────────
  Función: Channel Analyzer
  Qué hace realmente: Muestra ocupación por canal
  Por qué sirve: Encontrar canales menos congestionados
  ────────────────────────────────────────
  Función: PWN Scan
  Qué hace realmente: Detecta dispositivos en modo Pwnagotchi
  Por qué sirve: -
  ────────────────────────────────────────
  Función: PineScan
  Qué hace realmente: Detecta WiFi Pineapple devices
  Por qué sirve: Seguridad contra Evil Twin
  ────────────────────────────────────────
  Función: MultiSSID
  Qué hace realmente: Detecta APs con múltiples SSIDs (típico de Pineapple)
  Por qué sirve: -
  ────────────────────────────────────────
  Función: Raw Capture
  Qué hace realmente: Captura frames 802.11 sin procesar
  Por qué sirve: Para análisis con Wireshark
  ────────────────────────────────────────
  Función: SAE Commit
  Qué hace realmente: Sniff de handshake SAE (WPA3)
  Por qué sirve: -

  **Ataques**:
 ┌───────────────┬─────────────────────────────────────────────────┬──────────────────────────────────────┐
  │    Función    │               Qué hace realmente                │            Por qué sirve             │
  ├───────────────┼─────────────────────────────────────────────────┼──────────────────────────────────────┤
  │ Beacon Spam   │ Envía beacons falsos con SSIDs aleatorios       │ Confundir wardrivers, jamming visual │
  ├───────────────┼─────────────────────────────────────────────────┼──────────────────────────────────────┤
  │ Beacon List   │ Envía beacons desde una lista de SSIDs          │ Crear muchos APs falsos              │
  ├───────────────┼─────────────────────────────────────────────────┼──────────────────────────────────────┤
  │ Rick Roll     │ Spam de beacons con "Never Gonna Give You Up"   │ Lmao                                 │
  ├───────────────┼─────────────────────────────────────────────────┼──────────────────────────────────────┤
  │ Funny Beacon  │ Nombres graciosos en beacons                    │ Entretenimiento                      │
  ├───────────────┼─────────────────────────────────────────────────┼──────────────────────────────────────┤
  │ Deauth        │ Envía paquetes de deauth a todos (broadcast)    │ Desconectar clientes                 │
  ├───────────────┼─────────────────────────────────────────────────┼──────────────────────────────────────┤
  │ Deauth Target │ Deauth dirigido a un cliente específico         │ Desconectar objetivo único           │
  ├───────────────┼─────────────────────────────────────────────────┼──────────────────────────────────────┤
  │ AP Spam       │ Spam de APs con mismo SSID                      │ Confundir clientes                   │
  ├───────────────┼─────────────────────────────────────────────────┼──────────────────────────────────────┤
  │ Auth Attack   │ Envía frames de autenticación falsos            │ DoS por saturación                   │
  ├───────────────┼─────────────────────────────────────────────────┼──────────────────────────────────────┤
  │ Karma         │ Rogue AP que responde a cualquier probe request │ Evil Twin automático                 │
  ├───────────────┼─────────────────────────────────────────────────┼──────────────────────────────────────┤
  │ Bad Msg       │ Envía EAPOL malformed para confused el cliente  │ -                                    │
  ├───────────────┼─────────────────────────────────────────────────┼──────────────────────────────────────┤
  │ Assoc Sleep   │ Flood de association requests con slow bit      │ DoS                                  │
  ├───────────────┼─────────────────────────────────────────────────┼──────────────────────────────────────┤
  │ SAE Flood     │ Flood de SAE commits (WPA3)                     │ -                                    │
  ├───────────────┼─────────────────────────────────────────────────┼──────────────────────────────────────┤
  │ CSA           │ Channel Switch Announcement falso               │ Forzar cambio de canal               │
  ├───────────────┼─────────────────────────────────────────────────┼──────────────────────────────────────┤
  │ Quiet         │ Cesa de enviar beacons                          │ Hacer AP invisible                   │
  └───────────────┴─────────────────────────────────────────────────┴──────────────────────────────────────┘


### Bluetooth/BLE
  **Escaneo**:
   ┌────────────────┬───────────────────────────────────────────┬────────────────────────────────┐
  │    Función     │            Qué hace realmente             │         Por qué sirve          │
  ├────────────────┼───────────────────────────────────────────┼────────────────────────────────┤
  │ BLE Scan       │ Escanea dispositivos Bluetooth Low Energy │ Descubrir dispositivos         │
  ├────────────────┼───────────────────────────────────────────┼────────────────────────────────┤
  │ AirTag Scan    │ Detecta Apple AirTags cercanos            │ Encontrar trackers no deseados │
  ├────────────────┼───────────────────────────────────────────┼────────────────────────────────┤
  │ AirTag Monitor │ Monitor continuo de AirTags               │ Rastreo de movimiento          │
  ├────────────────┼───────────────────────────────────────────┼────────────────────────────────┤
  │ Flipper Sniff  │ Detecta Flipper Zero transmitiendo        │ -                              │
  ├────────────────┼───────────────────────────────────────────┼────────────────────────────────┤
  │ Skimmers       │ Detecta skimmers de tarjeta CC Bluetooth  │ Anti-fraude                    │
  ├────────────────┼───────────────────────────────────────────┼────────────────────────────────┤
  │ Flock          │ Detecta cámaras Flock                     │ -                              │
  ├────────────────┼───────────────────────────────────────────┼────────────────────────────────┤
  │ Meta Detect    │ Detecta Meta Ray-Ban smart glasses        │ -                              │
  ├────────────────┼───────────────────────────────────────────┼────────────────────────────────┤
  │ Analyzer       │ Analizador de actividad Bluetooth         │ -                              │
  └────────────────┴───────────────────────────────────────────┴────────────────────────────────┘
  **Ataques**:
  ┌────────────────┬────────────────────────────────────────────────────────┬──────────────────────┐
  │    Función     │                   Qué hace realmente                   │    Por qué sirve     │
  ├────────────────┼────────────────────────────────────────────────────────┼──────────────────────┤
  ├────────────────┼────────────────────────────────────────────────────────┼──────────────────────┤
  │ Swiftpair Spam │ Spam de notificaciones Windows Swift Pair              │ Molestar/denial      │
  ├────────────────┼────────────────────────────────────────────────────────┼──────────────────────┤
  │ Samsung Spam   │ Spam de conexiones Samsung                             │ -                    │
  ├────────────────┼────────────────────────────────────────────────────────┼──────────────────────┤
  │ Google Spam    │ Spam Google Fast Pair                                  │ -                    │
  ├────────────────┼────────────────────────────────────────────────────────┼──────────────────────┤
  │ Flipper Spam   │ Spam tipo Flipper Zero                                 │ -                    │
  ├────────────────┼────────────────────────────────────────────────────────┼──────────────────────┤
  │ Spam All       │ Ejecuta todos los ataques BLE de una vez               │ -                    │
  └────────────────┴────────────────────────────────────────────────────────┴──────────────────────┘

### RFID

  ┌─────────────────┬──────────────────────────────────────────────┬───────────────────────┐
  │     Función     │                   Qué hace                   │     Por qué sirve     │
  ├─────────────────┼──────────────────────────────────────────────┼───────────────────────┤
  │ Leer UID        │ Lee el identificador único de tarjeta Mifare │ Acceso/identificación │
  ├─────────────────┼──────────────────────────────────────────────┼───────────────────────┤
  │ Leer Sector     │ Lee sectores de memoria Mifare Classic       │ -                     │
  ├─────────────────┼──────────────────────────────────────────────┼───────────────────────┤
  │ Guardar Tarjeta │ Guarda hasta 10 UIDs en memoria              │ Acceso rápido         │
  ├─────────────────┼──────────────────────────────────────────────┼───────────────────────┤
  │ Ver Guardadas   │ Lista tarjetas guardadas                     │ -                     │
  └─────────────────┴──────────────────────────────────────────────┴───────────────────────┘
