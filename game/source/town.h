#ifndef TOWN_H
#define TOWN_H

#include <tonc.h>

#define TOWN_WIDTH  (64)
#define TOWN_HEIGHT (64)
#define TOWN_LENGTH (4096)

extern const unsigned short town_Tile_Layer_1[4096];
extern const unsigned short town_Tile_Layer_2[4096];
extern const unsigned short town_Tile_Layer_3[4096];

extern const u64 TOWN_collisionData[64];

#endif
