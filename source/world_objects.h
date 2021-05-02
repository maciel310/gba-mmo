#ifndef WORLD_OBJECTS_H
#define WORLD_OBJECTS_H

#include <tonc.h>
#include <stdlib.h>

#include "protos/world_object.pb.h"

struct world_object {
  u32 object_id;
  u32 x;
  u32 y;

  struct world_object* next;
};

extern struct world_object* world_object_head;

void update_world_object(WorldObject o);

#endif
