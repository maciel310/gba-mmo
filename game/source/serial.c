#include "serial.h"

void (*handle_server_update)(ServerUpdate);
void (*handle_skill_stats)(SkillStats);
void (*handle_world_object)(WorldObject);
void (*handle_world_object_removed)(u32);
void (*handle_bank_update)(BankEntry);

void serial_init(
    void (*server_update_callback)(ServerUpdate),
    void (*skill_stats_callback)(SkillStats),
    void (*world_object_callback)(WorldObject),
    void (*world_object_removed_callback)(u32),
    void (*bank_update_callback)(BankEntry)) {
  REG_RCNT = 0;
  REG_SIODATA32 = 0;
  REG_SIOCNT = SION_CLK_EXT | SION_ENABLE | SIO_MODE_32BIT | SIO_IRQ;

  handle_server_update = server_update_callback;
  handle_skill_stats = skill_stats_callback;
  handle_world_object = world_object_callback;
  handle_world_object_removed = world_object_removed_callback;
  handle_bank_update = bank_update_callback;

  irq_add(II_SERIAL, handle_serial);
}

bool decode_world_object(pb_istream_t *stream, const pb_field_t *field, void **arg) {
  WorldObject message = WorldObject_init_zero;
  pb_decode(stream, WorldObject_fields, &message);
  handle_world_object(message);

  return true;
}

bool decode_skill_stats(pb_istream_t *stream, const pb_field_t *field, void **arg) {
  SkillStats message = SkillStats_init_zero;
  pb_decode(stream, SkillStats_fields, &message);
  handle_skill_stats(message);

  return true;
}

bool decode_bank_entry(pb_istream_t *stream, const pb_field_t *field, void **arg) {
  BankEntry entry = BankEntry_init_zero;
  pb_decode(stream, BankEntry_fields, &entry);
  handle_bank_update(entry);

  return true;
};

bool decode_removed_world_object_id(pb_istream_t *stream, const pb_field_t *field, void **arg) {
  uint32_t id;
  pb_decode_varint32(stream, &id);
  handle_world_object_removed(id);

  return true;
}

size_t outgoing_length = 0;
size_t outgoing_position = 0;
u8 outgoing_buffer[30];

u32 message_length = 0;
u32 expected_message_length = 0;
u8 buffer[512];
void handle_serial() {
  u32 data = REG_SIODATA32;

  if (outgoing_position < outgoing_length) {
    REG_SIODATA32 = (outgoing_buffer[outgoing_position++] << 24);
    REG_SIODATA32 |= (outgoing_buffer[outgoing_position++] << 16);
    REG_SIODATA32 |= (outgoing_buffer[outgoing_position++] << 8);
    REG_SIODATA32 |= (outgoing_buffer[outgoing_position++]);
  } else {
    REG_SIODATA32 = 0;
  }

  REG_SIOCNT |= SION_ENABLE;
  if (expected_message_length == 0 && (data & 0xBEEF0000) == 0xBEEF0000) {
    expected_message_length = data & 0x0000FFFF;
    message_length = 0;
    return;
  }

  buffer[message_length++] = ((data >> 24) & 0xff);
  buffer[message_length++] = ((data >> 16) & 0xff);
  buffer[message_length++] = ((data >> 8) & 0xff);
  buffer[message_length++] = (data & 0xff);

  if (message_length >= expected_message_length) {
    if (data != 0xDEADBEEF) {
      // Received incorrect message terminator, ignore message since it is likely corrupt.
      expected_message_length = 0;
      message_length = 0;
      return;
    }

    ServerUpdate message = ServerUpdate_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(buffer, expected_message_length - 4);
    message.world_object.funcs.decode = decode_world_object;
    message.skill_stats.funcs.decode = decode_skill_stats;
    message.bank.funcs.decode = decode_bank_entry;
    message.removed_world_object_ids.funcs.decode = decode_removed_world_object_id;
    pb_decode(&stream, ServerUpdate_fields, &message);

    handle_server_update(message);

    expected_message_length = 0;
    message_length = 0;
  } else if (data == 0xDEADBEEF) {
    // Received terminator early, set up to expect new message.
    expected_message_length = 0;
    message_length = 0;
    return;
  }
}

void send_player_status(PlayerStatus* p) {
  if (outgoing_position >= outgoing_length) {
    pb_ostream_t stream = pb_ostream_from_buffer(outgoing_buffer+4, sizeof(outgoing_buffer)-4);
    pb_encode(&stream, PlayerStatus_fields, p);
    outgoing_buffer[0] = 0;
    outgoing_buffer[1] = 0;
    outgoing_buffer[2] = 0;
    // Number of uint32_t that will be read.
    outgoing_buffer[3] = (stream.bytes_written + 3) / 4;

    outgoing_length = stream.bytes_written + 4;
    outgoing_position = 0;
  }
}