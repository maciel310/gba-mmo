import protobuf from 'protobufjs';

const root = protobuf.loadSync([
  '../gba-mmo-protos/world_object.proto',
  '../gba-mmo-protos/network_messages.proto'
]);
const ServerUpdate = root.lookupType('ServerUpdate');
const PlayerStatus = root.lookupType('PlayerStatus');
const SkillStats = root.lookupType('SkillStats');
const Direction = root.lookupEnum('Direction');
const SpriteSize = root.lookupEnum('SpriteSize');
const Skill = root.lookupEnum('Skill');
const MapLocation = root.lookupEnum('MapLocation');

export {
  ServerUpdate,
  PlayerStatus,
  Direction,
  SpriteSize,
  SkillStats,
  Skill,
  MapLocation
};
