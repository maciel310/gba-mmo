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
    Direction_UNKNOWN = 0, 
    Direction_UP = 1, 
    Direction_DOWN = 2, 
    Direction_LEFT = 3, 
    Direction_RIGHT = 4 
} Direction;

/* Struct definitions */
typedef struct _ServerUpdate { 
    pb_callback_t world_object; 
} ServerUpdate;

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


/* Helper constants for enums */
#define _Direction_MIN Direction_UNKNOWN
#define _Direction_MAX Direction_RIGHT
#define _Direction_ARRAYSIZE ((Direction)(Direction_RIGHT+1))


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define ServerUpdate_init_default                {{{NULL}, NULL}}
#define PlayerStatus_init_default                {false, 0, false, 0, false, _Direction_MIN, false, 0}
#define ServerUpdate_init_zero                   {{{NULL}, NULL}}
#define PlayerStatus_init_zero                   {false, 0, false, 0, false, _Direction_MIN, false, 0}

/* Field tags (for use in manual encoding/decoding) */
#define ServerUpdate_world_object_tag            1
#define PlayerStatus_x_tag                       1
#define PlayerStatus_y_tag                       2
#define PlayerStatus_direction_tag               3
#define PlayerStatus_interaction_object_id_tag   4

/* Struct field encoding specification for nanopb */
#define ServerUpdate_FIELDLIST(X, a) \
X(a, CALLBACK, REPEATED, MESSAGE,  world_object,      1)
#define ServerUpdate_CALLBACK pb_default_field_callback
#define ServerUpdate_DEFAULT NULL
#define ServerUpdate_world_object_MSGTYPE WorldObject

#define PlayerStatus_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, INT32,    x,                 1) \
X(a, STATIC,   OPTIONAL, INT32,    y,                 2) \
X(a, STATIC,   OPTIONAL, UENUM,    direction,         3) \
X(a, STATIC,   OPTIONAL, INT32,    interaction_object_id,   4)
#define PlayerStatus_CALLBACK NULL
#define PlayerStatus_DEFAULT NULL

extern const pb_msgdesc_t ServerUpdate_msg;
extern const pb_msgdesc_t PlayerStatus_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define ServerUpdate_fields &ServerUpdate_msg
#define PlayerStatus_fields &PlayerStatus_msg

/* Maximum encoded size of messages (where known) */
/* ServerUpdate_size depends on runtime parameters */
#define PlayerStatus_size                        35

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
