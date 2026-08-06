import {MapLocation} from './proto.mjs';
import {SpriteSize} from './proto.mjs';
import {calculateDistance, moveTowards} from './utils.mjs';
import WorldObject from './world_object.mjs';

const wanderVectorOptions =
    [{x: 8, y: 0}, {x: -8, y: 0}, {x: 0, y: 8}, {x: 0, y: -8}];

export default class Npc extends WorldObject {
  spriteId = 0;

  position = {x: 0, y: 0};
  homePosition = {x: 0, y: 0};
  destinationPosition = {x: 0, y: 0};

  wanderRadius = 0;
  wanderVector = {x: 0, y: 0};
  wanderVectorOptions = [];

  constructor(config) {
    super();

    this.spriteId = config.spriteId;

    this.wanderRadius = config.wanderRadius;

    const spawnPointIndex =
        Math.floor(Math.random() * config.spawnPoints.length);
    this.homePosition = config.spawnPoints[spawnPointIndex];
    Object.assign(this.position, this.homePosition);
    Object.assign(this.destinationPosition, this.homePosition);

    this.wanderVectorOptions = wanderVectorOptions.map(v => {
      return {
        x: v.x * config.speed, y: v.y * config.speed
      }
    });
    this.wanderVector = this.wanderVectorOptions[0];
  }

  canInteract() {
    return true;
  }

  interact(player) {}

  tick() {
    if (this.position.x != this.destinationPosition.x ||
        this.position.y != this.destinationPosition.y) {
      this.position.x +=
          moveTowards(this.position.x, this.destinationPosition.x);
      this.position.y +=
          moveTowards(this.position.y, this.destinationPosition.y);
    } else {
      this.maybeChangeWanderVector();

      const newPosition = {
        x: this.position.x + this.wanderVector.x,
        y: this.position.y + this.wanderVector.y
      };
      if (calculateDistance(newPosition, this.homePosition) <
          this.wanderRadius) {
        this.destinationPosition.x = this.position.x + this.wanderVector.x;
        this.destinationPosition.y = this.position.y + this.wanderVector.y;
      }
    }
  }

  maybeChangeWanderVector() {
    const r = Math.random();
    if (r > 0.9) {
      this.changeWanderDirection();
    }
  }

  changeWanderDirection() {
    const dir = Math.floor(Math.random() * this.wanderVectorOptions.length);
    this.wanderVector = this.wanderVectorOptions[dir];
  }

  toWorldObject() {
    return {
      objectId: this.objectId,
      x: this.position.x,
      y: this.position.y,
      spriteId: this.spriteId,
      spriteSize: SpriteSize.values.TALL_16x32
    };
  }
}
