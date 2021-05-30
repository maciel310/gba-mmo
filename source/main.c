#include <tonc.h>

#include "all_gfx.h"
#include "protos/network_messages.pb.h"
#include "protos/world_object.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

#include "world_objects.h"
#include "serial.h"
#include "text.h"

#include "outside.h"

#define PLAYER_SCREEN_X 112
#define PLAYER_SCREEN_Y 72

OBJ_ATTR sprite[128];
s32 worldX = 0;
s32 worldY = 0;

void initializeSprites(void) {
  dma3_cpy(pal_obj_mem, spritesSharedPal, spritesSharedPalLen);
  dma3_cpy(&tile_mem[4][0], character_upTiles, character_downTilesLen);
  dma3_cpy(&tile_mem[4][16], character_downTiles, character_upTilesLen);
  dma3_cpy(&tile_mem[4][32], character_leftTiles, character_rightTilesLen);
  dma3_cpy(&tile_mem[4][48], character_rightTiles, character_leftTilesLen);
  dma3_cpy(&tile_mem[4][64], tree1Tiles, tree1TilesLen);
  dma3_cpy(&tile_mem[4][96], tree2Tiles, tree2TilesLen);
}

typedef struct {
  s32 x;
  s32 y;
  s32 dest_x;
  s32 dest_y;
  Direction d;
} Player;
Player p;

s32 skill_levels[_Skill_ARRAYSIZE] = {1, 1};

u32 interaction_world_object_id = 0;

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

  if (skill_levels[s.skill] != s.level) {
    skill_levels[s.skill] = s.level;
    char msg[90];
    sprintf(msg, "You've gained a level! You're now at %ld", s.level);
    text_display(msg);
    message_displayed = true;
  }
}

void load_assets_main() {
  dma3_cpy(&tile_mem[1], outside_mapTiles, outside_mapTilesLen);
  dma3_cpy(&se_mem[0], outside_mapMap, outside_mapMapLen);
  dma3_cpy(pal_bg_mem, outside_mapPal, outside_mapPalLen);

  initializeSprites();
}

void load_assets_menu() {
  dma3_cpy(&tile_mem[1], skillsTiles, skillsTilesLen);
  dma3_cpy(&se_mem[0], skillsMap, skillsMapLen);
  dma3_cpy(pal_bg_mem, skillsPal, skillsPalLen);
}

void show_menu() {
  VBlankIntrWait();
  u32 originalXOffset = REG_BG0HOFS;
  u32 originalYOffset = REG_BG0VOFS;
  u32 originalDispCnt = REG_DISPCNT;
  REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_BG1;
  REG_BG0HOFS = 0;
  REG_BG0VOFS = 0;
  tte_set_margins(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  load_assets_menu();

  char buffer[8];
  sprintf(buffer, "Level %d", skill_levels[Skill_WOODCUTTING]);
  tte_write_ex(50, 50, buffer, NULL);

  VBlankIntrDelay(10);

  while(1) {
    key_poll();

    if (key_hit(KEY_START)) {
      VBlankIntrDelay(10);
      break;
    }

    VBlankIntrWait();
  }

  VBlankIntrWait();
  tte_erase_screen();
  REG_BG0HOFS = originalXOffset;
  REG_BG0VOFS = originalYOffset;
  REG_DISPCNT = originalDispCnt;
  load_assets_main();
}

int main() {

  REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;
  REG_BGCNT[0] = BG_CBB(1) | BG_SBB(0) | BG_8BPP | BG_SIZE3 | BG_PRIO(1);

  irq_init(NULL);
  irq_enable(II_VBLANK);

  oam_init(sprite, 128);
  serial_init(show_network_message, show_skill_update);
  text_init();

  load_assets_main();

  p.x = p.dest_x = 192;
  p.y = p.dest_y = 152;
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
        if ((collisionData[tileY] & (1ULL << (64 - tileX))) == 0
            && (collisionData[tileY] & (1ULL << (64 - tileXW))) == 0
            && (collisionData[tileYH] & (1ULL << (64 - tileX))) == 0
            && (collisionData[tileYH] & (1ULL << (64 - tileXW))) == 0) {
          p.dest_x += horizontalSpeed;
          p.dest_y += verticalSpeed;
        }

        should_send_status = true;
      }
    }

    struct world_object* current = world_object_head;
    i = 1;
    while (current != NULL) {
      current->x += move_towards(current->x, current->dest_x);
      current->y += move_towards(current->y, current->dest_y);

      updateWorldObjectSpriteEntry(i, current);

      i++;
      current = current->next;
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

    worldX = p.x - PLAYER_SCREEN_X;
    worldY = p.y - PLAYER_SCREEN_Y;

    REG_BG0HOFS = worldX;
    REG_BG0VOFS = worldY;

    oam_copy(oam_mem, sprite, 128);
  }
}
