export const files = ["a", "b", "c", "d", "e", "f", "g", "h"];
export const ranks = ["1", "2", "3", "4", "5", "6", "7", "8"];

export const pieceSymbols = {
  wK: "♔",
  wQ: "♕",
  wR: "♖",
  wB: "♗",
  wN: "♘",
  wP: "♙",
  bK: "♚",
  bQ: "♛",
  bR: "♜",
  bB: "♝",
  bN: "♞",
  bP: "♟"
};

export function createInitialBoard() {
  const board = {};

  files.forEach((file) => {
    board[`${file}2`] = "wP";
    board[`${file}7`] = "bP";
  });

  board.a1 = "wR";
  board.b1 = "wN";
  board.c1 = "wB";
  board.d1 = "wQ";
  board.e1 = "wK";
  board.f1 = "wB";
  board.g1 = "wN";
  board.h1 = "wR";

  board.a8 = "bR";
  board.b8 = "bN";
  board.c8 = "bB";
  board.d8 = "bQ";
  board.e8 = "bK";
  board.f8 = "bB";
  board.g8 = "bN";
  board.h8 = "bR";

  return board;
}

export function getPieceColor(piece) {
  if (!piece) return null;
  return piece[0] === "w" ? "white" : "black";
}

export function getPieceName(piece) {
  if (!piece) return "empty";

  const names = {
    K: "king",
    Q: "queen",
    R: "rook",
    B: "bishop",
    N: "knight",
    P: "pawn"
  };

  return names[piece[1]] ?? "piece";
}

export function createBoardFromChess(chess) {
  const board = {};
  const chessBoard = chess.board();

  chessBoard.forEach((row, rowIndex) => {
    row.forEach((piece, fileIndex) => {
      if (!piece) return;

      const file = files[fileIndex];
      const rank = 8 - rowIndex;
      const square = `${file}${rank}`;

      board[square] = `${piece.color}${piece.type.toUpperCase()}`;
    });
  });

  return board;
}