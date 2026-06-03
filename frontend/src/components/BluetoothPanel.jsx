import { useRef, useState } from "react";
import { connectSmartChessboard } from "../lib/bleClient";
import { parseBoardMessage } from "../lib/chessUtils";

export default function BluetoothPanel({ onBleMove, onStatus }) {
  const connectionRef = useRef(null);

  const [connected, setConnected] = useState(false);
  const [deviceName, setDeviceName] = useState("");
  const [messages, setMessages] = useState([]);

  function addMessage(message) {
    setMessages((items) => [message, ...items].slice(0, 5));
  }

  async function handleConnect() {
    try {
      const connection = await connectSmartChessboard({
        onStatus,
        onMessage: (message) => {
          addMessage(message);

          const parsed = parseBoardMessage(message);

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
      setConnected(true);
      setDeviceName(connection.device.name ?? "SmartChess");
    } catch (error) {
      onStatus(`Error BLE: ${error.message}`);
    }
  }

  function handleDisconnect() {
    connectionRef.current?.disconnect();
    connectionRef.current = null;

    setConnected(false);
    setDeviceName("");

    onStatus("BLE desconectado manualmente.");
  }

  async function handlePing() {
    try {
      await connectionRef.current?.write("PING");
      onStatus("PING enviado al tablero.");
    } catch (error) {
      onStatus(`No se pudo enviar PING: ${error.message}`);
    }
  }

  async function handleStartInit() {
    try {
      await connectionRef.current?.write("START_INIT");
      onStatus("Comando START_INIT enviado al tablero.");
    } catch (error) {
      onStatus(`No se pudo enviar START_INIT: ${error.message}`);
    }
  }

  async function handleResetBoard() {
    try {
      await connectionRef.current?.write("RESET_BOARD");
      onStatus("Comando RESET_BOARD enviado al tablero.");
    } catch (error) {
      onStatus(`No se pudo enviar RESET_BOARD: ${error.message}`);
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

        <button type="button" onClick={handleStartInit} disabled={!connected}>
          Init
        </button>

        <button type="button" onClick={handleResetBoard} disabled={!connected}>
          Reset tablero
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