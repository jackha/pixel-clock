/* word clock */


#define WORD_KWART 0
#define WORD_TIEN_ 8
#define WORD_VIJF_ 16
#define WORD_VOOR 24
#define WORD_OVER 32
#define WORD_HALF 40
#define WORD_TWEE 48
#define WORD_TWAALF 56
#define WORD_ELF 64
#define WORD_NEGEN 72
#define WORD_ZES 80
#define WORD_EEN 88
#define WORD_ZEVEN 96
#define WORD_TIEN 104
#define WORD_DRIE 112
#define WORD_VIER 120
#define WORD_VIJF 128
#define WORD_ACHT 136
#define WORD_UUR 144


static const unsigned char wordmap[] PROGMEM = {
  0b11111000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,

  0b00001111,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,

  0b00000000,
  0b11110000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,

  0b00000000,
  0b00001111,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,

  0b00000000,
  0b00000000,
  0b11110000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,

  0b00000000,
  0b00000000,
  0b00001111,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,

  0b00000000,
  0b00000000,
  0b00000000,
  0b11110000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,

  0b00000000,
  0b00000000,
  0b00000000,
  0b11001111,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,

  0b00000000,
  0b00000000,
  0b00000000,
  0b00010011,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,

  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b11111000,
  0b00000000,
  0b00000000,
  0b00000000,

  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000111,
  0b00000000,
  0b00000000,
  0b00000000,

  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b01011000,
  0b00000000,
  0b00000000,
  0b00000000,

  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b01101011,
  0b00000000,
  0b00000000,

  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00010111,
  0b00000000,
  0b00000000,

  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b11011000,
  0b00000000,

  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00111100,
  0b00000000,

  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00110011,
  0b00000000,

  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b11110000,

  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000111
};


/***************************************************************************/
void draw_word(int start, byte r, byte g, byte b) {
  byte digit_row;
  for (int y=0; y<8; y++) {
    digit_row = pgm_read_byte(wordmap+start+y);
    for (int x=0; x<8; x++) {
      if ((digit_row & 0b1) > 0) {
        set_pixel((7 - x) + y * 8, r, g, b, brightness);
      }
      digit_row = digit_row >> 1;
    }
  }
  
}

void draw_clock_word(DateTime dt, RGB col_a, RGB col_b) {
  int hour;
  int minute = -1;  // vijf (0), tien (1) or kwart (2)
  int over_voor = -1;  // voor (0), over (1)
  int half = -1;  // half (0)
  int uur = -1;  // uur (0)
  byte r, g, b;
  long unsigned int col;
  float h;

  if (dt.minute() < 5) { 
    hour = dt.hour() % 12;
    minute = dt.minute() / 5 - 1;
    uur = 0;
  } else if (dt.minute() < 20) { 
    hour = dt.hour() % 12;
    minute = dt.minute() / 5 - 1;
    over_voor = 1;
  } else {
    hour = (dt.hour() + 1) % 12;
    if ((dt.minute() >= 20) && (dt.minute() < 25)) {
      minute = 1;
      over_voor = 0;
      half = 0;
    } else if ((dt.minute() >= 25) && (dt.minute() < 30)) {
      minute = 0;
      over_voor = 0;
      half = 0;
    } else if ((dt.minute() >= 30) && (dt.minute() < 35)) {
      half = 0;
    } else if ((dt.minute() >= 35) && (dt.minute() < 40)) {
      minute = 0;
      over_voor = 1;
      half = 0;
    } else if ((dt.minute() >= 40) && (dt.minute() < 45)) {
      minute = 1;
      over_voor = 1;
      half = 0;
    } else if ((dt.minute() >= 45) && (dt.minute() < 50)) {
      minute = 2;
      over_voor = 0;
    } else if ((dt.minute() >= 50) && (dt.minute() < 55)) {
      minute = 1;
      over_voor = 0;
    } else if ((dt.minute() >= 55)) {
      minute = 0;
      over_voor = 0;
    }
  }

  h = float(dt.second() + 30) / 60;
  if (h > 1) {h -= 1; }
  col = hsl_to_rgb(h, 1, 0.2);
  r = col >> 16;
  g = (col >> 8) & 0xff;
  b = col & 0xff;

  if (half == 0) {
      draw_word(WORD_HALF, r, g, b);
  }

  col = hsl_to_rgb(float(dt.second()) / 60, 1, 0.2);
  r = col >> 16;
  g = (col >> 8) & 0xff;
  b = col & 0xff;

  switch(minute) { 
    case 0:
      draw_word(WORD_VIJF_, r, g, b);
      break;
    case 1:
      draw_word(WORD_TIEN_, r, g, b);
      break;
    case 2:
      draw_word(WORD_KWART, r, g, b);
      break;
  }

  h = float(dt.hour() + 3) / 23;
  if (h > 1) {h -= 1; }
  col = hsl_to_rgb(h, 1, 0.2);
  r = col >> 16;
  g = (col >> 8) & 0xff;
  b = col & 0xff;

  switch(over_voor) { 
    case 0:
      draw_word(WORD_VOOR, r, g, b);
      break;
    case 1:
      draw_word(WORD_OVER, r, g, b);
      break;
  }

  col = hsl_to_rgb(float(dt.hour()) / 23.0, 1, 0.28);
  r = col >> 16;
  g = (col >> 8) & 0xff;
  b = col & 0xff;
  
  switch(hour) { 
    case 0:
      draw_word(WORD_TWAALF, r, g, b);
      break;
    case 1:
      draw_word(WORD_EEN, r, g, b);
      break;
    case 2:
      draw_word(WORD_TWEE, r, g, b);
      break;
    case 3:
      draw_word(WORD_DRIE, r, g, b);
      break;
    case 4:
      draw_word(WORD_VIER, r, g, b);
      break;
    case 5:
      draw_word(WORD_VIJF, r, g, b);
      break;
    case 6:
      draw_word(WORD_ZES, r, g, b);
      break;
    case 7:
      draw_word(WORD_ZEVEN, r, g, b);
      break;
    case 8:
      draw_word(WORD_ACHT, r, g, b);
      break;
    case 9:
      draw_word(WORD_NEGEN, r, g, b);
      break;
    case 10:
      draw_word(WORD_TIEN, r, g, b);
      break;
    case 11:
      draw_word(WORD_ELF, r, g, b);
      break;
  }

  col = hsl_to_rgb(float(dt.dayOfWeek()) / 6.0, 1, 0.28);
  r = col >> 16;
  g = (col >> 8) & 0xff;
  b = col & 0xff;

  if (uur == 0) {
      draw_word(WORD_UUR, r, g, b);
  }

}
