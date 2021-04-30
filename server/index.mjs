import {createSocket} from 'dgram';
import protobuf from 'protobufjs';

import Npc from './npc.mjs';

const server = createSocket('udp4');

const TICK_INTERVAL_MS = 1000 / 10;

const root = protobuf.loadSync([
  '../gba-mmo-protos/world_object.proto',
  '../gba-mmo-protos/network_messages.proto'
]);
const ServerUpdate = root.lookupType('ServerUpdate');

// Map of UDP client info to last-seen timestamp.
const clientList = new Map();

const npcs = [];

const mayor = new Npc(
    1, {x: 40, y: 40}, 10,
    [{x: 1, y: 0}, {x: -1, y: 0}, {x: 0, y: 1}, {x: 0, y: -1}]);

npcs.push(mayor);

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
  const [x, y] = message.toString().split(',');
  p1Pos.x = x;
  p1Pos.y = y;
});

setInterval(() => {
  const worldObjects = [];

  npcs.forEach((npc) => {
    npc.tick();
    worldObjects.push(npc.toWorldObject());
  });

  // TODO: Limit update size to 512 bytes per UDP message.
  const update = ServerUpdate.encode({worldObject: worldObjects}).finish();

  for (const clientInfo of clientList.values()) {
    server.send(update, clientInfo.port, clientInfo.address);
  }
}, TICK_INTERVAL_MS);

server.bind(5465);