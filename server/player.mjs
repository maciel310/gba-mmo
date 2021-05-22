import {Direction} from './proto.mjs';
import worldObjectTracker from './world_object_tracker.mjs';

export default class Player {
  x = 0;
  y = 0;
  direction = Direction.UP;

  message = '';

  updateWithStatus(playerStatus) {
    this.x = playerStatus.x;
    this.y = playerStatus.y;
    this.direction = playerStatus.direction;

    if (playerStatus.interactionObjectId) {
      const o = worldObjectTracker.getObject(playerStatus.interactionObjectId);
      this.message = o.interact();
    }
  }
}