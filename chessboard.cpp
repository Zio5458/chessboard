#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <cstring>

using namespace std;

enum Piece {
    EMPTY = 0,
    WHITE_PAWN = 1, WHITE_KNIGHT = 2, WHITE_BISHOP = 3, WHITE_ROOK = 4, WHITE_QUEEN = 5, WHITE_KING = 6,
    BLACK_PAWN = 7, BLACK_KNIGHT = 8, BLACK_BISHOP = 9, BLACK_ROOK = 10, BLACK_QUEEN = 11, BLACK_KING = 12
};

class ChessBoard {
private:
    Piece board[8][8];
    bool whiteToMove;
    int moveCount;
    bool whiteKingMoved;
    bool blackKingMoved;
    bool whiteKingsideRookMoved;
    bool whiteQueensideRookMoved;
    bool blackKingsideRookMoved;
    bool blackQueensideRookMoved;
    
public:
    ChessBoard() : whiteToMove(true), moveCount(0),
        whiteKingMoved(false), blackKingMoved(false),
        whiteKingsideRookMoved(false), whiteQueensideRookMoved(false),
        blackKingsideRookMoved(false), blackQueensideRookMoved(false) {
        initializeBoard();
    }
    
    void initializeBoard() {
        //Limpiar tablero
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                board[i][j] = EMPTY;
            }
        }
        
        //Posicion inicial de ajedrez
        //Piezas negras
        board[0][0] = BLACK_ROOK;   board[0][1] = BLACK_KNIGHT; board[0][2] = BLACK_BISHOP;
        board[0][3] = BLACK_QUEEN;  board[0][4] = BLACK_KING;   board[0][5] = BLACK_BISHOP;
        board[0][6] = BLACK_KNIGHT; board[0][7] = BLACK_ROOK;
        
        //Peones negros
        for (int i = 0; i < 8; i++) board[1][i] = BLACK_PAWN;
                
        //Peones blancos
        for (int i = 0; i < 8; i++) board[6][i] = WHITE_PAWN;
        
        //Piezas blancas
        board[7][0] = WHITE_ROOK;   board[7][1] = WHITE_KNIGHT; board[7][2] = WHITE_BISHOP;
        board[7][3] = WHITE_QUEEN;  board[7][4] = WHITE_KING;   board[7][5] = WHITE_BISHOP;
        board[7][6] = WHITE_KNIGHT; board[7][7] = WHITE_ROOK;
    }
    
    void displayBoard() {
        cout << "\n  a b c d e f g h\n";
        for (int i = 0; i < 8; i++) {
            cout << (8 - i) << " ";
            for (int j = 0; j < 8; j++) {
                cout << getPieceChar(board[i][j]) << " ";
            }
            cout << (8 - i) << "\n";
        }
        cout << "  a b c d e f g h\n";
        cout << "Turno: " << (whiteToMove ? "Blancas" : "Negras") << "\n\n";
    }
    
    char getPieceChar(Piece p) {
        switch(p) {
            case WHITE_PAWN:   return 'P';
            case WHITE_KNIGHT: return 'N';
            case WHITE_BISHOP: return 'B';
            case WHITE_ROOK:   return 'R';
            case WHITE_QUEEN:  return 'Q';
            case WHITE_KING:   return 'K';
            case BLACK_PAWN:   return 'p';
            case BLACK_KNIGHT: return 'n';
            case BLACK_BISHOP: return 'b';
            case BLACK_ROOK:   return 'r';
            case BLACK_QUEEN:  return 'q';
            case BLACK_KING:   return 'k';
            default:           return '.';
        }
    }
    
    bool isWhitePiece(Piece p) {
        return p >= WHITE_PAWN && p <= WHITE_KING;
    }
    
    bool isBlackPiece(Piece p) {
        return p >= BLACK_PAWN && p <= BLACK_KING;
    }
    
    bool isValidSquare(int row, int col) {
        return row >= 0 && row < 8 && col >= 0 && col < 8;
    }
    
    void coordToPos(string coord, int &row, int &col) {
        col = coord[0] - 'a';
        row = 8 - (coord[1] - '0');
    }
    
    bool canPieceMoveToSquare(Piece p, int fromRow, int fromCol, int toRow, int toCol) {
        if (!isValidSquare(toRow, toCol)) return false;
        if (fromRow == toRow && fromCol == toCol) return false;
        
        Piece targetPiece = board[toRow][toCol];
        
        //No capturar pieza propia
        if (isWhitePiece(p) && isWhitePiece(targetPiece)) return false;
        if (isBlackPiece(p) && isBlackPiece(targetPiece)) return false;
        
        int pieceName = (p <= WHITE_KING) ? p : (p - 6);
        
        switch(pieceName) {
            case WHITE_PAWN:
            case BLACK_PAWN:
                return isValidPawnMove(p, fromRow, fromCol, toRow, toCol);
            
            case WHITE_KNIGHT:
            case BLACK_KNIGHT:
                return isValidKnightMove(fromRow, fromCol, toRow, toCol);
            
            case WHITE_BISHOP:
            case BLACK_BISHOP:
                return isValidBishopMove(fromRow, fromCol, toRow, toCol);
            
            case WHITE_ROOK:
            case BLACK_ROOK:
                return isValidRookMove(fromRow, fromCol, toRow, toCol);
            
            case WHITE_QUEEN:
            case BLACK_QUEEN:
                return isValidQueenMove(fromRow, fromCol, toRow, toCol);
            
            case WHITE_KING:
            case BLACK_KING:
                return isValidKingMove(fromRow, fromCol, toRow, toCol);
            
            default:
                return false;
        }
    }
    
    bool isPathClear(int fromRow, int fromCol, int toRow, int toCol) {
        int deltaRow = (toRow > fromRow) ? 1 : (toRow < fromRow) ? -1 : 0;
        int deltaCol = (toCol > fromCol) ? 1 : (toCol < fromCol) ? -1 : 0;
        
        int currentRow = fromRow + deltaRow;
        int currentCol = fromCol + deltaCol;
        
        while (currentRow != toRow || currentCol != toCol) {
            if (board[currentRow][currentCol] != EMPTY) return false;
            currentRow += deltaRow;
            currentCol += deltaCol;
        }
        return true;
    }
    
    bool isValidPawnMove(Piece p, int fromRow, int fromCol, int toRow, int toCol) {
        bool isWhite = isWhitePiece(p);
        int direction = isWhite ? -1 : 1;
        int startRow = isWhite ? 6 : 1;
        
        //Movimiento hacia adelante
        if (fromCol == toCol) {
            if (toRow == fromRow + direction && board[toRow][toCol] == EMPTY) return true;
            if (fromRow == startRow && toRow == fromRow + 2 * direction && 
                board[fromRow + direction][fromCol] == EMPTY && board[toRow][toCol] == EMPTY) {
                return true;
            }
        }
        
        //Captura diagonal
        if (abs(toCol - fromCol) == 1 && toRow == fromRow + direction) {
            if (board[toRow][toCol] != EMPTY) {
                if ((isWhite && isBlackPiece(board[toRow][toCol])) ||
                    (!isWhite && isWhitePiece(board[toRow][toCol]))) {
                    return true;
                }
            }
        }
        
        return false;
    }
    
    bool isValidKnightMove(int fromRow, int fromCol, int toRow, int toCol) {
        int deltaRow = abs(toRow - fromRow);
        int deltaCol = abs(toCol - fromCol);
        return (deltaRow == 2 && deltaCol == 1) || (deltaRow == 1 && deltaCol == 2);
    }
    
    bool isValidBishopMove(int fromRow, int fromCol, int toRow, int toCol) {
        if (abs(toRow - fromRow) != abs(toCol - fromCol)) return false;
        return isPathClear(fromRow, fromCol, toRow, toCol);
    }
    
    bool isValidRookMove(int fromRow, int fromCol, int toRow, int toCol) {
        if (fromRow != toRow && fromCol != toCol) return false;
        return isPathClear(fromRow, fromCol, toRow, toCol);
    }
    
    bool isValidQueenMove(int fromRow, int fromCol, int toRow, int toCol) {
        return isValidRookMove(fromRow, fromCol, toRow, toCol) || 
               isValidBishopMove(fromRow, fromCol, toRow, toCol);
    }
    
    bool isValidKingMove(int fromRow, int fromCol, int toRow, int toCol) {
        return abs(toRow - fromRow) <= 1 && abs(toCol - fromCol) <= 1;
    }

    bool isSquareAttacked(int row, int col, bool byWhite) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                Piece attacker = board[i][j];
                if (attacker == EMPTY) continue;
                if (byWhite && !isWhitePiece(attacker)) continue;
                if (!byWhite && !isBlackPiece(attacker)) continue;
                if (canPieceMoveToSquare(attacker, i, j, row, col)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool isKingInCheck(bool white) {
        int kingRow = -1, kingCol = -1;
        Piece kingPiece = white ? WHITE_KING : BLACK_KING;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (board[i][j] == kingPiece) {
                    kingRow = i;
                    kingCol = j;
                    break;
                }
            }
            if (kingRow != -1) break;
        }
        if (kingRow == -1) return false;
        return isSquareAttacked(kingRow, kingCol, !white);
    }

    bool castle(string notation) {
        bool white = whiteToMove;
        int row = white ? 7 : 0;
        int kingCol = 4;
        int rookCol = (notation == "O-O") ? 7 : 0;
        int newKingCol = (notation == "O-O") ? 6 : 2;
        int newRookCol = (notation == "O-O") ? 5 : 3;

        if (white) {
            if (whiteKingMoved) {
                cout << "El rey blanco ya se movió.\n";
                return false;
            }
            if (notation == "O-O" && whiteKingsideRookMoved) {
                cout << "La torre blanca del flanco de rey ya se movió.\n";
                return false;
            }
            if (notation == "O-O-O" && whiteQueensideRookMoved) {
                cout << "La torre blanca del flanco de dama ya se movió.\n";
                return false;
            }
        } else {
            if (blackKingMoved) {
                cout << "El rey negro ya se movió.\n";
                return false;
            }
            if (notation == "O-O" && blackKingsideRookMoved) {
                cout << "La torre negra del flanco de rey ya se movió.\n";
                return false;
            }
            if (notation == "O-O-O" && blackQueensideRookMoved) {
                cout << "La torre negra del flanco de dama ya se movió.\n";
                return false;
            }
        }

        Piece kingPiece = white ? WHITE_KING : BLACK_KING;
        Piece rookPiece = white ? WHITE_ROOK : BLACK_ROOK;
        if (board[row][kingCol] != kingPiece || board[row][rookCol] != rookPiece) {
            cout << "Enroque inválido: rey o torre no están en la posición inicial.\n";
            return false;
        }

        if (isKingInCheck(white)) {
            cout << "No se puede enrocar mientras el rey está en jaque.\n";
            return false;
        }

        if (notation == "O-O") {
            if (board[row][5] != EMPTY || board[row][6] != EMPTY) {
                cout << "No se puede enrocar: las casillas necesarias no están vacías.\n";
                return false;
            }
            if (isSquareAttacked(row, 5, !white) || isSquareAttacked(row, 6, !white)) {
                cout << "No se puede enrocar: el rey pasaría por una casilla atacada.\n";
                return false;
            }
        } else {
            if (board[row][3] != EMPTY || board[row][2] != EMPTY || board[row][1] != EMPTY) {
                cout << "No se puede enrocar: las casillas necesarias no están vacías.\n";
                return false;
            }
            if (isSquareAttacked(row, 3, !white) || isSquareAttacked(row, 2, !white)) {
                cout << "No se puede enrocar: el rey pasaría por una casilla atacada.\n";
                return false;
            }
        }

        board[row][kingCol] = EMPTY;
        board[row][rookCol] = EMPTY;
        board[row][newKingCol] = kingPiece;
        board[row][newRookCol] = rookPiece;

        if (white) {
            whiteKingMoved = true;
            if (notation == "O-O") whiteKingsideRookMoved = true;
            else whiteQueensideRookMoved = true;
        } else {
            blackKingMoved = true;
            if (notation == "O-O") blackKingsideRookMoved = true;
            else blackQueensideRookMoved = true;
        }

        whiteToMove = !whiteToMove;
        moveCount++;
        cout << "Enroque realizado: " << notation << "\n";
        return true;
    }

    bool makeMove(string notation) {
        //enroque
        if (notation == "O-O" || notation == "O-O-O") {
            return castle(notation);
        }
        
        //Parse notacion algebraica
        int fromRow = -1, fromCol = -1, toRow = -1, toCol = -1;
        Piece pieceName = EMPTY;
        bool isCapture = false;
        
        //Detectar captura
        if (notation.find('x') != string::npos) {
            isCapture = true;
        }
        
        //Extraer posicion de destino
        if (notation.length() < 2) {
            cout << "Notación inválida.\n";
            return false;
        }
        
        string destCoord = notation.substr(notation.length() - 2);
        coordToPos(destCoord, toRow, toCol);
        
        if (!isValidSquare(toRow, toCol)) {
            cout << "Posicion de destino fuera del tablero.\n";
            return false;
        }
        
        //Determinar que pieza se mueve
        if (notation[0] >= 'a' && notation[0] <= 'h') {
            //Es un movimiento de peon
            pieceName = whiteToMove ? WHITE_PAWN : BLACK_PAWN;
            
            if (isCapture) {
                fromCol = notation[0] - 'a';
            } else {
                fromCol = toCol;
            }
        } else {
            //Es una pieza (N, B, R, Q, K)
            char piece = notation[0];
            switch(piece) {
                case 'N': pieceName = whiteToMove ? WHITE_KNIGHT : BLACK_KNIGHT; break;
                case 'B': pieceName = whiteToMove ? WHITE_BISHOP : BLACK_BISHOP; break;
                case 'R': pieceName = whiteToMove ? WHITE_ROOK : BLACK_ROOK; break;
                case 'Q': pieceName = whiteToMove ? WHITE_QUEEN : BLACK_QUEEN; break;
                case 'K': pieceName = whiteToMove ? WHITE_KING : BLACK_KING; break;
                default:
                    cout << "Pieza desconocida.\n";
                    return false;
            }
        }
        
        //Encontrar la pieza que se mueve
        if (fromRow == -1) {
            bool found = false;
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    if (board[i][j] == pieceName) {
                        if (canPieceMoveToSquare(pieceName, i, j, toRow, toCol)) {
                            if (!found || (fromCol != -1 && j == fromCol)) {
                                fromRow = i;
                                fromCol = j;
                                found = true;
                            }
                        }
                    }
                }
            }
            
            if (!found) {
                cout << "Movimiento ilegal. No hay pieza que pueda hacer este movimiento.\n";
                return false;
            }
        }
        
        //Actualizar flags de enroque si se mueve rey o torre
        if (board[fromRow][fromCol] == WHITE_KING) {
            whiteKingMoved = true;
        } else if (board[fromRow][fromCol] == BLACK_KING) {
            blackKingMoved = true;
        } else if (board[fromRow][fromCol] == WHITE_ROOK) {
            if (fromRow == 7 && fromCol == 0) whiteQueensideRookMoved = true;
            if (fromRow == 7 && fromCol == 7) whiteKingsideRookMoved = true;
        } else if (board[fromRow][fromCol] == BLACK_ROOK) {
            if (fromRow == 0 && fromCol == 0) blackQueensideRookMoved = true;
            if (fromRow == 0 && fromCol == 7) blackKingsideRookMoved = true;
        }

        //Realizar movimiento
        Piece capturedPiece = board[toRow][toCol];
        board[toRow][toCol] = board[fromRow][fromCol];
        board[fromRow][fromCol] = EMPTY;
        
        whiteToMove = !whiteToMove;
        moveCount++;
        
        cout << "Movimiento realizado: " << notation << "\n";
        if (capturedPiece != EMPTY) {
            cout << "(Pieza capturada: " << getPieceChar(capturedPiece) << ")\n";
        }
        
        return true;
    }
    
    void play() {
        string notation;
        
        cout << "\n=== AJEDREZ ===\n";
        cout << "Ingrese movimientos en notación algebraica (ej: e4, Nf3, exd5)\n";
        cout << "Escriba 'salir' para terminar.\n";
        
        while (true) {
            displayBoard();
            cout << "Ingrese movimiento: ";
            getline(cin, notation);
            
            if (notation == "salir" || notation == "exit" || notation == "quit") {
                cout << "¡Gracias por jugar!\n";
                break;
            }
            
            if (notation.empty()) {
                cout << "Por favor ingrese un movimiento válido.\n";
                continue;
            }
        
            notation.erase(remove_if(notation.begin(), notation.end(), ::isspace), notation.end());
            
            makeMove(notation);
        }
    }
};

int main() {
    ChessBoard game;
    game.play();
    return 0;
}
