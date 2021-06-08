#ifndef SERIAL_H
#define SERIAL_H

#include <tonc.h>
#include "protos/network_messages.pb.h"
#include "protos/world_object.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

void serial_init(
    void (*handle_network_message)(CSTR),
    void (*skill_stats_callback)(SkillStats),
    void (*position_update_callback)(PlayerStatus));
bool decode_world_object(pb_istream_t *stream, const pb_field_t *field, void **arg);
void handle_serial();
void send_player_status(PlayerStatus* p);

#endif