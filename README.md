# Instrucciones de Ejecucion

## Estructura del Proyecto

El proyecto tiene 3 partes:

a. El backend del ajedrez en C++
b. API local que conecta el frontend y backend
c. Frontend en React

## Compilar el Backend

```bash
g++ -std=c++17 main.cpp chessboard.cpp -o chess
./chess     # o en Windows: chess.exe
```

## Pruebas del Backend

```bash
g++ -std=c++17 -Wall -Wextra tests/test_chess.cpp chessboard.cpp -o tests/chess_tests
./tests/chess_tests     # o en Windows: tests/chess_tests.exe
```

## Motor CLI para la API Local

```bash
g++ -std=c++17 engine_cli.cpp chessboard.cpp -o chess_engine_cli
./chess_engine_cli      # o en Windows: chess_engine_cli.exe
```

### Comandos de Prueba

- `BOARD` - Mostrar el tablero
- `MOVE <pos_inicial> <pos_final>` - Realizar un movimiento
- `RESET` - Reiniciar el juego
- `QUIT` - Salir

## Ejecutar la API Local

```bash
cd local-api
npm install
npm run dev
```

La API queda disponible en `localhost:3030` y se puede revisar su salud con:

```
localhost:3030/api/health
```

## Ejecutar el Frontend

```bash
cd frontend
npm install
npm run dev
```

El frontend queda disponible en `localhost:5173`

## Ejecucion Completa

Se necesitan dos terminales:

### Terminal 1: API Local y Backend

```bash
g++ -std=c++17 engine_cli.cpp chessboard.cpp -o chess_engine_cli
cd local-api
npm install
npm run dev
```

### Terminal 2: Frontend

```bash
cd frontend
npm install
npm run dev
```

## Firmware del Tablero

El firmware del tablero está en la carpeta `firmware/`.

- `esp32_board_reader.ino` y `BoardPins.h` deben cargarse al ESP32 en ArduinoIDE
- El monitor serial debe abrirse a **115200 baud**
