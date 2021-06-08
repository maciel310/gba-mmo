import {createSocket} from 'dgram';

import Follower from './follower.mjs';
import Player from './player.mjs';
import {PlayerStatus, ServerUpdate, Skill} from './proto.mjs';
import worldObjectTracker from './world_object_tracker.mjs';

const server = createSocket('udp4');

const TICK_INTERVAL_MS = 1000 / 10;

// Map of UDP client info to last-seen timestamp.
const clientList = new Map();

server.on('error', (err) => {
  console.log('Error from UDP server', err);
});

server.on('message', (message, clientInfo) => {
  const clientKey = `${clientInfo.address}:${clientInfo.port}`;
  const client = clientList.get(clientKey);
  if (client != undefined) {
    clientList.get(clientKey).lastSeen = new Date();
  } else {
    const player = new Player();
    worldObjectTracker.addObject(new Follower(player));

    clientList.set(clientKey, {
      'address': clientInfo.address,
      'port': clientInfo.port,
      'lastSeen': new Date(),
      'player': player
    });
  }
  try {
    const p = PlayerStatus.decode(message);
    client.player.updateWithStatus(p);
  } catch (e) {
    console.log('Could not decode ', message.toString('hex'));
  }
});

setInterval(() => {
  worldObjectTracker.tick();
  const worldObjects = worldObjectTracker.getWorldObjects();

  for (const clientInfo of clientList.values()) {
    const player = clientInfo.player;
    const updateObject = {worldObject: worldObjects};
    if (player.message != '') {
      updateObject.networkMessage = player.message;
      player.message = '';
    }
    if (player.hasSkillUpdate) {
      updateObject.skillStats = player.getSkillStats();
      player.hasSkillUpdate = false;
    }
    if (player.hasPositionUpdate) {
      updateObject.playerStatus = player.getPositionUpdate();
      player.hasPositionUpdate = false;
    }
    // TODO: Limit update size to 512 bytes per UDP message.
    const update = ServerUpdate.encode(updateObject).finish();

    server.send(update, clientInfo.port, clientInfo.address);
  }
}, TICK_INTERVAL_MS);

server.bind(5465);
