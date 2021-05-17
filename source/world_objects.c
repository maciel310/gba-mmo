#include <tonc.h>
#include <stdlib.h>
#include <string.h>

#include "world_objects.h"

struct world_object* world_object_head = NULL;

struct world_object* convert_world_object(WorldObject o) {
  struct world_object* new_object = (struct world_object*) malloc(sizeof(struct world_object));
  new_object->object_id = o.object_id;
  new_object->x = o.x;
  new_object->y = o.y;
  new_object->dest_x = o.x;
  new_object->dest_y = o.y;
  new_object->sprite_id = o.sprite_id;
  new_object->next = NULL;

  return new_object;
}

void update_world_object(WorldObject o) {
  if (world_object_head == NULL) {
    world_object_head = convert_world_object(o);
    return;
  }

  struct world_object* current = world_object_head;
  struct world_object* previous = NULL;
  while (current != NULL) {
    if (current->object_id == o.object_id) {
      current->dest_x = o.x;
      current->dest_y = o.y;
      current->sprite_id = o.sprite_id;
      return;
    } else if (current->object_id > o.object_id) {
      struct world_object* new_object = convert_world_object(o);
      if (previous == NULL) {
        world_object_head = new_object;
      } else {
        previous->next = new_object;
      }
      new_object->next = current;
      return;
    }

    previous = current;
    current = current->next;
  }

  previous->next = convert_world_object(o);
}

// Returns the world_object next to the given position, or 0 if none found.
u32 find_next_to(s32 x, s32 y, Direction direction) {
  s32 object_x_min;
  s32 object_x_max;
  s32 object_y_min;
  s32 object_y_max;
  if (direction == Direction_UP) {
    object_x_min = x - 8;
    object_x_max = x + 16;
    object_y_min = y - 16;
    object_y_max = y + 8;
  } else if (direction == Direction_DOWN) {
    object_x_min = x - 8;
    object_x_max = x + 16;
    object_y_min = y + 8;
    object_y_max = y + 24;
  } else if (direction == Direction_LEFT) {
    object_x_min = x - 16;
    object_x_max = x - 8;
    object_y_min = y - 8;
    object_y_max = y + 8;
  } else {
    object_x_min = x + 8;
    object_x_max = x + 16;
    object_y_min = y - 8;
    object_y_max = y + 8;
  }

  struct world_object* current = world_object_head;
  while (current != NULL) {
    if (current->is_on_screen
        && current->x >= object_x_min && current->x <= object_x_max
        && current->y >= object_y_min && current->y <= object_y_max) {
      return current->object_id;
    }

    current = current->next;
  }
  return 0;
}
