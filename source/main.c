
#include <tonc.h>

#include "all_gfx.h"

OBJ_ATTR sprite[128];


void initializeSprites(void) {
  dma3_cpy(pal_obj_mem, spritesSharedPal, spritesSharedPalLen);

  dma3_cpy(&tile_mem[4][0], characterTiles, characterTilesLen);
}


typedef struct {
  u32 x;
  u32 y;
} Player;

void updatePlayerSpriteEntry(Player *p) {
  obj_set_attr(&sprite[0], ATTR0_4BPP | ATTR0_TALL | ATTR0_REG, ATTR1_SIZE_16x32, ATTR2_PALBANK(0) | ATTR2_ID(0));
  obj_set_pos(&sprite[0], p->x, p->y);
}


int main() {

  irq_init(NULL);
  irq_enable(II_VBLANK);

  oam_init(sprite, 128);

  initializeSprites();

  REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;
  REG_BGCNT[0] = BG_CBB(1) | BG_SBB(0) | BG_8BPP | BG_SIZE0;

	dma3_cpy(&tile_mem[1], mapTiles, mapTilesLen);
	dma3_cpy(&se_mem[0], mapMap, mapMapLen);
	dma3_cpy(pal_bg_mem, mapPal, mapPalLen);

  Player p;
  p.x = 93;
  p.y = 70;

  while (1) {
    key_poll();

    if (key_is_down(KEY_RIGHT) && p.x < SCREEN_WIDTH - 16) {
      p.x++;
    }
    if (key_is_down(KEY_LEFT) && p.x > 0) {
      p.x--;
    }
    if (key_is_down(KEY_UP) && p.y > 0) {
      p.y--;
    }
    if (key_is_down(KEY_DOWN) && p.y < SCREEN_HEIGHT - 32) {
      p.y++;
    }

    VBlankIntrWait();

    updatePlayerSpriteEntry(&p);
    oam_copy(oam_mem, sprite, 128);
  }
}
