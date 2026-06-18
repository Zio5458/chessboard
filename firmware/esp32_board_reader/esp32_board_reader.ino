#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <FastLED.h>

const char* DEVICE_NAME = "SmartChessBoard";

#define SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define RX_CHARACTERISTIC_UUID "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define TX_CHARACTERISTIC_UUID "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

// I2C
const int I2C_SDA_PIN = 8;
const int I2C_SCL_PIN = 9;

const byte ARDUINO_COUNT = 4;
const byte ARDUINO_ADDRESSES[ARDUINO_COUNT] = {
  0x10, 0x11, 0x12, 0x13
};

// LEDs WS2812B
#define LED_PIN 4
#define NUM_LEDS 64
#define LED_BRIGHTNESS 20

// Cambia esto según cómo conectes físicamente la cadena de LEDs.
// false: LED0=a1, LED1=b1, LED2=c1 ... LED7=h1, LED8=a2...
// true:  LED0=a1, LED1=b1 ... LED7=h1, LED8=h2, LED9=g2...
const bool LED_SERPENTINE = false;

// Si tu primer LED está en a8 en vez de a1, cambia esto a false.
const bool LED_ORIGIN_A1 = true;

CRGB leds[NUM_LEDS];

const unsigned long POLL_INTERVAL_MS = 60;
const unsigned long DEBOUNCE_MS = 80;
const unsigned long REPORT_INTERVAL_MS = 2000;
const unsigned long ILLEGAL_ALERT_MS = 2500;

BLECharacteristic* txCharacteristic = nullptr;
bool deviceConnected = false;

uint64_t rawBoardMask = 0;
uint64_t candidateBoardMask = 0;
uint64_t stableBoardMask = 0;
uint64_t previousStableBoardMask = 0;

bool pieceLifted = false;
String liftedFromSquare = "";

bool initialFlashDone = false;
bool redAlertActive = false;
unsigned long redAlertSince = 0;

unsigned long candidateSince = 0;
unsigned long lastPoll = 0;
unsigned long lastReport = 0;

const char* squareMap[ARDUINO_COUNT][16] = {
  {
    "a1", "b1", "c1", "d1",
    "a2", "b2", "c2", "d2",
    "a3", "b3", "c3", "d3",
    "a4", "b4", "c4", "d4"
  },
  {
    "e1", "f1", "g1", "h1",
    "e2", "f2", "g2", "h2",
    "e3", "f3", "g3", "h3",
    "e4", "f4", "g4", "h4"
  },
  {
    "a5", "b5", "c5", "d5",
    "a6", "b6", "c6", "d6",
    "a7", "b7", "c7", "d7",
    "a8", "b8", "c8", "d8"
  },
  {
    "e5", "f5", "g5", "h5",
    "e6", "f6", "g6", "h6",
    "e7", "f7", "g7", "h7",
    "e8", "f8", "g8", "h8"
  }
};

void setupI2C();
void setupBLE();
void setupLEDs();

bool readAllArduinos(uint64_t& fullMask);
bool readArduinoMask(byte address, uint16_t& localMask);

void sendBleMessage(String message);
void sendBoardSummary();

void processBoardChange(uint64_t oldMask, uint64_t newMask);
void printChangedSquares(uint64_t mask, bool occupied);

String offsetToSquare(int offset);
String firstSquareFromMask(uint64_t mask);
int countBits64(uint64_t value);
bool isSquareOccupied(uint64_t mask, const char* square);
void printBoard(uint64_t mask);
void clearLiftedPiece();

void clearAllLeds();
void flashAllGreenOnce();
void maybeFlashInitialOk();
bool isInitialPositionOccupancy(uint64_t mask);
int squareToLedIndex(const String& square);
void setSquareColor(const String& square, CRGB color);
String getToken(String text, int tokenIndex);
void showLegalMovesFromCommand(String command);
void showIllegalFromCommand(String command);
void showIllegalSquares(String fromSquare, String toSquare);

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* server) override {
    deviceConnected = true;
    Serial.println("BLE cliente conectado.");
  }

  void onDisconnect(BLEServer* server) override {
    deviceConnected = false;
    Serial.println("BLE cliente desconectado.");
    BLEDevice::startAdvertising();
  }
};

class RxCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    String value = characteristic->getValue();
    value.trim();

    if (value.length() == 0) {
      return;
    }

    Serial.print("RX BLE: ");
    Serial.println(value);

    if (value == "PING") {
      sendBleMessage("PONG");
      return;
    }

    if (value == "SEND_BOARD") {
      sendBoardSummary();
      return;
    }

    if (value == "RESET_BASELINE") {
      previousStableBoardMask = stableBoardMask;
      clearLiftedPiece();
      clearAllLeds();
      sendBleMessage("BASELINE_RESET");
      return;
    }

    if (value == "CLEAR_LEDS") {
      clearAllLeds();
      sendBleMessage("LED_OK CLEARED");
      return;
    }

    if (value == "MOVE_OK") {
      clearAllLeds();
      sendBleMessage("LED_OK MOVE_OK");
      return;
    }

    if (value == "INIT_FLASH") {
      flashAllGreenOnce();
      sendBleMessage("LED_OK INIT_FLASH");
      return;
    }

    if (value.startsWith("LEGAL_MOVES")) {
      showLegalMovesFromCommand(value);
      sendBleMessage("LED_OK LEGAL_MOVES");
      return;
    }

    if (value.startsWith("ILLEGAL")) {
      showIllegalFromCommand(value);
      sendBleMessage("LED_OK ILLEGAL");
      return;
    }

    sendBleMessage("ACK " + value);
  }
};

void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("=====================================");
  Serial.println("ESP32-S3 SmartChess I2C + BLE + LEDs");
  Serial.println("=====================================");

  setupI2C();
  setupLEDs();
  setupBLE();

  bool ok = readAllArduinos(rawBoardMask);

  if (ok) {
    candidateBoardMask = rawBoardMask;
    stableBoardMask = rawBoardMask;
    previousStableBoardMask = rawBoardMask;
  } else {
    Serial.println("Advertencia: no se pudieron leer todos los Arduinos al iniciar.");
  }

  candidateSince = millis();
  lastPoll = millis();
  lastReport = millis();

  printBoard(stableBoardMask);
  maybeFlashInitialOk();
}

void loop() {
  if ((millis() - lastPoll) >= POLL_INTERVAL_MS) {
    lastPoll = millis();

    uint64_t newRawMask = 0;
    bool ok = readAllArduinos(newRawMask);

    if (ok) {
      rawBoardMask = newRawMask;

      if (rawBoardMask != candidateBoardMask) {
        candidateBoardMask = rawBoardMask;
        candidateSince = millis();
      }

      if ((millis() - candidateSince) >= DEBOUNCE_MS &&
          candidateBoardMask != stableBoardMask) {
        previousStableBoardMask = stableBoardMask;
        stableBoardMask = candidateBoardMask;

        processBoardChange(previousStableBoardMask, stableBoardMask);
        printBoard(stableBoardMask);
        maybeFlashInitialOk();
      }
    }
  }

  if (redAlertActive && (millis() - redAlertSince) >= ILLEGAL_ALERT_MS) {
    redAlertActive = false;
    clearAllLeds();
  }

  if ((millis() - lastReport) >= REPORT_INTERVAL_MS) {
    lastReport = millis();

    Serial.println();
    Serial.println("---- REPORTE ESP32 ----");
    Serial.print("Piezas detectadas: ");
    Serial.println(countBits64(stableBoardMask));

    if (pieceLifted) {
      Serial.print("Pieza levantada desde: ");
      Serial.println(liftedFromSquare);
    }
  }

  delay(10);
}

// ===================== I2C =====================

void setupI2C() {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);

  Serial.println("I2C iniciado.");
  Serial.print("SDA: GPIO");
  Serial.println(I2C_SDA_PIN);
  Serial.print("SCL: GPIO");
  Serial.println(I2C_SCL_PIN);
}

bool readAllArduinos(uint64_t& fullMask) {
  fullMask = 0;
  bool allOk = true;

  for (byte slave = 0; slave < ARDUINO_COUNT; slave++) {
    uint16_t localMask = 0;

    bool ok = readArduinoMask(ARDUINO_ADDRESSES[slave], localMask);

    if (!ok) {
      Serial.print("No se pudo leer Arduino ");
      Serial.print(slave);
      Serial.print(" en direccion 0x");
      Serial.println(ARDUINO_ADDRESSES[slave], HEX);

      allOk = false;
      continue;
    }

    for (byte bit = 0; bit < 16; bit++) {
      if (localMask & (1 << bit)) {
        int globalOffset = slave * 16 + bit;
        fullMask |= (1ULL << globalOffset);
      }
    }
  }

  return allOk;
}

bool readArduinoMask(byte address, uint16_t& localMask) {
  localMask = 0;

  byte bytesRead = Wire.requestFrom(address, (byte)3);

  if (bytesRead < 2) {
    while (Wire.available()) {
      Wire.read();
    }

    return false;
  }

  byte low = Wire.read();
  byte high = Wire.read();

  if (Wire.available()) {
    byte slaveId = Wire.read();
    (void)slaveId;
  }

  while (Wire.available()) {
    Wire.read();
  }

  localMask = ((uint16_t)high << 8) | low;
  return true;
}

// ===================== BLE =====================

void setupBLE() {
  BLEDevice::init(DEVICE_NAME);

  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService* service = server->createService(SERVICE_UUID);

  txCharacteristic = service->createCharacteristic(
    TX_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );

  txCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic* rxCharacteristic = service->createCharacteristic(
    RX_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );

  rxCharacteristic->setCallbacks(new RxCallbacks());

  service->start();

  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);

  BLEDevice::startAdvertising();

  Serial.println("BLE iniciado.");
  Serial.println("Busque SmartChessBoard desde el frontend.");
}

void sendBleMessage(String message) {
  if (!message.endsWith("\n")) {
    message += "\n";
  }

  Serial.print("BLE TX: ");
  Serial.print(message);

  if (!deviceConnected || txCharacteristic == nullptr) {
    return;
  }

  txCharacteristic->setValue((uint8_t*)message.c_str(), message.length());
  txCharacteristic->notify();
}

// ===================== PROCESAMIENTO DEL TABLERO =====================

void processBoardChange(uint64_t oldMask, uint64_t newMask) {
  uint64_t removedMask = oldMask & ~newMask;
  uint64_t addedMask = newMask & ~oldMask;

  int removedCount = countBits64(removedMask);
  int addedCount = countBits64(addedMask);

  Serial.println();
  Serial.println("---- CAMBIOS DEL TABLERO ----");

  printChangedSquares(removedMask, false);
  printChangedSquares(addedMask, true);

  if (removedCount == 1 && addedCount == 1) {
    String fromSquare = firstSquareFromMask(removedMask);
    String toSquare = firstSquareFromMask(addedMask);

    clearLiftedPiece();

    if (fromSquare == toSquare) {
      clearAllLeds();
      sendBleMessage("MOVE_CANCELLED " + fromSquare);
      return;
    }

    String moveMessage = "MOVE ";
    moveMessage += fromSquare;
    moveMessage += " ";
    moveMessage += toSquare;
    moveMessage += " NORMAL";

    sendBleMessage(moveMessage);
    return;
  }

  if (removedCount == 1 && addedCount == 0) {
    String square = firstSquareFromMask(removedMask);

    pieceLifted = true;
    liftedFromSquare = square;

    String liftedMessage = "LIFTED ";
    liftedMessage += square;

    sendBleMessage(liftedMessage);
    return;
  }

  if (removedCount == 0 && addedCount == 1) {
    String toSquare = firstSquareFromMask(addedMask);

    if (pieceLifted) {
      String fromSquare = liftedFromSquare;

      clearLiftedPiece();

      if (fromSquare == toSquare) {
        clearAllLeds();
        sendBleMessage("MOVE_CANCELLED " + toSquare);
        return;
      }

      String moveMessage = "MOVE ";
      moveMessage += fromSquare;
      moveMessage += " ";
      moveMessage += toSquare;
      moveMessage += " NORMAL";

      sendBleMessage(moveMessage);
      return;
    }

    sendBleMessage("CHANGE " + toSquare + " OCCUPIED");
    return;
  }

  if (removedCount == 0 && addedCount == 0) {
    return;
  }

  clearLiftedPiece();

  String summary = "BOARD_CHANGED removed=";
  summary += String(removedCount);
  summary += " added=";
  summary += String(addedCount);

  sendBleMessage(summary);
}

void clearLiftedPiece() {
  pieceLifted = false;
  liftedFromSquare = "";
}

void printChangedSquares(uint64_t mask, bool occupied) {
  for (int offset = 0; offset < 64; offset++) {
    if (mask & (1ULL << offset)) {
      String square = offsetToSquare(offset);

      Serial.print("CHANGE ");
      Serial.print(square);
      Serial.print(" -> ");
      Serial.println(occupied ? "OCCUPIED" : "EMPTY");
    }
  }
}

// ===================== LEDS WS2812B =====================

void setupLEDs() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(LED_BRIGHTNESS);
  clearAllLeds();

  Serial.println("LEDs WS2812B iniciados.");
  Serial.print("LED_PIN: GPIO");
  Serial.println(LED_PIN);
  Serial.print("NUM_LEDS: ");
  Serial.println(NUM_LEDS);
  Serial.print("BRILLO: ");
  Serial.println(LED_BRIGHTNESS);
}

void clearAllLeds() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  redAlertActive = false;
}

void flashAllGreenOnce() {
  Serial.println("Parpadeo inicial verde.");

  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(300);

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void maybeFlashInitialOk() {
  if (initialFlashDone) {
    return;
  }

  if (isInitialPositionOccupancy(stableBoardMask)) {
    initialFlashDone = true;
    flashAllGreenOnce();
    sendBleMessage("INIT_OK");
  }
}

bool isInitialPositionOccupancy(uint64_t mask) {
  if (countBits64(mask) != 32) {
    return false;
  }

  for (char file = 'a'; file <= 'h'; file++) {
    char square[3];
    square[0] = file;
    square[2] = '\0';

    square[1] = '1';
    if (!isSquareOccupied(mask, square)) return false;

    square[1] = '2';
    if (!isSquareOccupied(mask, square)) return false;

    square[1] = '7';
    if (!isSquareOccupied(mask, square)) return false;

    square[1] = '8';
    if (!isSquareOccupied(mask, square)) return false;
  }

  return true;
}

int squareToLedIndex(const String& square) {
  if (square.length() < 2) {
    return -1;
  }

  char fileChar = square.charAt(0);
  char rankChar = square.charAt(1);

  if (fileChar < 'a' || fileChar > 'h') {
    return -1;
  }

  if (rankChar < '1' || rankChar > '8') {
    return -1;
  }

  int file = fileChar - 'a';
  int rank = rankChar - '1';

  if (!LED_ORIGIN_A1) {
    rank = 7 - rank;
  }

  int ledFile = file;

  if (LED_SERPENTINE && (rank % 2 == 1)) {
    ledFile = 7 - file;
  }

  int index = rank * 8 + ledFile;

  if (index < 0 || index >= NUM_LEDS) {
    return -1;
  }

  return index;
}

void setSquareColor(const String& square, CRGB color) {
  int index = squareToLedIndex(square);

  if (index < 0) {
    Serial.print("LED invalido para casilla: ");
    Serial.println(square);
    return;
  }

  leds[index] = color;
}

void showLegalMovesFromCommand(String command) {
  command.trim();

  clearAllLeds();
  redAlertActive = false;

  Serial.println("Mostrando movimientos legales:");

  int tokenIndex = 1;
  bool any = false;

  while (true) {
    String square = getToken(command, tokenIndex);

    if (square.length() == 0) {
      break;
    }

    Serial.print("Legal: ");
    Serial.println(square);

    setSquareColor(square, CRGB::Green);
    any = true;
    tokenIndex++;
  }

  FastLED.show();

  if (!any) {
    Serial.println("No se recibieron casillas legales.");
  }
}

void showIllegalFromCommand(String command) {
  String fromSquare = getToken(command, 1);
  String toSquare = getToken(command, 2);

  if (fromSquare.length() == 0 || toSquare.length() == 0) {
    Serial.println("Comando ILLEGAL invalido.");
    return;
  }

  showIllegalSquares(fromSquare, toSquare);
}

void showIllegalSquares(String fromSquare, String toSquare) {
  Serial.print("Movimiento ilegal: ");
  Serial.print(fromSquare);
  Serial.print(" -> ");
  Serial.println(toSquare);

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  setSquareColor(fromSquare, CRGB::Red);
  setSquareColor(toSquare, CRGB::Red);

  FastLED.show();

  redAlertActive = true;
  redAlertSince = millis();
}

String getToken(String text, int tokenIndex) {
  text.trim();

  int currentToken = 0;
  int start = 0;

  while (start < text.length()) {
    while (start < text.length() && text.charAt(start) == ' ') {
      start++;
    }

    if (start >= text.length()) {
      break;
    }

    int end = text.indexOf(' ', start);

    if (end == -1) {
      end = text.length();
    }

    if (currentToken == tokenIndex) {
      return text.substring(start, end);
    }

    currentToken++;
    start = end + 1;
  }

  return "";
}

// ===================== TABLERO Y CONVERSIONES =====================

String offsetToSquare(int offset) {
  int slave = offset / 16;
  int localBit = offset % 16;

  if (slave < 0 || slave >= ARDUINO_COUNT) {
    return "??";
  }

  return String(squareMap[slave][localBit]);
}

String firstSquareFromMask(uint64_t mask) {
  for (int offset = 0; offset < 64; offset++) {
    if (mask & (1ULL << offset)) {
      return offsetToSquare(offset);
    }
  }

  return "??";
}

int countBits64(uint64_t value) {
  int count = 0;

  while (value != 0) {
    count += value & 1ULL;
    value >>= 1;
  }

  return count;
}

bool isSquareOccupied(uint64_t mask, const char* square) {
  for (int slave = 0; slave < ARDUINO_COUNT; slave++) {
    for (int bit = 0; bit < 16; bit++) {
      if (String(squareMap[slave][bit]) == String(square)) {
        int offset = slave * 16 + bit;
        return mask & (1ULL << offset);
      }
    }
  }

  return false;
}

void printBoard(uint64_t mask) {
  Serial.println();
  Serial.println("Estado actual del tablero:");
  Serial.print("Piezas detectadas: ");
  Serial.println(countBits64(mask));

  for (int rank = 8; rank >= 1; rank--) {
    Serial.print(rank);
    Serial.print("  ");

    for (char file = 'a'; file <= 'h'; file++) {
      char square[3];
      square[0] = file;
      square[1] = char('0' + rank);
      square[2] = '\0';

      Serial.print(isSquareOccupied(mask, square) ? "[X]" : "[ ]");
    }

    Serial.println();
  }

  Serial.println("    a  b  c  d  e  f  g  h");
}

void sendBoardSummary() {
  String message = "BOARD_COUNT ";
  message += String(countBits64(stableBoardMask));

  sendBleMessage(message);
}