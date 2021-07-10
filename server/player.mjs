import {randomBytes} from 'crypto'

import {Direction, Item, MapLocation, PlayerStatus, Skill, SpriteSize} from './proto.mjs';
import {getPlayer, setPlayer} from './storage.mjs';
import WorldObject from './world_object.mjs';
import worldObjectTracker from './world_object_tracker.mjs';

// NOTE: If updated change in proto and ROM file.
const MAX_INVENTORY_SIZE = 18;

export default class Player {
  playerToken = '';
  username = '';

  x = 0;
  y = 0;
  currentMap = MapLocation.values.TOWN;
  direction = Direction.UP;

  message = '';
  interface = undefined;

  skillExperience = {};
  hasSkillUpdate = true;
  hasPositionUpdate = false;

  inventory = [];
  bank = {};
  bankChanged = true;

  resourceInteraction = undefined;

  changes = undefined;

  npcState = {};

  worldObject;

  static async load(playerToken) {
    const playerObject = await getPlayer(playerToken);
    if (!playerObject) {
      return null;
    }

    const p = new Player();
    p.username = playerObject.username;
    p.skillExperience = playerObject.skillExp;
    p.x = playerObject.x;
    p.y = playerObject.y;
    p.currentMap = playerObject.currentMap;
    p.hasPositionUpdate = true;
    p.playerToken = playerToken;
    p.inventory = playerObject.inventory;
    Object.assign(p.npcState, playerObject.npcState);
    Object.assign(p.bank, playerObject.bank);

    const playerWorldObject = new PlayerWorldObject(p);
    p.worldObject = playerWorldObject;
    worldObjectTracker.addObject(playerWorldObject, p.currentMap);

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

  logout() {
    worldObjectTracker.removeObject(this.worldObject.objectId);
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
    // Convert to plain JS object to distinguish between unset fields...
    const playerStatusObject = PlayerStatus.toObject(playerStatus);

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
        if (o.isSkillResource()) {
          if (!this.inventory.includes(o.requiredItem)) {
            this.message =
                `You need a ${Item.valuesById[o.requiredItem]} to do that!`;
          } else if (!this.hasInventoryRoom()) {
            this.message = 'Your bag is already full!';
          } else {
            this.resourceInteraction = o;
          }
        } else {
          this.message = o.interact(this);
        }
      }
    }

    if (playerStatusObject.depositInventoryIndex != undefined) {
      this.bankInventoryItem(playerStatus.depositInventoryIndex);
    }

    if (playerStatusObject.withdrawBankItem != undefined) {
      this.withdrawBankItems(playerStatus.withdrawBankItem, 1);
    }
  }

  teleport(map, x, y) {
    this.x = x;
    this.y = y;
    this.currentMap = map;
    this.hasPositionUpdate = true;

    this.worldObject.setMap(map);

    this.savePlayerStatus({x: this.x, y: this.y, currentMap: this.currentMap});
  }

  addExp(skill, amount) {
    this.skillExperience[skill] += amount;
    this.hasSkillUpdate = true;

    this.savePlayerStatus({skillExp: this.skillExperience});
  }

  hasInventoryRoom(count = 1) {
    return this.inventory.length + count <= MAX_INVENTORY_SIZE;
  }

  addItem(item) {
    if (!this.hasInventoryRoom()) {
      return;
    }

    this.inventory.push(item);
    this.savePlayerStatus({inventory: this.inventory});
  }

  bankInventoryItem(index) {
    if (this.inventory[index] == undefined) {
      return;
    }

    const [item] = this.inventory.splice(index, 1);
    if (this.bank.hasOwnProperty(item)) {
      this.bank[item]++;
    } else {
      this.bank[item] = 1;
    }

    this.savePlayerStatus({bank: this.bank, inventory: this.inventory});
  }

  withdrawBankItems(type, count) {
    if (!this.hasInventoryRoom(count)) {
      return;
    }
    if (!this.bank.hasOwnProperty(type) || this.bank[type] < count) {
      return;
    }

    this.bank[type] -= count;
    for (let i = 0; i < count; i++) {
      this.inventory.push(type);
    }

    this.savePlayerStatus({bank: this.bank, inventory: this.inventory});
  }

  updateNpcState(npc, state) {
    this.npcState[npc] = state;

    this.savePlayerStatus({npcState: this.npcState});
  }

  async maybeSaveChanges() {
    if (this.changes) {
      if (this.changes.hasOwnProperty('bank')) {
        this.bankChanged = true;
      }

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

  encodeBank() {
    const encodedBank = [];
    for (let key of Object.keys(this.bank)) {
      encodedBank.push({
        item: key,
        quantity: this.bank[key],
      });
    }
    return encodedBank;
  }
}

class PlayerWorldObject extends WorldObject {
  player;
  spriteId;

  constructor(player) {
    super();

    this.player = player;
  }

  canInteract() {
    return true;
  }

  interact(player) {
    return `${this.player.username} says hello!`;
  }

  tick() {
    switch (this.player.direction) {
      case Direction.values.UP:
        this.spriteId = 0;
        break;
      case Direction.values.DOWN:
        this.spriteId = 16;
        break;
      case Direction.values.LEFT:
        this.spriteId = 32;
        break;
      default:
        this.spriteId = 48;
        break;
    }
  }

  toWorldObject() {
    return {
      objectId: this.objectId,
      x: this.player.x,
      y: this.player.y,
      spriteId: this.spriteId,
      spriteSize: SpriteSize.values.TALL_16x32,
      isSolid: false,
    };
  }

  isSkillResource() {
    return false;
  }
}