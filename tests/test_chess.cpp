#include "../ChessBoard.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace {

int testsRun = 0;
int testsFailed = 0;
std::string currentTest;

void startTest(const std::string& name) {
    currentTest = name;
    std::cout << "\n[TEST] " << name << "\n";
}

void fail(const std::string& message, int line) {
    testsFailed++;
    std::cout << "  FAIL line " << line << ": " << message << "\n";
}

#define EXPECT_TRUE(expr) do { testsRun++; if (!(expr)) fail("Expected true: " #expr, __LINE__); } while (0)
#define EXPECT_FALSE(expr) do { testsRun++; if ((expr)) fail("Expected false: " #expr, __LINE__); } while (0)
#define EXPECT_EQ(actual, expected) do { testsRun++; auto a = (actual); auto e = (expected); if (!(a == e)) fail(std::string("Expected equality: ") + #actual + " == " + #expected, __LINE__); } while (0)
#define EXPECT_NE(actual, expected) do { testsRun++; auto a = (actual); auto e = (expected); if (a == e) fail(std::string("Expected inequality: ") + #actual + " != " + #expected, __LINE__); } while (0)

bool contains(const std::vector<std::string>& values, const std::string& target) {
    return std::find(values.begin(), values.end(), target) != values.end();
}

std::vector<std::string> initialOccupiedSquares() {
    std::vector<std::string> squares;
    const char files[] = {'a','b','c','d','e','f','g','h'};
    const char ranks[] = {'1','2','7','8'};

    for (char rank : ranks) {
        for (char file : files) {
            std::string square;
            square.push_back(file);
            square.push_back(rank);
            squares.push_back(square);
        }
    }

    return squares;
}

void expectMoveValid(const MoveResult& result, MoveType expectedType, const std::string& label) {
    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.moveType, expectedType);
    if (!result.valid) {
        std::cout << "  Context: " << label << " -> " << result.message << "\n";
    }
}

void expectMoveInvalid(const MoveResult& result, const std::string& label) {
    EXPECT_FALSE(result.valid);
    EXPECT_EQ(result.moveType, MoveType::INVALID);
    if (result.valid) {
        std::cout << "  Context: " << label << " should be invalid\n";
    }
}

void testInitialBoard() {
    startTest("Initial board state");
    ChessBoard game;

    EXPECT_TRUE(game.isWhiteTurn());
    EXPECT_EQ(game.getGameStatus(), GameStatus::WHITE_TO_MOVE);
    EXPECT_EQ(game.getPieceAt("e1"), WHITE_KING);
    EXPECT_EQ(game.getPieceAt("e8"), BLACK_KING);
    EXPECT_EQ(game.getPieceAt("a1"), WHITE_ROOK);
    EXPECT_EQ(game.getPieceAt("h8"), BLACK_ROOK);
    EXPECT_EQ(game.getPieceAt("e2"), WHITE_PAWN);
    EXPECT_EQ(game.getPieceAt("e7"), BLACK_PAWN);
    EXPECT_EQ(game.getPieceAt("e4"), EMPTY);
    EXPECT_EQ(game.getExpectedOccupiedSquares().size(), static_cast<size_t>(32));
}

void testBasicValidAndInvalidMoves() {
    startTest("Basic valid and invalid moves");

    {
        ChessBoard game;
        MoveResult result = game.processMove("e2", "e4");
        expectMoveValid(result, MoveType::NORMAL, "e2 e4");
        EXPECT_EQ(game.getPieceAt("e2"), EMPTY);
        EXPECT_EQ(game.getPieceAt("e4"), WHITE_PAWN);
        EXPECT_FALSE(game.isWhiteTurn());
        EXPECT_EQ(game.getGameStatus(), GameStatus::BLACK_TO_MOVE);
    }

    {
        ChessBoard game;
        expectMoveInvalid(game.processMove("e2", "e5"), "e2 e5 illegal pawn move");
        EXPECT_TRUE(game.isWhiteTurn());
        EXPECT_EQ(game.getPieceAt("e2"), WHITE_PAWN);
        EXPECT_EQ(game.getPieceAt("e5"), EMPTY);
    }

    {
        ChessBoard game;
        expectMoveInvalid(game.processMove("e3", "e4"), "empty origin");
        expectMoveInvalid(game.processMove("e7", "e5"), "wrong turn piece");
        expectMoveInvalid(game.processMove("e1", "e2"), "capture own piece");
        expectMoveInvalid(game.processMove("a1", "a3"), "blocked rook path");
    }
}

void testKnightAndCapture() {
    startTest("Knight movement and normal capture");

    {
        ChessBoard game;
        MoveResult result = game.processMove("g1", "f3");
        expectMoveValid(result, MoveType::NORMAL, "g1 f3");
        EXPECT_EQ(game.getPieceAt("g1"), EMPTY);
        EXPECT_EQ(game.getPieceAt("f3"), WHITE_KNIGHT);
    }

    {
        ChessBoard game;
        expectMoveValid(game.processMove("e2", "e4"), MoveType::NORMAL, "e2 e4");
        expectMoveValid(game.processMove("d7", "d5"), MoveType::NORMAL, "d7 d5");
        MoveResult capture = game.processMove("e4", "d5");
        expectMoveValid(capture, MoveType::CAPTURE, "e4 d5 capture");
        EXPECT_EQ(capture.capturedPiece, BLACK_PAWN);
        EXPECT_EQ(game.getPieceAt("e4"), EMPTY);
        EXPECT_EQ(game.getPieceAt("d5"), WHITE_PAWN);
    }
}

void testFoolsMate() {
    startTest("Fool's mate checkmate detection");
    ChessBoard game;

    expectMoveValid(game.processMove("f2", "f3"), MoveType::NORMAL, "f2 f3");
    expectMoveValid(game.processMove("e7", "e5"), MoveType::NORMAL, "e7 e5");
    expectMoveValid(game.processMove("g2", "g4"), MoveType::NORMAL, "g2 g4");

    MoveResult mate = game.processMove("d8", "h4");
    EXPECT_TRUE(mate.valid);
    EXPECT_TRUE(mate.isCheck);
    EXPECT_TRUE(mate.isCheckmate);
    EXPECT_FALSE(mate.isStalemate);
    EXPECT_EQ(mate.gameStatus, GameStatus::CHECKMATE);
    EXPECT_EQ(game.getGameStatus(), GameStatus::CHECKMATE);

    MoveResult afterMate = game.processMove("a2", "a3");
    expectMoveInvalid(afterMate, "cannot move after checkmate");
}

void testCastling() {
    startTest("Castling rules");

    {
        ChessBoard game;
        expectMoveInvalid(game.processMove("e1", "g1"), "castling blocked at start");
    }

    {
        ChessBoard game;
        expectMoveValid(game.processMove("g1", "f3"), MoveType::NORMAL, "clear knight");
        expectMoveValid(game.processMove("a7", "a6"), MoveType::NORMAL, "black waiting move");
        expectMoveValid(game.processMove("e2", "e4"), MoveType::NORMAL, "clear bishop path");
        expectMoveValid(game.processMove("a6", "a5"), MoveType::NORMAL, "black waiting move");
        expectMoveValid(game.processMove("f1", "e2"), MoveType::NORMAL, "clear bishop");
        expectMoveValid(game.processMove("b7", "b6"), MoveType::NORMAL, "black waiting move");

        MoveResult castle = game.processMove("e1", "g1");
        expectMoveValid(castle, MoveType::CASTLING_KINGSIDE, "white kingside castle");
        EXPECT_EQ(game.getPieceAt("g1"), WHITE_KING);
        EXPECT_EQ(game.getPieceAt("f1"), WHITE_ROOK);
        EXPECT_EQ(game.getPieceAt("e1"), EMPTY);
        EXPECT_EQ(game.getPieceAt("h1"), EMPTY);
    }
}

void testPromotion() {
    startTest("Pawn promotion");
    ChessBoard game;

    expectMoveValid(game.processMove("a2", "a4"), MoveType::NORMAL, "a2 a4");
    expectMoveValid(game.processMove("h7", "h5"), MoveType::NORMAL, "h7 h5");
    expectMoveValid(game.processMove("a4", "a5"), MoveType::NORMAL, "a4 a5");
    expectMoveValid(game.processMove("h5", "h4"), MoveType::NORMAL, "h5 h4");
    expectMoveValid(game.processMove("a5", "a6"), MoveType::NORMAL, "a5 a6");
    expectMoveValid(game.processMove("g7", "g5"), MoveType::NORMAL, "g7 g5");
    expectMoveValid(game.processMove("a6", "b7"), MoveType::CAPTURE, "a6 b7 capture");
    expectMoveValid(game.processMove("g5", "g4"), MoveType::NORMAL, "g5 g4");

    MoveResult needsPromotion = game.processMove("b7", "a8");
    EXPECT_FALSE(needsPromotion.valid);
    EXPECT_TRUE(needsPromotion.requiresPromotion);
    EXPECT_EQ(game.getPieceAt("b7"), WHITE_PAWN);
    EXPECT_EQ(game.getPieceAt("a8"), BLACK_ROOK);

    MoveResult promotion = game.processMove("b7", "a8", WHITE_QUEEN);
    expectMoveValid(promotion, MoveType::PROMOTION, "b7 a8 Q");
    EXPECT_EQ(game.getPieceAt("a8"), WHITE_QUEEN);
    EXPECT_EQ(game.getPieceAt("b7"), EMPTY);
    EXPECT_EQ(promotion.capturedPiece, BLACK_ROOK);
}

void testEnPassant() {
    startTest("En passant");
    ChessBoard game;

    expectMoveValid(game.processMove("e2", "e4"), MoveType::NORMAL, "e2 e4");
    expectMoveValid(game.processMove("a7", "a6"), MoveType::NORMAL, "a7 a6");
    expectMoveValid(game.processMove("e4", "e5"), MoveType::NORMAL, "e4 e5");
    expectMoveValid(game.processMove("d7", "d5"), MoveType::NORMAL, "d7 d5");

    MoveResult ep = game.processMove("e5", "d6");
    expectMoveValid(ep, MoveType::EN_PASSANT, "e5 d6 en passant");
    EXPECT_EQ(ep.capturedPiece, BLACK_PAWN);
    EXPECT_EQ(game.getPieceAt("d6"), WHITE_PAWN);
    EXPECT_EQ(game.getPieceAt("d5"), EMPTY);
    EXPECT_EQ(game.getPieceAt("e5"), EMPTY);
}

void testPhysicalInitialComparison() {
    startTest("Physical initial-position comparison");
    ChessBoard game;

    std::vector<std::string> initial = initialOccupiedSquares();
    BoardComparison correct = game.comparePhysicalToInitialPosition(initial);
    EXPECT_TRUE(correct.matches);
    EXPECT_TRUE(correct.missingSquares.empty());
    EXPECT_TRUE(correct.extraSquares.empty());
    EXPECT_TRUE(game.isInitialPhysicalPositionCorrect(initial));

    std::vector<std::string> wrong = initial;
    wrong.erase(std::remove(wrong.begin(), wrong.end(), "e2"), wrong.end());
    wrong.push_back("e4");

    BoardComparison incorrect = game.comparePhysicalToInitialPosition(wrong);
    EXPECT_FALSE(incorrect.matches);
    EXPECT_TRUE(contains(incorrect.missingSquares, "e2"));
    EXPECT_TRUE(contains(incorrect.extraSquares, "e4"));

    BoardComparison empty = game.comparePhysicalToInitialPosition({});
    EXPECT_FALSE(empty.matches);
    EXPECT_EQ(empty.missingSquares.size(), static_cast<size_t>(32));
}

void testPhysicalLogicalComparison() {
    startTest("Physical logical-board comparison");
    ChessBoard game;

    BoardComparison initial = game.comparePhysicalToLogical(initialOccupiedSquares());
    EXPECT_TRUE(initial.matches);

    expectMoveValid(game.processMove("e2", "e4"), MoveType::NORMAL, "e2 e4");

    std::vector<std::string> physicalAfterMove = initialOccupiedSquares();
    physicalAfterMove.erase(std::remove(physicalAfterMove.begin(), physicalAfterMove.end(), "e2"), physicalAfterMove.end());
    physicalAfterMove.push_back("e4");

    BoardComparison afterMove = game.comparePhysicalToLogical(physicalAfterMove);
    EXPECT_TRUE(afterMove.matches);

    BoardComparison empty = game.comparePhysicalToLogical({});
    EXPECT_FALSE(empty.matches);
    EXPECT_NE(empty.missingSquares.size(), static_cast<size_t>(0));
}

} // namespace

int main() {
    testInitialBoard();
    testBasicValidAndInvalidMoves();
    testKnightAndCapture();
    testFoolsMate();
    testCastling();
    testPromotion();
    testEnPassant();
    testPhysicalInitialComparison();
    testPhysicalLogicalComparison();

    std::cout << "\n==============================\n";
    std::cout << "Assertions run: " << testsRun << "\n";
    std::cout << "Failures: " << testsFailed << "\n";
    std::cout << "==============================\n";

    return testsFailed == 0 ? 0 : 1;
}
