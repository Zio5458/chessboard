#include <Wire.h>

#define SLAVE_ID 3

const byte I2C_ADDRESS = 0x10 + SLAVE_ID;

const byte SQUARE_COUNT = 16;

const byte squarePins[SQUARE_COUNT] = {
  2, 3, 4, 5,
  6, 7, 8, 9,
  10, 11, 12, 13,
  A0, A1, A2, A3
};

const unsigned long DEBOUNCE_MS = 40;
const unsigned long REPORT_INTERVAL_MS = 1500;

volatile uint16_t stableMask = 0;

uint16_t candidateMask = 0;
uint16_t lastPrintedMask = 0;

unsigned long candidateSince = 0;
unsigned long lastReport = 0;

void setup() {
  Serial.begin(115200);
  delay(500);

  for (byte i = 0; i < SQUARE_COUNT; i++) {
    pinMode(squarePins[i], INPUT_PULLUP);
  }

  uint16_t initialMask = readSquaresMask();

  noInterrupts();
  stableMask = initialMask;
  interrupts();

  candidateMask = initialMask;
  lastPrintedMask = initialMask;
  candidateSince = millis();
  lastReport = millis();

  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(onI2CRequest);

  Serial.println();
  Serial.println("=================================");
  Serial.print("SLAVE_ID: ");
  Serial.println(SLAVE_ID);
  Serial.print("Direccion I2C: 0x");
  Serial.println(I2C_ADDRESS, HEX);
  Serial.println("=================================");
  Serial.println();

  printMask(initialMask);
}

void loop() {
  uint16_t rawMask = readSquaresMask();

  if (rawMask != candidateMask) {
    candidateMask = rawMask;
    candidateSince = millis();
  }

  if ((millis() - candidateSince) >= DEBOUNCE_MS) {
    uint16_t currentStable;

    noInterrupts();
    currentStable = stableMask;
    interrupts();

    if (candidateMask != currentStable) {
      noInterrupts();
      stableMask = candidateMask;
      interrupts();

      printChanges(currentStable, candidateMask);
      lastPrintedMask = candidateMask;
    }
  }

  if ((millis() - lastReport) >= REPORT_INTERVAL_MS) {
    lastReport = millis();

    uint16_t currentStable;

    noInterrupts();
    currentStable = stableMask;
    interrupts();

    Serial.println();
    Serial.println("---- REPORTE ARDUINO ----");
    printMask(currentStable);
  }

  delay(10);
}

uint16_t readSquaresMask() {
  uint16_t mask = 0;

  for (byte i = 0; i < SQUARE_COUNT; i++) {
    bool occupied = digitalRead(squarePins[i]) == LOW;

    if (occupied) {
      mask |= (1 << i);
    }
  }

  return mask;
}

void onI2CRequest() {
  uint16_t maskCopy = stableMask;

  byte response[3];
  response[0] = lowByte(maskCopy);
  response[1] = highByte(maskCopy);
  response[2] = SLAVE_ID;

  Wire.write(response, 3);
}

void printMask(uint16_t mask) {
  Serial.print("Mascara estable: 0b");

  for (int i = 15; i >= 0; i--) {
    Serial.print((mask & (1 << i)) ? "1" : "0");
  }

  Serial.println();

  Serial.print("Casillas activas locales: ");

  bool any = false;

  for (byte i = 0; i < SQUARE_COUNT; i++) {
    if (mask & (1 << i)) {
      Serial.print(i);
      Serial.print(" ");
      any = true;
    }
  }

  if (!any) {
    Serial.print("ninguna");
  }

  Serial.println();
}

void printChanges(uint16_t oldMask, uint16_t newMask) {
  Serial.println();
  Serial.println("---- CAMBIOS LOCALES ----");

  for (byte i = 0; i < SQUARE_COUNT; i++) {
    bool oldState = oldMask & (1 << i);
    bool newState = newMask & (1 << i);

    if (oldState != newState) {
      Serial.print("Local ");
      Serial.print(i);
      Serial.print(" -> ");
      Serial.println(newState ? "OCCUPIED" : "EMPTY");
    }
  }
}