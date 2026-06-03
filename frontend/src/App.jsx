import { useEffect, useMemo, useRef, useState } from "react";
import { Chess } from "chess.js";

import ChessBoard from "./components/ChessBoard";
import PlayerPanel from "./components/PlayerPanel";
import MovePanel from "./components/MovePanel";
import BluetoothPanel from "./components/BluetoothPanel";

import { createBoardFromChess } from "./lib/initialBoard";

const INITIAL_TIME_SECONDS = 10 * 60;

function getStatusFromGame(game, moveResult) {
  if (game.isCheckmate()) {
    return `Jaque mate. Movimiento final: ${moveResult.san}`;
  }

  if (game.isStalemate()) {
    return "Ahogado. La partida termino en empate.";
  }

  if (game.isDraw()) {
    return "La partida termino en tablas.";
  }

  if (game.isCheck()) {
    return `Jaque. Movimiento: ${moveResult.san}`;
  }

  return `Movimiento valido: ${moveResult.san}`;
}

function normalizePromotion(promotion) {
  if (!promotion) return undefined;

  const value = promotion.toLowerCase();

  if (["q", "r", "b", "n"].includes(value)) {
    return value;
  }

  return undefined;
}

export default function App() {
  const chessRef = useRef(new Chess());

  const [board, setBoard] = useState(() => createBoardFromChess(chessRef.current));
  const [currentTurn, setCurrentTurn] = useState("w");

  const [whiteSeconds, setWhiteSeconds] = useState(INITIAL_TIME_SECONDS);
  const [blackSeconds, setBlackSeconds] = useState(INITIAL_TIME_SECONDS);

  const [gameStarted, setGameStarted] = useState(false);
  const [pendingMove, setPendingMove] = useState(null);
  const [lastMove, setLastMove] = useState(null);

  const [capturedByWhite, setCapturedByWhite] = useState([]);
  const [capturedByBlack, setCapturedByBlack] = useState([]);

  const [history, setHistory] = useState([]);
  const [status, setStatus] = useState("Frontend listo. Ingrese un movimiento o conecte BLE.");

  const turnLabel = useMemo(() => {
    return currentTurn === "w" ? "Blancas" : "Negras";
  }, [currentTurn]);

  useEffect(() => {
    if (!gameStarted) return;

    if (chessRef.current.isGameOver()) return;

    const interval = setInterval(() => {
      if (currentTurn === "w") {
        setWhiteSeconds((value) => Math.max(0, value - 1));
      } else {
        setBlackSeconds((value) => Math.max(0, value - 1));
      }
    }, 1000);

    return () => clearInterval(interval);
  }, [gameStarted, currentTurn]);

  function handlePendingMove(move, message) {
    setPendingMove(move);
    setStatus(message);
  }

  function handleVerifyMove() {
    if (!pendingMove) {
      setStatus("No hay movimiento pendiente.");
      return;
    }

    const game = chessRef.current;

    if (game.isGameOver()) {
      setStatus("La partida ya termino. Reinicie para jugar de nuevo.");
      return;
    }

    let moveResult = null;

    try {
      moveResult = game.move({
        from: pendingMove.from,
        to: pendingMove.to,
        promotion: normalizePromotion(pendingMove.promotion)
      });
    } catch (error) {
      setStatus(
        `Movimiento invalido: ${pendingMove.from} -> ${pendingMove.to}. ${
          pendingMove.promotion ? "" : "Si es promocion, use Q, R, B o N."
        }`
      );
      return;
    }

    if (!moveResult) {
      setStatus(`Movimiento invalido: ${pendingMove.from} -> ${pendingMove.to}.`);
      return;
    }

    setBoard(createBoardFromChess(game));
    setLastMove({
      from: moveResult.from,
      to: moveResult.to,
      kind: pendingMove.kind,
      promotion: pendingMove.promotion
    });

    setGameStarted(true);

    if (moveResult.captured) {
      const capturedColor = moveResult.color === "w" ? "b" : "w";
      const capturedPiece = `${capturedColor}${moveResult.captured.toUpperCase()}`;

      if (moveResult.color === "w") {
        setCapturedByWhite((pieces) => [...pieces, capturedPiece]);
      } else {
        setCapturedByBlack((pieces) => [...pieces, capturedPiece]);
      }
    }

    setHistory((items) => [
      ...items,
      `${moveResult.color === "w" ? "White" : "Black"}: ${moveResult.san}`
    ]);

    setCurrentTurn(game.turn());
    setStatus(getStatusFromGame(game, moveResult));
    setPendingMove(null);
  }

  function handleReset() {
    chessRef.current = new Chess();

    setBoard(createBoardFromChess(chessRef.current));
    setCurrentTurn("w");
    setWhiteSeconds(INITIAL_TIME_SECONDS);
    setBlackSeconds(INITIAL_TIME_SECONDS);
    setGameStarted(false);
    setPendingMove(null);
    setLastMove(null);
    setCapturedByWhite([]);
    setCapturedByBlack([]);
    setHistory([]);
    setStatus("Partida reiniciada.");
  }

  return (
    <main className="app-shell">
      <section className="left-panel">
        <PlayerPanel
          name="Black"
          seconds={blackSeconds}
          active={gameStarted && currentTurn === "b"}
          capturedPieces={capturedByBlack}
        />

        <MovePanel
          pendingMove={pendingMove}
          onPendingMove={handlePendingMove}
          onVerifyMove={handleVerifyMove}
          onReset={handleReset}
        />

        <PlayerPanel
          name="White"
          seconds={whiteSeconds}
          active={gameStarted && currentTurn === "w"}
          capturedPieces={capturedByWhite}
        />

        <BluetoothPanel
          onBleMove={handlePendingMove}
          onStatus={setStatus}
        />

        <section className="status-card">
          <h3>Estado</h3>
          <p>{status}</p>
          <p>
            Turno: <strong>{turnLabel}</strong>
          </p>
          {pendingMove && (
            <p>
              Pendiente: <strong>{pendingMove.from} → {pendingMove.to}</strong>
            </p>
          )}
        </section>

        <section className="history-card">
          <h3>Historial</h3>
          {history.length === 0 ? (
            <p className="muted">Sin movimientos.</p>
          ) : (
            <ol>
              {history.map((item, index) => (
                <li key={`${item}-${index}`}>{item}</li>
              ))}
            </ol>
          )}
        </section>
      </section>

      <section className="board-panel">
        <ChessBoard board={board} lastMove={lastMove} />
      </section>
    </main>
  );
}