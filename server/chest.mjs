import {Interface, SpriteSize} from './proto.mjs';
import WorldObject from './world_object.mjs';

export default class Chest extends WorldObject {
  x = 144;
  y = 256;

  canInteract() {
    return true;
  }

  interact(player) {
    player.interface = Interface.values.BANK;
  }

  toWorldObject() {
    return {
      objectId: this.objectId,
      x: this.x,
      y: this.y,
      spriteId: 128,
      spriteSize: SpriteSize.values.SQUARE_32x32,
      isSolid: true,
    };
  }
}