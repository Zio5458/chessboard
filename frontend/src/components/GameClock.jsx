function formatTime(seconds) {
  const safeSeconds = Math.max(0, seconds);
  const minutes = Math.floor(safeSeconds / 60);
  const remainingSeconds = safeSeconds % 60;

  return `${minutes}:${String(remainingSeconds).padStart(2, "0")}`;
}

export default function GameClock({ label, seconds, active }) {
  return (
    <div className={`game-clock ${active ? "active" : ""}`}>
      <span>{label}: </span>
      <strong>{formatTime(seconds)}</strong>
    </div>
  );
}