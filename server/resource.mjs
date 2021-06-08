import {Skill, SpriteSize} from './proto.mjs';
import WorldObject from './world_object.mjs';

export default class Resource extends WorldObject {
  spriteIds = [];
  spriteIndex = 0;

  position = {x: 0, y: 0};

  regenTicks = 0;
  regenCountdown = 0;

  constructor(spriteIds, position, regenTicks) {
    super();

    this.spriteIds = spriteIds;
    this.spriteIndex = 0;

    this.regenTicks = regenTicks;

    Object.assign(this.position, position);
  }

  interact(player) {
    if (this.spriteIndex == 0) {
      this.spriteIndex = 1;
      this.regenCountdown = this.regenTicks;
      player.addExp(Skill.values.WOODCUTTING, 15);
      return '';
    }
  }

  tick() {
    if (this.regenCountdown > 0) {
      this.regenCountdown--;
      if (this.regenCountdown == 0) {
        this.spriteIndex = 0;
      }
    }
  }

  toWorldObject() {
    return {
      objectId: this.objectId,
      x: this.position.x,
      y: this.position.y,
      spriteId: this.spriteIds[this.spriteIndex],
      spriteSize: SpriteSize.values.SQUARE_32x32,
      isSolid: true,
    };
  }
}
