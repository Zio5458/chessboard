const API_BASE_URL = "http://localhost:3030/api";

async function readResponse(response) {
  const data = await response.json();

  if (!response.ok) {
    throw new Error(data.message ?? "Error comunicando con el backend.");
  }

  return data;
}

export async function getGameState() {
  const response = await fetch(`${API_BASE_URL}/state`);
  return readResponse(response);
}

export async function resetGame() {
  const response = await fetch(`${API_BASE_URL}/reset`, {
    method: "POST"
  });

  return readResponse(response);
}

export async function verifyMove(move) {
  const response = await fetch(`${API_BASE_URL}/move`, {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify({
      from: move.from,
      to: move.to,
      promotion: move.promotion
    })
  });

  return readResponse(response);
}

export async function getLegalMoves(from) {
  const response = await fetch(`${API_BASE_URL}/legal-moves?from=${encodeURIComponent(from)}`);
  return readResponse(response);
}
