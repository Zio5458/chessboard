#include "ChessBoard.h"

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

std::string escapeJson(const std::string& value) {
    std::string escaped;

    for (char ch : value) {
        switch (ch) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += ch; break;
        }
    }

    return escaped;
}

std::string boolJson(bool value) {
    return value ? "true" : "false";
}

std::string pieceToCode(Piece piece) {
    switch (piece) {
        case WHITE_PAWN: return "wP";
        case WHITE_KNIGHT: return "wN";
        case WHITE_BISHOP: return "wB";
        case WHITE_ROOK: return "wR";
        case WHITE_QUEEN: return "wQ";
        case WHITE_KING: return "wK";

        case BLACK_PAWN: return "bP";
        case BLACK_KNIGHT: return "bN";
        case BLACK_BISHOP: return "bB";
        case BLACK_ROOK: return "bR";
        case BLACK_QUEEN: return "bQ";
        case BLACK_KING: return "bK";

        default: return "";
    }
}

std::string pieceJson(Piece piece) {
    std::string code = pieceToCode(piece);

    if (code.empty()) {
        return "null";
    }

    return "\"" + code + "\"";
}

std::string moveTypeToString(MoveType moveType) {
    switch (moveType) {
        case MoveType::NORMAL: return "NORMAL";
        case MoveType::CAPTURE: return "CAPTURE";
        case MoveType::CASTLING_KINGSIDE: return "CASTLING_KINGSIDE";
        case MoveType::CASTLING_QUEENSIDE: return "CASTLING_QUEENSIDE";
        case MoveType::PROMOTION: return "PROMOTION";
        case MoveType::EN_PASSANT: return "EN_PASSANT";
        case MoveType::INVALID: return "INVALID";
        default: return "INVALID";
    }
}

std::string gameStatusToString(GameStatus status) {
    switch (status) {
        case GameStatus::INITIALIZING: return "INITIALIZING";
        case GameStatus::WHITE_TO_MOVE: return "WHITE_TO_MOVE";
        case GameStatus::BLACK_TO_MOVE: return "BLACK_TO_MOVE";
        case GameStatus::WHITE_IN_CHECK: return "WHITE_IN_CHECK";
        case GameStatus::BLACK_IN_CHECK: return "BLACK_IN_CHECK";
        case GameStatus::CHECKMATE: return "CHECKMATE";
        case GameStatus::STALEMATE: return "STALEMATE";
        case GameStatus::DRAW: return "DRAW";
        default: return "UNKNOWN";
    }
}

Piece promotionFromLetter(char letter, bool white) {
    char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(letter)));

    if (upper == 'Q') return white ? WHITE_QUEEN : BLACK_QUEEN;
    if (upper == 'R') return white ? WHITE_ROOK : BLACK_ROOK;
    if (upper == 'B') return white ? WHITE_BISHOP : BLACK_BISHOP;
    if (upper == 'N') return white ? WHITE_KNIGHT : BLACK_KNIGHT;

    return EMPTY;
}

std::string boardToJson(const ChessBoard& game) {
    const std::string files = "abcdefgh";
    std::ostringstream json;

    json << "{";

    bool first = true;

    for (char file : files) {
        for (int rank = 1; rank <= 8; rank++) {
            std::string square;
            square += file;
            square += static_cast<char>('0' + rank);

            Piece piece = game.getPieceAt(square);

            if (piece == EMPTY) {
                continue;
            }

            if (!first) {
                json << ",";
            }

            json << "\"" << square << "\":" << pieceJson(piece);
            first = false;
        }
    }

    json << "}";

    return json.str();
}

std::string stateJson(const ChessBoard& game, const std::string& message = "Estado actual.") {
    std::ostringstream json;

    json << "{";
    json << "\"valid\":true,";
    json << "\"message\":\"" << escapeJson(message) << "\",";
    json << "\"turn\":\"" << (game.isWhiteTurn() ? "w" : "b") << "\",";
    json << "\"gameStatus\":\"" << gameStatusToString(game.getGameStatus()) << "\",";
    json << "\"board\":" << boardToJson(game);
    json << "}";

    return json.str();
}

std::string moveResultJson(
    const ChessBoard& game,
    const MoveResult& result,
    const std::string& fallbackFrom,
    const std::string& fallbackTo
) {
    std::ostringstream json;

    std::string from = result.from.empty() ? fallbackFrom : result.from;
    std::string to = result.to.empty() ? fallbackTo : result.to;

    json << "{";
    json << "\"valid\":" << boolJson(result.valid) << ",";
    json << "\"message\":\"" << escapeJson(result.message) << "\",";
    json << "\"from\":\"" << escapeJson(from) << "\",";
    json << "\"to\":\"" << escapeJson(to) << "\",";
    json << "\"movedPiece\":" << pieceJson(result.movedPiece) << ",";
    json << "\"capturedPiece\":" << pieceJson(result.capturedPiece) << ",";
    json << "\"moveType\":\"" << moveTypeToString(result.moveType) << "\",";
    json << "\"isCheck\":" << boolJson(result.isCheck) << ",";
    json << "\"isCheckmate\":" << boolJson(result.isCheckmate) << ",";
    json << "\"isStalemate\":" << boolJson(result.isStalemate) << ",";
    json << "\"requiresPromotion\":" << boolJson(result.requiresPromotion) << ",";
    json << "\"gameStatus\":\"" << gameStatusToString(result.gameStatus) << "\",";
    json << "\"turn\":\"" << (game.isWhiteTurn() ? "w" : "b") << "\",";
    json << "\"board\":" << boardToJson(game);
    json << "}";

    return json.str();
}

int main() {
    ChessBoard game;

    std::string line;

    while (std::getline(std::cin, line)) {
        std::istringstream stream(line);

        std::string command;
        stream >> command;

        if (command == "PING") {
            std::cout << "{\"valid\":true,\"message\":\"PONG\"}" << std::endl;
        }

        else if (command == "RESET") {
            game.resetBoard();
            std::cout << stateJson(game, "Partida reiniciada.") << std::endl;
        }

        else if (command == "BOARD") {
            std::cout << stateJson(game) << std::endl;
        }

        else if (command == "MOVE") {
            std::string from;
            std::string to;
            std::string promotionText;

            stream >> from >> to >> promotionText;

            Piece promotionPiece = EMPTY;

            if (!promotionText.empty()) {
                promotionPiece = promotionFromLetter(promotionText[0], game.isWhiteTurn());
            }

            MoveResult result = game.processMove(from, to, promotionPiece);

            std::cout << moveResultJson(game, result, from, to) << std::endl;
        }

        else if (command == "QUIT") {
            std::cout << "{\"valid\":true,\"message\":\"Cerrando motor.\"}" << std::endl;
            break;
        }

        else {
            std::cout << "{\"valid\":false,\"message\":\"Comando desconocido.\"}" << std::endl;
        }
    }

    return 0;
}