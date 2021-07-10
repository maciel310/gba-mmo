import Chest from './chest.mjs';
import LumberjackNpc from './npcs/lumberjack.mjs';
import MayorNpc from './npcs/mayor.mjs';
import {MapLocation} from './proto.mjs';
import {Item, Skill} from './proto.mjs';
import Resource from './resource.mjs';

class WorldObjectTracker {
  nextObjectId = 1;

  // Map of ObjectId to Object
  objectMap = new Map();

  availableIds = [];

  constructor() {
    this.spawnPermanentNpcs();
    this.spawnResources();

    this.addObject(new Chest(), MapLocation.values.TOWN);
  }

  spawnPermanentNpcs() {
    this.addObject(new MayorNpc(), MapLocation.values.TOWN);
    this.addObject(new LumberjackNpc(), MapLocation.values.LUMBER_RIDGE);
  }

  spawnResources() {
    // Lumber Ridge
    const trees = [
      {x: 25, y: 20},
      {x: 29, y: 33},
      {x: 19, y: 36},
      {x: 36, y: 37},
      {x: 29, y: 40},
      {x: 46, y: 37},
      {x: 53, y: 34},
      {x: 53, y: 41},
      {x: 41, y: 43},
      {x: 51, y: 48},
      {x: 52, y: 57},
      {x: 44, y: 50},
      {x: 37, y: 50},
      {x: 31, y: 47},
    ];
    for (let tree of trees) {
      const treeConfig = {
        spriteIds: [96, 64],
        position: {x: tree.x * 8, y: tree.y * 8},
        regenTicks: 100,
        skillType: Skill.values.WOODCUTTING,
        itemType: Item.values.WOOD,
        requiredItem: Item.values.HATCHET,
      };
      this.addObject(new Resource(treeConfig), MapLocation.values.LUMBER_RIDGE);
    }

    // Var Rock
    const rocks = [
      {x: 42, y: 29},
      {x: 35, y: 33},
      {x: 40, y: 37},
      {x: 39, y: 44},
      {x: 42, y: 49},
      {x: 48, y: 50},
      {x: 53, y: 46},
      {x: 54, y: 39},
      {x: 49, y: 33},
    ];
    for (let rock of rocks) {
      const rockConfig = {
        spriteIds: [192, 160],
        position: {x: rock.x * 8, y: rock.y * 8},
        regenTicks: 100,
        skillType: Skill.values.MINING,
        itemType: Item.values.ROCK,
        requiredItem: Item.values.PICKAXE,
      };
      this.addObject(new Resource(rockConfig), MapLocation.values.VAR_ROCK);
    }
  }

  getNextId() {
    if (this.availableIds.length > 0) {
      return this.availableIds.shift();
    }
    return this.nextObjectId++;
  }

  addObject(o, map) {
    const id = this.getNextId();
    this.objectMap.set(id, o);
    o.setObjectId(id);
    o.setMap(map);
  }

  getObject(id) {
    return this.objectMap.get(id);
  }

  removeObject(id) {
    this.objectMap.delete(id);
    this.availableIds.push(id);
  }

  tick() {
    this.objectMap.forEach(v => v.tick());
  }

  getWorldObjects() {
    const worldObjectMap = new Map();
    Object.values(MapLocation.values).forEach(map => {
      if (map == MapLocation.values.UNKNOWN_MAP) {
        return;
      }

      worldObjectMap.set(map, []);
    });

    for (let object of this.objectMap.values()) {
      worldObjectMap.get(object.map).push(object.toWorldObject());
    }

    return worldObjectMap;
  }
}

export default new WorldObjectTracker();
