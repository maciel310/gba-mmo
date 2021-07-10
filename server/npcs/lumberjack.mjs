import Npc from '../npc.mjs';
import {Item} from '../proto.mjs';

const config = {
  'spawnPoints': [{'x': 200, 'y': 200}],
  'wanderRadius': 16,
  'speed': 1,
  'spriteId': 16,
};

const NPC_TYPE = 'LUMBERJACK';

const ConversationStates = {
  FIRST_WELCOME_1: 'FIRST_WELCOME_1',
  FIRST_WELCOME_2: 'FIRST_WELCOME_2',
  FIRST_WELCOME_NO_SPACE: 'FIRST_WELCOME_NO_SPACE',
  MISSING_HATCHET: 'MISSING_HATCHET',
  MISSING_HATCHET_NO_SPACE: 'MISSING_HATCHET_NO_SPACE',
  GENERAL_WELCOME: 'GENERAL_WELCOME',
};

export default class LumberjackNpc extends Npc {
  constructor() {
    super(config);
  }

  interact(player) {
    this.updateConversationState(player);

    switch (player.npcState[NPC_TYPE].nextMessage) {
      case ConversationStates.FIRST_WELCOME_1:
        return 'Welcome to Lumber Ridge! I see this is your first time here.';
      case ConversationStates.FIRST_WELCOME_2:
        return 'Here, take this hatchet! You can use it to chop   down trees in the area for wood.';
      case ConversationStates.FIRST_WELCOME_NO_SPACE:
        return 'I\'ve got something to give you, but looks like you have no room in your bag!';
      case ConversationStates.MISSING_HATCHET:
        return 'It looks like you\'ve gotten rid of the hatchet I    gave you. Here\'s another one.';
      case ConversationStates.MISSING_HATCHET_NO_SPACE:
        return 'If you need another hatchet clear some room in   your bag and I\'ll give you another one.';
      case ConversationStates.GENERAL_WELCOME:
        return 'Welcome again to Lumber Ridge!';
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
      player.addItem(Item.values.HATCHET);
    } else if (
        (currentState.nextMessage == ConversationStates.FIRST_WELCOME_1 ||
         currentState.nextMessage ==
             ConversationStates.FIRST_WELCOME_NO_SPACE) &&
        !player.hasInventoryRoom()) {
      currentState.nextMessage = ConversationStates.FIRST_WELCOME_NO_SPACE;
    } else if (!this._playerHasHatchet(player) && player.hasInventoryRoom()) {
      // TODO: Instead of giving another for free sell it instead.
      currentState.nextMessage = ConversationStates.MISSING_HATCHET;
      player.addItem(Item.values.HATCHET);
    } else if (!this._playerHasHatchet(player) && !player.hasInventoryRoom()) {
      currentState.nextMessage = ConversationStates.MISSING_HATCHET_NO_SPACE;
    } else {
      currentState.nextMessage = ConversationStates.GENERAL_WELCOME;
    }

    player.updateNpcState(NPC_TYPE, currentState);
  }

  _playerHasHatchet(player) {
    return player.inventory.includes(Item.values.HATCHET) ||
        player.bank[Item.values.HATCHET] > 0;
  }
}
