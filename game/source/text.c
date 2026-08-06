#include "text.h"

#include "tonc.h"

void win_textbox(uint bgnr, int left, int top, int right, int bottom, uint bldy) {
  REG_WIN0H = left<<8 | right;
  REG_WIN0V = top<<8 | bottom;
  REG_WIN0CNT = WIN_ALL | WIN_BLD;
  REG_WINOUTCNT = WIN_ALL;

  REG_BLDCNT = (BLD_ALL&~BIT(bgnr)) | BLD_WHITE;
  REG_BLDY = bldy;

  REG_DISPCNT |= DCNT_WIN0;

  tte_set_margins(left, top, right, bottom);
}

void text_init() {
  REG_DISPCNT |= DCNT_BG1;

  tte_init_chr4c(
      1,
      BG_CBB(2)|BG_SBB(31),
      0xF000,
      bytes2word(13,15,0,0),
      CLR_RED,
      &verdana9_b4Font,
      (fnDrawg)chr4c_drawg_b4cts_fast);
}

void text_display(CSTR text) {
  win_textbox(1, 8, 160-32+4, 232, 160-4, 13);
  tte_write("#{P;es;ci:13}");
  tte_write(text);
}

void text_close() {
  REG_BLDCNT = 0;
  BIT_CLEAR(REG_DISPCNT, DCNT_WIN0);
  tte_erase_screen();
}