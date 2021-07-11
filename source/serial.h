#ifndef SERIAL_H
#define SERIAL_H

#include <tonc.h>
#include "protos/network_messages.pb.h"
#include "protos/world_object.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

void serial_init(
    void (*server_update_callback)(ServerUpdate),
    void (*skill_stats_callback)(SkillStats),
    void (*world_object_callback)(WorldObject),
    void (*world_object_removed_callback)(u32),
    void (*bank_update_callback)(BankEntry));
bool decode_world_object(pb_istream_t *stream, const pb_field_t *field, void **arg);
void handle_serial();
void send_player_status(PlayerStatus* p);

#endif