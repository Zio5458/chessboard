import express from "express";
import cors from "cors";
import { spawn } from "child_process";
import path from "path";
import { fileURLToPath } from "url";
import fs from "fs";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
const PORT = 3030;

app.use(cors());
app.use(express.json());

function resolveEnginePath() {
  const windowsPath = path.resolve(__dirname, "..", "chess_engine_cli.exe");
  const unixPath = path.resolve(__dirname, "..", "chess_engine_cli");

  if (fs.existsSync(windowsPath)) {
    return windowsPath;
  }

  if (fs.existsSync(unixPath)) {
    return unixPath;
  }

  throw new Error(
    "No se encontro chess_engine_cli. Compile primero con: g++ -std=c++17 engine_cli.cpp chessboard.cpp -o chess_engine_cli"
  );
}

const enginePath = resolveEnginePath();

console.log(`Starting C++ chess engine: ${enginePath}`);

const engine = spawn(enginePath, [], {
  cwd: path.resolve(__dirname, ".."),
  stdio: ["pipe", "pipe", "pipe"]
});

let outputBuffer = "";
const pendingResponses = [];

engine.stdout.on("data", (data) => {
  outputBuffer += data.toString();

  const lines = outputBuffer.split(/\r?\n/);
  outputBuffer = lines.pop() ?? "";

  for (const line of lines) {
    const cleanLine = line.trim();

    if (!cleanLine) {
      continue;
    }

    const pending = pendingResponses.shift();

    if (!pending) {
      console.warn("Unexpected engine output:", cleanLine);
      continue;
    }

    try {
      const parsed = JSON.parse(cleanLine);
      pending.resolve(parsed);
    } catch (error) {
      pending.reject(
        new Error(`No se pudo parsear respuesta del motor: ${cleanLine}`)
      );
    }
  }
});

engine.stderr.on("data", (data) => {
  console.error("[engine stderr]", data.toString());
});

engine.on("exit", (code) => {
  console.error(`C++ engine exited with code ${code}`);

  while (pendingResponses.length > 0) {
    const pending = pendingResponses.shift();
    pending.reject(new Error("El motor C++ se cerro."));
  }
});

function sendEngineCommand(command) {
  return new Promise((resolve, reject) => {
    pendingResponses.push({ resolve, reject });
    engine.stdin.write(`${command}\n`);
  });
}

app.get("/api/health", (req, res) => {
  res.json({
    ok: true,
    message: "Local API running",
    enginePath
  });
});

app.get("/api/state", async (req, res) => {
  try {
    const result = await sendEngineCommand("BOARD");
    res.json(result);
  } catch (error) {
    res.status(500).json({
      valid: false,
      message: error.message
    });
  }
});

app.post("/api/reset", async (req, res) => {
  try {
    const result = await sendEngineCommand("RESET");
    res.json(result);
  } catch (error) {
    res.status(500).json({
      valid: false,
      message: error.message
    });
  }
});


app.get("/api/legal-moves", async (req, res) => {
  try {
    const { from } = req.query;

    if (!from) {
      return res.status(400).json({
        valid: false,
        message: "Debe enviar from."
      });
    }

    const result = await sendEngineCommand(`LEGAL ${from}`);
    res.json(result);
  } catch (error) {
    res.status(500).json({
      valid: false,
      message: error.message
    });
  }
});

app.post("/api/move", async (req, res) => {
  try {
    const { from, to, promotion } = req.body;

    if (!from || !to) {
      return res.status(400).json({
        valid: false,
        message: "Debe enviar from y to."
      });
    }

    const promotionPart = promotion ? ` ${promotion}` : "";
    const command = `MOVE ${from} ${to}${promotionPart}`;

    const result = await sendEngineCommand(command);

    res.json(result);
  } catch (error) {
    res.status(500).json({
      valid: false,
      message: error.message
    });
  }
});

process.on("SIGINT", () => {
  try {
    engine.stdin.write("QUIT\n");
  } catch {
    // ignore
  }

  process.exit(0);
});

app.listen(PORT, () => {
  console.log(`Local API listening on http://localhost:${PORT}`);
});