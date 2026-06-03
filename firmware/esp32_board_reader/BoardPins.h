#ifndef BOARD_PINS_H
#define BOARD_PINS_H

const int FILE_COUNT = 8;
const int RANK_COUNT = 8;

//Files / letras: a-h
//Indice 0 = a, 1 = b, ..., 7 = h
const int filePins[FILE_COUNT] = {
  8,   // A
  18,  // B
  17,  // C
  16,  // D
  15,  // E
  7,   // F
  6,   // G
  5    // H
};

//Ranks / numeros: 1-8
//Indice 0 = 1, 1 = 2, ..., 7 = 8
const int rankPins[RANK_COUNT] = {
  2,   // 1
  42,  // 2
  41,  // 3
  40,  // 4
  39,  // 5
  38,  // 6
  37,  // 7
  36   // 8
};

const char fileNames[FILE_COUNT] = {
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
};

const char rankNames[RANK_COUNT] = {
  '1', '2', '3', '4', '5', '6', '7', '8'
};

#endif