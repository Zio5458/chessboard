#include "BoardPins.h"

const bool USE_INTERNAL_PULLUPS = true;

const int FILE_ACTIVE_LEVEL = LOW;
const int FILE_IDLE_LEVEL = HIGH;
const int OCCUPIED_READ_LEVEL = LOW;

const unsigned long DEBOUNCE_MS = 40;
const unsigned long INIT_REPORT_MS = 3000;
const unsigned long INIT_STABLE_MS = 1000;
const unsigned long MOVE_WINDOW_MS = 1800;

const int SCAN_SETTLE_US = 120;

// ==================================================
// ESTRUCTURAS
// ==================================================

struct Square {
  int file;
  int rank;
  bool valid;
};

struct ChangeEvent {
  Square square;
  bool occupied;
};

enum ReaderState {
  INIT_BOARD,
  GAME_RUNNING
};

ReaderState readerState = INIT_BOARD;

// ==================================================
// ESTADO DEL TABLERO
// ==================================================

bool stableBoard[FILE_COUNT][RANK_COUNT];
bool previousBoard[FILE_COUNT][RANK_COUNT];
bool candidateBoard[FILE_COUNT][RANK_COUNT];
bool rawBoard[FILE_COUNT][RANK_COUNT];

unsigned long candidateSince = 0;
unsigned long lastInitReport = 0;
unsigned long initialValidSince = 0;
bool initialCandidate = false;

const int MAX_BUFFERED_CHANGES = 12;
ChangeEvent moveBuffer[MAX_BUFFERED_CHANGES];
int moveBufferCount = 0;
unsigned long moveBufferStart = 0;

// ==================================================
// PROTOTIPOS
// ==================================================

void setupPins();
void scanBoard(bool dest[FILE_COUNT][RANK_COUNT]);

void copyBoard(bool src[FILE_COUNT][RANK_COUNT], bool dest[FILE_COUNT][RANK_COUNT]);
bool boardsEqual(bool a[FILE_COUNT][RANK_COUNT], bool b[FILE_COUNT][RANK_COUNT]);

void handleStableBoardUpdate();
void handleInitialization();
void startGameMode();

int countOccupied(bool board[FILE_COUNT][RANK_COUNT]);
bool isExpectedInitialSquare(int file, int rank);
bool isInitialPositionCorrect(bool board[FILE_COUNT][RANK_COUNT]);

void printBoard(bool board[FILE_COUNT][RANK_COUNT]);
void printSquare(Square sq);
String squareToString(Square sq);
bool sameSquare(Square a, Square b);

void processGameChanges();
void addBufferedChange(Square sq, bool occupied);
void tryResolveBufferedMove();
void flushUnresolvedMove();

bool hasVacancy(int file, int rank);
bool hasOccupation(int file, int rank);
void emitMove(Square from, Square to, const char* moveKind);
void clearMoveBuffer();

// ==================================================
// SETUP / LOOP
// ==================================================

void setup() {
  Serial.begin(115200);
  delay(500);

  setupPins();

  scanBoard(stableBoard);
  copyBoard(stableBoard, previousBoard);
  copyBoard(stableBoard, candidateBoard);
  candidateSince = millis();

  Serial.println();
  Serial.println("======================================");
  Serial.println("ESP32-S3 SMART CHESSBOARD READER");
  Serial.println("Modo actual: INICIALIZACION");
  Serial.println("Coloque las 32 piezas en posicion inicial.");
  Serial.println("El lector imprimira el tablero y errores.");
  Serial.println("======================================");
  Serial.println();

  printBoard(stableBoard);
  lastInitReport = millis();
}

void loop() {
  scanBoard(rawBoard);

  if (!boardsEqual(rawBoard, candidateBoard)) {
    copyBoard(rawBoard, candidateBoard);
    candidateSince = millis();
  }

  if ((millis() - candidateSince) >= DEBOUNCE_MS &&
      !boardsEqual(candidateBoard, stableBoard)) {
    copyBoard(candidateBoard, stableBoard);
    handleStableBoardUpdate();
  }

  if (readerState == INIT_BOARD) {
    handleInitialization();
  }

  if (readerState == GAME_RUNNING &&
      moveBufferCount > 0 &&
      (millis() - moveBufferStart) > MOVE_WINDOW_MS) {
    flushUnresolvedMove();
  }

  delay(20);
}

// ==================================================
// PINES Y ESCANEO
// ==================================================

void setupPins() {
  for (int f = 0; f < FILE_COUNT; f++) {
    pinMode(filePins[f], OUTPUT);
    digitalWrite(filePins[f], FILE_IDLE_LEVEL);
  }

  for (int r = 0; r < RANK_COUNT; r++) {
    if (USE_INTERNAL_PULLUPS) {
      pinMode(rankPins[r], INPUT_PULLUP);
    } else {
      pinMode(rankPins[r], INPUT);
    }
  }
}

void scanBoard(bool dest[FILE_COUNT][RANK_COUNT]) {
  for (int f = 0; f < FILE_COUNT; f++) {
    digitalWrite(filePins[f], FILE_IDLE_LEVEL);
  }

  for (int f = 0; f < FILE_COUNT; f++) {
    digitalWrite(filePins[f], FILE_ACTIVE_LEVEL);
    delayMicroseconds(SCAN_SETTLE_US);

    for (int r = 0; r < RANK_COUNT; r++) {
      int reading = digitalRead(rankPins[r]);
      dest[f][r] = (reading == OCCUPIED_READ_LEVEL);
    }

    digitalWrite(filePins[f], FILE_IDLE_LEVEL);
  }
}

// ==================================================
// MANEJO DE ESTADOS
// ==================================================

void handleStableBoardUpdate() {
  if (readerState == INIT_BOARD) {
    Serial.println();
    Serial.println("[INIT] Cambio estable detectado.");
    printBoard(stableBoard);
  } else {
    processGameChanges();
  }
}

void handleInitialization() {
  if ((millis() - lastInitReport) >= INIT_REPORT_MS) {
    lastInitReport = millis();

    int occupied = countOccupied(stableBoard);

    Serial.println();
    Serial.println("---- MODO INICIALIZACION ----");
    Serial.print("Piezas detectadas: ");
    Serial.print(occupied);
    Serial.println(" / 32");

    printBoard(stableBoard);

    if (occupied < 32) {
      Serial.print("Faltan ");
      Serial.print(32 - occupied);
      Serial.println(" pieza(s).");
    } else if (occupied > 32) {
      Serial.print("Hay ");
      Serial.print(occupied - 32);
      Serial.println(" pieza(s) extra.");
    }

    if (occupied == 32 && !isInitialPositionCorrect(stableBoard)) {
      Serial.println("Hay 32 piezas, pero la posicion inicial no es correcta.");

      Serial.print("Faltan piezas en: ");
      bool firstMissing = true;

      for (int f = 0; f < FILE_COUNT; f++) {
        for (int r = 0; r < RANK_COUNT; r++) {
          if (isExpectedInitialSquare(f, r) && !stableBoard[f][r]) {
            if (!firstMissing) Serial.print(", ");
            printSquare({f, r, true});
            firstMissing = false;
          }
        }
      }

      if (firstMissing) Serial.print("ninguna");
      Serial.println();

      Serial.print("Piezas extra en: ");
      bool firstExtra = true;

      for (int f = 0; f < FILE_COUNT; f++) {
        for (int r = 0; r < RANK_COUNT; r++) {
          if (!isExpectedInitialSquare(f, r) && stableBoard[f][r]) {
            if (!firstExtra) Serial.print(", ");
            printSquare({f, r, true});
            firstExtra = false;
          }
        }
      }

      if (firstExtra) Serial.print("ninguna");
      Serial.println();
    }

    Serial.println("-----------------------------");
  }

  if (isInitialPositionCorrect(stableBoard)) {
    if (!initialCandidate) {
      initialCandidate = true;
      initialValidSince = millis();

      Serial.println();
      Serial.println("Posicion inicial correcta detectada.");
      Serial.println("Mantenga el tablero estable por 1 segundo...");
    }

    if ((millis() - initialValidSince) >= INIT_STABLE_MS) {
      startGameMode();
    }
  } else {
    initialCandidate = false;
  }
}

void startGameMode() {
  readerState = GAME_RUNNING;
  copyBoard(stableBoard, previousBoard);
  clearMoveBuffer();

  Serial.println();
  Serial.println("======================================");
  Serial.println("PARTIDA INICIADA");
  Serial.println("Salida serial de movimientos:");
  Serial.println("MOVE e2 e4");
  Serial.println("======================================");
  Serial.println();

  printBoard(stableBoard);
}

// ==================================================
// LOGICA DE INICIALIZACION
// ==================================================

int countOccupied(bool board[FILE_COUNT][RANK_COUNT]) {
  int count = 0;

  for (int f = 0; f < FILE_COUNT; f++) {
    for (int r = 0; r < RANK_COUNT; r++) {
      if (board[f][r]) {
        count++;
      }
    }
  }

  return count;
}

bool isExpectedInitialSquare(int file, int rank) {
  // rank index 0 = rank 1
  // rank index 1 = rank 2
  // rank index 6 = rank 7
  // rank index 7 = rank 8
  return rank == 0 || rank == 1 || rank == 6 || rank == 7;
}

bool isInitialPositionCorrect(bool board[FILE_COUNT][RANK_COUNT]) {
  for (int f = 0; f < FILE_COUNT; f++) {
    for (int r = 0; r < RANK_COUNT; r++) {
      bool expected = isExpectedInitialSquare(f, r);

      if (board[f][r] != expected) {
        return false;
      }
    }
  }

  return true;
}

// ==================================================
// DETECCION DE MOVIMIENTOS
// ==================================================

void processGameChanges() {
  for (int f = 0; f < FILE_COUNT; f++) {
    for (int r = 0; r < RANK_COUNT; r++) {
      if (stableBoard[f][r] != previousBoard[f][r]) {
        Square sq = {f, r, true};
        bool occupied = stableBoard[f][r];

        Serial.print("CHANGE ");
        printSquare(sq);
        Serial.print(" ");
        Serial.println(occupied ? "OCCUPIED" : "EMPTY");

        addBufferedChange(sq, occupied);
      }
    }
  }

  copyBoard(stableBoard, previousBoard);
  tryResolveBufferedMove();
}

void addBufferedChange(Square sq, bool occupied) {
  if (moveBufferCount == 0) {
    moveBufferStart = millis();
  }

  if (moveBufferCount >= MAX_BUFFERED_CHANGES) {
    Serial.println("WARN Buffer de movimiento lleno. Reiniciando buffer.");
    clearMoveBuffer();
    moveBufferStart = millis();
  }

  moveBuffer[moveBufferCount].square = sq;
  moveBuffer[moveBufferCount].occupied = occupied;
  moveBufferCount++;
}

void tryResolveBufferedMove() {
  if (moveBufferCount < 2) {
    return;
  }

  // Enroque blanco corto: e1 y h1 se vacian; g1 y f1 se ocupan.
  if (hasVacancy(4, 0) && hasVacancy(7, 0) &&
      hasOccupation(6, 0) && hasOccupation(5, 0)) {
    emitMove({4, 0, true}, {6, 0, true}, "CASTLING_KINGSIDE");
    return;
  }

  // Enroque blanco largo: e1 y a1 se vacian; c1 y d1 se ocupan.
  if (hasVacancy(4, 0) && hasVacancy(0, 0) &&
      hasOccupation(2, 0) && hasOccupation(3, 0)) {
    emitMove({4, 0, true}, {2, 0, true}, "CASTLING_QUEENSIDE");
    return;
  }

  // Enroque negro corto: e8 y h8 se vacian; g8 y f8 se ocupan.
  if (hasVacancy(4, 7) && hasVacancy(7, 7) &&
      hasOccupation(6, 7) && hasOccupation(5, 7)) {
    emitMove({4, 7, true}, {6, 7, true}, "CASTLING_KINGSIDE");
    return;
  }

  // Enroque negro largo: e8 y a8 se vacian; c8 y d8 se ocupan.
  if (hasVacancy(4, 7) && hasVacancy(0, 7) &&
      hasOccupation(2, 7) && hasOccupation(3, 7)) {
    emitMove({4, 7, true}, {2, 7, true}, "CASTLING_QUEENSIDE");
    return;
  }

  Square vacantSquares[4];
  Square occupiedSquares[4];
  int vacantCount = 0;
  int occupiedCount = 0;

  for (int i = 0; i < moveBufferCount; i++) {
    if (moveBuffer[i].occupied) {
      if (occupiedCount < 4) {
        occupiedSquares[occupiedCount++] = moveBuffer[i].square;
      }
    } else {
      if (vacantCount < 4) {
        vacantSquares[vacantCount++] = moveBuffer[i].square;
      }
    }
  }

  // Movimiento normal:
  // una casilla se vacia y otra se ocupa.
  if (vacantCount == 1 && occupiedCount == 1) {
    emitMove(vacantSquares[0], occupiedSquares[0], "NORMAL");
    return;
  }

  // Captura normal:
  // destino se vacia, origen se vacia, destino se ocupa.
  // Buscamos una casilla que aparezca como vacia y luego ocupada.
  if (vacantCount == 2 && occupiedCount == 1) {
    Square to = occupiedSquares[0];
    int sameAsDestination = -1;
    int originCandidate = -1;

    for (int i = 0; i < vacantCount; i++) {
      if (sameSquare(vacantSquares[i], to)) {
        sameAsDestination = i;
      } else {
        originCandidate = i;
      }
    }

    if (sameAsDestination != -1 && originCandidate != -1) {
      emitMove(vacantSquares[originCandidate], to, "CAPTURE");
      return;
    }

    // Caso especial o ambiguo:
    // Puede ser en passant u otra secuencia fisica.
    Serial.println("AMBIGUOUS_MOVE");
    Serial.print("Destino ocupado: ");
    printSquare(to);
    Serial.println();

    Serial.println("Candidatos posibles:");
    for (int i = 0; i < vacantCount; i++) {
      Serial.print("MOVE_CANDIDATE ");
      printSquare(vacantSquares[i]);
      Serial.print(" ");
      printSquare(to);
      Serial.print(" REMOVED ");

      for (int j = 0; j < vacantCount; j++) {
        if (j != i) {
          printSquare(vacantSquares[j]);
        }
      }

      Serial.println();
    }

    clearMoveBuffer();
    return;
  }
}

void emitMove(Square from, Square to, const char* moveKind) {
  Serial.print("MOVE ");
  printSquare(from);
  Serial.print(" ");
  printSquare(to);
  Serial.print(" ");
  Serial.println(moveKind);

  clearMoveBuffer();
}

void flushUnresolvedMove() {
  Serial.println();
  Serial.println("UNRESOLVED_CHANGES");

  for (int i = 0; i < moveBufferCount; i++) {
    Serial.print(i);
    Serial.print(": ");
    printSquare(moveBuffer[i].square);
    Serial.print(" -> ");
    Serial.println(moveBuffer[i].occupied ? "OCCUPIED" : "EMPTY");
  }

  Serial.println("Fin de cambios no resueltos.");
  Serial.println();

  clearMoveBuffer();
}

void clearMoveBuffer() {
  moveBufferCount = 0;
  moveBufferStart = 0;
}

bool hasVacancy(int file, int rank) {
  for (int i = 0; i < moveBufferCount; i++) {
    if (!moveBuffer[i].occupied &&
        moveBuffer[i].square.file == file &&
        moveBuffer[i].square.rank == rank) {
      return true;
    }
  }

  return false;
}

bool hasOccupation(int file, int rank) {
  for (int i = 0; i < moveBufferCount; i++) {
    if (moveBuffer[i].occupied &&
        moveBuffer[i].square.file == file &&
        moveBuffer[i].square.rank == rank) {
      return true;
    }
  }

  return false;
}

// ==================================================
// UTILIDADES
// ==================================================

void copyBoard(bool src[FILE_COUNT][RANK_COUNT], bool dest[FILE_COUNT][RANK_COUNT]) {
  for (int f = 0; f < FILE_COUNT; f++) {
    for (int r = 0; r < RANK_COUNT; r++) {
      dest[f][r] = src[f][r];
    }
  }
}

bool boardsEqual(bool a[FILE_COUNT][RANK_COUNT], bool b[FILE_COUNT][RANK_COUNT]) {
  for (int f = 0; f < FILE_COUNT; f++) {
    for (int r = 0; r < RANK_COUNT; r++) {
      if (a[f][r] != b[f][r]) {
        return false;
      }
    }
  }

  return true;
}

bool sameSquare(Square a, Square b) {
  return a.valid && b.valid &&
         a.file == b.file &&
         a.rank == b.rank;
}

String squareToString(Square sq) {
  if (!sq.valid) {
    return "--";
  }

  String value = "";
  value += fileNames[sq.file];
  value += rankNames[sq.rank];

  return value;
}

void printSquare(Square sq) {
  Serial.print(squareToString(sq));
}

void printBoard(bool board[FILE_COUNT][RANK_COUNT]) {
  Serial.println("Estado actual del tablero:");

  for (int r = RANK_COUNT - 1; r >= 0; r--) {
    Serial.print(rankNames[r]);
    Serial.print("  ");

    for (int f = 0; f < FILE_COUNT; f++) {
      Serial.print(board[f][r] ? "[X]" : "[ ]");
    }

    Serial.println();
  }

  Serial.print("   ");

  for (int f = 0; f < FILE_COUNT; f++) {
    Serial.print(" ");
    Serial.print(fileNames[f]);
    Serial.print(" ");
  }

  Serial.println();
}