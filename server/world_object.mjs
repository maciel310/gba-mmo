export default class WorldObject {
  objectId;
  map;

  constructor() {}

  setObjectId(objectId) {
    this.objectId = objectId;
  }

  setMap(map) {
    this.map = map;
  }

  tick() {}

  toWorldObject() {}

  isSkillResource() {
    return false;
  }

  canInteract() {
    return false;
  }

  interact() {
    return '';
  }
}