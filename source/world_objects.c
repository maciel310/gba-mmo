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
      current->x = o.x;
      current->y = o.y;
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

