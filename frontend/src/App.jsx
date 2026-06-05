import { useEffect, useMemo, useState } from "react";

import ChessBoard from "./components/ChessBoard";
import PlayerPanel from "./components/PlayerPanel";
import MovePanel from "./components/MovePanel";
import BluetoothPanel from "./components/BluetoothPanel";

import { createInitialBoard } from "./lib/initialBoard";
import { getGameState, resetGame, verifyMove } from "./lib/backendClient";

const INITIAL_TIME_SECONDS = 10 * 60;

function getStatusMessage(result) {
  if (!result.valid) {
    return `Movimiento invalido: ${result.message}`;
  }

  if (result.isCheckmate) {
    return `Jaque mate. ${result.message}`;
  }

  if (result.isStalemate) {
    return `Ahogado. ${result.message}`;
  }

  if (result.isCheck) {
    return `Jaque. ${result.message}`;
  }

  return result.message;
}

export default function App() {
  const [board, setBoard] = useState(() => createInitialBoard());
  const [currentTurn, setCurrentTurn] = useState("w");

  const [whiteSeconds, setWhiteSeconds] = useState(INITIAL_TIME_SECONDS);
  const [blackSeconds, setBlackSeconds] = useState(INITIAL_TIME_SECONDS);

  const [gameStarted, setGameStarted] = useState(false);
  const [pendingMove, setPendingMove] = useState(null);
  const [lastMove, setLastMove] = useState(null);

  const [capturedByWhite, setCapturedByWhite] = useState([]);
  const [capturedByBlack, setCapturedByBlack] = useState([]);

  const [history, setHistory] = useState([]);
  const [status, setStatus] = useState("Conectando con backend C++...");

  const turnLabel = useMemo(() => {
    return currentTurn === "w" ? "Blancas" : "Negras";
  }, [currentTurn]);

  useEffect(() => {
    async function initializeBackend() {
      try {
        const state = await resetGame();

        if (state.board) {
          setBoard(state.board);
        }

        if (state.turn) {
          setCurrentTurn(state.turn);
        }

        setStatus("Backend C++ conectado. Ingrese un movimiento o conecte BLE.");
      } catch (error) {
        setStatus(`No se pudo conectar con backend C++: ${error.message}`);
      }
    }

    initializeBackend();
  }, []);

  useEffect(() => {
    if (!gameStarted) return;

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

  async function handleVerifyMove() {
    if (!pendingMove) {
      setStatus("No hay movimiento pendiente.");
      return;
    }

    let result;

    try {
      result = await verifyMove(pendingMove);
    } catch (error) {
      setStatus(`Error consultando backend C++: ${error.message}`);
      return;
    }

    if (!result.valid) {
      setStatus(getStatusMessage(result));
      return;
    }

    setBoard(result.board);
    setCurrentTurn(result.turn);
    setLastMove({
      from: result.from,
      to: result.to,
      kind: result.moveType,
      promotion: pendingMove.promotion
    });

    setGameStarted(true);

    if (result.capturedPiece) {
      if (result.movedPiece?.[0] === "w") {
        setCapturedByWhite((pieces) => [...pieces, result.capturedPiece]);
      } else if (result.movedPiece?.[0] === "b") {
        setCapturedByBlack((pieces) => [...pieces, result.capturedPiece]);
      }
    }

    setHistory((items) => [
      ...items,
      `${result.movedPiece?.[0] === "w" ? "White" : "Black"}: ${result.from} -> ${result.to} ${result.moveType}`
    ]);

    setStatus(getStatusMessage(result));
    setPendingMove(null);
  }

  async function handleReset() {
    try {
      const state = await resetGame();

      setBoard(state.board ?? createInitialBoard());
      setCurrentTurn(state.turn ?? "w");
      setWhiteSeconds(INITIAL_TIME_SECONDS);
      setBlackSeconds(INITIAL_TIME_SECONDS);
      setGameStarted(false);
      setPendingMove(null);
      setLastMove(null);
      setCapturedByWhite([]);
      setCapturedByBlack([]);
      setHistory([]);
      setStatus("Partida reiniciada desde backend C++.");
    } catch (error) {
      setStatus(`No se pudo reiniciar backend C++: ${error.message}`);
    }
  }

  async function handleRefreshState() {
    try {
      const state = await getGameState();

      setBoard(state.board ?? createInitialBoard());
      setCurrentTurn(state.turn ?? "w");
      setStatus("Estado actualizado desde backend C++.");
    } catch (error) {
      setStatus(`No se pudo obtener estado: ${error.message}`);
    }
  }

  return (
    <main className="app-shell">
      <section className="left-panel">
        <MovePanel
          pendingMove={pendingMove}
          onPendingMove={handlePendingMove}
          onVerifyMove={handleVerifyMove}
          onReset={handleReset}
        />

        <button className="sync-button" type="button" onClick={handleRefreshState}>
          Sincronizar Backend
        </button>

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
      </section>

      <section className="game-stage">
        <div className="board-panel">
          <ChessBoard board={board} lastMove={lastMove} />
        </div>

        <aside className="right-sidebar">
          <PlayerPanel
            name="Black"
            seconds={blackSeconds}
            active={gameStarted && currentTurn === "b"}
            capturedPieces={capturedByBlack}
          />

          <section className="history-card side-history-card">
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

          <PlayerPanel
            name="White"
            seconds={whiteSeconds}
            active={gameStarted && currentTurn === "w"}
            capturedPieces={capturedByWhite}
          />
        </aside>
      </section>
    </main>
  );
}