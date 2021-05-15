#ifndef WORLD_OBJECTS_H
#define WORLD_OBJECTS_H

#include <tonc.h>
#include <stdlib.h>

#include "protos/network_messages.pb.h"
#include "protos/world_object.pb.h"

struct world_object {
  u32 object_id;
  s32 x;
  s32 y;
  s32 dest_x;
  s32 dest_y;
  u32 sprite_id;
  bool is_on_screen;

  struct world_object* next;
};

extern struct world_object* world_object_head;

void update_world_object(WorldObject o);

u32 find_next_to(s32 x, s32 y, Direction direction);

#endif
