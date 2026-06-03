import { useRef, useState } from "react";
import { connectSmartChessboard } from "../lib/bleClient";
import { parseMoveText } from "../lib/chessUtils";

export default function BluetoothPanel({ onBleMove, onStatus }) {
  const connectionRef = useRef(null);
  const [connected, setConnected] = useState(false);

  async function handleConnect() {
    try {
      const connection = await connectSmartChessboard({
        onStatus,
        onMessage: (message) => {
          onStatus(`BLE RX: ${message}`);

          const parsed = parseMoveText(message);

          if (parsed.ok) {
            onBleMove(parsed.move, `Movimiento recibido por BLE: ${parsed.move.from} -> ${parsed.move.to}`);
          }
        }
      });

      connectionRef.current = connection;
      setConnected(true);
    } catch (error) {
      onStatus(`Error BLE: ${error.message}`);
    }
  }

  function handleDisconnect() {
    connectionRef.current?.disconnect();
    connectionRef.current = null;
    setConnected(false);
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

  return (
    <section className="bluetooth-panel">
      <button type="button" onClick={handleConnect} disabled={connected}>
        Conectar BLE
      </button>

      <button type="button" onClick={handleDisconnect} disabled={!connected}>
        Desconectar
      </button>

      <button type="button" onClick={handlePing} disabled={!connected}>
        PING
      </button>
    </section>
  );
}