import { pieceSymbols } from "../lib/initialBoard";

export default function CapturedPieces({ pieces }) {
  return (
    <div className="captured-pieces">
      {pieces.length === 0 ? (
        <span className="empty-captures">Sin capturas</span>
      ) : (
        pieces.map((piece, index) => (
          <span key={`${piece}-${index}`} className="captured-piece">
            {pieceSymbols[piece] ?? "?"}
          </span>
        ))
      )}
    </div>
  );
}