#include <tonc.h>
#include <stdlib.h>
#include <string.h>

#include "world_objects.h"

struct world_object {
  u32 object_id;
  u32 x;
  u32 y;

  struct world_object* next;
};

struct world_object* head = NULL;

struct world_object* convert_world_object(WorldObject o) {
  struct world_object* new_object = (struct world_object*) malloc(sizeof(struct world_object));
  new_object->object_id = o.object_id;
  new_object->x = o.x;
  new_object->y = o.y;
  new_object->next = NULL;

  return new_object;
}

void update_world_object(WorldObject o) {
  if (head == NULL) {
    head = convert_world_object(o);
    return;
  }

  struct world_object* current = head;
  struct world_object* previous = NULL;
  while (current != NULL) {
    if (current->object_id == o.object_id) {
      current->x = o.x;
      current->y = o.y;
      return;
    } else if (current->object_id > o.object_id) {
      struct world_object* new_object = convert_world_object(o);
      if (previous == NULL) {
        head = new_object;
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

