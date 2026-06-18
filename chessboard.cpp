#include "ChessBoard.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <set>

namespace
{

    bool isWhitePieceStatic(Piece p)
    {
        return p >= WHITE_PAWN && p <= WHITE_KING;
    }

    bool isBlackPieceStatic(Piece p)
    {
        return p >= BLACK_PAWN && p <= BLACK_KING;
    }

    bool isSameColorStatic(Piece a, Piece b)
    {
        return (isWhitePieceStatic(a) && isWhitePieceStatic(b)) ||
               (isBlackPieceStatic(a) && isBlackPieceStatic(b));
    }

    bool isOpponentPieceStatic(Piece p, bool white)
    {
        if (p == EMPTY)
        {
            return false;
        }
        return white ? isBlackPieceStatic(p) : isWhitePieceStatic(p);
    }

    bool isValidSquareStatic(int row, int col)
    {
        return row >= 0 && row < 8 && col >= 0 && col < 8;
    }

    bool isPathClearOnBoard(const Piece boardState[8][8], int fromRow, int fromCol, int toRow, int toCol)
    {
        int deltaRow = (toRow > fromRow) ? 1 : (toRow < fromRow) ? -1
                                                                 : 0;
        int deltaCol = (toCol > fromCol) ? 1 : (toCol < fromCol) ? -1
                                                                 : 0;

        if (deltaRow != 0 && deltaCol != 0 && std::abs(toRow - fromRow) != std::abs(toCol - fromCol))
        {
            return false;
        }

        int currentRow = fromRow + deltaRow;
        int currentCol = fromCol + deltaCol;

        while (currentRow != toRow || currentCol != toCol)
        {
            if (boardState[currentRow][currentCol] != EMPTY)
            {
                return false;
            }
            currentRow += deltaRow;
            currentCol += deltaCol;
        }

        return true;
    }

    bool isSlidingAttacker(Piece piece, bool byWhite, int deltaRow, int deltaCol)
    {
        if (piece == EMPTY)
        {
            return false;
        }

        if (byWhite && !isWhitePieceStatic(piece))
        {
            return false;
        }
        if (!byWhite && !isBlackPieceStatic(piece))
        {
            return false;
        }

        bool straight = deltaRow == 0 || deltaCol == 0;
        bool diagonal = std::abs(deltaRow) == std::abs(deltaCol);

        if (straight)
        {
            return piece == WHITE_ROOK || piece == WHITE_QUEEN || piece == BLACK_ROOK || piece == BLACK_QUEEN;
        }
        if (diagonal)
        {
            return piece == WHITE_BISHOP || piece == WHITE_QUEEN || piece == BLACK_BISHOP || piece == BLACK_QUEEN;
        }

        return false;
    }

    bool isSquareAttackedOnBoard(const Piece boardState[8][8], int row, int col, bool byWhite)
    {
        if (!isValidSquareStatic(row, col))
        {
            return false;
        }

        int pawnRow = byWhite ? row + 1 : row - 1;
        int pawnOffsets[2] = {-1, 1};
        for (int offset : pawnOffsets)
        {
            int pawnCol = col + offset;
            if (!isValidSquareStatic(pawnRow, pawnCol))
            {
                continue;
            }
            Piece attacker = boardState[pawnRow][pawnCol];
            if (byWhite && attacker == WHITE_PAWN)
            {
                return true;
            }
            if (!byWhite && attacker == BLACK_PAWN)
            {
                return true;
            }
        }

        int knightDeltas[8][2] = {
            {2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};
        for (auto &delta : knightDeltas)
        {
            int r = row + delta[0];
            int c = col + delta[1];
            if (!isValidSquareStatic(r, c))
            {
                continue;
            }
            Piece attacker = boardState[r][c];
            if (byWhite && attacker == WHITE_KNIGHT)
            {
                return true;
            }
            if (!byWhite && attacker == BLACK_KNIGHT)
            {
                return true;
            }
        }

        for (int dr = -1; dr <= 1; dr++)
        {
            for (int dc = -1; dc <= 1; dc++)
            {
                if (dr == 0 && dc == 0)
                {
                    continue;
                }
                int r = row + dr;
                int c = col + dc;
                if (!isValidSquareStatic(r, c))
                {
                    continue;
                }
                Piece attacker = boardState[r][c];
                if (byWhite && attacker == WHITE_KING)
                {
                    return true;
                }
                if (!byWhite && attacker == BLACK_KING)
                {
                    return true;
                }
            }
        }

        int directions[8][2] = {
            {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
        for (auto &direction : directions)
        {
            int dr = direction[0];
            int dc = direction[1];
            int currentRow = row + dr;
            int currentCol = col + dc;
            while (isValidSquareStatic(currentRow, currentCol))
            {
                Piece attacker = boardState[currentRow][currentCol];
                if (attacker != EMPTY)
                {
                    if (isSlidingAttacker(attacker, byWhite, dr, dc))
                    {
                        return true;
                    }
                    break;
                }
                currentRow += dr;
                currentCol += dc;
            }
        }

        return false;
    }

    bool isKingInCheckOnBoard(const Piece boardState[8][8], bool white)
    {
        Piece kingPiece = white ? WHITE_KING : BLACK_KING;
        int kingRow = -1;
        int kingCol = -1;
        for (int row = 0; row < 8; row++)
        {
            for (int col = 0; col < 8; col++)
            {
                if (boardState[row][col] == kingPiece)
                {
                    kingRow = row;
                    kingCol = col;
                    break;
                }
            }
            if (kingRow != -1)
            {
                break;
            }
        }
        if (kingRow == -1)
        {
            return false;
        }
        return isSquareAttackedOnBoard(boardState, kingRow, kingCol, !white);
    }

    char pieceToChar(Piece p)
    {
        switch (p)
        {
        case WHITE_PAWN:
            return 'P';
        case WHITE_KNIGHT:
            return 'N';
        case WHITE_BISHOP:
            return 'B';
        case WHITE_ROOK:
            return 'R';
        case WHITE_QUEEN:
            return 'Q';
        case WHITE_KING:
            return 'K';
        case BLACK_PAWN:
            return 'p';
        case BLACK_KNIGHT:
            return 'n';
        case BLACK_BISHOP:
            return 'b';
        case BLACK_ROOK:
            return 'r';
        case BLACK_QUEEN:
            return 'q';
        case BLACK_KING:
            return 'k';
        default:
            return '.';
        }
    }

    std::string gameStatusToString(GameStatus status)
    {
        switch (status)
        {
        case GameStatus::INITIALIZING:
            return "Inicializando";
        case GameStatus::WHITE_TO_MOVE:
            return "Blancas para mover";
        case GameStatus::BLACK_TO_MOVE:
            return "Negras para mover";
        case GameStatus::WHITE_IN_CHECK:
            return "Jaque a las blancas";
        case GameStatus::BLACK_IN_CHECK:
            return "Jaque a las negras";
        case GameStatus::CHECKMATE:
            return "Jaque mate";
        case GameStatus::STALEMATE:
            return "Ahogado";
        case GameStatus::DRAW:
            return "Tablas";
        }
        return "Desconocido";
    }

} // anonymous namespace

ChessBoard::ChessBoard()
{
    resetBoard();
}

void ChessBoard::resetBoard()
{
    initializeBoard();
    whiteToMove = true;
    whiteKingMoved = false;
    blackKingMoved = false;
    whiteKingsideRookMoved = false;
    whiteQueensideRookMoved = false;
    blackKingsideRookMoved = false;
    blackQueensideRookMoved = false;
    clearEnPassant();
    gameStatus = GameStatus::WHITE_TO_MOVE;
    moveHistory.clear();
}

void ChessBoard::initializeBoard()
{
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            board[row][col] = EMPTY;
        }
    }
    board[0][0] = BLACK_ROOK;
    board[0][1] = BLACK_KNIGHT;
    board[0][2] = BLACK_BISHOP;
    board[0][3] = BLACK_QUEEN;
    board[0][4] = BLACK_KING;
    board[0][5] = BLACK_BISHOP;
    board[0][6] = BLACK_KNIGHT;
    board[0][7] = BLACK_ROOK;
    for (int col = 0; col < 8; col++)
    {
        board[1][col] = BLACK_PAWN;
        board[6][col] = WHITE_PAWN;
    }
    board[7][0] = WHITE_ROOK;
    board[7][1] = WHITE_KNIGHT;
    board[7][2] = WHITE_BISHOP;
    board[7][3] = WHITE_QUEEN;
    board[7][4] = WHITE_KING;
    board[7][5] = WHITE_BISHOP;
    board[7][6] = WHITE_KNIGHT;
    board[7][7] = WHITE_ROOK;
}

GameStatus ChessBoard::getGameStatus() const
{
    return gameStatus;
}

Piece ChessBoard::getPieceAt(const std::string &coord) const
{
    int row = 0;
    int col = 0;
    if (!coordToPos(coord, row, col))
    {
        return EMPTY;
    }
    return getPieceAt(row, col);
}

Piece ChessBoard::getPieceAt(int row, int col) const
{
    if (!isValidSquare(row, col))
    {
        return EMPTY;
    }
    return board[row][col];
}

bool ChessBoard::isWhiteTurn() const
{
    return whiteToMove;
}

std::vector<std::string> ChessBoard::getOccupiedSquares() const
{
    return collectExpectedOccupiedSquares(false);
}

std::vector<std::string> ChessBoard::getExpectedOccupiedSquares() const
{
    return collectExpectedOccupiedSquares(false);
}

std::vector<std::string> ChessBoard::collectExpectedOccupiedSquares(bool initialPositionOnly) const
{
    std::vector<std::string> squares;
    if (initialPositionOnly)
    {
        const int rankRows[4] = {0, 1, 6, 7};
        for (int rowIndex = 0; rowIndex < 4; rowIndex++)
        {
            int row = rankRows[rowIndex];
            for (int col = 0; col < 8; col++)
            {
                squares.push_back(posToCoord(row, col));
            }
        }
        return squares;
    }
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            if (board[row][col] != EMPTY)
            {
                squares.push_back(posToCoord(row, col));
            }
        }
    }
    return squares;
}

BoardComparison ChessBoard::comparePhysicalToLogical(const std::vector<std::string> &physicallyOccupiedSquares) const
{
    std::vector<std::string> expected = getExpectedOccupiedSquares();
    std::set<std::string> expectedSet(expected.begin(), expected.end());

    std::set<std::string> physicalSet;
    BoardComparison result;

    for (const std::string &coord : physicallyOccupiedSquares)
    {
        int row = 0;
        int col = 0;

        if (!coordToPos(coord, row, col))
        {
            result.extraSquares.push_back(coord);
            continue;
        }

        physicalSet.insert(posToCoord(row, col));
    }

    for (const std::string &square : expectedSet)
    {
        if (physicalSet.find(square) == physicalSet.end())
        {
            result.missingSquares.push_back(square);
        }
    }

    for (const std::string &square : physicalSet)
    {
        if (expectedSet.find(square) == expectedSet.end())
        {
            result.extraSquares.push_back(square);
        }
    }

    result.matches = result.missingSquares.empty() && result.extraSquares.empty();
    return result;
}

BoardComparison ChessBoard::comparePhysicalToInitialPosition(const std::vector<std::string> &physicallyOccupiedSquares) const
{
    std::vector<std::string> expected = collectExpectedOccupiedSquares(true);
    std::set<std::string> expectedSet(expected.begin(), expected.end());

    std::set<std::string> physicalSet;
    BoardComparison result;

    for (const std::string &coord : physicallyOccupiedSquares)
    {
        int row = 0;
        int col = 0;

        if (!coordToPos(coord, row, col))
        {
            result.extraSquares.push_back(coord);
            continue;
        }

        physicalSet.insert(posToCoord(row, col));
    }

    for (const std::string &square : expectedSet)
    {
        if (physicalSet.find(square) == physicalSet.end())
        {
            result.missingSquares.push_back(square);
        }
    }

    for (const std::string &square : physicalSet)
    {
        if (expectedSet.find(square) == expectedSet.end())
        {
            result.extraSquares.push_back(square);
        }
    }

    result.matches = result.missingSquares.empty() &&
                     result.extraSquares.empty() &&
                     physicalSet.size() == expectedSet.size();

    return result;
}

std::vector<std::string> ChessBoard::getMissingInitialSquares(const std::vector<std::string> &physicallyOccupiedSquares) const
{
    return comparePhysicalToInitialPosition(physicallyOccupiedSquares).missingSquares;
}

std::vector<std::string> ChessBoard::getExtraInitialSquares(const std::vector<std::string> &physicallyOccupiedSquares) const
{
    return comparePhysicalToInitialPosition(physicallyOccupiedSquares).extraSquares;
}

bool ChessBoard::isInitialPhysicalPositionCorrect(const std::vector<std::string> &physicallyOccupiedSquares) const
{
    BoardComparison comparison = comparePhysicalToInitialPosition(physicallyOccupiedSquares);
    return comparison.matches;
}

void ChessBoard::displayBoard() const
{
    std::cout << "\n  a b c d e f g h\n";
    for (int row = 0; row < 8; row++)
    {
        std::cout << (8 - row) << " ";
        for (int col = 0; col < 8; col++)
        {
            std::cout << pieceToChar(board[row][col]) << " ";
        }
        std::cout << (8 - row) << "\n";
    }
    std::cout << "  a b c d e f g h\n";
    std::cout << "Estado: " << gameStatusToString(gameStatus) << "\n";
    std::cout << "Turno actual: " << (whiteToMove ? "Blancas" : "Negras") << "\n\n";
}

bool ChessBoard::coordToPos(const std::string &coord, int &row, int &col) const
{
    if (coord.size() != 2)
    {
        return false;
    }
    char file = std::tolower(static_cast<unsigned char>(coord[0]));
    char rank = coord[1];
    if (file < 'a' || file > 'h')
    {
        return false;
    }
    if (rank < '1' || rank > '8')
    {
        return false;
    }
    col = file - 'a';
    row = 8 - (rank - '0');
    return isValidSquare(row, col);
}

std::string ChessBoard::posToCoord(int row, int col) const
{
    if (!isValidSquare(row, col))
    {
        return std::string();
    }
    std::string coord;
    coord.push_back(static_cast<char>('a' + col));
    coord.push_back(static_cast<char>('8' - row));
    return coord;
}

bool ChessBoard::isValidSquare(int row, int col) const
{
    return row >= 0 && row < 8 && col >= 0 && col < 8;
}

bool ChessBoard::isWhitePiece(Piece p) const
{
    return isWhitePieceStatic(p);
}

bool ChessBoard::isBlackPiece(Piece p) const
{
    return isBlackPieceStatic(p);
}

bool ChessBoard::isSameColor(Piece a, Piece b) const
{
    return isSameColorStatic(a, b);
}

bool ChessBoard::isCurrentPlayerPiece(Piece p) const
{
    if (p == EMPTY)
    {
        return false;
    }
    return whiteToMove ? isWhitePiece(p) : isBlackPiece(p);
}

bool ChessBoard::isOpponentPiece(Piece p, bool white) const
{
    return isOpponentPieceStatic(p, white);
}

bool ChessBoard::isValidPawnMove(int fromRow, int fromCol, int toRow, int toCol, bool &outCapture, bool &outEnPassant) const
{
    outCapture = false;
    outEnPassant = false;
    if (!isValidSquare(toRow, toCol))
    {
        return false;
    }
    Piece pawn = board[fromRow][fromCol];
    bool white = isWhitePiece(pawn);
    int direction = white ? -1 : 1;
    int startRow = white ? 6 : 1;
    Piece destination = board[toRow][toCol];
    if (fromCol == toCol)
    {
        if (toRow == fromRow + direction && destination == EMPTY)
        {
            return true;
        }
        if (fromRow == startRow && toRow == fromRow + 2 * direction)
        {
            int betweenRow = fromRow + direction;
            if (board[betweenRow][fromCol] == EMPTY && destination == EMPTY)
            {
                return true;
            }
        }
        return false;
    }
    if (std::abs(toCol - fromCol) == 1 && toRow == fromRow + direction)
    {
        if (destination != EMPTY)
        {
            if (isOpponentPiece(destination, white))
            {
                outCapture = true;
                return true;
            }
            return false;
        }
        if (hasEnPassantTarget && toRow == enPassantTargetRow && toCol == enPassantTargetCol)
        {
            int capturedRow = fromRow;
            int capturedCol = toCol;
            if (!isValidSquare(capturedRow, capturedCol))
            {
                return false;
            }
            Piece capturedPawn = board[capturedRow][capturedCol];
            if (capturedPawn != EMPTY && isOpponentPiece(capturedPawn, white) &&
                ((white && capturedPawn == BLACK_PAWN) || (!white && capturedPawn == WHITE_PAWN)))
            {
                outEnPassant = true;
                outCapture = true;
                return true;
            }
        }
    }
    return false;
}

bool ChessBoard::isValidKnightMove(int fromRow, int fromCol, int toRow, int toCol) const
{
    int deltaRow = std::abs(toRow - fromRow);
    int deltaCol = std::abs(toCol - fromCol);
    return (deltaRow == 2 && deltaCol == 1) || (deltaRow == 1 && deltaCol == 2);
}

bool ChessBoard::isValidBishopMove(int fromRow, int fromCol, int toRow, int toCol) const
{
    if (std::abs(toRow - fromRow) != std::abs(toCol - fromCol))
    {
        return false;
    }
    return isPathClearOnBoard(board, fromRow, fromCol, toRow, toCol);
}

bool ChessBoard::isValidRookMove(int fromRow, int fromCol, int toRow, int toCol) const
{
    if (fromRow != toRow && fromCol != toCol)
    {
        return false;
    }
    return isPathClearOnBoard(board, fromRow, fromCol, toRow, toCol);
}

bool ChessBoard::isValidQueenMove(int fromRow, int fromCol, int toRow, int toCol) const
{
    return isValidRookMove(fromRow, fromCol, toRow, toCol) ||
           isValidBishopMove(fromRow, fromCol, toRow, toCol);
}

bool ChessBoard::isValidKingMove(int fromRow, int fromCol, int toRow, int toCol) const
{
    int deltaRow = std::abs(toRow - fromRow);
    int deltaCol = std::abs(toCol - fromCol);
    return deltaRow <= 1 && deltaCol <= 1 && (deltaRow != 0 || deltaCol != 0);
}

bool ChessBoard::isSquareAttacked(int row, int col, bool byWhite) const
{
    return isSquareAttackedOnBoard(board, row, col, byWhite);
}

bool ChessBoard::isKingInCheck(bool white) const
{
    return isKingInCheckOnBoard(board, white);
}

bool ChessBoard::wouldLeaveKingInCheck(int fromRow, int fromCol, int toRow, int toCol, Piece promotionPiece, bool isEnPassant) const
{
    Piece boardCopy[8][8];
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            boardCopy[row][col] = board[row][col];
        }
    }
    Piece movingPiece = boardCopy[fromRow][fromCol];
    Piece effectivePiece = promotionPiece != EMPTY ? promotionPiece : movingPiece;
    boardCopy[fromRow][fromCol] = EMPTY;
    boardCopy[toRow][toCol] = effectivePiece;
    if (isEnPassant)
    {
        int captureRow = fromRow;
        int captureCol = toCol;
        if (isValidSquare(captureRow, captureCol))
        {
            boardCopy[captureRow][captureCol] = EMPTY;
        }
    }
    return isKingInCheckOnBoard(boardCopy, isWhitePiece(movingPiece));
}

bool ChessBoard::isValidPromotionPiece(Piece promotionPiece, bool white) const
{
    if (promotionPiece == EMPTY)
    {
        return false;
    }
    if (white)
    {
        return promotionPiece == WHITE_QUEEN || promotionPiece == WHITE_ROOK ||
               promotionPiece == WHITE_BISHOP || promotionPiece == WHITE_KNIGHT;
    }
    return promotionPiece == BLACK_QUEEN || promotionPiece == BLACK_ROOK ||
           promotionPiece == BLACK_BISHOP || promotionPiece == BLACK_KNIGHT;
}

bool ChessBoard::isCastlingAttempt(Piece movedPiece, int fromRow, int fromCol, int toRow, int toCol) const
{
    if (movedPiece != WHITE_KING && movedPiece != BLACK_KING)
    {
        return false;
    }
    if (fromCol != 4)
    {
        return false;
    }
    if (movedPiece == WHITE_KING && fromRow == 7 && toRow == 7 && (toCol == 6 || toCol == 2))
    {
        return true;
    }
    if (movedPiece == BLACK_KING && fromRow == 0 && toRow == 0 && (toCol == 6 || toCol == 2))
    {
        return true;
    }
    return false;
}

bool ChessBoard::canCastle(int fromRow, int fromCol, int toRow, int toCol) const
{
    if (!isCastlingAttempt(board[fromRow][fromCol], fromRow, fromCol, toRow, toCol))
    {
        return false;
    }
    bool white = whiteToMove;
    if ((white && whiteKingMoved) || (!white && blackKingMoved))
    {
        return false;
    }
    int rookCol = (toCol == 6) ? 7 : 0;
    Piece rookPiece = white ? WHITE_ROOK : BLACK_ROOK;
    if (board[fromRow][rookCol] != rookPiece)
    {
        return false;
    }
    if (isKingInCheck(white))
    {
        return false;
    }
    if (toCol == 6)
    {
        if (board[fromRow][5] != EMPTY || board[fromRow][6] != EMPTY)
        {
            return false;
        }
        if (isSquareAttacked(fromRow, 5, !white) || isSquareAttacked(fromRow, 6, !white))
        {
            return false;
        }
        if ((white && whiteKingsideRookMoved) || (!white && blackKingsideRookMoved))
        {
            return false;
        }
    }
    else
    {
        if (board[fromRow][3] != EMPTY || board[fromRow][2] != EMPTY || board[fromRow][1] != EMPTY)
        {
            return false;
        }
        if (isSquareAttacked(fromRow, 3, !white) || isSquareAttacked(fromRow, 2, !white))
        {
            return false;
        }
        if ((white && whiteQueensideRookMoved) || (!white && blackQueensideRookMoved))
        {
            return false;
        }
    }
    return true;
}

MoveResult ChessBoard::attemptCastling(int fromRow, int fromCol, int toRow, int toCol)
{
    MoveResult result;
    result.from = posToCoord(fromRow, fromCol);
    result.to = posToCoord(toRow, toCol);
    result.movedPiece = board[fromRow][fromCol];
    result.capturedPiece = EMPTY;
    result.valid = false;
    result.moveType = MoveType::INVALID;
    result.gameStatus = gameStatus;
    if (!canCastle(fromRow, fromCol, toRow, toCol))
    {
        if (isKingInCheck(whiteToMove))
        {
            result.message = "No se puede enrocar mientras el rey esta en jaque.";
        }
        else
        {
            result.message = "Condiciones de enroque no se cumplen.";
        }
        return result;
    }
    bool white = whiteToMove;
    int rookCol = (toCol == 6) ? 7 : 0;
    int newRookCol = (toCol == 6) ? 5 : 3;
    Piece kingPiece = white ? WHITE_KING : BLACK_KING;
    Piece rookPiece = white ? WHITE_ROOK : BLACK_ROOK;
    board[fromRow][fromCol] = EMPTY;
    board[fromRow][rookCol] = EMPTY;
    board[fromRow][toCol] = kingPiece;
    board[fromRow][newRookCol] = rookPiece;
    updateCastlingFlagsForMovedPiece(kingPiece, fromRow, fromCol);
    if (toCol == 6)
    {
        if (white)
        {
            whiteKingsideRookMoved = true;
        }
        else
        {
            blackKingsideRookMoved = true;
        }
        result.moveType = MoveType::CASTLING_KINGSIDE;
        result.message = "Enroque corto realizado.";
    }
    else
    {
        if (white)
        {
            whiteQueensideRookMoved = true;
        }
        else
        {
            blackQueensideRookMoved = true;
        }
        result.moveType = MoveType::CASTLING_QUEENSIDE;
        result.message = "Enroque largo realizado.";
    }
    clearEnPassant();
    whiteToMove = !whiteToMove;
    updateGameStatusAfterMove();
    result.valid = true;
    result.isCheck = (gameStatus == GameStatus::WHITE_IN_CHECK || gameStatus == GameStatus::BLACK_IN_CHECK);
    result.isCheckmate = (gameStatus == GameStatus::CHECKMATE);
    result.isStalemate = (gameStatus == GameStatus::STALEMATE);
    result.gameStatus = gameStatus;
    moveHistory.push_back(result);
    return result;
}

void ChessBoard::updateCastlingFlagsForMovedPiece(Piece movedPiece, int fromRow, int fromCol)
{
    if (movedPiece == WHITE_KING)
    {
        whiteKingMoved = true;
    }
    else if (movedPiece == BLACK_KING)
    {
        blackKingMoved = true;
    }
    else if (movedPiece == WHITE_ROOK)
    {
        if (fromRow == 7 && fromCol == 0)
            whiteQueensideRookMoved = true;
        if (fromRow == 7 && fromCol == 7)
            whiteKingsideRookMoved = true;
    }
    else if (movedPiece == BLACK_ROOK)
    {
        if (fromRow == 0 && fromCol == 0)
            blackQueensideRookMoved = true;
        if (fromRow == 0 && fromCol == 7)
            blackKingsideRookMoved = true;
    }
}

void ChessBoard::updateCastlingFlagsForCapturedPiece(Piece capturedPiece, int toRow, int toCol)
{
    if (capturedPiece == WHITE_ROOK)
    {
        if (toRow == 7 && toCol == 0)
            whiteQueensideRookMoved = true;
        if (toRow == 7 && toCol == 7)
            whiteKingsideRookMoved = true;
    }
    else if (capturedPiece == BLACK_ROOK)
    {
        if (toRow == 0 && toCol == 0)
            blackQueensideRookMoved = true;
        if (toRow == 0 && toCol == 7)
            blackKingsideRookMoved = true;
    }
}

void ChessBoard::clearEnPassant()
{
    hasEnPassantTarget = false;
    enPassantTargetRow = -1;
    enPassantTargetCol = -1;
}

void ChessBoard::setEnPassantTarget(int row, int col)
{
    hasEnPassantTarget = true;
    enPassantTargetRow = row;
    enPassantTargetCol = col;
}


std::vector<std::string> ChessBoard::getLegalMovesFrom(const std::string &from) const
{
    std::vector<std::string> moves;

    if (gameStatus == GameStatus::CHECKMATE ||
        gameStatus == GameStatus::STALEMATE ||
        gameStatus == GameStatus::DRAW)
    {
        return moves;
    }

    int fromRow = 0;
    int fromCol = 0;

    if (!coordToPos(from, fromRow, fromCol))
    {
        return moves;
    }

    Piece piece = board[fromRow][fromCol];

    if (piece == EMPTY || !isCurrentPlayerPiece(piece))
    {
        return moves;
    }

    auto addMoveIfNeeded = [&](int toRow, int toCol)
    {
        if (!isValidSquare(toRow, toCol))
        {
            return;
        }

        std::string to = posToCoord(toRow, toCol);

        if (std::find(moves.begin(), moves.end(), to) == moves.end())
        {
            moves.push_back(to);
        }
    };

    auto canTryDestination = [&](int toRow, int toCol, Piece promotionPiece, bool expectedEnPassant)
    {
        if (!isValidSquare(toRow, toCol))
        {
            return false;
        }

        Piece destination = board[toRow][toCol];

        if (isSameColor(destination, piece))
        {
            return false;
        }

        bool isCapture = false;
        bool isEnPassant = false;
        bool validMove = false;

        if (piece == WHITE_PAWN || piece == BLACK_PAWN)
        {
            validMove = isValidPawnMove(fromRow, fromCol, toRow, toCol, isCapture, isEnPassant);

            if (isEnPassant != expectedEnPassant)
            {
                return false;
            }

            bool promotionZone =
                (isWhitePiece(piece) && toRow == 0) ||
                (isBlackPiece(piece) && toRow == 7);

            if (promotionZone && promotionPiece == EMPTY)
            {
                return false;
            }

            if (!promotionZone && promotionPiece != EMPTY)
            {
                return false;
            }

            if (!validMove)
            {
                return false;
            }

            if (promotionZone && !isValidPromotionPiece(promotionPiece, isWhitePiece(piece)))
            {
                return false;
            }
        }
        else if (piece == WHITE_KNIGHT || piece == BLACK_KNIGHT)
        {
            validMove = isValidKnightMove(fromRow, fromCol, toRow, toCol);
        }
        else if (piece == WHITE_BISHOP || piece == BLACK_BISHOP)
        {
            validMove = isValidBishopMove(fromRow, fromCol, toRow, toCol);
        }
        else if (piece == WHITE_ROOK || piece == BLACK_ROOK)
        {
            validMove = isValidRookMove(fromRow, fromCol, toRow, toCol);
        }
        else if (piece == WHITE_QUEEN || piece == BLACK_QUEEN)
        {
            validMove = isValidQueenMove(fromRow, fromCol, toRow, toCol);
        }
        else if (piece == WHITE_KING || piece == BLACK_KING)
        {
            if (isCastlingAttempt(piece, fromRow, fromCol, toRow, toCol))
            {
                return canCastle(fromRow, fromCol, toRow, toCol);
            }

            if (!isValidKingMove(fromRow, fromCol, toRow, toCol))
            {
                return false;
            }

            if (isSquareAttacked(toRow, toCol, !whiteToMove))
            {
                return false;
            }

            validMove = true;
        }

        if (!validMove)
        {
            return false;
        }

        if (wouldLeaveKingInCheck(fromRow, fromCol, toRow, toCol, promotionPiece, expectedEnPassant))
        {
            return false;
        }

        return true;
    };

    auto addIfLegal = [&](int toRow, int toCol, Piece promotionPiece = EMPTY, bool expectedEnPassant = false)
    {
        if (canTryDestination(toRow, toCol, promotionPiece, expectedEnPassant))
        {
            addMoveIfNeeded(toRow, toCol);
        }
    };

    if (piece == WHITE_PAWN || piece == BLACK_PAWN)
    {
        int direction = isWhitePiece(piece) ? -1 : 1;
        int startRow = isWhitePiece(piece) ? 6 : 1;
        int oneStepRow = fromRow + direction;
        int twoStepRow = fromRow + 2 * direction;

        Piece promotionOptions[4] = {
            isWhitePiece(piece) ? WHITE_QUEEN : BLACK_QUEEN,
            isWhitePiece(piece) ? WHITE_ROOK : BLACK_ROOK,
            isWhitePiece(piece) ? WHITE_BISHOP : BLACK_BISHOP,
            isWhitePiece(piece) ? WHITE_KNIGHT : BLACK_KNIGHT};

        if (isValidSquare(oneStepRow, fromCol) && board[oneStepRow][fromCol] == EMPTY)
        {
            bool promotionZone =
                (isWhitePiece(piece) && oneStepRow == 0) ||
                (isBlackPiece(piece) && oneStepRow == 7);

            if (promotionZone)
            {
                for (Piece promotionOption : promotionOptions)
                {
                    addIfLegal(oneStepRow, fromCol, promotionOption, false);
                }
            }
            else
            {
                addIfLegal(oneStepRow, fromCol, EMPTY, false);
            }
        }

        if (fromRow == startRow &&
            isValidSquare(twoStepRow, fromCol) &&
            isValidSquare(oneStepRow, fromCol) &&
            board[oneStepRow][fromCol] == EMPTY &&
            board[twoStepRow][fromCol] == EMPTY)
        {
            addIfLegal(twoStepRow, fromCol, EMPTY, false);
        }

        for (int deltaCol : {-1, 1})
        {
            int toRow = fromRow + direction;
            int toCol = fromCol + deltaCol;

            if (!isValidSquare(toRow, toCol))
            {
                continue;
            }

            bool promotionZone =
                (isWhitePiece(piece) && toRow == 0) ||
                (isBlackPiece(piece) && toRow == 7);

            if (board[toRow][toCol] != EMPTY)
            {
                if (!isOpponentPiece(board[toRow][toCol], isWhitePiece(piece)))
                {
                    continue;
                }

                if (promotionZone)
                {
                    for (Piece promotionOption : promotionOptions)
                    {
                        addIfLegal(toRow, toCol, promotionOption, false);
                    }
                }
                else
                {
                    addIfLegal(toRow, toCol, EMPTY, false);
                }
            }
            else if (hasEnPassantTarget && toRow == enPassantTargetRow && toCol == enPassantTargetCol)
            {
                addIfLegal(toRow, toCol, EMPTY, true);
            }
        }

        return moves;
    }

    if (piece == WHITE_KNIGHT || piece == BLACK_KNIGHT)
    {
        int deltas[8][2] = {
            {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
            {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};

        for (auto &delta : deltas)
        {
            addIfLegal(fromRow + delta[0], fromCol + delta[1]);
        }

        return moves;
    }

    if (piece == WHITE_BISHOP || piece == BLACK_BISHOP ||
        piece == WHITE_ROOK || piece == BLACK_ROOK ||
        piece == WHITE_QUEEN || piece == BLACK_QUEEN)
    {
        int directions[8][2] = {
            {1, 0}, {-1, 0}, {0, 1}, {0, -1},
            {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

        int startDirection = 0;
        int endDirection = 8;

        if (piece == WHITE_BISHOP || piece == BLACK_BISHOP)
        {
            startDirection = 4;
        }
        else if (piece == WHITE_ROOK || piece == BLACK_ROOK)
        {
            endDirection = 4;
        }

        for (int directionIndex = startDirection; directionIndex < endDirection; directionIndex++)
        {
            int dr = directions[directionIndex][0];
            int dc = directions[directionIndex][1];

            for (int step = 1; step < 8; step++)
            {
                int toRow = fromRow + dr * step;
                int toCol = fromCol + dc * step;

                if (!isValidSquare(toRow, toCol))
                {
                    break;
                }

                addIfLegal(toRow, toCol);

                if (board[toRow][toCol] != EMPTY)
                {
                    break;
                }
            }
        }

        return moves;
    }

    if (piece == WHITE_KING || piece == BLACK_KING)
    {
        for (int dr = -1; dr <= 1; dr++)
        {
            for (int dc = -1; dc <= 1; dc++)
            {
                if (dr == 0 && dc == 0)
                {
                    continue;
                }

                addIfLegal(fromRow + dr, fromCol + dc);
            }
        }

        addIfLegal(fromRow, 6);
        addIfLegal(fromRow, 2);

        return moves;
    }

    return moves;
}


bool ChessBoard::hasAnyLegalMove(bool white) const
{
    for (int fromRow = 0; fromRow < 8; fromRow++)
    {
        for (int fromCol = 0; fromCol < 8; fromCol++)
        {
            Piece piece = board[fromRow][fromCol];
            if (piece == EMPTY)
            {
                continue;
            }
            if ((white && !isWhitePiece(piece)) || (!white && !isBlackPiece(piece)))
            {
                continue;
            }
            auto canTryDestination = [&](int toRow, int toCol, Piece promotionPiece, bool enPassant)
            {
                if (!isValidSquare(toRow, toCol))
                {
                    return false;
                }
                Piece destination = board[toRow][toCol];
                if (isSameColor(destination, piece))
                {
                    return false;
                }
                bool isCapture = false;
                bool isEP = false;
                bool validMove = false;
                if (piece == WHITE_PAWN || piece == BLACK_PAWN)
                {
                    validMove = isValidPawnMove(fromRow, fromCol, toRow, toCol, isCapture, isEP);
                    if (isEP != enPassant)
                    {
                        return false;
                    }
                    bool promotionZone = (isWhitePiece(piece) && toRow == 0) || (isBlackPiece(piece) && toRow == 7);
                    if (promotionZone && promotionPiece == EMPTY)
                    {
                        return false;
                    }
                    if (!promotionZone && promotionPiece != EMPTY)
                    {
                        return false;
                    }
                    if (!validMove)
                    {
                        return false;
                    }
                    if (promotionZone && !isValidPromotionPiece(promotionPiece, isWhitePiece(piece)))
                    {
                        return false;
                    }
                }
                else if (piece == WHITE_KNIGHT || piece == BLACK_KNIGHT)
                {
                    validMove = isValidKnightMove(fromRow, fromCol, toRow, toCol);
                }
                else if (piece == WHITE_BISHOP || piece == BLACK_BISHOP)
                {
                    validMove = isValidBishopMove(fromRow, fromCol, toRow, toCol);
                }
                else if (piece == WHITE_ROOK || piece == BLACK_ROOK)
                {
                    validMove = isValidRookMove(fromRow, fromCol, toRow, toCol);
                }
                else if (piece == WHITE_QUEEN || piece == BLACK_QUEEN)
                {
                    validMove = isValidQueenMove(fromRow, fromCol, toRow, toCol);
                }
                else if (piece == WHITE_KING || piece == BLACK_KING)
                {
                    if (isCastlingAttempt(piece, fromRow, fromCol, toRow, toCol))
                    {
                        return canCastle(fromRow, fromCol, toRow, toCol);
                    }
                    if (!isValidKingMove(fromRow, fromCol, toRow, toCol))
                    {
                        return false;
                    }
                    if (isSquareAttacked(toRow, toCol, !white))
                    {
                        return false;
                    }
                    validMove = true;
                }
                if (!validMove)
                {
                    return false;
                }
                if (wouldLeaveKingInCheck(fromRow, fromCol, toRow, toCol, promotionPiece, enPassant))
                {
                    return false;
                }
                return true;
            };
            if (piece == WHITE_PAWN || piece == BLACK_PAWN)
            {
                int direction = isWhitePiece(piece) ? -1 : 1;
                int oneStepRow = fromRow + direction;
                if (isValidSquare(oneStepRow, fromCol) && board[oneStepRow][fromCol] == EMPTY)
                {
                    bool promotionZone = (isWhitePiece(piece) && oneStepRow == 0) || (isBlackPiece(piece) && oneStepRow == 7);
                    if (!promotionZone)
                    {
                        if (canTryDestination(oneStepRow, fromCol, EMPTY, false))
                        {
                            return true;
                        }
                    }
                    else
                    {
                        Piece options[4] = {isWhitePiece(piece) ? WHITE_QUEEN : BLACK_QUEEN,
                                            isWhitePiece(piece) ? WHITE_ROOK : BLACK_ROOK,
                                            isWhitePiece(piece) ? WHITE_BISHOP : BLACK_BISHOP,
                                            isWhitePiece(piece) ? WHITE_KNIGHT : BLACK_KNIGHT};
                        for (Piece opt : options)
                        {
                            if (canTryDestination(oneStepRow, fromCol, opt, false))
                            {
                                return true;
                            }
                        }
                    }
                }
                int startRow = isWhitePiece(piece) ? 6 : 1;
                int twoStepRow = fromRow + 2 * direction;
                if (fromRow == startRow && isValidSquare(twoStepRow, fromCol) && board[oneStepRow][fromCol] == EMPTY && board[twoStepRow][fromCol] == EMPTY)
                {
                    if (canTryDestination(twoStepRow, fromCol, EMPTY, false))
                    {
                        return true;
                    }
                }
                for (int deltaCol : {-1, 1})
                {
                    int toCol = fromCol + deltaCol;
                    int toRow = fromRow + direction;
                    if (!isValidSquare(toRow, toCol))
                    {
                        continue;
                    }
                    if (board[toRow][toCol] != EMPTY)
                    {
                        if (!isOpponentPiece(board[toRow][toCol], white))
                        {
                            continue;
                        }
                        bool promotionZone = (isWhitePiece(piece) && toRow == 0) || (isBlackPiece(piece) && toRow == 7);
                        if (!promotionZone)
                        {
                            if (canTryDestination(toRow, toCol, EMPTY, false))
                            {
                                return true;
                            }
                        }
                        else
                        {
                            Piece options[4] = {isWhitePiece(piece) ? WHITE_QUEEN : BLACK_QUEEN,
                                                isWhitePiece(piece) ? WHITE_ROOK : BLACK_ROOK,
                                                isWhitePiece(piece) ? WHITE_BISHOP : BLACK_BISHOP,
                                                isWhitePiece(piece) ? WHITE_KNIGHT : BLACK_KNIGHT};
                            for (Piece opt : options)
                            {
                                if (canTryDestination(toRow, toCol, opt, false))
                                {
                                    return true;
                                }
                            }
                        }
                    }
                    else if (hasEnPassantTarget && toRow == enPassantTargetRow && toCol == enPassantTargetCol)
                    {
                        if (canTryDestination(toRow, toCol, EMPTY, true))
                        {
                            return true;
                        }
                    }
                }
                continue;
            }
            if (piece == WHITE_KNIGHT || piece == BLACK_KNIGHT)
            {
                int deltas[8][2] = {{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};
                for (auto &delta : deltas)
                {
                    int toRow = fromRow + delta[0];
                    int toCol = fromCol + delta[1];
                    if (canTryDestination(toRow, toCol, EMPTY, false))
                    {
                        return true;
                    }
                }
                continue;
            }
            if (piece == WHITE_BISHOP || piece == BLACK_BISHOP)
            {
                int directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
                for (auto &delta : directions)
                {
                    for (int step = 1; step < 8; step++)
                    {
                        int toRow = fromRow + delta[0] * step;
                        int toCol = fromCol + delta[1] * step;
                        if (!isValidSquare(toRow, toCol))
                        {
                            break;
                        }
                        if (canTryDestination(toRow, toCol, EMPTY, false))
                        {
                            return true;
                        }
                        if (board[toRow][toCol] != EMPTY)
                        {
                            break;
                        }
                    }
                }
                continue;
            }
            if (piece == WHITE_ROOK || piece == BLACK_ROOK)
            {
                int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
                for (auto &delta : directions)
                {
                    for (int step = 1; step < 8; step++)
                    {
                        int toRow = fromRow + delta[0] * step;
                        int toCol = fromCol + delta[1] * step;
                        if (!isValidSquare(toRow, toCol))
                        {
                            break;
                        }
                        if (canTryDestination(toRow, toCol, EMPTY, false))
                        {
                            return true;
                        }
                        if (board[toRow][toCol] != EMPTY)
                        {
                            break;
                        }
                    }
                }
                continue;
            }
            if (piece == WHITE_QUEEN || piece == BLACK_QUEEN)
            {
                int directions[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
                for (auto &delta : directions)
                {
                    for (int step = 1; step < 8; step++)
                    {
                        int toRow = fromRow + delta[0] * step;
                        int toCol = fromCol + delta[1] * step;
                        if (!isValidSquare(toRow, toCol))
                        {
                            break;
                        }
                        if (canTryDestination(toRow, toCol, EMPTY, false))
                        {
                            return true;
                        }
                        if (board[toRow][toCol] != EMPTY)
                        {
                            break;
                        }
                    }
                }
                continue;
            }
            if (piece == WHITE_KING || piece == BLACK_KING)
            {
                for (int dr = -1; dr <= 1; dr++)
                {
                    for (int dc = -1; dc <= 1; dc++)
                    {
                        if (dr == 0 && dc == 0)
                        {
                            continue;
                        }
                        int toRow = fromRow + dr;
                        int toCol = fromCol + dc;
                        if (canTryDestination(toRow, toCol, EMPTY, false))
                        {
                            return true;
                        }
                    }
                }
                if (canCastle(fromRow, fromCol, fromRow, 6) || canCastle(fromRow, fromCol, fromRow, 2))
                {
                    return true;
                }
                continue;
            }
        }
    }
    return false;
}

MoveResult ChessBoard::processMove(const std::string &from, const std::string &to, Piece promotionPiece)
{
    MoveResult result;
    result.from = from;
    result.to = to;
    result.valid = false;
    result.moveType = MoveType::INVALID;
    result.gameStatus = gameStatus;
    if (gameStatus == GameStatus::CHECKMATE || gameStatus == GameStatus::STALEMATE || gameStatus == GameStatus::DRAW)
    {
        result.message = "La partida ya termino.";
        return result;
    }
    int fromRow = 0;
    int fromCol = 0;
    int toRow = 0;
    int toCol = 0;
    if (!coordToPos(from, fromRow, fromCol))
    {
        result.message = "Casilla de origen invalida.";
        return result;
    }
    if (!coordToPos(to, toRow, toCol))
    {
        result.message = "Casilla de destino invalida.";
        return result;
    }
    Piece movedPiece = board[fromRow][fromCol];
    Piece targetPiece = board[toRow][toCol];
    result.movedPiece = movedPiece;
    result.capturedPiece = targetPiece;
    if (movedPiece == EMPTY)
    {
        result.message = "No hay pieza en la casilla de origen.";
        return result;
    }
    if (!isCurrentPlayerPiece(movedPiece))
    {
        result.message = "La pieza seleccionada no pertenece al jugador actual.";
        return result;
    }
    if (isSameColor(targetPiece, movedPiece))
    {
        result.message = "No se puede capturar una pieza propia.";
        return result;
    }
    if (isCastlingAttempt(movedPiece, fromRow, fromCol, toRow, toCol))
    {
        return attemptCastling(fromRow, fromCol, toRow, toCol);
    }
    bool isCapture = false;
    bool isEnPassant = false;
    bool moveValid = false;
    bool promotionNeeded = false;
    bool isPromotionMove = false;
    if (movedPiece == WHITE_PAWN || movedPiece == BLACK_PAWN)
    {
        moveValid = isValidPawnMove(fromRow, fromCol, toRow, toCol, isCapture, isEnPassant);
        promotionNeeded = (isWhitePiece(movedPiece) && toRow == 0) || (isBlackPiece(movedPiece) && toRow == 7);
        if (!moveValid)
        {
            result.message = "La pieza no puede moverse de esa forma.";
            return result;
        }
        if (promotionNeeded && promotionPiece == EMPTY)
        {
            result.requiresPromotion = true;
            result.message = "Este movimiento requiere promocion.";
            return result;
        }
        if (!promotionNeeded && promotionPiece != EMPTY)
        {
            result.message = "La promocion solo es valida en la ultima fila.";
            return result;
        }
        if (promotionNeeded && !isValidPromotionPiece(promotionPiece, isWhitePiece(movedPiece)))
        {
            result.message = "Pieza de promocion invalida.";
            return result;
        }
        isPromotionMove = promotionNeeded;
    }
    else
    {
        if (promotionPiece != EMPTY)
        {
            result.message = "La promocion solo es valida para peones que llegan a la ultima fila.";
            return result;
        }
        if (movedPiece == WHITE_KNIGHT || movedPiece == BLACK_KNIGHT)
        {
            moveValid = isValidKnightMove(fromRow, fromCol, toRow, toCol);
        }
        else if (movedPiece == WHITE_BISHOP || movedPiece == BLACK_BISHOP)
        {
            moveValid = isValidBishopMove(fromRow, fromCol, toRow, toCol);
        }
        else if (movedPiece == WHITE_ROOK || movedPiece == BLACK_ROOK)
        {
            moveValid = isValidRookMove(fromRow, fromCol, toRow, toCol);
        }
        else if (movedPiece == WHITE_QUEEN || movedPiece == BLACK_QUEEN)
        {
            moveValid = isValidQueenMove(fromRow, fromCol, toRow, toCol);
        }
        else if (movedPiece == WHITE_KING || movedPiece == BLACK_KING)
        {
            if (!isValidKingMove(fromRow, fromCol, toRow, toCol))
            {
                result.message = "La pieza no puede moverse de esa forma.";
                return result;
            }
            if (isSquareAttacked(toRow, toCol, !whiteToMove))
            {
                result.message = "El rey no puede moverse a una casilla atacada.";
                return result;
            }
            moveValid = true;
        }
        if (!moveValid)
        {
            result.message = "La pieza no puede moverse de esa forma.";
            return result;
        }
    }
    if (wouldLeaveKingInCheck(fromRow, fromCol, toRow, toCol, promotionPiece, isEnPassant))
    {
        result.message = "El movimiento dejaria al rey propio en jaque.";
        return result;
    }
    Piece actualCapturedPiece = targetPiece;
    if (isEnPassant)
    {
        int captureRow = fromRow;
        int captureCol = toCol;
        actualCapturedPiece = board[captureRow][captureCol];
        board[captureRow][captureCol] = EMPTY;
    }
    board[fromRow][fromCol] = EMPTY;
    if (isPromotionMove)
    {
        board[toRow][toCol] = promotionPiece;
    }
    else
    {
        board[toRow][toCol] = movedPiece;
    }
    updateCastlingFlagsForMovedPiece(movedPiece, fromRow, fromCol);
    if (actualCapturedPiece != EMPTY)
    {
        if (!isEnPassant)
        {
            updateCastlingFlagsForCapturedPiece(actualCapturedPiece, toRow, toCol);
        }
        else
        {
            int captureRow = fromRow;
            int captureCol = toCol;
            updateCastlingFlagsForCapturedPiece(actualCapturedPiece, captureRow, captureCol);
        }
    }
    if (movedPiece == WHITE_PAWN || movedPiece == BLACK_PAWN)
    {
        int direction = isWhitePiece(movedPiece) ? -1 : 1;
        int startRow = isWhitePiece(movedPiece) ? 6 : 1;
        if (fromRow == startRow && toRow == fromRow + 2 * direction)
        {
            setEnPassantTarget(fromRow + direction, fromCol);
        }
        else
        {
            clearEnPassant();
        }
    }
    else
    {
        clearEnPassant();
    }
    whiteToMove = !whiteToMove;
    updateGameStatusAfterMove();
    result.valid = true;
    result.capturedPiece = actualCapturedPiece;
    result.movedPiece = isPromotionMove ? promotionPiece : movedPiece;
    if (isEnPassant)
    {
        result.moveType = MoveType::EN_PASSANT;
        result.message = "En passant realizado.";
    }
    else if (isPromotionMove)
    {
        result.moveType = MoveType::PROMOTION;
        result.message = "Promocion realizada.";
    }
    else if (actualCapturedPiece != EMPTY)
    {
        result.moveType = MoveType::CAPTURE;
        result.message = "Captura realizada.";
    }
    else
    {
        result.moveType = MoveType::NORMAL;
        result.message = "Movimiento valido.";
    }
    
    bool deliveredCheck = isKingInCheck(whiteToMove);
    result.isCheck = deliveredCheck || (gameStatus == GameStatus::CHECKMATE);
    result.isCheckmate = (gameStatus == GameStatus::CHECKMATE);
    result.isStalemate = (gameStatus == GameStatus::STALEMATE);
    result.requiresPromotion = false;
    result.gameStatus = gameStatus;
    if (result.isCheckmate)
    {
        result.message = "Jaque mate.";
    }
    else if (result.isStalemate)
    {
        result.message = "Ahogado.";
    }
    else if (result.isCheck)
    {
        result.message += " Jaque.";
    }
    moveHistory.push_back(result);
    return result;
}

void ChessBoard::updateGameStatusAfterMove()
{
    bool sideToMoveWhite = whiteToMove;
    bool sideInCheck = isKingInCheck(sideToMoveWhite);
    bool sideHasMove = hasAnyLegalMove(sideToMoveWhite);

    if (!sideHasMove)
    {
        if (sideInCheck)
        {
            gameStatus = GameStatus::CHECKMATE;
        }
        else
        {
            gameStatus = GameStatus::STALEMATE;
        }
        return;
    }

    if (sideInCheck)
    {
        gameStatus = sideToMoveWhite ? GameStatus::WHITE_IN_CHECK : GameStatus::BLACK_IN_CHECK;
        return;
    }

    gameStatus = whiteToMove ? GameStatus::WHITE_TO_MOVE : GameStatus::BLACK_TO_MOVE;
}

const std::vector<MoveResult> &ChessBoard::getMoveHistory() const
{
    return moveHistory;
}