import Chest from './chest.mjs';
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
  }

  spawnResources() {
    for (let y = 128; y <= 384; y += 32) {
      this.addObject(
          new Resource([96, 64], {x: 256, y}, 100),
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
