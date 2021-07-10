/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.5 */

#ifndef PB_NETWORK_MESSAGES_PB_H_INCLUDED
#define PB_NETWORK_MESSAGES_PB_H_INCLUDED
#include <pb.h>
#include "world_object.pb.h"

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Enum definitions */
typedef enum _Direction { 
    Direction_UNKNOWN_DIRECTION = 0, 
    Direction_UP = 1, 
    Direction_DOWN = 2, 
    Direction_LEFT = 3, 
    Direction_RIGHT = 4 
} Direction;

typedef enum _MapLocation { 
    MapLocation_UNKNOWN_MAP = 0, 
    MapLocation_LUMBER_RIDGE = 1, 
    MapLocation_VAR_ROCK = 2, 
    MapLocation_TOWN = 3 
} MapLocation;

typedef enum _Skill { 
    Skill_UNKNOWN_SKILL = 0, 
    Skill_WOODCUTTING = 1 
} Skill;

typedef enum _Item { 
    Item_UNKNOWN_ITEM = 0, 
    Item_WOOD = 1, 
    Item_ROCK = 2, 
    Item_HATCHET = 3, 
    Item_PICKAXE = 4 
} Item;

typedef enum _Interface { 
    Interface_UNKNOWN_INTERFACE = 0, 
    Interface_BANK = 1 
} Interface;

/* Struct definitions */
typedef struct _PlayerStatus { 
    bool has_x;
    int32_t x; 
    bool has_y;
    int32_t y; 
    bool has_direction;
    Direction direction; 
    bool has_interaction_object_id;
    int32_t interaction_object_id; 
} PlayerStatus;

typedef struct _SkillStats { 
    bool has_skill;
    Skill skill; 
    bool has_level;
    int32_t level; 
    bool has_exp;
    int32_t exp; 
} SkillStats;

typedef struct _ServerUpdate { 
    pb_callback_t world_object; 
    bool has_network_message;
    char network_message[90]; 
    pb_callback_t skill_stats; 
    /* Only filled out on initial connection to server, or if server-side change of player location. */
    bool has_player_status;
    PlayerStatus player_status; 
    bool has_interacting_skill;
    Skill interacting_skill; 
    bool has_current_map;
    MapLocation current_map; 
    /* NOTE: If max_count is updated change in server and ROM. */
    pb_size_t inventory_count;
    Item inventory[18]; 
    bool has_launch_interface;
    Interface launch_interface; 
} ServerUpdate;


/* Helper constants for enums */
#define _Direction_MIN Direction_UNKNOWN_DIRECTION
#define _Direction_MAX Direction_RIGHT
#define _Direction_ARRAYSIZE ((Direction)(Direction_RIGHT+1))

#define _MapLocation_MIN MapLocation_UNKNOWN_MAP
#define _MapLocation_MAX MapLocation_TOWN
#define _MapLocation_ARRAYSIZE ((MapLocation)(MapLocation_TOWN+1))

#define _Skill_MIN Skill_UNKNOWN_SKILL
#define _Skill_MAX Skill_WOODCUTTING
#define _Skill_ARRAYSIZE ((Skill)(Skill_WOODCUTTING+1))

#define _Item_MIN Item_UNKNOWN_ITEM
#define _Item_MAX Item_PICKAXE
#define _Item_ARRAYSIZE ((Item)(Item_PICKAXE+1))

#define _Interface_MIN Interface_UNKNOWN_INTERFACE
#define _Interface_MAX Interface_BANK
#define _Interface_ARRAYSIZE ((Interface)(Interface_BANK+1))


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define ServerUpdate_init_default                {{{NULL}, NULL}, false, "", {{NULL}, NULL}, false, PlayerStatus_init_default, false, _Skill_MIN, false, _MapLocation_MIN, 0, {_Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN}, false, _Interface_MIN}
#define PlayerStatus_init_default                {false, 0, false, 0, false, _Direction_MIN, false, 0}
#define SkillStats_init_default                  {false, _Skill_MIN, false, 0, false, 0}
#define ServerUpdate_init_zero                   {{{NULL}, NULL}, false, "", {{NULL}, NULL}, false, PlayerStatus_init_zero, false, _Skill_MIN, false, _MapLocation_MIN, 0, {_Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN, _Item_MIN}, false, _Interface_MIN}
#define PlayerStatus_init_zero                   {false, 0, false, 0, false, _Direction_MIN, false, 0}
#define SkillStats_init_zero                     {false, _Skill_MIN, false, 0, false, 0}

/* Field tags (for use in manual encoding/decoding) */
#define PlayerStatus_x_tag                       1
#define PlayerStatus_y_tag                       2
#define PlayerStatus_direction_tag               3
#define PlayerStatus_interaction_object_id_tag   4
#define SkillStats_skill_tag                     1
#define SkillStats_level_tag                     2
#define SkillStats_exp_tag                       3
#define ServerUpdate_world_object_tag            1
#define ServerUpdate_network_message_tag         2
#define ServerUpdate_skill_stats_tag             3
#define ServerUpdate_player_status_tag           4
#define ServerUpdate_interacting_skill_tag       5
#define ServerUpdate_current_map_tag             6
#define ServerUpdate_inventory_tag               7
#define ServerUpdate_launch_interface_tag        8

/* Struct field encoding specification for nanopb */
#define ServerUpdate_FIELDLIST(X, a) \
X(a, CALLBACK, REPEATED, MESSAGE,  world_object,      1) \
X(a, STATIC,   OPTIONAL, STRING,   network_message,   2) \
X(a, CALLBACK, REPEATED, MESSAGE,  skill_stats,       3) \
X(a, STATIC,   OPTIONAL, MESSAGE,  player_status,     4) \
X(a, STATIC,   OPTIONAL, UENUM,    interacting_skill,   5) \
X(a, STATIC,   OPTIONAL, UENUM,    current_map,       6) \
X(a, STATIC,   REPEATED, UENUM,    inventory,         7) \
X(a, STATIC,   OPTIONAL, UENUM,    launch_interface,   8)
#define ServerUpdate_CALLBACK pb_default_field_callback
#define ServerUpdate_DEFAULT NULL
#define ServerUpdate_world_object_MSGTYPE WorldObject
#define ServerUpdate_skill_stats_MSGTYPE SkillStats
#define ServerUpdate_player_status_MSGTYPE PlayerStatus

#define PlayerStatus_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, INT32,    x,                 1) \
X(a, STATIC,   OPTIONAL, INT32,    y,                 2) \
X(a, STATIC,   OPTIONAL, UENUM,    direction,         3) \
X(a, STATIC,   OPTIONAL, INT32,    interaction_object_id,   4)
#define PlayerStatus_CALLBACK NULL
#define PlayerStatus_DEFAULT NULL

#define SkillStats_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, UENUM,    skill,             1) \
X(a, STATIC,   OPTIONAL, INT32,    level,             2) \
X(a, STATIC,   OPTIONAL, INT32,    exp,               3)
#define SkillStats_CALLBACK NULL
#define SkillStats_DEFAULT NULL

extern const pb_msgdesc_t ServerUpdate_msg;
extern const pb_msgdesc_t PlayerStatus_msg;
extern const pb_msgdesc_t SkillStats_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define ServerUpdate_fields &ServerUpdate_msg
#define PlayerStatus_fields &PlayerStatus_msg
#define SkillStats_fields &SkillStats_msg

/* Maximum encoded size of messages (where known) */
/* ServerUpdate_size depends on runtime parameters */
#define PlayerStatus_size                        35
#define SkillStats_size                          24

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
