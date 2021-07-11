#include <tonc.h>
#include <stdlib.h>
#include <string.h>

#include "world_objects.h"

struct world_object* world_object_head = NULL;

bool sprite_collision_map[64][64];


const u32 sprite_type_lut[12] = {
  ATTR0_SQUARE, ATTR0_SQUARE, ATTR0_SQUARE, ATTR0_SQUARE,
  ATTR0_WIDE, ATTR0_WIDE, ATTR0_WIDE, ATTR0_WIDE,
  ATTR0_TALL, ATTR0_TALL, ATTR0_TALL, ATTR0_TALL
};

const u32 sprite_size_lut[12] = {
  ATTR1_SIZE_8x8,
  ATTR1_SIZE_16x16,
  ATTR1_SIZE_32x32,
  ATTR1_SIZE_64x64,

  ATTR1_SIZE_16x8,
  ATTR1_SIZE_32x8,
  ATTR1_SIZE_32x16,
  ATTR1_SIZE_64x32,

  ATTR1_SIZE_8x16,
  ATTR1_SIZE_8x32,
  ATTR1_SIZE_16x32,
  ATTR1_SIZE_32x64,
};

const u32 sprite_tile_width_lut[12] = {
  1, 2, 4, 8,
  2, 4, 4, 8,
  1, 1, 2, 4,
};

const u32 sprite_tile_height_lut[12] = {
  1, 2, 4, 8,
  1, 1, 2, 4,
  2, 4, 4, 8,
};

void convert_world_object(WorldObject proto_object, struct world_object* struct_object) {
  struct_object->object_id = proto_object.object_id;
  struct_object->sprite_id = proto_object.sprite_id;
  struct_object->sprite_size = proto_object.sprite_size;
  struct_object->is_solid = proto_object.is_solid;

  if (abs(struct_object->dest_y - proto_object.y) > 32 ||
      abs(struct_object->dest_x - proto_object.x) > 32) {
    struct_object->x = proto_object.x;
    struct_object->y = proto_object.y;
  }
  struct_object->dest_x = proto_object.x;
  struct_object->dest_y = proto_object.y;
}

void remove_world_object(u32 id) {
  struct world_object* current = world_object_head;
  struct world_object* previous = NULL;
  while (current != NULL) {
    if (current->object_id == id) {
      struct world_object* to_delete = current;

      if (previous == NULL) {
        world_object_head = current->next;
      } else {
        previous->next = current->next;
      }

      free(to_delete);
      return;
    }

    previous = current;
    current = current->next;
  }
}

void clear_all_world_objects() {
  struct world_object* current = world_object_head;
  while (current != NULL) {
    struct world_object* to_delete = current;
    current = current->next;

    free(to_delete);
  }
  world_object_head = NULL;
}

bool update_world_object(WorldObject o) {
  if (world_object_head == NULL) {
    world_object_head = (struct world_object*) malloc(sizeof(struct world_object));
    convert_world_object(o, world_object_head);
    world_object_head->next = NULL;
    return true;
  }

  struct world_object* current = world_object_head;
  struct world_object* previous = NULL;
  while (current != NULL) {
    if (current->object_id == o.object_id) {
      convert_world_object(o, current);
      current->dest_x = o.x;
      current->dest_y = o.y;
      current->sprite_id = o.sprite_id;
      return false;
    } else if (current->object_id > o.object_id) {
      struct world_object* new_object = (struct world_object*) malloc(sizeof(struct world_object));
      convert_world_object(o, new_object);
      if (previous == NULL) {
        world_object_head = new_object;
      } else {
        previous->next = new_object;
      }
      new_object->next = current;
      return true;
    }

    previous = current;
    current = current->next;
  }

  struct world_object* new_object = (struct world_object*) malloc(sizeof(struct world_object));
  convert_world_object(o, new_object);
  new_object->next = NULL;
  previous->next = new_object;
  return true;
}

// Returns the world_object next to the given position, or 0 if none found.
u32 find_next_to(s32 x, s32 y, Direction direction) {
  s32 x_tile = x / 8;
  s32 y_tile = y / 8;

  s32 object_x_tile_min;
  s32 object_x_tile_max;
  s32 object_y_tile_min;
  s32 object_y_tile_max;
  if (direction == Direction_UP) {
    object_x_tile_min = x_tile - 1;
    object_x_tile_max = x_tile + 1;
    object_y_tile_min = y_tile + 1;
    object_y_tile_max = y_tile + 1;
  } else if (direction == Direction_DOWN) {
    object_x_tile_min = x_tile - 1;
    object_x_tile_max = x_tile + 1;
    object_y_tile_min = y_tile + 3;
    object_y_tile_max = y_tile + 3;
  } else if (direction == Direction_LEFT) {
    object_x_tile_min = x_tile;
    object_x_tile_max = x_tile;
    object_y_tile_min = y_tile - 1;
    object_y_tile_max = y_tile + 2;
  } else {
    object_x_tile_min = x_tile + 2;
    object_x_tile_max = x_tile + 2;
    object_y_tile_min = y_tile - 1;
    object_y_tile_max = y_tile + 1;
  }

  struct world_object* current = world_object_head;
  while (current != NULL) {
    if (current->is_on_screen) {
      for (s32 x = 0; x < sprite_tile_width_lut[current->sprite_size]; x++) {
        for (s32 y = 0; y < sprite_tile_height_lut[current->sprite_size]; y++) {
          s32 object_tile_x = current->dest_x / 8 + x;
          s32 object_tile_y = current->dest_y / 8 + y;

          if (object_tile_x >= object_x_tile_min && object_tile_x <= object_x_tile_max
              && object_tile_y >= object_y_tile_min && object_tile_y <= object_y_tile_max) {
            return current->object_id;
          }
        }
      }
    }

    current = current->next;
  }
  return 0;
}

void regenerate_sprite_collision_map() {
  u32 x = 0;
  u32 y = 0;
  for (x = 0; x < 64; x++) {
    for (y = 0; y < 64; y++) {
      sprite_collision_map[x][y] = false;
    }
  }

  struct world_object* current = world_object_head;
  while (current != NULL) {
    if (current->is_solid) {
      s32 tileX = current->dest_x / 8;
      s32 tileY = current->dest_y / 8;
      for (s32 x = 0; x < sprite_tile_width_lut[current->sprite_size]; x++) {
        for (s32 y = 0; y < sprite_tile_height_lut[current->sprite_size]; y++) {
          sprite_collision_map[tileX + x][tileY + y] = true;
        }
      }
    }

    current = current->next;
  }
}