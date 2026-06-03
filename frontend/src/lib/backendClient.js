// Esta es una capa temporal.
// Despues la podemos reemplazar por:
// 1. llamada a una API local,
// 2. WebAssembly,
// 3. puente con Electron,
// 4. o comunicacion directa con un proceso C++.
// Por ahora sirve para que el frontend tenga una funcion estable:
// verifyMove(move)

export async function verifyMove(move) {
  return {
    valid: true,
    message: `Movimiento aceptado visualmente: ${move.from} -> ${move.to}`,
    move
  };
}