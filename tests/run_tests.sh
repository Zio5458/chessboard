#!/usr/bin/env bash
set -e

g++ -std=c++17 -Wall -Wextra tests/test_chess.cpp chessboard.cpp -o tests/chess_tests
./tests/chess_tests
