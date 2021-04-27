import {createSocket} from 'dgram';

const server = createSocket('udp4');

const TICK_INTERVAL_MS = 1000 / 10;

// Map of UDP client info to last-seen timestamp.
const clientList = new Map();

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
  for (const clientInfo of clientList.values()) {
    server.send(`${p1Pos.y},${p1Pos.x}`, clientInfo.port, clientInfo.address);
  }
}, TICK_INTERVAL_MS);

server.bind(5465);