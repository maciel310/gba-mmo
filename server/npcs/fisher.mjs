import Npc from '../npc.mjs';
import {Item} from '../proto.mjs';

const config = {
  'spawnPoints': [{'x': 176, 'y': 336}],
  'wanderRadius': 16,
  'speed': 1,
  'spriteId': 16,
};

const NPC_TYPE = 'FISHER';

const ConversationStates = {
  FIRST_WELCOME_1: 'FIRST_WELCOME_1',
  FIRST_WELCOME_2: 'FIRST_WELCOME_2',
  FIRST_WELCOME_NO_SPACE: 'FIRST_WELCOME_NO_SPACE',
  MISSING_ROD: 'MISSING_ROD',
  MISSING_ROD_NO_SPACE: 'MISSING_ROD_NO_SPACE',
  GENERAL_WELCOME: 'GENERAL_WELCOME',
};

export default class FisherNpc extends Npc {
  constructor() {
    super(config);
  }

  interact(player) {
    this.updateConversationState(player);

    switch (player.npcState[NPC_TYPE].nextMessage) {
      case ConversationStates.FIRST_WELCOME_1:
        return 'Are you interested in a bit of fishing? This is a great spot for it!';
      case ConversationStates.FIRST_WELCOME_2:
        return 'Here, take this ROD. Cast it into those rocks\mover there and you\'re sure to catch something!';
      case ConversationStates.FIRST_WELCOME_NO_SPACE:
        return 'I\'ve got something to give you, but looks like you have no room in your bag!';
      case ConversationStates.MISSING_ROD:
        return 'It looks like you\'ve gotten rid of the rod I\ngave you. Here\'s another one.';
      case ConversationStates.MISSING_ROD_NO_SPACE:
        return 'If you need another rod clear some room in\nyour bag and I\'ll give you another one.';
      case ConversationStates.GENERAL_WELCOME:
        return 'Once I caught a fish that was as tall as me!';
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
      player.addItem(Item.values.ROD);
    } else if (
        (currentState.nextMessage == ConversationStates.FIRST_WELCOME_1 ||
         currentState.nextMessage ==
             ConversationStates.FIRST_WELCOME_NO_SPACE) &&
        !player.hasInventoryRoom()) {
      currentState.nextMessage = ConversationStates.FIRST_WELCOME_NO_SPACE;
    } else if (!this._playerHasRod(player) && player.hasInventoryRoom()) {
      // TODO: Instead of giving another for free sell it instead.
      currentState.nextMessage = ConversationStates.MISSING_ROD;
      player.addItem(Item.values.ROD);
    } else if (!this._playerHasRod(player) && !player.hasInventoryRoom()) {
      currentState.nextMessage = ConversationStates.MISSING_ROD_NO_SPACE;
    } else {
      currentState.nextMessage = ConversationStates.GENERAL_WELCOME;
    }

    player.updateNpcState(NPC_TYPE, currentState);
  }

  _playerHasRod(player) {
    return player.inventory.includes(Item.values.ROD) ||
        player.bank[Item.values.ROD] > 0;
  }
}
