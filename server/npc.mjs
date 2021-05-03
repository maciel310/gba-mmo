import {calculateDistance} from './utils.mjs';
import WorldObject from './world_object.mjs';

const wanderVectorOptions =
    [{x: 1, y: 0}, {x: -1, y: 0}, {x: 0, y: 1}, {x: 0, y: -1}];

export default class Npc extends WorldObject {
  objectId = 0;

  position = {x: 0, y: 0};
  homePosition = {x: 0, y: 0};

  wanderRadius = 0;
  wanderVector = {x: 0, y: 0};
  wanderVectorOptions = [];

  constructor(config) {
    super();

    this.objectId = config.objectId;
    this.wanderRadius = config.wanderRadius;

    const spawnPointIndex =
        Math.floor(Math.random() * config.spawnPoints.length);
    this.homePosition = config.spawnPoints[spawnPointIndex];
    Object.assign(this.position, this.homePosition);

    this.wanderVectorOptions = wanderVectorOptions.map(v => {
      return {
        x: v.x * config.speed, y: v.y * config.speed
      }
    });
    this.wanderVector = wanderVectorOptions[0];
  }

  tick() {
    this.maybeChangeWanderVector();

    const newPosition = {
      x: this.position.x + this.wanderVector.x,
      y: this.position.y + this.wanderVector.y
    };
    if (calculateDistance(newPosition, this.homePosition) < this.wanderRadius) {
      this.position.x += this.wanderVector.x;
      this.position.y += this.wanderVector.y;
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
    };
  }
}
