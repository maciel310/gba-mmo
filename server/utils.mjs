

export function calculateDistance(pos1, pos2) {
  const dx = Math.abs(pos1.x - pos2.x);
  const dy = Math.abs(pos1.y - pos2.y);
  return Math.sqrt(dx * dx + dy * dy);
}

export function moveTowards(current, dest) {
  if (Math.abs(dest - current) > 32) {
    return dest - current;
  } else if (current > dest) {
    return -1;
  } else if (current < dest) {
    return 1;
  } else {
    return 0;
  }
}