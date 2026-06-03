import { files, pieceSymbols } from "../lib/initialBoard";

export default function ChessBoard({ board, lastMove }) {
  const ranksDescending = [8, 7, 6, 5, 4, 3, 2, 1];

  function isLastMoveSquare(square) {
    return lastMove && (lastMove.from === square || lastMove.to === square);
  }

  return (
    <div className="board-wrapper">
      <div className="chess-board">
        {ranksDescending.map((rank) =>
          files.map((file, fileIndex) => {
            const square = `${file}${rank}`;
            const rankIndex = rank - 1;

            // a1 debe ser casilla oscura.
            const isLight = (fileIndex + rankIndex) % 2 === 1;

            const piece = board[square];
            const pieceColorClass =
              piece?.[0] === "w"
                ? "white-piece"
                : piece?.[0] === "b"
                  ? "black-piece"
                  : "";

            return (
              <div
                key={square}
                className={[
                  "square",
                  isLight ? "light" : "dark",
                  isLastMoveSquare(square) ? "last-move" : ""
                ].join(" ")}
              >
                <span className="square-label">{square}</span>
                <span className={`piece ${pieceColorClass}`}>
                  {pieceSymbols[piece] ?? ""}
                </span>
              </div>
            );
          })
        )}
      </div>
    </div>
  );
}