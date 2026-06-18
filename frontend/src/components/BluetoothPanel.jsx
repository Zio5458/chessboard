import { useRef, useState } from "react";
import { connectSmartChessboard } from "../lib/bleClient";
import { getLegalMoves } from "../lib/backendClient";
import { parseBoardMessage } from "../lib/chessUtils";

export default function BluetoothPanel({ onBleMove, onStatus, onCommandReady }) {
  const connectionRef = useRef(null);

  const [connected, setConnected] = useState(false);
  const [deviceName, setDeviceName] = useState("");
  const [messages, setMessages] = useState([]);

  function addMessage(message) {
    setMessages((items) => [message, ...items].slice(0, 5));
  }

  async function sendBoardCommand(command) {
    if (!connectionRef.current) {
      throw new Error("No hay conexion BLE activa.");
    }

    await connectionRef.current.write(command);
  }

  async function handleLiftedSquare(square) {
    try {
      const result = await getLegalMoves(square);
      const moves = result.moves ?? [];

      if (moves.length === 0) {
        await sendBoardCommand("CLEAR_LEDS");
        onStatus(`Pieza levantada en ${square}. No hay movimientos legales.`);
        return;
      }

      await sendBoardCommand(`LEGAL_MOVES ${moves.join(" ")}`);
      onStatus(`Pieza levantada en ${square}. Movimientos legales: ${moves.join(", ")}.`);
    } catch (error) {
      onStatus(`No se pudieron obtener movimientos legales para ${square}: ${error.message}`);
    }
  }

  async function handleConnect() {
    try {
      const connection = await connectSmartChessboard({
        onStatus,
        onMessage: async (message) => {
          addMessage(message);

          const parsed = parseBoardMessage(message);

          if (parsed.type === "LIFTED") {
            await handleLiftedSquare(parsed.square);
            return;
          }

          if (parsed.type === "MOVE") {
            onBleMove(
              parsed.move,
              `${parsed.message}. Presione "Verificar Movimiento" para validarlo.`
            );
            return;
          }

          onStatus(parsed.message);
        }
      });

      connectionRef.current = connection;

      if (onCommandReady) {
        onCommandReady(() => async (command) => {
          await connection.write(command);
        });
      }

      setConnected(true);
      setDeviceName(connection.device.name ?? "SmartChess");
    } catch (error) {
      onStatus(`Error BLE: ${error.message}`);
    }
  }

  function handleDisconnect() {
    connectionRef.current?.disconnect();
    connectionRef.current = null;

    if (onCommandReady) {
      onCommandReady(null);
    }

    setConnected(false);
    setDeviceName("");

    onStatus("BLE desconectado manualmente.");
  }

  async function handlePing() {
    try {
      await sendBoardCommand("PING");
      onStatus("PING enviado al tablero.");
    } catch (error) {
      onStatus(`No se pudo enviar PING: ${error.message}`);
    }
  }

  async function handleInitFlash() {
    try {
      await sendBoardCommand("INIT_FLASH");
      onStatus("Comando INIT_FLASH enviado al tablero.");
    } catch (error) {
      onStatus(`No se pudo enviar INIT_FLASH: ${error.message}`);
    }
  }

  async function handleClearLeds() {
    try {
      await sendBoardCommand("CLEAR_LEDS");
      onStatus("Comando CLEAR_LEDS enviado al tablero.");
    } catch (error) {
      onStatus(`No se pudo enviar CLEAR_LEDS: ${error.message}`);
    }
  }

  async function handleResetBaseline() {
    try {
      await sendBoardCommand("RESET_BASELINE");
      onStatus("Comando RESET_BASELINE enviado al tablero.");
    } catch (error) {
      onStatus(`No se pudo enviar RESET_BASELINE: ${error.message}`);
    }
  }

  return (
    <section className="bluetooth-panel">
      <div className="ble-title">
        <strong>BLE</strong>
        <span className={connected ? "ble-connected" : "ble-disconnected"}>
          {connected ? `Conectado: ${deviceName}` : "Desconectado"}
        </span>
      </div>

      <div className="ble-buttons">
        <button type="button" onClick={handleConnect} disabled={connected}>
          Conectar
        </button>

        <button type="button" onClick={handleDisconnect} disabled={!connected}>
          Desconectar
        </button>

        <button type="button" onClick={handlePing} disabled={!connected}>
          PING
        </button>

        <button type="button" onClick={handleInitFlash} disabled={!connected}>
          Flash LEDs
        </button>

        <button type="button" onClick={handleClearLeds} disabled={!connected}>
          Apagar LEDs
        </button>

        <button type="button" onClick={handleResetBaseline} disabled={!connected}>
          Reset baseline
        </button>
      </div>

      {messages.length > 0 && (
        <div className="ble-log">
          {messages.map((message, index) => (
            <div key={`${message}-${index}`} className="ble-log-line">
              {message}
            </div>
          ))}
        </div>
      )}
    </section>
  );
}
