export default class WorldObject {
  objectId;

  constructor() {}

  setObjectId(objectId) {
    this.objectId = objectId;
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