#include <tonc.h>

#include "all_gfx.h"
#include "protos/network_messages.pb.h"
#include "protos/world_object.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

#include "world_objects.h"
#include "serial.h"

#include "outside.h"

#define PLAYER_SCREEN_X 112
#define PLAYER_SCREEN_Y 72

OBJ_ATTR sprite[128];
s32 worldX = 0;
s32 worldY = 0;

void initializeSprites(void) {
  dma3_cpy(pal_obj_mem, spritesSharedPal, spritesSharedPalLen);
  dma3_cpy(&tile_mem[4][0], character_downTiles, character_downTilesLen);
  dma3_cpy(&tile_mem[4][8], character_upTiles, character_upTilesLen);
  dma3_cpy(&tile_mem[4][16], character_rightTiles, character_rightTilesLen);
  dma3_cpy(&tile_mem[4][24], character_leftTiles, character_leftTilesLen);
}

enum direction {
  DOWN = 0,
  UP = 8,
  RIGHT = 16,
  LEFT = 24
};

typedef struct {
  s32 x;
  s32 y;
  s32 dest_x;
  s32 dest_y;
  enum direction d;
} Player;
Player p;

void updateWorldObjectSpriteEntry(int i, struct world_object *o) {
  if (o->x > worldX - 32 && o->x < worldX + SCREEN_WIDTH && o->y > worldY - 32 && o->y < worldY + SCREEN_HEIGHT) {
    obj_unhide(&sprite[i], ATTR0_MODE(0));
    obj_set_attr(&sprite[i], ATTR0_4BPP | ATTR0_TALL | ATTR0_REG, ATTR1_SIZE_16x32, ATTR2_PALBANK(0) | ATTR2_ID(o->sprite_id));
    obj_set_pos(&sprite[i], o->x - worldX, o->y - worldY);
  } else {
    obj_hide(&sprite[i]);
  }
}

void update_player_sprite_entry() {
  obj_set_attr(&sprite[0], ATTR0_4BPP | ATTR0_TALL | ATTR0_REG, ATTR1_SIZE_16x32, ATTR2_PALBANK(0) | ATTR2_ID(p.d));
  obj_set_pos(&sprite[0], PLAYER_SCREEN_X, PLAYER_SCREEN_Y);
}

void update_player_direction(s32 h, s32 v) {
  enum direction new_direction;
  if (v > 0) {
    new_direction = DOWN;
  } else if (h > 0) {
    new_direction = RIGHT;
  } else if (h < 0) {
    new_direction = LEFT;
  } else {
    new_direction = UP;
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
  player_status.has_x = true;
  player_status.has_y = true;
  send_player_status(&player_status);
}

s32 move_towards(s32 current, s32 dest) {
  if (current > dest) {
    return -1;
  } else if (current < dest) {
    return 1;
  } else {
    return 0;
  }
}

int main() {

  irq_init(NULL);
  irq_enable(II_VBLANK);
  serial_init();

  oam_init(sprite, 128);

  REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ | DCNT_OBJ_1D;
  REG_BGCNT[0] = BG_CBB(1) | BG_SBB(0) | BG_8BPP | BG_SIZE3 | BG_PRIO(1);
  REG_BGCNT[1] = BG_CBB(1) | BG_SBB(4) | BG_8BPP | BG_SIZE3 | BG_PRIO(0);

  dma3_cpy(&tile_mem[1], OverworldTiles, OverworldTilesLen);
  dma3_cpy(&se_mem[0], Tile_Layer_1, OUTSIDE_LENGTH*sizeof(short));
  dma3_cpy(&se_mem[4], Tile_Layer_2, OUTSIDE_LENGTH*sizeof(short));
  dma3_cpy(pal_bg_mem, OverworldPal, OverworldPalLen);

  initializeSprites();

  obj_set_attr(&sprite[0], ATTR0_4BPP | ATTR0_TALL | ATTR0_REG, ATTR1_SIZE_16x32, ATTR2_PALBANK(0) | ATTR2_ID(0));
  obj_set_pos(&sprite[0], PLAYER_SCREEN_X, PLAYER_SCREEN_Y);

  p.x = p.dest_x = 192;
  p.y = p.dest_y = 152;
  p.d = DOWN;

  u32 i;
  while (1) {

    key_poll();

    if (p.dest_x != p.x || p.dest_y != p.y) {
      p.x += move_towards(p.x, p.dest_x);
      p.y += move_towards(p.y, p.dest_y);
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

        send_status();
      }
    }

    VBlankIntrWait();

    worldX = p.x - PLAYER_SCREEN_X;
    worldY = p.y - PLAYER_SCREEN_Y;

    REG_BG0HOFS = worldX;
    REG_BG0VOFS = worldY;
    REG_BG1HOFS = worldX;
    REG_BG1VOFS = worldY;

    struct world_object* current = world_object_head;
    i = 1;
    while (current != NULL) {
      current->x += move_towards(current->x, current->dest_x);
      current->y += move_towards(current->y, current->dest_y);

      updateWorldObjectSpriteEntry(i, current);

      i++;
      current = current->next;
    }

    oam_copy(oam_mem, sprite, 128);
  }
}
