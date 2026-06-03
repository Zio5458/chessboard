import { useState } from "react";
import { parseMoveText } from "../lib/chessUtils";

export default function MovePanel({
  pendingMove,
  onPendingMove,
  onVerifyMove,
  onReset
}) {
  const [text, setText] = useState("");

  function handleSubmit(event) {
    event.preventDefault();

    const parsed = parseMoveText(text);

    if (!parsed.ok) {
      onPendingMove(null, parsed.error);
      return;
    }

    onPendingMove(parsed.move, `Movimiento pendiente: ${parsed.move.from} -> ${parsed.move.to}`);
    setText("");
  }

  return (
    <section className="move-panel">
      <form onSubmit={handleSubmit} className="move-form">
        <input
          value={text}
          onChange={(event) => setText(event.target.value)}
          placeholder="e2 e4 o MOVE e2 e4 NORMAL"
        />
        <button type="submit">Cargar</button>
      </form>

      <button
        className="verify-button"
        type="button"
        disabled={!pendingMove}
        onClick={onVerifyMove}
      >
        Verificar Movimiento
      </button>

      <button className="reset-button" type="button" onClick={onReset}>
        Reiniciar
      </button>
    </section>
  );
}