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
  dma3_cpy(&tile_mem[4][0], characterTiles, characterTilesLen);
  dma3_cpy(&tile_mem[4][8], character2Tiles, character2TilesLen);
}

typedef struct {
  s32 x;
  s32 y;
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

int main() {

  REG_RCNT = 0;
  REG_SIOCNT = SION_CLK_EXT | SION_ENABLE | SIO_MODE_32BIT | SIO_IRQ;

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

  p.x = 93;
  p.y = 55;

  u32 i;
  while (1) {

    key_poll();

    bool posChanged = false;
    if (key_is_down(KEY_RIGHT)) {
      p.x++;
      posChanged = true;
    }
    if (key_is_down(KEY_LEFT)) {
      p.x--;
      posChanged = true;
    }
    if (key_is_down(KEY_UP)) {
      p.y--;
      posChanged = true;
    }
    if (key_is_down(KEY_DOWN)) {
      p.y++;
      posChanged = true;
    }

    if (posChanged) {
      //REG_SIODATA32 = (p.x << 16) | p.y;
      //REG_SIOCNT |= SION_ENABLE;
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
      updateWorldObjectSpriteEntry(i, current);

      i++;
      current = current->next;
    }

    oam_copy(oam_mem, sprite, 128);
  }
}
