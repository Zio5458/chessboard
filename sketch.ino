// Tablero de ajedrez inteligente - lectura de matriz 8x8 con ESP32
// Wokwi: slide switches + diodos
//
// Convención de la matriz:
// - Filas: OUTPUT. Solo una fila se activa en LOW durante el escaneo.
// - Columnas: INPUT_PULLUP.
// - Si una celda está "cerrada", la columna lee LOW cuando su fila está activa.
// - Diodo: ANODE hacia switch/columna, CATHODE hacia fila.
//
// El slide switch tiene pin 2 como común. En este diagrama se usa pin 1.
// Si el switch se comporta "al revés", movelo al otro lado o cambia en diagram.json swXX:1 por swXX:3.

const int ROWS = 8;
const int COLS = 8;

const int rowPins[ROWS] = {13, 14, 27, 26, 25, 33, 32, 23};
const int colPins[COLS] = {22, 21, 19, 18, 5, 17, 16, 4};

// Para imprimir coordenadas tipo ajedrez
const char files[COLS] = {'a','b','c','d','e','f','g','h'};
const char ranks[ROWS] = {'1','2','3','4','5','6','7','8'};

bool currentState[ROWS][COLS];
bool previousState[ROWS][COLS];

struct Square {
  int row;
  int col;
  bool valid;
};

void setup() {
  Serial.begin(115200);
  delay(300);
  for (int r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }
  for (int c = 0; c < COLS; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }

  clearStates();

  Serial.println();
  Serial.println("Matriz 8x8 iniciada.");
  Serial.println("Cambie switches para simular piezas ocupando/vaciando casillas.");
  Serial.println("Una jugada normal debe verse como: una casilla se vacia y otra se ocupa.");
  Serial.println();
}

void loop() {
  scanBoard();
  detectChangesAndMove();
  delay(80);
}

void clearStates() {
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      currentState[r][c] = false;
      previousState[r][c] = false;
    }
  }
}

void scanBoard() {
  for (int r = 0; r < ROWS; r++) {
    // Desactivar todas las filas
    for (int i = 0; i < ROWS; i++) {
      digitalWrite(rowPins[i], HIGH);
    }
    // Activar solamente la fila actual
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(100);
    // Leer columnas
    for (int c = 0; c < COLS; c++) {
      currentState[r][c] = (digitalRead(colPins[c]) == LOW);
    }
  }
  // Desactivar filas al terminar
  for (int r = 0; r < ROWS; r++) {
    digitalWrite(rowPins[r], HIGH);
  }
}

void detectChangesAndMove() {
  int changes = 0;
  Square from = {-1, -1, false}; // casilla que paso de ocupada a vacia
  Square to   = {-1, -1, false}; // casilla que paso de vacia a ocupada

  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      if (currentState[r][c] != previousState[r][c]) {
        changes++;

        Serial.print("Cambio: ");
        printSquare(r, c);

        if (previousState[r][c] && !currentState[r][c]) {
          Serial.println(" -> vacia");
          from = {r, c, true};
        } else if (!previousState[r][c] && currentState[r][c]) {
          Serial.println(" -> ocupada");
          to = {r, c, true};
        }

        previousState[r][c] = currentState[r][c];
      }
    }
  }

  if (changes == 0) return;

  printBoard();

  if (changes == 2 && from.valid && to.valid) {
    Serial.print("Movimiento detectado: ");
    printSquare(from.row, from.col);
    Serial.print(" -> ");
    printSquare(to.row, to.col);
    Serial.println();

  } else {
    Serial.print("Estado cambiado con ");
    Serial.print(changes);
    Serial.println(" cambio(s).");
    Serial.println("Esto puede ser una captura, enroque, correccion de pieza, setup inicial o movimiento incompleto.");
    Serial.println("Para la validacion final conviene comparar el tablero fisico contra movimientos legales simulados.");
  }

  Serial.println();
}

void printSquare(int r, int c) {
  Serial.print(files[c]);
  Serial.print(ranks[r]);
}

void printBoard() {
  Serial.println("Estado actual del tablero:");

  for (int r = ROWS - 1; r >= 0; r--) {
    Serial.print(ranks[r]);
    Serial.print("  ");
    for (int c = 0; c < COLS; c++) {
      Serial.print(currentState[r][c] ? "[X]" : "[ ]");
    }
    Serial.println();
  }

  Serial.print("   ");
  for (int c = 0; c < COLS; c++) {
    Serial.print(" ");
    Serial.print(files[c]);
    Serial.print(" ");
  }
  Serial.println();
}
