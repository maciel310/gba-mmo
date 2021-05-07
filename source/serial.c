#include "serial.h"
#include "world_objects.h"

void serial_init() {
  irq_add(II_SERIAL, handle_serial);
}

bool decode_world_object(pb_istream_t *stream, const pb_field_t *field, void **arg) {
  WorldObject message = WorldObject_init_zero;
  pb_decode(stream, WorldObject_fields, &message);
  update_world_object(message);

  return true;
}

size_t message_length = 0;
size_t expected_message_length = 0;
u8 buffer[128];
void handle_serial() {
  u32 data = REG_SIODATA32;
  REG_SIOCNT |= SION_ENABLE;
  if (expected_message_length == 0) {
    expected_message_length = data;
    message_length = 0;
    return;
  }

  buffer[message_length++] = ((data >> 24) & 0xff);
  buffer[message_length++] = ((data >> 16) & 0xff);
  buffer[message_length++] = ((data >> 8) & 0xff);
  buffer[message_length++] = (data & 0xff);

  if (message_length >= expected_message_length) {
    ServerUpdate message = ServerUpdate_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(buffer, expected_message_length);
    message.world_object.funcs.decode = decode_world_object;
    pb_decode(&stream, ServerUpdate_fields, &message);

    expected_message_length = 0;
    message_length = 0;
  }
}
