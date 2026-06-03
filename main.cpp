#include "ChessBoard.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cctype>

int main() {
    ChessBoard game;
    std::cout << "\n=== AJEDREZ BACKEND DEMO ===\n";
    std::cout << "Ingrese movimientos como: e2 e4" << "\n";
    std::cout << "Use una tercera letra para promocionar: e7 e8 Q" << "\n";
    std::cout << "Escriba 'salir' para terminar." << "\n\n";
    std::string input;
    while (true) {
        game.displayBoard();
        std::cout << "Movimiento: ";
        std::getline(std::cin, input);
        if (input.empty()) {
            continue;
        }
        if (input == "salir" || input == "exit" || input == "quit") {
            break;
        }

        std::string from;
        std::string to;
        std::string promoLetter;
        std::istringstream stream(input);
        stream >> from >> to >> promoLetter;
        if (from.size() == 4 && to.empty()) {
            to = from.substr(2);
            from = from.substr(0, 2);
        }

        Piece promotionPiece = EMPTY;
        if (!promoLetter.empty()) {
            char letter = std::toupper(static_cast<unsigned char>(promoLetter[0]));
            bool white = game.isWhiteTurn();
            if (letter == 'Q') {
                promotionPiece = white ? WHITE_QUEEN : BLACK_QUEEN;
            } else if (letter == 'R') {
                promotionPiece = white ? WHITE_ROOK : BLACK_ROOK;
            } else if (letter == 'B') {
                promotionPiece = white ? WHITE_BISHOP : BLACK_BISHOP;
            } else if (letter == 'N') {
                promotionPiece = white ? WHITE_KNIGHT : BLACK_KNIGHT;
            }
        }

        MoveResult result = game.processMove(from, to, promotionPiece);
        std::cout << result.message << "\n";
        if (result.requiresPromotion && promoLetter.empty()) {
            std::cout << "Seleccione pieza de promocion (Q,R,B,N) y reingrese el movimiento." << "\n";
        }
    }

    std::cout << "Saliendo del demo." << "\n";
    return 0;
}
