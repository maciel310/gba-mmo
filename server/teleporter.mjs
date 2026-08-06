import {MapLocation} from './proto.mjs';

export default class Teleporter {
  source = MapLocation.values.TOWN;
  destination = MapLocation.values.TOWN;
  destinationX = 0;
  destinationY = 0;
  xTiles = [];
  yTiles = [];

  constructor(
      x, y, width, height, source, destination, destinationX, destinationY) {
    for (let i = x; i < x + width; i += 8) {
      this.xTiles.push(Math.floor(i / 8));
    }
    for (let i = y; i < y + height; i += 8) {
      this.yTiles.push(Math.floor(i / 8));
    }

    this.source = source;
    this.destination = destination;
    this.destinationX = destinationX;
    this.destinationY = destinationY;
  }

  tick(player) {
    if (player.currentMap != this.source) {
      return;
    }
    const playerXTile = Math.floor(player.x / 8);
    const playerYTile = Math.floor((player.y + 16) / 8);
    if (this.xTiles.includes(playerXTile) &&
        this.yTiles.includes(playerYTile)) {
      player.teleport(this.destination, this.destinationX, this.destinationY);
    }
  }
}