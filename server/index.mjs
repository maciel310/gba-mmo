import {createSocket} from 'dgram';

import Player from './player.mjs';
import {PlayerStatus, ServerUpdate} from './proto.mjs';
import worldObjectTracker from './world_object_tracker.mjs';

const server = createSocket('udp4');

const TICK_INTERVAL_MS = 1000 / 10;

// Map of UDP client info to last-seen timestamp.
const clientList = new Map();

server.on('error', (err) => {
  console.log('Error from UDP server', err);
});

server.on('message', async (message, clientInfo) => {
  const clientKey = `${clientInfo.address}:${clientInfo.port}`;
  const client = clientList.get(clientKey);
  if (message.toString().startsWith('login~~')) {
    if (client == undefined) {
      const playerToken = message.toString().replace(/^login~~/, '').trim();

      const player = await Player.load(playerToken);
      if (player == null) {
        server.send('failed', clientInfo.port, clientInfo.address);
        return;
      }

      server.send('success', clientInfo.port, clientInfo.address);

      clientList.set(clientKey, {
        'address': clientInfo.address,
        'port': clientInfo.port,
        'lastSeen': new Date(),
        'player': player
      });
    }
  } else if (message.toString().startsWith('create~~')) {
    const [username, email] =
        message.toString().replace(/^create~~/, '').trim().split('~~');
    const cookie = await Player.create(username, email);
    server.send(`success~~${cookie}`, clientInfo.port, clientInfo.address);
  } else {
    try {
      const p = PlayerStatus.decode(message);
      client.player.updateWithStatus(p);

      clientList.get(clientKey).lastSeen = new Date();
    } catch (e) {
      console.log('Could not decode ', message.toString('hex'));
    }
  }
});

setInterval(() => {
  worldObjectTracker.tick();
  const worldObjects = worldObjectTracker.getWorldObjects();

  for (const clientInfo of clientList.values()) {
    const player = clientInfo.player;
    player.tick();

    const updateObject = {
      worldObject: worldObjects,
      currentMap: player.currentMap
    };
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
    if (player.resourceInteraction != undefined) {
      updateObject.interactingSkill = player.resourceInteraction.skillType;
    }

    // TODO: Limit update size to 512 bytes per UDP message.
    const update = ServerUpdate.encode(updateObject).finish();

    server.send(update, clientInfo.port, clientInfo.address);
  }
}, TICK_INTERVAL_MS);

server.bind(5465);
