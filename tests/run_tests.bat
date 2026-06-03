@echo off
g++ -std=c++17 -Wall -Wextra tests\test_chess.cpp chessboard.cpp -o tests\chess_tests.exe
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
tests\chess_tests.exe
