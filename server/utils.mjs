

export function calculateDistance(pos1, pos2) {
  const dx = Math.abs(pos1.x - pos2.x);
  const dy = Math.abs(pos1.y - pos2.y);
  return Math.sqrt(dx * dx + dy * dy);
}