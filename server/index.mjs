import {createSocket} from 'dgram';
import protobuf from 'protobufjs';

import WorldObjectTracker from './world_object_tracker.mjs';

const server = createSocket('udp4');

const TICK_INTERVAL_MS = 1000 / 10;

const root = protobuf.loadSync([
  '../gba-mmo-protos/world_object.proto',
  '../gba-mmo-protos/network_messages.proto'
]);
const ServerUpdate = root.lookupType('ServerUpdate');
const PlayerStatus = root.lookupType('PlayerStatus');

// Map of UDP client info to last-seen timestamp.
const clientList = new Map();

const worldObjectTracker = new WorldObjectTracker();

server.on('error', (err) => {
  console.log('Error from UDP server', err);
});

const p1Pos = {
  x: 0,
  y: 0
};

server.on('message', (message, clientInfo) => {
  clientList.set(`${clientInfo.address}:${clientInfo.port}`, {
    'address': clientInfo.address,
    'port': clientInfo.port,
    'lastSeen': new Date()
  });
  try {
    const p = PlayerStatus.decode(message);
    p1Pos.x = p.x;
    p1Pos.y = p.y;
  } catch (e) {
    console.log('Could not decode ', message.toString('hex'));
  }
});

setInterval(() => {
  worldObjectTracker.tick();
  const worldObjects = worldObjectTracker.getWorldObjects();

  worldObjects.push(
      {objectId: 22, x: p1Pos.x - 10, y: p1Pos.y - 10, spriteId: 0});

  // TODO: Limit update size to 512 bytes per UDP message.
  const update = ServerUpdate.encode({worldObject: worldObjects}).finish();

  for (const clientInfo of clientList.values()) {
    server.send(update, clientInfo.port, clientInfo.address);
  }
}, TICK_INTERVAL_MS);

server.bind(5465);