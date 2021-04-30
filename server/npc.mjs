import {calculateDistance} from './utils.mjs';

export default class Npc {
  position = {x: 0, y: 0};
  homePosition = {x: 0, y: 0};

  wanderRadius = 0;
  wanderVector = {x: 0, y: 0};
  wanderVectorOptions = [];

  constructor(homePosition, wanderRadius, wanderVectorOptions) {
    Object.assign(this.position, homePosition);
    this.homePosition = homePosition;
    this.wanderRadius = wanderRadius;
    this.wanderVectorOptions = wanderVectorOptions;
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
}
