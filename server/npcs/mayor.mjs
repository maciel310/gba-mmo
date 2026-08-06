import Npc from '../npc.mjs';
import {Item} from '../proto.mjs';

const config = {
  'spawnPoints': [{'x': 200, 'y': 200}],
  'wanderRadius': 16,
  'speed': 1,
  'spriteId': 16,
}

const NPC_TYPE = 'MAYOR';

const ConversationStates = {
  FIRST_WELCOME_1: 'FIRST_WELCOME_1',
  FIRST_WELCOME_2: 'FIRST_WELCOME_2',
  QUEST_START_1: 'QUEST_START_1',
  QUEST_START_2: 'QUEST_START_2',
  QUEST_NEED_WOOD: 'QUEST_NEED_WOOD',
  QUEST_NEED_ROCK: 'QUEST_NEED_ROCK',
  QUEST_COMPLETED: 'QUEST_COMPLETED',
  AFTER_QUEST: 'AFTER_QUEST',
};

export default class MayorNpc extends Npc {
  constructor() {
    super(config);
  }

  interact(player) {
    this.updateConversationState(player);

    switch (player.npcState[NPC_TYPE].conversation) {
      case ConversationStates.FIRST_WELCOME_1:
        return 'Welcome! Take a look around the town. There\'s\na bank chest to the west, and portals to';
      case ConversationStates.FIRST_WELCOME_2:
        return 'Lumber Ridge and Var Rock to the east.';
      case ConversationStates.QUEST_START_1:
        return 'If it isn\'t too much trouble, could you help me\nout? I could really use some Wood and Rock.';
      case ConversationStates.QUEST_START_2:
        return 'I\'ll reward you handsomely for your trouble!';
      case ConversationStates.QUEST_NEED_WOOD:
        return 'You can find Wood by cutting down a tree in\nLumber Ridge.';
      case ConversationStates.QUEST_NEED_ROCK:
        return 'There\'s and old quarry in Var Rock where you\ncould gather some Rock.';
      case ConversationStates.QUEST_COMPLETED:
        return 'Excellent! You\'ve brought me the supplies.\nHere\'s 1,000 coins as a thank you.';
      case ConversationStates.AFTER_QUEST:
        return 'Thanks again for your help!';
    }
  }

  updateConversationState(player) {
    let currentState = {};
    if (player.npcState.hasOwnProperty(NPC_TYPE)) {
      currentState = player.npcState[NPC_TYPE];
    }

    if (currentState.conversation == undefined) {
      currentState.conversation = ConversationStates.FIRST_WELCOME_1;
    } else if (
        currentState.conversation == ConversationStates.FIRST_WELCOME_1) {
      currentState.conversation = ConversationStates.FIRST_WELCOME_2;
    } else if (
        currentState.conversation == ConversationStates.FIRST_WELCOME_2) {
      currentState.conversation = ConversationStates.QUEST_START_1;
    } else if (currentState.conversation == ConversationStates.QUEST_START_1) {
      currentState.conversation = ConversationStates.QUEST_START_2;
    } else if (
        !currentState.questCompleted &&
        !player.inventory.includes(Item.values.WOOD)) {
      currentState.conversation = ConversationStates.QUEST_NEED_WOOD;
    } else if (
        !currentState.questCompleted &&
        !player.inventory.includes(Item.values.ROCK)) {
      currentState.conversation = ConversationStates.QUEST_NEED_ROCK;
    } else if (!currentState.questCompleted) {
      // TODO: Take items from inventory and grant quest reward.
      currentState.questCompleted = true;
      currentState.conversation = ConversationStates.QUEST_COMPLETED;

      player.takeItem(Item.values.ROCK);
      player.takeItem(Item.values.WOOD);
      player.updateCoins(1000);
    } else {
      currentState.conversation = ConversationStates.AFTER_QUEST;
    }

    player.updateNpcState(NPC_TYPE, currentState);
  }
}