
import Npc from './npc.mjs';
import MayorConfig from './npcs/mayor.mjs';

export default class WorldObjectTracker {
  nextObjectId = 1;

  // Map of ObjectId to Object
  objectMap = new Map();

  constructor() {
    this.spawnPermanentNpcs();
  }

  spawnPermanentNpcs() {
    this.addObject(new Npc(MayorConfig));
  }

  addObject(o) {
    this.objectMap.set(this.nextObjectId, o);
    this.nextObjectId++;
  }

  tick() {
    this.objectMap.forEach(v => v.tick());
    console.log(this.objectMap.get(1).position);
  }

  getWorldObjects() {
    [...this.objectMap.values()].map(v => v.toWorldObject());
  }
}