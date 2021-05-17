import protobuf from 'protobufjs';

const root = protobuf.loadSync([
  '../gba-mmo-protos/world_object.proto',
  '../gba-mmo-protos/network_messages.proto'
]);
const ServerUpdate = root.lookupType('ServerUpdate');
const PlayerStatus = root.lookupType('PlayerStatus');
const Direction = root.lookupEnum('Direction');

export {ServerUpdate, PlayerStatus, Direction};