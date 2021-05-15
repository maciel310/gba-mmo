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
    this.addObject(new Npc(MayorConfig));
    this.addObject(new Npc(MayorConfig));
    this.addObject(new Npc(MayorConfig));
    this.addObject(new Npc(MayorConfig));
    this.addObject(new Npc(MayorConfig));
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