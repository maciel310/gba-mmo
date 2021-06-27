import {Direction, Skill} from './proto.mjs';
import worldObjectTracker from './world_object_tracker.mjs';

export default class Player {
  x = 0;
  y = 0;
  direction = Direction.UP;

  message = '';

  skillExperience = {};
  hasSkillUpdate = false;
  hasPositionUpdate = false;

  resourceInteraction = undefined;

  constructor() {
    Object.values(Skill.values).forEach(skill => {
      if (skill == Skill.values.UNKNOWN_SKILL) {
        return;
      }

      this.skillExperience[skill] = 0;
    });

    this.x = 240;
    this.y = 240;
    this.hasPositionUpdate = true;
  }

  tick() {
    if (this.resourceInteraction != undefined) {
      let success = this.resourceInteraction.interact(this);
      if (success) {
        this.resourceInteraction = undefined;
      }
    }
  }

  updateWithStatus(playerStatus) {
    if (!this.hasPositionUpdate) {
      this.x = playerStatus.x;
      this.y = playerStatus.y;
    }
    this.direction = playerStatus.direction;

    if (playerStatus.interactionObjectId) {
      const o = worldObjectTracker.getObject(playerStatus.interactionObjectId);
      if (o.isSkillResource()) {
        this.resourceInteraction = o;
      } else {
        this.message = o.interact(this);
      }
    }
  }

  addExp(skill, amount) {
    this.skillExperience[skill] += amount;
    this.hasSkillUpdate = true;
  }

  getSkillStats() {
    return Object.entries(this.skillExperience).map(([skill, exp]) => {
      return {
        skill, exp, level: Math.ceil(exp / 100)
      }
    });
  }

  getPositionUpdate() {
    return {x: this.x, y: this.y};
  }
}
