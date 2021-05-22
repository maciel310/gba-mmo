import Npc from './npc.mjs';
import MayorConfig from './npcs/mayor.mjs';
import Resource from './resource.mjs';

export default class WorldObjectTracker {
  nextObjectId = 1;

  // Map of ObjectId to Object
  objectMap = new Map();

  constructor() {
    this.spawnPermanentNpcs();
    this.spawnResources();
  }

  spawnPermanentNpcs() {
    this.addObject(new Npc(MayorConfig));
    this.addObject(new Npc(MayorConfig));
    this.addObject(new Npc(MayorConfig));
    this.addObject(new Npc(MayorConfig));
    this.addObject(new Npc(MayorConfig));
    this.addObject(new Npc(MayorConfig));
  }

  spawnResources() {
    for (let y = 128; y <= 384; y += 32) {
      this.addObject(new Resource([96, 64], {x: 256, y}, 100));
    }
  }

  addObject(o) {
    this.objectMap.set(this.nextObjectId, o);
    o.setObjectId(this.nextObjectId);
    this.nextObjectId++;
  }

  getObject(id) {
    return this.objectMap.get(id);
  }

  tick() {
    this.objectMap.forEach(v => v.tick());
  }

  getWorldObjects() {
    return [...this.objectMap.values()].map(v => v.toWorldObject());
  }
}