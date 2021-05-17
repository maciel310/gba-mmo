import {SpriteSize} from './proto.mjs';
import WorldObject from './world_object.mjs';

export default class Follower extends WorldObject {
  spriteId = 0;

  position = {x: 0, y: 0};
  playerPosition = {};

  constructor(playerPosition) {
    super();

    this.playerPosition = playerPosition;
  }

  tick() {
    if (this.playerPosition.direction == 1) {
      this.position.x = this.playerPosition.x;
      this.position.y = this.playerPosition.y + 16;
      this.spriteId = 0;
    } else if (this.playerPosition.direction == 2) {
      this.position.x = this.playerPosition.x;
      this.position.y = this.playerPosition.y - 16;
      this.spriteId = 16;
    } else if (this.playerPosition.direction == 3) {
      this.position.x = this.playerPosition.x + 16;
      this.position.y = this.playerPosition.y;
      this.spriteId = 32;
    } else if (this.playerPosition.direction == 4) {
      this.position.x = this.playerPosition.x - 16;
      this.position.y = this.playerPosition.y;
      this.spriteId = 48;
    }
  }

  toWorldObject() {
    return {
      objectId: this.objectId,
      x: this.position.x,
      y: this.position.y,
      spriteId: this.spriteId,
      spriteSize: SpriteSize.values.TALL_16x32
    };
  }
}
