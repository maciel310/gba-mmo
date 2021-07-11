import {createSocket} from 'dgram';

import Player from './player.mjs';
import {Interface, MapLocation, PlayerStatus, ServerUpdate} from './proto.mjs';
import Teleporter from './teleporter.mjs';
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

      // Remove any other instances of that player from the client list.
      for (const [clientKey, client] of clientList) {
        if (client.player.playerToken == playerToken) {
          removePlayer(clientKey);
        }
      }

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
      console.log('Error processing: ', message.toString('hex'), e);
    }
  }
});

const teleporters = new Map();
teleporters.set(MapLocation.values.TOWN, [
  new Teleporter(
      432, 240, 32, 32, MapLocation.values.TOWN, MapLocation.values.VAR_ROCK,
      200, 224),
  new Teleporter(
      432, 344, 32, 32, MapLocation.values.TOWN,
      MapLocation.values.LUMBER_RIDGE, 224, 224),
]);
teleporters.set(MapLocation.values.VAR_ROCK, [
  new Teleporter(
      144, 224, 32, 32, MapLocation.values.VAR_ROCK, MapLocation.values.TOWN,
      392, 232),
]);
teleporters.set(MapLocation.values.LUMBER_RIDGE, [
  new Teleporter(
      168, 224, 32, 32, MapLocation.values.LUMBER_RIDGE,
      MapLocation.values.TOWN, 392, 336),
]);

// Log users out after 1 minute of no received messages.
const LOGOUT_LIMIT_MS = 1 * 60 * 1000;

setInterval(() => {
  worldObjectTracker.tick();
  const worldObjects = worldObjectTracker.getWorldObjects();

  for (const [clientKey, clientInfo] of clientList) {
    if ((new Date() - clientInfo.lastSeen) > LOGOUT_LIMIT_MS) {
      removePlayer(clientKey);
    }

    const player = clientInfo.player;
    player.tick();

    teleporters.get(player.currentMap).forEach(teleporter => {
      teleporter.tick(player);
    });

    const updateObject = {
      worldObject: worldObjects.get(player.currentMap)
                       .filter(
                           worldObject => worldObject.objectId !=
                               player.worldObject.objectId),
      removedWorldObjectIds: worldObjectTracker.availableIds,
      currentMap: player.currentMap,
      inventory: player.inventory,
    };
    if (player.message != '') {
      updateObject.networkMessage = player.message;
      player.message = '';
    }
    if (player.interface != undefined) {
      updateObject.launchInterface = player.interface;
      switch (player.interface) {
        case Interface.values.BANK:
          player.bankChanged = true;
          break;
      }
      player.interface = undefined;
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
    if (player.bankChanged) {
      updateObject.bank = player.encodeBank();
      player.bankChanged = false;
    }
    if (player.coinsChanged) {
      updateObject.coinCount = player.coins;
      player.coinsChanged = false;
    }

    // TODO: Limit update size to 512 bytes per UDP message.
    const update = ServerUpdate.encode(updateObject).finish();

    server.send(update, clientInfo.port, clientInfo.address);
  }
}, TICK_INTERVAL_MS);

server.bind(5465);

function removePlayer(clientKey) {
  clientList.get(clientKey).player.logout();
  clientList.delete(clientKey);
}