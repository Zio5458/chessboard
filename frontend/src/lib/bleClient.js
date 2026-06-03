const SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const RX_CHARACTERISTIC_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const TX_CHARACTERISTIC_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

export async function connectSmartChessboard({ onMessage, onStatus }) {
  if (!navigator.bluetooth) {
    throw new Error("Web Bluetooth no esta disponible en este navegador.");
  }

  onStatus?.("Buscando dispositivo BLE...");

  const device = await navigator.bluetooth.requestDevice({
    filters: [{ namePrefix: "SmartChess" }],
    optionalServices: [SERVICE_UUID]
  });

  onStatus?.(`Conectando a ${device.name ?? "ESP32-S3"}...`);

  const server = await device.gatt.connect();
  const service = await server.getPrimaryService(SERVICE_UUID);

  const txCharacteristic = await service.getCharacteristic(TX_CHARACTERISTIC_UUID);
  const rxCharacteristic = await service.getCharacteristic(RX_CHARACTERISTIC_UUID);

  const decoder = new TextDecoder();
  const encoder = new TextEncoder();

  let buffer = "";

  await txCharacteristic.startNotifications();

  txCharacteristic.addEventListener("characteristicvaluechanged", (event) => {
    const chunk = decoder.decode(event.target.value);
    buffer += chunk;

    const lines = buffer.split(/\r?\n/);
    buffer = lines.pop() ?? "";

    lines.forEach((line) => {
      const cleanLine = line.trim();

      if (cleanLine) {
        onMessage?.(cleanLine);
      }
    });
  });

  device.addEventListener("gattserverdisconnected", () => {
    onStatus?.("Dispositivo BLE desconectado.");
  });

  onStatus?.("Tablero conectado por BLE.");

  return {
    device,
    async write(command) {
      const payload = encoder.encode(`${command}\n`);
      await rxCharacteristic.writeValue(payload);
    },
    disconnect() {
      if (device.gatt.connected) {
        device.gatt.disconnect();
      }
    }
  };
}