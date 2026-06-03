export function isValidSquare(square) {
  return /^[a-h][1-8]$/.test(square);
}

export function parseMoveText(input) {
  const text = input.trim();

  if (!text) {
    return { ok: false, error: "Ingrese un movimiento." };
  }

  const parts = text.split(/\s+/);

  if (parts[0]?.toUpperCase() === "MOVE") {
    const from = parts[1];
    const to = parts[2];
    const kind = parts[3] ?? "NORMAL";
    const promotion = parts[4] ?? null;

    if (!isValidSquare(from) || !isValidSquare(to)) {
      return { ok: false, error: "Mensaje MOVE invalido." };
    }

    return {
      ok: true,
      move: {
        from,
        to,
        kind,
        promotion
      }
    };
  }

  const compact = text.replace(/\s+/g, "");

  if (/^[a-h][1-8][a-h][1-8][qrbnQRBN]?$/.test(compact)) {
    return {
      ok: true,
      move: {
        from: compact.slice(0, 2),
        to: compact.slice(2, 4),
        kind: compact.length === 5 ? "PROMOTION" : "NORMAL",
        promotion: compact.length === 5 ? compact[4].toUpperCase() : null
      }
    };
  }

  if (parts.length >= 2 && isValidSquare(parts[0]) && isValidSquare(parts[1])) {
    let kind = "NORMAL";
    let promotion = null;

    if (parts[2]) {
      const third = parts[2].toUpperCase();

      if (["Q", "R", "B", "N"].includes(third)) {
        kind = "PROMOTION";
        promotion = third;
      } else {
        kind = parts[2];
      }
    }

    if (parts[3]) {
      promotion = parts[3].toUpperCase();
    }

    return {
      ok: true,
      move: {
        from: parts[0],
        to: parts[1],
        kind,
        promotion
      }
    };
  }

  return {
    ok: false,
    error: "Formato invalido. Use e2 e4, e2e4 o MOVE e2 e4 NORMAL."
  };
}

export function applyMoveToBoard(board, move) {
  const nextBoard = { ...board };
  const movingPiece = nextBoard[move.from];

  if (!movingPiece) {
    return {
      ok: false,
      error: `No hay pieza en ${move.from}.`
    };
  }

  const capturedPiece = nextBoard[move.to] ?? null;
  let finalPiece = movingPiece;

  delete nextBoard[move.from];

  if (move.kind === "EN_PASSANT") {
    const destinationFile = move.to[0];
    const destinationRank = Number(move.to[1]);
    const capturedRank = movingPiece[0] === "w" ? destinationRank - 1 : destinationRank + 1;
    const capturedSquare = `${destinationFile}${capturedRank}`;

    delete nextBoard[capturedSquare];
  }

  if (move.kind === "CASTLING_KINGSIDE") {
    if (move.from === "e1" && move.to === "g1") {
      nextBoard.f1 = nextBoard.h1;
      delete nextBoard.h1;
    }

    if (move.from === "e8" && move.to === "g8") {
      nextBoard.f8 = nextBoard.h8;
      delete nextBoard.h8;
    }
  }

  if (move.kind === "CASTLING_QUEENSIDE") {
    if (move.from === "e1" && move.to === "c1") {
      nextBoard.d1 = nextBoard.a1;
      delete nextBoard.a1;
    }

    if (move.from === "e8" && move.to === "c8") {
      nextBoard.d8 = nextBoard.a8;
      delete nextBoard.a8;
    }
  }

  if (move.kind === "PROMOTION" && move.promotion) {
    const color = movingPiece[0];
    finalPiece = `${color}${move.promotion.toUpperCase()}`;
  }

  nextBoard[move.to] = finalPiece;

  return {
    ok: true,
    board: nextBoard,
    movedPiece: movingPiece,
    capturedPiece
  };
}