#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <string>
#include <vector>

enum Piece {
    EMPTY = 0,
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING
};

enum class GameStatus {
    INITIALIZING,
    WHITE_TO_MOVE,
    BLACK_TO_MOVE,
    WHITE_IN_CHECK,
    BLACK_IN_CHECK,
    CHECKMATE,
    STALEMATE,
    DRAW
};

enum class MoveType {
    NORMAL,
    CAPTURE,
    CASTLING_KINGSIDE,
    CASTLING_QUEENSIDE,
    PROMOTION,
    EN_PASSANT,
    INVALID
};

struct Move {
    std::string from;
    std::string to;
    Piece promotionPiece = EMPTY;
};

struct MoveResult {
    bool valid = false;
    std::string message;
    std::string from;
    std::string to;
    Piece movedPiece = EMPTY;
    Piece capturedPiece = EMPTY;
    MoveType moveType = MoveType::INVALID;
    bool isCheck = false;
    bool isCheckmate = false;
    bool isStalemate = false;
    bool requiresPromotion = false;
    GameStatus gameStatus = GameStatus::INITIALIZING;
};

struct BoardComparison {
    bool matches = false;
    std::vector<std::string> missingSquares;
    std::vector<std::string> extraSquares;
};

class ChessBoard {
public:
    ChessBoard();

    void resetBoard();

    MoveResult processMove(const std::string& from, const std::string& to, Piece promotionPiece = EMPTY);

    GameStatus getGameStatus() const;

    Piece getPieceAt(const std::string& coord) const;
    Piece getPieceAt(int row, int col) const;

    bool isWhiteTurn() const;

    std::vector<std::string> getOccupiedSquares() const;

    std::vector<std::string> getMissingInitialSquares(const std::vector<std::string>& physicallyOccupiedSquares) const;
    std::vector<std::string> getExtraInitialSquares(const std::vector<std::string>& physicallyOccupiedSquares) const;

    bool isInitialPhysicalPositionCorrect(const std::vector<std::string>& physicallyOccupiedSquares) const;

    std::vector<std::string> getExpectedOccupiedSquares() const;
    BoardComparison comparePhysicalToLogical(const std::vector<std::string>& physicallyOccupiedSquares) const;
    BoardComparison comparePhysicalToInitialPosition(const std::vector<std::string>& physicallyOccupiedSquares) const;

    void displayBoard() const;
    const std::vector<MoveResult>& getMoveHistory() const;

private:
    Piece board[8][8];
    bool whiteToMove;
    bool whiteKingMoved;
    bool blackKingMoved;
    bool whiteKingsideRookMoved;
    bool whiteQueensideRookMoved;
    bool blackKingsideRookMoved;
    bool blackQueensideRookMoved;
    int enPassantTargetRow;
    int enPassantTargetCol;
    bool hasEnPassantTarget;
    GameStatus gameStatus;
    std::vector<MoveResult> moveHistory;

    void initializeBoard();
    void updateGameStatusAfterMove();
    void updateCastlingFlagsForMovedPiece(Piece movedPiece, int fromRow, int fromCol);
    void updateCastlingFlagsForCapturedPiece(Piece capturedPiece, int toRow, int toCol);
    bool coordToPos(const std::string& coord, int& row, int& col) const;
    std::string posToCoord(int row, int col) const;
    bool isValidSquare(int row, int col) const;
    bool isWhitePiece(Piece p) const;
    bool isBlackPiece(Piece p) const;
    bool isSameColor(Piece a, Piece b) const;
    bool isOpponentPiece(Piece p, bool white) const;
    bool isValidPawnMove(int fromRow, int fromCol, int toRow, int toCol, bool& outCapture, bool& outEnPassant) const;
    bool isValidKnightMove(int fromRow, int fromCol, int toRow, int toCol) const;
    bool isValidBishopMove(int fromRow, int fromCol, int toRow, int toCol) const;
    bool isValidRookMove(int fromRow, int fromCol, int toRow, int toCol) const;
    bool isValidQueenMove(int fromRow, int fromCol, int toRow, int toCol) const;
    bool isValidKingMove(int fromRow, int fromCol, int toRow, int toCol) const;
    bool isSquareAttacked(int row, int col, bool byWhite) const;
    bool isKingInCheck(bool white) const;
    bool wouldLeaveKingInCheck(int fromRow, int fromCol, int toRow, int toCol, Piece promotionPiece, bool isEnPassant) const;
    bool isValidPromotionPiece(Piece promotionPiece, bool white) const;
    bool isCastlingAttempt(Piece movedPiece, int fromRow, int fromCol, int toRow, int toCol) const;
    bool isCurrentPlayerPiece(Piece p) const;
    MoveResult attemptCastling(int fromRow, int fromCol, int toRow, int toCol);
    bool canCastle(int fromRow, int fromCol, int toRow, int toCol) const;
    bool hasAnyLegalMove(bool white) const;
    void clearEnPassant();
    void setEnPassantTarget(int row, int col);
    std::vector<std::string> collectExpectedOccupiedSquares(bool initialPositionOnly) const;
};

#endif // CHESSBOARD_H
