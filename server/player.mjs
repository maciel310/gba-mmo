import {randomBytes} from 'crypto'

import {Direction, MapLocation, Skill} from './proto.mjs';
import {getPlayer, setPlayer} from './storage.mjs';
import worldObjectTracker from './world_object_tracker.mjs';

// NOTE: If updated change in proto and ROM file.
const MAX_INVENTORY_SIZE = 18;

export default class Player {
  playerToken = '';

  x = 0;
  y = 0;
  currentMap = MapLocation.values.TOWN;
  direction = Direction.UP;

  message = '';

  skillExperience = {};
  hasSkillUpdate = false;
  hasPositionUpdate = false;

  inventory = [];

  resourceInteraction = undefined;

  changes = undefined;

  static async load(playerToken) {
    const playerObject = await getPlayer(playerToken);
    if (!playerObject) {
      return null;
    }

    const p = new Player();
    p.skillExperience = playerObject.skillExp;
    p.x = playerObject.x;
    p.y = playerObject.y;
    p.currentMap = playerObject.currentMap;
    p.hasPositionUpdate = true;
    p.playerToken = playerToken;
    p.inventory = playerObject.inventory;

    return p;
  }

  static async create(username, email) {
    const playerToken =
        randomBytes(10).toString('base64').replace(/[^a-zA-Z0-9]/g, '');

    const playerObject = {
      playerToken,
      username,
      email,
      skillExp: [],
      x: 240,
      y: 240,
      currentMap: MapLocation.values.TOWN,
      inventory: [],
    };
    Object.values(Skill.values).forEach(skill => {
      if (skill == Skill.values.UNKNOWN_SKILL) {
        return;
      }

      playerObject.skillExp[skill] = 0;
    });

    await setPlayer(playerToken, playerObject);

    return playerToken;
  }

  tick() {
    this.handleResourceInteraction();

    this.maybeSaveChanges();
  }

  handleResourceInteraction() {
    if (this.resourceInteraction != undefined) {
      if (this.inventory.length >= MAX_INVENTORY_SIZE) {
        this.resourceInteraction = undefined;
        return;
      }

      let success = this.resourceInteraction.interact(this);
      if (success) {
        this.resourceInteraction = undefined;
      }
    }
  }

  updateWithStatus(playerStatus) {
    if (!this.hasPositionUpdate) {
      if (this.x != playerStatus.x || this.y != playerStatus.y) {
        this.resourceInteraction = undefined;
      }

      this.x = playerStatus.x;
      this.y = playerStatus.y;
      this.savePlayerStatus({x: this.x, y: this.y});
    }
    this.direction = playerStatus.direction;

    if (playerStatus.interactionObjectId) {
      const o = worldObjectTracker.getObject(playerStatus.interactionObjectId);
      if (o.canInteract()) {
        if (o.isSkillResource() && this.inventory.length < MAX_INVENTORY_SIZE) {
          this.resourceInteraction = o;
        } else {
          this.message = o.interact(this);
        }
      }
    }
  }

  teleport(map, x, y) {
    this.x = x;
    this.y = y;
    this.currentMap = map;
    this.hasPositionUpdate = true;

    this.savePlayerStatus({x: this.x, y: this.y, currentMap: this.currentMap});
  }

  addExp(skill, amount) {
    this.skillExperience[skill] += amount;
    this.hasSkillUpdate = true;

    this.savePlayerStatus({skillExp: this.skillExperience});
  }

  addItem(item) {
    if (this.inventory.length >= MAX_INVENTORY_SIZE) {
      return;
    }

    this.inventory.push(item);
    this.savePlayerStatus({inventory: this.inventory});
  }

  async maybeSaveChanges() {
    if (this.changes) {
      const playerObject = await getPlayer(this.playerToken);

      Object.assign(playerObject, this.changes);

      await setPlayer(this.playerToken, playerObject);

      this.changes = undefined;
    }
  }

  async savePlayerStatus(changes) {
    if (this.changes == undefined) {
      this.changes = {};
    }

    Object.assign(this.changes, changes);
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
