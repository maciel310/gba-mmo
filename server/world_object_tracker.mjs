import Chest from './chest.mjs';
import LumberjackNpc from './npcs/lumberjack.mjs';
import MayorNpc from './npcs/mayor.mjs';
import {MapLocation} from './proto.mjs';
import Resource from './resource.mjs';

class WorldObjectTracker {
  nextObjectId = 1;

  // Map of ObjectId to Object
  objectMap = new Map();

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
      this.addObject(
          new Resource([96, 64], {x: tree.x * 8, y: tree.y * 8}, 100),
          MapLocation.values.LUMBER_RIDGE);
    }
  }

  addObject(o, map) {
    this.objectMap.set(this.nextObjectId, o);
    o.setObjectId(this.nextObjectId);
    o.setMap(map);
    this.nextObjectId++;
  }

  getObject(id) {
    return this.objectMap.get(id);
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
