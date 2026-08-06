#include <tonc.h>

#include "all_gfx.h"
#include "protos/network_messages.pb.h"
#include "protos/world_object.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

#include "world_objects.h"
#include "serial.h"
#include "text.h"

#include "lumber_ridge.h"
#include "var_rock.h"
#include "town.h"

#define PLAYER_SCREEN_X 112
#define PLAYER_SCREEN_Y 72

OBJ_ATTR sprite[128];
s32 worldX = 0;
s32 worldY = 0;

u32 skillSpriteLut[_Skill_ARRAYSIZE] = {
  0, // Skill_UNKNOWN_SKILL
  256, // Skill_WOODCUTTING
  264, // Skill_MINING
  272, // Skill_FISHING
};

void initialize_sprites() {
  dma3_cpy(pal_obj_mem, spritesSharedPal, spritesSharedPalLen);
  dma3_cpy(&tile_mem[4][0], character_upTiles, character_downTilesLen);
  dma3_cpy(&tile_mem[4][16], character_downTiles, character_upTilesLen);
  dma3_cpy(&tile_mem[4][32], character_leftTiles, character_rightTilesLen);
  dma3_cpy(&tile_mem[4][48], character_rightTiles, character_leftTilesLen);
  dma3_cpy(&tile_mem[4][64], tree1Tiles, tree1TilesLen);
  dma3_cpy(&tile_mem[4][96], tree2Tiles, tree2TilesLen);
  dma3_cpy(&tile_mem[4][128], chestTiles, chestTilesLen);
  dma3_cpy(&tile_mem[4][160], rock1Tiles, rock1TilesLen);
  dma3_cpy(&tile_mem[4][192], rock2Tiles, rock2TilesLen);
  dma3_cpy(&tile_mem[4][224], fishing_spotTiles, fishing_spotTilesLen);

  dma3_cpy(&tile_mem[4][skillSpriteLut[Skill_WOODCUTTING]], axeTiles, axeTilesLen);
  dma3_cpy(&tile_mem[4][skillSpriteLut[Skill_MINING]], pickTiles, pickTilesLen);
  dma3_cpy(&tile_mem[4][skillSpriteLut[Skill_FISHING]], rodTiles, rodTilesLen);
}

u8 itemSpriteLut[_Item_ARRAYSIZE] = {
  0, // UNKNOWN_ITEM
  32, // WOOD
  64, // ROCK
  96, // HATCHET
  128, // PICKAXE
  160, // ROD
  192, // RAW_FISH
  224, // COOKED_FISH
};

#define MENU_OUTLINE_32_SPRITE 0

void initialize_menu_sprites() {
  dma3_cpy(pal_obj_mem, menu_spritesSharedPal, menu_spritesSharedPalLen);

  dma3_cpy(&tile_mem[4][MENU_OUTLINE_32_SPRITE], outline_32Tiles, outline_32TilesLen);

  dma3_cpy(&tile_mem[4][itemSpriteLut[Item_WOOD]], woodTiles, woodTilesLen);
  dma3_cpy(&tile_mem[4][itemSpriteLut[Item_ROCK]], rockTiles, rockTilesLen);
  dma3_cpy(&tile_mem[4][itemSpriteLut[Item_HATCHET]], hatchetTiles, hatchetTilesLen);
  dma3_cpy(&tile_mem[4][itemSpriteLut[Item_PICKAXE]], pickaxeTiles, pickaxeTilesLen);
  dma3_cpy(&tile_mem[4][itemSpriteLut[Item_ROD]], fishing_rodTiles, fishing_rodTilesLen);
  dma3_cpy(&tile_mem[4][itemSpriteLut[Item_RAW_FISH]], fish_rawTiles, fish_rawTilesLen);
  dma3_cpy(&tile_mem[4][itemSpriteLut[Item_COOKED_FISH]], fish_cookedTiles, fish_cookedTilesLen);
}

// NOTE: If updated change in proto and server code.
#define INVENTORY_SIZE 18

typedef struct {
  s32 x;
  s32 y;
  s32 dest_x;
  s32 dest_y;
  Direction d;
  Item inventory[INVENTORY_SIZE];
  s32 coin_count;
} Player;
Player p;

s32 skill_levels[_Skill_ARRAYSIZE] = {-1, -1, -1, -1};

s32 bank[_Item_ARRAYSIZE] = {0, 0, 0, 0, 0};

u32 interaction_world_object_id = 0;

s32 deposit_inventory_index = -1;
Item withdraw_bank_item_type = Item_UNKNOWN_ITEM;

Skill active_skill = Skill_UNKNOWN_SKILL;
bool new_active_skill = false;

MapLocation new_map = MapLocation_UNKNOWN_MAP;
MapLocation current_map = MapLocation_TOWN;
const u64 *collisionData = TOWN_collisionData;

Interface launch_interface = Interface_UNKNOWN_INTERFACE;

bool first_server_update_received = false;

void updateWorldObjectSpriteEntry(int i, struct world_object *o) {
  if (o->x > worldX - 32 && o->x < worldX + SCREEN_WIDTH && o->y > worldY - 32 && o->y < worldY + SCREEN_HEIGHT) {
    obj_unhide(&sprite[i], ATTR0_MODE(0));
    obj_set_attr(
      &sprite[i],
      ATTR0_8BPP | sprite_type_lut[o->sprite_size] | ATTR0_REG,
      sprite_size_lut[o->sprite_size], ATTR2_PALBANK(0) | ATTR2_ID(o->sprite_id) | ATTR2_PRIO(1));
    obj_set_pos(&sprite[i], o->x - worldX, o->y - worldY);
    o->is_on_screen = true;
  } else {
    obj_hide(&sprite[i]);
    o->is_on_screen = false;
  }
}

void update_player_sprite_entry() {
  int sprite_id;
  switch (p.d) {
    case Direction_UP:
      sprite_id = 0;
      break;
    case Direction_DOWN:
      sprite_id = 16;
      break;
    case Direction_LEFT:
      sprite_id = 32;
      break;
    default:
      sprite_id = 48;
      break;
  }
  obj_set_attr(&sprite[0], ATTR0_8BPP | ATTR0_TALL | ATTR0_REG, ATTR1_SIZE_16x32, ATTR2_PALBANK(0) | ATTR2_ID(sprite_id));
  obj_set_pos(&sprite[0], PLAYER_SCREEN_X, PLAYER_SCREEN_Y);

  if (active_skill != Skill_UNKNOWN_SKILL) {
    obj_unhide(&sprite[1], ATTR0_MODE(0));
    obj_set_attr(&sprite[1], ATTR0_8BPP | ATTR0_SQUARE | ATTR0_REG, ATTR1_SIZE_16x16, ATTR2_PALBANK(0) | ATTR2_ID(skillSpriteLut[active_skill]));
    obj_set_pos(&sprite[1], PLAYER_SCREEN_X, PLAYER_SCREEN_Y - 16);
  } else {
    obj_hide(&sprite[1]);
  }
}

void update_player_direction(s32 h, s32 v) {
  Direction new_direction;
  if (v > 0) {
    new_direction = Direction_DOWN;
  } else if (h > 0) {
    new_direction = Direction_RIGHT;
  } else if (h < 0) {
    new_direction = Direction_LEFT;
  } else {
    new_direction = Direction_UP;
  }

  if (new_direction != p.d) {
    p.d = new_direction;
    update_player_sprite_entry();
  }
}

void send_status() {
  PlayerStatus player_status = PlayerStatus_init_default;
  player_status.x = p.x;
  player_status.y = p.y;
  player_status.direction = p.d;
  player_status.has_x = true;
  player_status.has_y = true;
  player_status.has_direction = true;

  if (interaction_world_object_id != 0) {
    player_status.interaction_object_id = interaction_world_object_id;
    player_status.has_interaction_object_id = true;
    interaction_world_object_id = 0;
  }

  if (deposit_inventory_index != -1) {
    player_status.deposit_inventory_index = deposit_inventory_index;
    player_status.has_deposit_inventory_index = true;
    deposit_inventory_index = -1;
  }

  if (withdraw_bank_item_type != Item_UNKNOWN_ITEM) {
    player_status.withdraw_bank_item = withdraw_bank_item_type;
    player_status.has_withdraw_bank_item = true;
    withdraw_bank_item_type = Item_UNKNOWN_ITEM;
  }

  send_player_status(&player_status);
}

s32 move_towards(s32 current, s32 dest) {
  if (abs(dest - current) > 32) {
    return dest - current;
  } else if (current > dest) {
    return -1;
  } else if (current < dest) {
    return 1;
  } else {
    return 0;
  }
}

bool message_displayed = false;
void show_network_message(CSTR message) {
  text_display(message);
  message_displayed = true;
}

void show_skill_update(SkillStats s) {
  if (s.skill == Skill_UNKNOWN_SKILL) {
    return;
  }

  if (skill_levels[s.skill] == -1) {
    skill_levels[s.skill] = s.level;
  } else if (skill_levels[s.skill] != s.level) {
    skill_levels[s.skill] = s.level;
    char msg[90];
    sprintf(msg, "You've gained a level! You're now at %ld", s.level);
    text_display(msg);
    message_displayed = true;
  }
}

void update_server_player_position(PlayerStatus s) {
  p.x = p.dest_x = s.x;
  p.y = p.dest_y = s.y;
}

void update_bank(BankEntry b) {
  bank[b.item] = b.quantity;
}

void update_state_with_server_update(ServerUpdate s) {
  first_server_update_received = true;

  if (s.has_network_message) {
    show_network_message(s.network_message);
  }
  if (s.has_launch_interface) {
    launch_interface = s.launch_interface;
  }
  if (s.has_player_status) {
    update_server_player_position(s.player_status);
  }
  if (s.has_interacting_skill && active_skill != s.interacting_skill) {
    active_skill = s.interacting_skill;
    new_active_skill = true;
  } else if (!s.has_interacting_skill && active_skill != Skill_UNKNOWN_SKILL) {
    active_skill = Skill_UNKNOWN_SKILL;
    new_active_skill = true;
  }

  if (s.current_map != current_map) {
    new_map = s.current_map;
  }

  for (u32 i = 0; i < INVENTORY_SIZE; i++) {
    p.inventory[i] = s.inventory[i];
  }

  if (s.has_coin_count) {
    p.coin_count = s.coin_count;
  }
}

bool new_world_object_received = false;
void world_object_received(WorldObject w) {
  bool is_new = update_world_object(w);
  new_world_object_received = new_world_object_received | is_new;
}

void world_object_removed(u32 id) {
  remove_world_object(id);
  new_world_object_received = true;
}

void load_assets_main() {
  if (current_map == MapLocation_LUMBER_RIDGE) {
    dma3_cpy(&tile_mem[1], lumber_ridge_mapTiles, lumber_ridge_mapTilesLen);
    dma3_cpy(&se_mem[0], lumber_ridge_mapMap, lumber_ridge_mapMapLen);
    dma3_cpy(pal_bg_mem, lumber_ridge_mapPal, lumber_ridge_mapPalLen);
    collisionData = LUMBER_RIDGE_collisionData;
  } else if (current_map == MapLocation_VAR_ROCK) {
    dma3_cpy(&tile_mem[1], var_rock_mapTiles, var_rock_mapTilesLen);
    dma3_cpy(&se_mem[0], var_rock_mapMap, var_rock_mapMapLen);
    dma3_cpy(pal_bg_mem, var_rock_mapPal, var_rock_mapPalLen);
    collisionData = VAR_ROCK_collisionData;
  } else if (current_map == MapLocation_TOWN) {
    dma3_cpy(&tile_mem[1], town_mapTiles, town_mapTilesLen);
    dma3_cpy(&se_mem[0], town_mapMap, town_mapMapLen);
    dma3_cpy(pal_bg_mem, town_mapPal, town_mapPalLen);
    collisionData = TOWN_collisionData;
  }

  initialize_sprites();
}

void load_assets_skills() {
  dma3_cpy(&tile_mem[1], skillsTiles, skillsTilesLen);
  dma3_cpy(&se_mem[0], skillsMap, skillsMapLen);
  dma3_cpy(pal_bg_mem, skillsPal, skillsPalLen);
}

void load_assets_inventory() {
  dma3_cpy(&tile_mem[1], inventoryTiles, inventoryTilesLen);
  dma3_cpy(&se_mem[0], inventoryMap, inventoryMapLen);
  dma3_cpy(pal_bg_mem, inventoryPal, inventoryPalLen);
}

void load_assets_bank_withdraw() {
  dma3_cpy(&tile_mem[1], bank_withdrawTiles, bank_withdrawTilesLen);
  dma3_cpy(&se_mem[0], bank_withdrawMap, bank_withdrawMapLen);
  dma3_cpy(pal_bg_mem, bank_withdrawPal, bank_withdrawPalLen);
}

void load_assets_bank_deposit() {
  dma3_cpy(&tile_mem[1], bank_depositTiles, bank_depositTilesLen);
  dma3_cpy(&se_mem[0], bank_depositMap, bank_depositMapLen);
  dma3_cpy(pal_bg_mem, bank_depositPal, bank_depositPalLen);
}

void load_assets_connecting() {
  dma3_cpy(&tile_mem[1], connectingTiles, connectingTilesLen);
  dma3_cpy(&se_mem[0], connectingMap, connectingMapLen);
  dma3_cpy(pal_bg_mem, connectingPal, connectingPalLen);
}

void show_skill_stats() {
  load_assets_skills();

  char buffer[8];
  sprintf(buffer, "Level %d", skill_levels[Skill_WOODCUTTING]);
  tte_write_ex(50, 42, buffer, NULL);

  sprintf(buffer, "Level %d", skill_levels[Skill_MINING]);
  tte_write_ex(50, 82, buffer, NULL);

  sprintf(buffer, "Level %d", skill_levels[Skill_FISHING]);
  tte_write_ex(50, 122, buffer, NULL);
}

void show_inventory_sprite(OBJ_ATTR* menu_sprite, u32 i, u32 row, u32 col, Item item) {
  obj_unhide(&menu_sprite[i], ATTR0_MODE(0));
  obj_set_attr(
    &menu_sprite[i],
    ATTR0_8BPP | ATTR0_SQUARE | ATTR0_REG,
    ATTR1_SIZE_32x32, ATTR2_PALBANK(0) | itemSpriteLut[item] | ATTR2_PRIO(1));
  obj_set_pos(&menu_sprite[i], 12 + col * 36, 32 + row * 40);
}

void show_outline_sprite(OBJ_ATTR* menu_sprite, u32 x, u32 y) {
  obj_unhide(&menu_sprite[127], ATTR0_MODE(0));
  obj_set_attr(
    &menu_sprite[127],
    ATTR0_8BPP | ATTR0_SQUARE | ATTR0_REG,
    ATTR1_SIZE_32x32, ATTR2_PALBANK(0) | MENU_OUTLINE_32_SPRITE | ATTR2_PRIO(1));
  obj_set_pos(&menu_sprite[127], x, y);
}

#define COLUMN_COUNT 6
#define ROW_COUNT INVENTORY_SIZE / COLUMN_COUNT
void show_inventory() {
  load_assets_inventory();
  initialize_menu_sprites();
}

void update_inventory(OBJ_ATTR* menu_sprite) {
  for (u32 row = 0; row < ROW_COUNT; row++) {
    for (u32 col = 0; col < COLUMN_COUNT; col++) {
      u32 i = row * COLUMN_COUNT + col;

      Item item = p.inventory[i];
      if (item == Item_UNKNOWN_ITEM) {
        obj_hide(&menu_sprite[i]);
      } else {
        show_inventory_sprite(menu_sprite, i, row, col, item);
      }
    }
  }
}

void show_coin_count() {
  char coin_buffer[20];
  sprintf(coin_buffer, "%d coins", p.coin_count);
  tte_write_ex(20, 144, coin_buffer, NULL);
}

void show_bank_withdraw() {
  load_assets_bank_withdraw();
  initialize_menu_sprites();
}

void update_bank_withdraw(OBJ_ATTR* menu_sprite) {
  tte_erase_screen();

  char qty_buffer[3];
  for (u32 itemIndex = _Item_MIN+1; itemIndex <= _Item_MAX; itemIndex++) {
    u32 row = (itemIndex-1) / (COLUMN_COUNT / 2);
    u32 col = ((itemIndex-1) % (COLUMN_COUNT / 2)) * 2;

    if (bank[itemIndex] != 0) {
      show_inventory_sprite(menu_sprite, itemIndex, row, col, itemIndex);
      sprintf(qty_buffer, "%d", bank[itemIndex]);
      tte_write_ex(48 + col * 36, 48 + row * 40, qty_buffer, NULL);
    } else {
      obj_hide(&menu_sprite[itemIndex]);
    }
  }

  show_coin_count();
}

void show_bank_deposit() {
  load_assets_bank_deposit();
  initialize_menu_sprites();
}

void update_bank_deposit(OBJ_ATTR* menu_sprite) {
  for (u32 row = 0; row < ROW_COUNT; row++) {
    for (u32 col = 0; col < COLUMN_COUNT; col++) {
      u32 i = row * COLUMN_COUNT + col;

      Item item = p.inventory[i];
      if (item == Item_UNKNOWN_ITEM) {
        obj_hide(&menu_sprite[i]);
      } else {
        show_inventory_sprite(menu_sprite, i, row, col, item);
      }
    }
  }

  show_coin_count();
}

typedef enum _menu_type { 
  MENU_TYPE_SKILL_STATS = 0, 
  MENU_TYPE_INVENTORY = 1,
  MENU_TYPE_WITHDRAW = 2,
  MENU_TYPE_DEPOSIT = 3,
} MenuType;

void show_menu() {
  VBlankIntrWait();

  OBJ_ATTR menu_sprite[128];
  oam_init(menu_sprite, 128);
  oam_copy(oam_mem, menu_sprite, 128);

  u32 originalXOffset = REG_BG0HOFS;
  u32 originalYOffset = REG_BG0VOFS;
  u32 originalDispCnt = REG_DISPCNT;
  REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ | DCNT_OBJ_1D;
  REG_BG0HOFS = 0;
  REG_BG0VOFS = 0;
  tte_set_margins(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  MenuType menu_type = MENU_TYPE_SKILL_STATS;
  s32 outline_index = -1;
  bool outline_changed = false;

  if (launch_interface == Interface_BANK) {
    menu_type = MENU_TYPE_WITHDRAW;
    launch_interface = Interface_UNKNOWN_INTERFACE;
    outline_index = 0;
    outline_changed = true;
  }

  bool menu_type_changed = true;
  bool should_send_status;

  while(1) {
    should_send_status = false;

    key_poll();

    if (menu_type == MENU_TYPE_WITHDRAW) {
      if (key_hit(KEY_R)) {
        menu_type = MENU_TYPE_DEPOSIT;
        menu_type_changed = true;
        outline_index = 0;
        outline_changed = true;
      } else if (key_hit(KEY_LEFT)) {
        if (outline_index > 0) {
          outline_index -= 2;
          outline_changed = true;
        }
      } else if (key_hit(KEY_RIGHT)) {
        if (outline_index < INVENTORY_SIZE-3) {
          outline_index += 2;
          outline_changed = true;
        }
      } else if (key_hit(KEY_A)) {
        Item item = (Item) (outline_index / 2 + 1);
        if (bank[item] > 0) {
          withdraw_bank_item_type = item;
          should_send_status = true;
        }
      }
    } else if (menu_type == MENU_TYPE_DEPOSIT) {
      if (key_hit(KEY_L)) {
        menu_type = MENU_TYPE_WITHDRAW;
        menu_type_changed = true;

        outline_index = 0;
        outline_changed = true;
      } else if (key_hit(KEY_LEFT)) {
        if (outline_index > 0) {
          outline_index--;
          outline_changed = true;
        }
      } else if (key_hit(KEY_RIGHT)) {
        if (outline_index < INVENTORY_SIZE-1) {
          outline_index++;
          outline_changed = true;
        }
      } else if (key_hit(KEY_A)) {
        if (p.inventory[outline_index] != Item_UNKNOWN_ITEM) {
          deposit_inventory_index = outline_index;
          should_send_status = true;
        }
      }
    } else {
      if (key_hit(KEY_L)) {
        menu_type = MENU_TYPE_SKILL_STATS;
        menu_type_changed = true;
      } else if (key_hit(KEY_R)) {
        menu_type = MENU_TYPE_INVENTORY;
        menu_type_changed = true;
      }
    }

    if (key_hit(KEY_START)) {
      VBlankIntrDelay(10);
      break;
    }

    switch (menu_type) {
      case MENU_TYPE_SKILL_STATS:
        if (menu_type_changed) {
          menu_type_changed = false;
          oam_init(menu_sprite, 128);
          tte_erase_screen();
          show_skill_stats();
        }
        break;
      case MENU_TYPE_INVENTORY:
        if (menu_type_changed) {
          menu_type_changed = false;
          oam_init(menu_sprite, 128);
          tte_erase_screen();
          show_inventory();
        }

        update_inventory(menu_sprite);
        break;
      case MENU_TYPE_WITHDRAW:
        if (menu_type_changed) {
          menu_type_changed = false;
          oam_init(menu_sprite, 128);
          tte_erase_screen();
          show_bank_withdraw();
        }

        update_bank_withdraw(menu_sprite);
        break;
      case MENU_TYPE_DEPOSIT:
        if (menu_type_changed) {
          menu_type_changed = false;
          oam_init(menu_sprite, 128);
          tte_erase_screen();
          show_bank_deposit();
        }

        update_bank_deposit(menu_sprite);
        break;
    }

    if (outline_index != -1 && outline_changed) {
      outline_changed = false;
      u32 outline_x = 12 + (outline_index % COLUMN_COUNT) * 36;
      u32 outline_y = 32 + (outline_index / COLUMN_COUNT) * 40;
      show_outline_sprite(menu_sprite, outline_x, outline_y);
    }

    if (should_send_status) {
      send_status();
    }

    VBlankIntrWait();

    oam_copy(oam_mem, menu_sprite, 128);
  }

  VBlankIntrWait();
  tte_erase_screen();
  REG_BG0HOFS = originalXOffset;
  REG_BG0VOFS = originalYOffset;
  REG_DISPCNT = originalDispCnt;
  load_assets_main();
  oam_copy(oam_mem, sprite, 128);
}

int main() {

  REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;
  REG_BGCNT[0] = BG_CBB(1) | BG_SBB(0) | BG_8BPP | BG_SIZE3 | BG_PRIO(1);

  irq_init(NULL);
  irq_enable(II_VBLANK);

  oam_init(sprite, 128);
  serial_init(update_state_with_server_update, show_skill_update, world_object_received, world_object_removed, update_bank);
  text_init();

  load_assets_connecting();

  while (!first_server_update_received) {
    VBlankIntrWait();
  }

  // Delay an additional 5 seconds to ensure we're fully loaded.
  // TODO: Remove this hack once the disconnect bug is fixed.
  VBlankIntrDelay(60 * 5);

  load_assets_main();

  p.d = Direction_DOWN;
  update_player_sprite_entry();

  bool should_send_status;
  u32 i;
  while (1) {
    should_send_status = false;

    key_poll();

    if (key_hit(KEY_START)) {
      show_menu();
    }

    if (launch_interface != Interface_UNKNOWN_INTERFACE) {
      show_menu();
    }

    if (p.dest_x != p.x || p.dest_y != p.y) {
      p.x += move_towards(p.x, p.dest_x);
      p.y += move_towards(p.y, p.dest_y);

      if (p.x == p.dest_x && p.y == p.dest_y) {
        should_send_status = true;
      }
    } else {
      s32 horizontalSpeed = key_tri_horz() * 8;
      s32 verticalSpeed = horizontalSpeed == 0 ? key_tri_vert() * 8 : 0;

      if (horizontalSpeed != 0 || verticalSpeed != 0) {
        update_player_direction(horizontalSpeed, verticalSpeed);

        s32 tileX = (p.x + horizontalSpeed) / 8 + 1;
        s32 tileXW = (p.x + horizontalSpeed + 8) / 8 + 1;
        s32 tileY = (p.y + verticalSpeed + 16) / 8;
        s32 tileYH = (p.y + verticalSpeed + 16) / 8 + 1;
        if (
          // Map collisions
          ((collisionData[tileY] & (1ULL << (64 - tileX))) == 0
            && (collisionData[tileY] & (1ULL << (64 - tileXW))) == 0
            && (collisionData[tileYH] & (1ULL << (64 - tileX))) == 0
            && (collisionData[tileYH] & (1ULL << (64 - tileXW))) == 0)
            // Sprite collisions
            && sprite_collision_map[tileX][tileY] == false) {
          p.dest_x += horizontalSpeed;
          p.dest_y += verticalSpeed;
        }

        should_send_status = true;
      }
    }

    bool solid_world_object_moved = false;
    struct world_object* current = world_object_head;
    i = 2;
    while (current != NULL) {
      if (current->is_solid && (current->x != current->dest_x || current->y != current->dest_y)) {
        solid_world_object_moved = true;
      }
      current->x += move_towards(current->x, current->dest_x);
      current->y += move_towards(current->y, current->dest_y);

      updateWorldObjectSpriteEntry(i, current);

      i++;
      current = current->next;
    }
    for (; i < 128; i++) {
      obj_hide(&sprite[i]);
    }

    if (new_world_object_received || solid_world_object_moved) {
      new_world_object_received = false;
      regenerate_sprite_collision_map();
    }

    // Interact with WorldObject
    if (key_hit(KEY_A)) {
      if (message_displayed) {
        text_close();
        message_displayed = false;
      } else {
        interaction_world_object_id = find_next_to(p.x, p.y, p.d);
        if (interaction_world_object_id != 0) {
          should_send_status = true;
        }
      }
    }

    if (should_send_status) {
      send_status();
    }

    VBlankIntrWait();

    if (new_map != MapLocation_UNKNOWN_MAP) {
      current_map = new_map;
      new_map = MapLocation_UNKNOWN_MAP;

      load_assets_main();
      clear_all_world_objects();
      for (u32 i = 2; i < 128; i++) {
        obj_hide(&sprite[i]);
      }
    }

    worldX = p.x - PLAYER_SCREEN_X;
    worldY = p.y - PLAYER_SCREEN_Y;

    REG_BG0HOFS = worldX;
    REG_BG0VOFS = worldY;

    if (new_active_skill) {
      update_player_sprite_entry();
    }

    oam_copy(oam_mem, sprite, 128);
  }
}
