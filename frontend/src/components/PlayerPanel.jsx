import GameClock from "./GameClock";
import CapturedPieces from "./CapturedPieces";

export default function PlayerPanel({
  name,
  seconds,
  active,
  capturedPieces
}) {
  return (
    <section className="player-panel">
      <GameClock label={name} seconds={seconds} active={active} />
      <CapturedPieces pieces={capturedPieces} />
    </section>
  );
}