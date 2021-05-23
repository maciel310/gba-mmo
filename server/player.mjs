import {Direction, Skill} from './proto.mjs';
import worldObjectTracker from './world_object_tracker.mjs';

export default class Player {
  x = 0;
  y = 0;
  direction = Direction.UP;

  message = '';

  skillExperience = {};
  hasSkillUpdate = false;

  constructor() {
    Object.values(Skill.values).forEach(skill => {
      if (skill == Skill.values.UNKNOWN_SKILL) {
        return;
      }

      this.skillExperience[skill] = 0;
    });
  }

  updateWithStatus(playerStatus) {
    this.x = playerStatus.x;
    this.y = playerStatus.y;
    this.direction = playerStatus.direction;

    if (playerStatus.interactionObjectId) {
      const o = worldObjectTracker.getObject(playerStatus.interactionObjectId);
      this.message = o.interact(this);
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
}
