import Npc from '../npc.mjs';
import {Item} from '../proto.mjs';

const config = {
  'spawnPoints': [{'x': 200, 'y': 200}],
  'wanderRadius': 16,
  'speed': 1,
  'spriteId': 16,
};

const NPC_TYPE = 'MINER';

const ConversationStates = {
  FIRST_WELCOME_1: 'FIRST_WELCOME_1',
  FIRST_WELCOME_2: 'FIRST_WELCOME_2',
  FIRST_WELCOME_NO_SPACE: 'FIRST_WELCOME_NO_SPACE',
  MISSING_PICKAXE: 'MISSING_PICKAXE',
  MISSING_PICKAXE_NO_SPACE: 'MISSING_PICKAXE_NO_SPACE',
  GENERAL_WELCOME: 'GENERAL_WELCOME',
};

export default class MinerNpc extends Npc {
  constructor() {
    super(config);
  }

  interact(player) {
    this.updateConversationState(player);

    switch (player.npcState[NPC_TYPE].nextMessage) {
      case ConversationStates.FIRST_WELCOME_1:
        return 'Hello there, and welcome to Var Rock! Feel free to use our quarry whenever you\'d like.';
      case ConversationStates.FIRST_WELCOME_2:
        return 'Here, take this PICKAXE. You can use it to \ngather some rocks.';
      case ConversationStates.FIRST_WELCOME_NO_SPACE:
        return 'I\'ve got something to give you, but looks like you have no room in your bag!';
      case ConversationStates.MISSING_PICKAXE:
        return 'It looks like you\'ve gotten rid of the pickaxe I\ngave you. Here\'s another one.';
      case ConversationStates.MISSING_PICKAXE_NO_SPACE:
        return 'If you need another pickaxe clear some room in\nyour bag and I\'ll give you another one.';
      case ConversationStates.GENERAL_WELCOME:
        return 'Welcome again to Var Rock!';
    }
  }

  updateConversationState(player) {
    let currentState = {};
    if (player.npcState.hasOwnProperty(NPC_TYPE)) {
      currentState = player.npcState[NPC_TYPE];
    }

    if (currentState.nextMessage == undefined) {
      currentState.nextMessage = ConversationStates.FIRST_WELCOME_1;
    } else if (
        (currentState.nextMessage == ConversationStates.FIRST_WELCOME_1 ||
         currentState.nextMessage ==
             ConversationStates.FIRST_WELCOME_NO_SPACE) &&
        player.hasInventoryRoom()) {
      currentState.nextMessage = ConversationStates.FIRST_WELCOME_2;
      player.addItem(Item.values.PICKAXE);
    } else if (
        (currentState.nextMessage == ConversationStates.FIRST_WELCOME_1 ||
         currentState.nextMessage ==
             ConversationStates.FIRST_WELCOME_NO_SPACE) &&
        !player.hasInventoryRoom()) {
      currentState.nextMessage = ConversationStates.FIRST_WELCOME_NO_SPACE;
    } else if (!this._playerHasPickaxe(player) && player.hasInventoryRoom()) {
      // TODO: Instead of giving another for free sell it instead.
      currentState.nextMessage = ConversationStates.MISSING_PICKAXE;
      player.addItem(Item.values.PICKAXE);
    } else if (!this._playerHasPickaxe(player) && !player.hasInventoryRoom()) {
      currentState.nextMessage = ConversationStates.MISSING_PICKAXE_NO_SPACE;
    } else {
      currentState.nextMessage = ConversationStates.GENERAL_WELCOME;
    }

    player.updateNpcState(NPC_TYPE, currentState);
  }

  _playerHasPickaxe(player) {
    return player.inventory.includes(Item.values.PICKAXE) ||
        player.bank[Item.values.PICKAXE] > 0;
  }
}
