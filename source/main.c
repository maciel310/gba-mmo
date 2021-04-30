#include <tonc.h>

#include "all_gfx.h"
#include "protos/network_messages.pb.h"
#include "protos/world_object.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

OBJ_ATTR sprite[128];


void initializeSprites(void) {
  dma3_cpy(pal_obj_mem, spritesSharedPal, spritesSharedPalLen);

  dma3_cpy(&tile_mem[4][0], characterTiles, characterTilesLen);
}


typedef struct {
  u32 x;
  u32 y;
} Player;
Player p;
Player p2;

void updatePlayerSpriteEntry(u32 i, Player *p) {
  obj_set_attr(&sprite[i], ATTR0_4BPP | ATTR0_TALL | ATTR0_REG, ATTR1_SIZE_16x32, ATTR2_PALBANK(0) | ATTR2_ID(0));
  obj_set_pos(&sprite[i], p->x, p->y);
}

void handle_serial() {
  u32 remoteXY = REG_SIODATA32;
  p2.x = (remoteXY >> 16);
  p2.y = (remoteXY & 0x00FF);
}

bool decode_world_object(pb_istream_t *stream, const pb_field_t *field, void **arg) {
  WorldObject message = WorldObject_init_zero;
  pb_decode(stream, WorldObject_fields, &message);
  p2.x = message.x;
  p2.y = message.y;

  return true;
}

int main() {

  REG_RCNT = 0;
  REG_SIOCNT = SION_CLK_EXT | SION_ENABLE | SIO_MODE_32BIT | SIO_IRQ;

  irq_init(NULL);
  irq_enable(II_VBLANK);
  irq_add(II_SERIAL, handle_serial);

  oam_init(sprite, 128);

  initializeSprites();

  REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;
  REG_BGCNT[0] = BG_CBB(1) | BG_SBB(0) | BG_8BPP | BG_SIZE0;

	dma3_cpy(&tile_mem[1], mapTiles, mapTilesLen);
	dma3_cpy(&se_mem[0], mapMap, mapMapLen);
	dma3_cpy(pal_bg_mem, mapPal, mapPalLen);

  p.x = 93;
  p.y = 55;

  uint8_t buffer[128] = {0x0a, 0x06, 0x08, 0x01, 0x10, 0x2f, 0x18, 0x28};
  size_t message_length = 8;
  ServerUpdate message = ServerUpdate_init_zero;
  pb_istream_t stream = pb_istream_from_buffer(buffer, message_length);
  message.world_object.funcs.decode = decode_world_object;
  pb_decode(&stream, ServerUpdate_fields, &message);

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

    VBlankIntrWait();

    updatePlayerSpriteEntry(0, &p);
    updatePlayerSpriteEntry(1, &p2);
    oam_copy(oam_mem, sprite, 128);
  }
}
