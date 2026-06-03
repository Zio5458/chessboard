#include "../ChessBoard.h"

#include <iostream>
#include <string>
#include <vector>

static int assertionsRun = 0;
static int failures = 0;

void assertTrue(bool condition, const std::string& message) {
    assertionsRun++;

    if (!condition) {
        failures++;
        std::cerr << "[FAIL] " << message << std::endl;
    }
}

void assertEqualPiece(Piece actual, Piece expected, const std::string& message) {
    assertionsRun++;

    if (actual != expected) {
        failures++;
        std::cerr << "[FAIL] " << message << std::endl;
        std::cerr << "       Expected piece enum: " << static_cast<int>(expected)
                  << ", got: " << static_cast<int>(actual) << std::endl;
    }
}

MoveResult expectValidMove(
    ChessBoard& game,
    const std::string& from,
    const std::string& to,
    Piece promotionPiece = EMPTY
) {
    MoveResult result = game.processMove(from, to, promotionPiece);

    assertTrue(
        result.valid,
        "Expected valid move " + from + " -> " + to + ". Message: " + result.message
    );

    return result;
}

void testMorphyOperaGameFullSimulation() {
    std::cout << "\n[TEST] Full classic game simulation - Morphy Opera Game\n";

    ChessBoard game;

    // Morphy vs Duke/Count, Paris 1858
    //
    // 1. e4 e5
    // 2. Nf3 d6
    // 3. d4 Bg4
    // 4. dxe5 Bxf3
    // 5. Qxf3 dxe5
    // 6. Bc4 Nf6
    // 7. Qb3 Qe7
    // 8. Nc3 c6
    // 9. Bg5 b5
    // 10. Nxb5 cxb5
    // 11. Bxb5+ Nbd7
    // 12. O-O-O Rd8
    // 13. Rxd7 Rxd7
    // 14. Rd1 Qe6
    // 15. Bxd7+ Nxd7
    // 16. Qb8+ Nxb8
    // 17. Rd8#

    expectValidMove(game, "e2", "e4");
    expectValidMove(game, "e7", "e5");

    expectValidMove(game, "g1", "f3");
    expectValidMove(game, "d7", "d6");

    expectValidMove(game, "d2", "d4");
    expectValidMove(game, "c8", "g4");

    expectValidMove(game, "d4", "e5");
    expectValidMove(game, "g4", "f3");

    expectValidMove(game, "d1", "f3");
    expectValidMove(game, "d6", "e5");

    expectValidMove(game, "f1", "c4");
    expectValidMove(game, "g8", "f6");

    expectValidMove(game, "f3", "b3");
    expectValidMove(game, "d8", "e7");

    expectValidMove(game, "b1", "c3");
    expectValidMove(game, "c7", "c6");

    expectValidMove(game, "c1", "g5");
    expectValidMove(game, "b7", "b5");

    expectValidMove(game, "c3", "b5");
    expectValidMove(game, "c6", "b5");

    MoveResult bishopCheck = expectValidMove(game, "c4", "b5");
    assertTrue(bishopCheck.isCheck, "Move c4 -> b5 should give check.");

    expectValidMove(game, "b8", "d7");

    MoveResult castle = expectValidMove(game, "e1", "c1");
    assertTrue(
        castle.moveType == MoveType::CASTLING_QUEENSIDE,
        "Move e1 -> c1 should be queenside castling."
    );

    expectValidMove(game, "a8", "d8");

    expectValidMove(game, "d1", "d7");
    expectValidMove(game, "d8", "d7");

    expectValidMove(game, "h1", "d1");
    expectValidMove(game, "e7", "e6");

    MoveResult secondCheck = expectValidMove(game, "b5", "d7");
    assertTrue(secondCheck.isCheck, "Move b5 -> d7 should give check.");

    expectValidMove(game, "f6", "d7");

    MoveResult queenCheck = expectValidMove(game, "b3", "b8");
    assertTrue(queenCheck.isCheck, "Move b3 -> b8 should give check.");

    expectValidMove(game, "d7", "b8");

    MoveResult mate = expectValidMove(game, "d1", "d8");

    assertTrue(mate.isCheck, "Final move d1 -> d8 should give check.");
    assertTrue(mate.isCheckmate, "Final move d1 -> d8 should be checkmate.");
    assertTrue(
        game.getGameStatus() == GameStatus::CHECKMATE,
        "Game status should be CHECKMATE after final move."
    );

    assertEqualPiece(game.getPieceAt("d8"), WHITE_ROOK, "White rook should finish on d8.");
    assertEqualPiece(game.getPieceAt("e8"), BLACK_KING, "Black king should remain on e8.");
}

int main() {
    testMorphyOperaGameFullSimulation();

    std::cout << "\n==============================" << std::endl;
    std::cout << "Assertions run: " << assertionsRun << std::endl;
    std::cout << "Failures: " << failures << std::endl;
    std::cout << "==============================" << std::endl;

    if (failures > 0) {
        return 1;
    }

    return 0;
}