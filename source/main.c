#include <tonc.h>

#include "all_gfx.h"
#include "protos/network_messages.pb.h"
#include "protos/world_object.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

#include "world_objects.h"
#include "serial.h"

OBJ_ATTR sprite[128];


void initializeSprites(void) {
  dma3_cpy(pal_obj_mem, spritesSharedPal, spritesSharedPalLen);

  dma3_cpy(&tile_mem[4][0], characterTiles, characterTilesLen);
  dma3_cpy(&tile_mem[4][8], character2Tiles, character2TilesLen);
}

typedef struct {
  u32 object_id;
  u32 x;
  u32 y;
} Player;
Player p;

void updatePlayerSpriteEntry(Player *p) {
  obj_set_attr(&sprite[0], ATTR0_4BPP | ATTR0_TALL | ATTR0_REG, ATTR1_SIZE_16x32, ATTR2_PALBANK(0) | ATTR2_ID(0));
  obj_set_pos(&sprite[0], p->x, p->y);
}

void updateWorldObjectSpriteEntry(int i, struct world_object *o) {
  obj_set_attr(&sprite[i], ATTR0_4BPP | ATTR0_TALL | ATTR0_REG, ATTR1_SIZE_16x32, ATTR2_PALBANK(0) | ATTR2_ID(o->sprite_id));
  obj_set_pos(&sprite[i], o->x, o->y);
}

int main() {

  REG_RCNT = 0;
  REG_SIOCNT = SION_CLK_EXT | SION_ENABLE | SIO_MODE_32BIT | SIO_IRQ;

  irq_init(NULL);
  irq_enable(II_VBLANK);
  serial_init();

  oam_init(sprite, 128);

  initializeSprites();

  REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;
  REG_BGCNT[0] = BG_CBB(1) | BG_SBB(0) | BG_8BPP | BG_SIZE0;

	dma3_cpy(&tile_mem[1], mapTiles, mapTilesLen);
	dma3_cpy(&se_mem[0], mapMap, mapMapLen);
	dma3_cpy(pal_bg_mem, mapPal, mapPalLen);

  p.x = 93;
  p.y = 55;

  u32 serial_data[14][6] = {
    {20, 0x0a080801, 0x10291828, 0x20080a08, 0x08021029, 0x18282008},
    {20, 0x0a080801, 0x102a1828, 0x20080a08, 0x0802102a, 0x18282008},
    {20, 0x0a080801, 0x102b1828, 0x20080a08, 0x0802102b, 0x18282008},
    {20, 0x0a080801, 0x102c1828, 0x20080a08, 0x0802102c, 0x18282008},
    {20, 0x0a080801, 0x102d1828, 0x20080a08, 0x0802102d, 0x18282008},
    {20, 0x0a080801, 0x102e1828, 0x20080a08, 0x0802102e, 0x18282008},
    {20, 0x0a080801, 0x102d1828, 0x20080a08, 0x0802102f, 0x18282008},
    {20, 0x0a080801, 0x102c1828, 0x20080a08, 0x08021030, 0x18282008},
    {20, 0x0a080801, 0x102b1828, 0x20080a08, 0x08021031, 0x18282008},
    {20, 0x0a080801, 0x102a1828, 0x20080a08, 0x08021031, 0x18282008},
    {20, 0x0a080801, 0x10291828, 0x20080a08, 0x08021031, 0x18282008},
    {20, 0x0a080801, 0x10281828, 0x20080a08, 0x08021031, 0x18282008},
    {20, 0x0a080801, 0x10271828, 0x20080a08, 0x08021031, 0x18282008},
    {20, 0x0a080801, 0x10261828, 0x20080a08, 0x08021031, 0x18282008},
  };
  u32 serial_data_index = 0;

  u32 i;
  while (1) {

    key_poll();

    bool posChanged = false;
    if (key_is_down(KEY_RIGHT) && p.x < SCREEN_WIDTH - 16) {
      p.x++;
      posChanged = true;
    }
    if (key_is_down(KEY_LEFT) && p.x > 0) {
      p.x--;
      posChanged = true;
    }
    if (key_is_down(KEY_UP) && p.y > 0) {
      p.y--;
      posChanged = true;
    }
    if (key_is_down(KEY_DOWN) && p.y < SCREEN_HEIGHT - 32) {
      p.y++;
      posChanged = true;
    }

    if (posChanged) {
      REG_SIODATA32 = (p.x << 16) | p.y;
      REG_SIOCNT |= SION_ENABLE;
    }

    for (i = 0; i < 6; i++) {
      REG_SIODATA32 = serial_data[serial_data_index][i];
      handle_serial();
    }
    serial_data_index = (serial_data_index + 1) % 14;

    VBlankIntrWait();

    updatePlayerSpriteEntry(&p);

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
