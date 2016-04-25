/*

ds3231 realtime clock + 8x8 ws2812 "display" (aka neopixels) = awesome clock!


*/

#include "font.h"
#include "bitmap.h"
#include "table.h"

#include <EEPROM.h>

// Date and time functions using RX8025 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "Sodaq_DS3231.h"
//#include "pitches.h"  // from: toneMelody
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <Encoder.h>  // rotary encoder

#define LDR_PIN A0

Encoder rot_enc(A2, A1);  // rotary encoder: D5, D6
#define BUT_A_PIN A3  // button next to rotary encoder

#define NEOPIXEL_PIN 9  // we plug an 8x8 onto it
#define NEOPIXEL_NUM 64

#define MODE_SET_CLOCK 99 // go to this mode temporary and then go back to the old mode 

#define MODE_NORMAL 0  // use the 2x5 font
#define MODE_NORMAL_REV 1  // use the 2x5 font
#define MODE_NORMAL_ALT 2  // use the 2x5 font
#define MODE_NORMAL_ALT_REV 3  // use the 2x5 font
#define MODE_NORMAL_ALT2 4  // use the 2x5 font
#define MODE_NORMAL_ALT2_REV 5  // use the 2x5 font
#define MODE_NORMAL_ALT3 6  // use the 2x5 font
#define MODE_NORMAL_ALT3_REV 7  // use the 2x5 font
#define MODE_BCD 8 // binary coded digital 
#define MODE_ANALOG 9  // use pixel arms
#define MODE_PARTY 10  // no clock - just colors!
#define MODE_HEX 11  // jack's hex clock: a day is 0x0000..0xFFFF, where the highest byte is displayed as hex and lowest as bits
#define MODE_GAME_OF_LIFE 12  // conways game of life
#define MODE_4X4 13  // conways game of life
#define MODE_WORD 14  // word clock
#define MODE_ROBOT 15

#define NUM_CLOCK_MODES 16  //

// every time you press button A, the program mode advances
#define PGM_MODE_TM 0  // time
#define PGM_MODE_LO 1  // layout
#define PGM_MODE_GM 2  // game
#define PGM_MODE_CO 3  // color
#define PGM_MODE_BR 4  // brightness

#define PGM_MODE_TE 5  // tetris

#define PGM_MODE_SWITCH_DELAY 1000

// in tm mode, every b press advances the time edit mode
#define PGM_TM_NONE 0
#define PGM_TM_HOUR 1
#define PGM_TM_MINUTE 2

#ifdef __AVR__
 #include <avr/io.h>
 #include <avr/pgmspace.h>
#elif defined(ESP8266)
 #include <pgmspace.h>
#else
 #define PROGMEM
#endif

struct RGB {
   byte   r;
   byte   g;
   byte   b;

   byte h;  // hue, for memory purposes only
   byte s;  // saturation
   byte l;  // luminosity
};

struct RGB col_a; 
struct RGB col_b; 

long enc_pos  = -999;  // rotary encoder position
// for colors
long neg_enc_pos  = -999;  // rotary encoder position
long pos_enc_pos  = -999;  // rotary encoder position
volatile long new_enc_pos = -999;
volatile long old_enc_pos = -999;  // detection of movement
uint16_t fast_counter = 0;  // just cycles, for use in timing / moving stuff other than seconds

unsigned long start_approx_millis_time = 0;
unsigned long approx_millis = 0;  // approximate millis

// game of life in RGB! 0 is current buffer, 1 is future
byte game_of_life[2][3*64];

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEOPIXEL_NUM, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);


char weekDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
//year, month, date, hour, min, sec and week-day(starts from 0 and goes to 6)
//writing any non-existent time-data may interfere with normal operation of the RTC.
//Take care of week-day also.
//DateTime dt(2016, 2, 10, 15, 18, 0, 5);


// notes in the melody:
int melody[] = {
//  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
   35, 100, 200, 60, 32, 76, 250, 0
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

int clock_mode = MODE_NORMAL;
int last_clock_mode = MODE_NORMAL;
int pgm_mode = PGM_MODE_LO;

int pgm_mode_tm_state = PGM_TM_NONE;
int hour_inc = 0;  // increase/decrease hour because we want to change it - is auto modded (%)
int minute_inc = 0;

int last_but_a_val = HIGH;
int last_but_b_val = HIGH;
int but_a_val = HIGH;
int but_b_val = HIGH;
int but_counter = 0;

int color_idx = 0;
int tmp_int;

uint16_t ldr_value;
uint16_t brightness;
uint16_t brightness_offset;
int brightness_target;

uint32_t last_action_ts;  // when was the last action? too long -> go back to layout mode

#define TETRIS_SIZE_X 8
#define TETRIS_SIZE_Y 10
byte tetris_playfield[TETRIS_SIZE_Y*TETRIS_SIZE_X]; // we use the 'tetris index' below for its contents, 255 is empty.


#define TETRIS_I 0
#define TETRIS_J 1
#define TETRIS_L 2
#define TETRIS_O 3
#define TETRIS_S 4
#define TETRIS_T 5
#define TETRIS_Z 6
#define TETRIS_EMPTY 255

// RGB colors in order of appearance
static const unsigned char tetris_colors[] PROGMEM = {
  0x00, 0xff, 0xff,  // I, cyan
  0xff, 0x00, 0x00,  // J, blue
  0xff, 0x80, 0x00,  // L, orange
  0xff, 0xff, 0x00,  // O, yellow
  0x00, 0xff, 0x00,  // S, green
  0xa0, 0x20, 0xf0,  // T, purple
  0xff, 0x00, 0x00,  // Z, red
};

static const unsigned char tetris_blocks[] PROGMEM = {
// I
  0b0000,
  0b1111,
  0b0000,
  0b0000,

  0b0010,
  0b0010,
  0b0010,
  0b0010,

  0b0000,
  0b0000,
  0b1111,
  0b0000,

  0b0100,
  0b0100,
  0b0100,
  0b0100,

// J
  0b1000,
  0b1110,
  0b0000,
  0b0000,

  0b0110,
  0b0100,
  0b0100,
  0b0000,

  0b0000,
  0b1110,
  0b0010,
  0b0000,

  0b0100,
  0b0100,
  0b1100,
  0b0000,

// L
  0b0100,
  0b0100,
  0b0110,
  0b0000,

  0b0000,
  0b1110,
  0b1000,
  0b0000,

  0b1100,
  0b0100,
  0b0100,
  0b0000,

  0b0010,
  0b1110,
  0b0000,
  0b0000,
// O
  0b0110,
  0b0110,
  0b0000,
  0b0000,

  0b0110,
  0b0110,
  0b0000,
  0b0000,

  0b0110,
  0b0110,
  0b0000,
  0b0000,

  0b0110,
  0b0110,
  0b0000,
  0b0000,

// S
  0b0110,
  0b1100,
  0b0000,
  0b0000,

  0b0100,
  0b0110,
  0b0010,
  0b0000,

  0b0000,
  0b0110,
  0b1100,
  0b0000,

  0b1000,
  0b1100,
  0b0100,
  0b0000,

// T
  0b0100,
  0b1110,
  0b0000,
  0b0000,

  0b0100,
  0b0110,
  0b0100,
  0b0000,

  0b0000,
  0b1110,
  0b0100,
  0b0000,

  0b0100,
  0b1100,
  0b0100,
  0b0000,

// Z
  0b1100,
  0b0110,
  0b0000,
  0b0000,

  0b0010,
  0b0110,
  0b0100,
  0b0000,

  0b0000,
  0b1100,
  0b0110,
  0b0000,

  0b0100,
  0b1100,
  0b1000,
  0b0000,
};

void reset_tetris() {
  for (int y=0; y<TETRIS_SIZE_Y; y++) {
    for (int x=0; x<TETRIS_SIZE_X; x++) {
      tetris_playfield[y*TETRIS_SIZE_X+x] = TETRIS_EMPTY;
    }
  }
}

// block 0..6, rotation 0..3
// return true if collision or out of bounds, false if ok
boolean tetris_check_collision(int x_, int y_, int block, int rotation) {
  byte tetromino_row;
  int tetromino_offset = block*16 + (rotation % 4)*4;
  byte xx, yy;
  for (int y=0; y<4; y++) {
    tetromino_row = pgm_read_byte(tetris_blocks+tetromino_offset+y);
    for (int x=0; x<4; x++) {
      if ((tetromino_row & 0b1) > 0) {
        xx = 3 - x + x_;
        yy = y + y_;
        if ((xx >= 0) && (xx < TETRIS_SIZE_X) && (yy >= 0) && (yy < TETRIS_SIZE_Y)) {
          //set_pixel(xx + yy * 8, r, g, b, brightness);
          if (tetris_playfield[yy*TETRIS_SIZE_X+xx] != TETRIS_EMPTY) {
            return true;
          }
        } else {
          return true;  // one of the blocks is out of bounds
        }
      }
      tetromino_row = tetromino_row >> 1;
    }
  }
  return false;
}

// display playfield, special function for 8x8 display
// hope this is fast
void tetris_display(int x_offset, int y_offset, int brightness) {
  byte r, g, b;
  byte tetromino;
  for (int y=0; y<8; y++) {
    for (int x=0; x<8; x++) {
      tetromino = tetris_playfield[(y+y_offset)*8+x+x_offset];
      if (tetromino != TETRIS_EMPTY) {
        r = pgm_read_byte(tetris_colors + 3 * tetromino + 0);
        g = pgm_read_byte(tetris_colors + 3 * tetromino + 1);
        b = pgm_read_byte(tetris_colors + 3 * tetromino + 2);

        set_pixel(y*8+x, r, g, b, brightness);  
      }
    }
  }
}

// just 'plot' the piece in place, no collision check
void tetris_place_tetromino(int x_, int y_, int block, int rotation) {
  byte tetromino_row;
  int tetromino_offset = block*16 + (rotation % 4)*4;
  byte xx, yy;
  for (int y=0; y<4; y++) {
    tetromino_row = pgm_read_byte(tetris_blocks+tetromino_offset+y);
    for (int x=0; x<4; x++) {
      if ((tetromino_row & 0b1) > 0) {
        xx = 3 - x + x_;
        yy = y + y_;
        if ((xx >= 0) && (xx < TETRIS_SIZE_X) && (yy >= 0) && (yy < TETRIS_SIZE_Y)) {
          tetris_playfield[yy*TETRIS_SIZE_X+xx] = block;
        }
      }
      tetromino_row = tetromino_row >> 1;
    }
  }  
}

void tetris() {
  reset_tetris();
  int block = TETRIS_S;
  int rotation = 1;
  int x = 3;
  int y = 0;
  boolean game_over = false;
  long int current_millis = millis();
  long int next_move_millis;
  int block_duration = 500;

  int but_a_val, last_but_a_val;
  long int enc_pos;
  rot_enc.write(0);

  while (!game_over) {
    block = (block + 1) % 7;  // TODO: implement "Random Generator"
    rotation = (rotation + 1) % 4;
    x = 3-rotation;
    y = 0;
    game_over = tetris_check_collision(x, y+1, block, rotation) && (y == 0);
    next_move_millis = current_millis + block_duration;
    while (!tetris_check_collision(x, y+1, block, rotation)) {
      pixels.clear();
      // provide offset of playfield and brightness
      tetris_display(0, 2, 100);
      // convert playfield coordinates to screen coordinates
      display_tetromino(x, y-2, block, rotation, 100);  
      pixels.show();
      current_millis = millis();
      if (current_millis > next_move_millis) {
        y += 1;
        next_move_millis = current_millis + block_duration;
      }
      // game controls
      last_but_a_val = but_a_val;
      but_a_val = digitalRead(BUT_A_PIN);
      enc_pos = new_enc_pos / 4;
      if ((enc_pos < 0) && (!tetris_check_collision(x-1, y, block, rotation))) {
        x -= 1;
        rot_enc.write(0);
      }
      if ((enc_pos > 0) && (!tetris_check_collision(x+1, y, block, rotation))) {
        x += 1;
        rot_enc.write(0);
      }
      if (
        (but_a_val == LOW) && 
        (last_but_a_val == HIGH) && 
        (!tetris_check_collision(x+1, y, block, (rotation + 1) % 4))) {
          
        rotation = (rotation + 1) % 4;
      }
    }
    tetris_place_tetromino(x, y, block, rotation);
    // TODO: check for full lines and shift all
  }
  delay(1000);
}

ISR(TIMER1_COMPA_vect){  //change the 0 to 1 for timer1 and 2 for timer2
    new_enc_pos = rot_enc.read();
}

void setup () 
  {

  cli();//stop interrupts


//set timer1 interrupt at 16*10^6/1024/11
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 8MHz increments
  OCR1A = 10;// (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set prescaler CS10, CS11 CS12
  //TCCR1B |= (1 << CS10);  // no prescaling  
  //TCCR1B |= (1 << CS11);  // /8  
  //TCCR1B |= (1 << CS10) + (1 << CS11);  // /64  
  //TCCR1B |= (1 << CS12);  // 256  
  TCCR1B |= (1 << CS10) | (1 << CS12);  // 1024  
  //TCCR1B |= (1 << CS12) | (1 << CS11);  // external clock on t1 pin, falling edge
  //TCCR1B |= (1 << CS12) | (1 << CS11) |  (1 << CS10);  // external clock on t1 pin, rising edge
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();//allow interrupts

    pinMode(BUT_A_PIN, INPUT_PULLUP);
    //pinMode(BUT_B_PIN, INPUT_PULLUP);

    pinMode(LDR_PIN, INPUT_PULLUP);

    Serial.begin(57600);
    
    Wire.begin();
    rtc.begin();
    pixels.begin();

    but_a_val = digitalRead(BUT_A_PIN);
    last_but_a_val = but_a_val;

    read_eeprom();
      rot_enc.write(4*clock_mode);
  // prevent readouts on start
  new_enc_pos = rot_enc.read();
  enc_pos = new_enc_pos;

  init_game_of_life();  

  last_action_ts = rtc.now().getEpoch(); 

// testing tetris stuff
//
//  for (int b=0; b<7; b++) {
//  for (int y=-4; y<9; y++) {
//    pixels.clear();
//   display_tetromino(3, y, b*16 + (y/4)*4, 100);
////   display_tetromino(4, 0, TETRIS_Z + 1*4, 100);
////   display_tetromino(0, 4, TETRIS_I + 0*4, 100);
////   display_tetromino(4, 4, TETRIS_O + 2*4, 100);
//   delay(100);
//   pixels.show();
//  }
//  }
//  delay(1000);
  tetris();
}

uint32_t old_ts;

/***************************************************************************/


void display_big_bitmap(int location_offset_x, int location_offset_y, int bitmap_offset, 
  byte r1, byte g1, byte b1,
  byte r2, byte g2, byte b2,
  byte r3, byte g3, byte b3) 
  {
  byte pixels_value, pix_value;
  byte xx, yy;
  byte r, g, b;
  
  // screen coords
  for (int y=0; y<8; y++) {
    for (int x=0; x<8; x++) {
        // image source location
        xx = x + location_offset_x;
        yy = y + location_offset_y;

        if ((xx < 0) || (xx > 31) || (yy < 0) || (yy > 7)) {
          continue;
        }
        // is slightly inefficient, because most bytes are fetched 4 times, oh well
        pixels_value = pgm_read_byte(big_bitmaps+bitmap_offset+yy*8+xx/4);  
        pix_value = (pixels_value >> (2* (3 - xx % 4))) & 0b11;
        switch(pix_value) {
          case 0:
            //r = r0; g=g0; b=b0; break;
            continue; break;
          case 1:
            r = r1; g=g1; b=b1; break;
          case 2:
            r = r2; g=g2; b=b2; break;
          case 3:
            r = r3; g=g3; b=b3; break;          
        }
        set_pixel(y * 8 + x, r, g, b, brightness);
    }
  }
}


/***************************************************************************/
// normal clock with digits from 'font'
void draw_normal_clock(DateTime dt, RGB col_a, RGB col_b, int y0, int y1, int y2, int y3, int dot_location) {
  draw_single_digit(0, y0, dt.hour() / 10, col_a.r, col_a.g, col_a.b);
  draw_single_digit(2, y1, dt.hour() % 10, col_b.r, col_b.g, col_b.b);
  draw_single_digit(4, y2, dt.minute() / 10, col_a.r, col_a.g, col_a.b);
  draw_single_digit(6, y3, dt.minute() % 10, col_b.r, col_b.g, col_b.b);
  if (dt.second() % 2 == 0) {
    set_pixel(dot_location, col_b.r,col_b.g,col_b.b, brightness);
  }
}

/***************************************************************************/
void draw_edit_clock(DateTime dt, int hour_inc, int minute_inc) {
  // make it blink
  if (pgm_mode_tm_state == PGM_TM_MINUTE) {
    draw_single_digit(0, 1, dt.hour() / 10, col_a.r, col_a.g, col_a.b);
    draw_single_digit(2, 1, dt.hour() % 10, col_b.r, col_b.g, col_b.b);
    if ((fast_counter >> 6) % 2 == 0) {
      draw_single_digit(4, 1, minute_inc / 10, col_a.r, col_a.g, col_a.b);
      draw_single_digit(6, 1, minute_inc % 10, col_b.r, col_b.g, col_b.b);
    }
  } else if (pgm_mode_tm_state == PGM_TM_HOUR) {
    if ((fast_counter >> 6) % 2 == 0) {
      draw_single_digit(0, 1, hour_inc / 10, col_a.r, col_a.g, col_a.b);
      draw_single_digit(2, 1, hour_inc % 10, col_b.r, col_b.g, col_b.b);
    }
    draw_single_digit(4, 1, dt.minute() / 10, col_a.r, col_a.g, col_a.b);
    draw_single_digit(6, 1, dt.minute() % 10, col_b.r, col_b.g, col_b.b);
  }
}

/***************************************************************************/
void draw_bcd_column(uint16_t col_nr, byte value, byte r, byte g, byte b) {
  for (byte by=0; by<8; by++) {
    if ((value & 0b1) > 0) {
      //pixels.setPixelColor(col_nr+(7-by)*8, pixels.Color(r, g, b));
      set_pixel(col_nr+(7-by)*8, r, g, b, brightness);
    }
    value = value >> 1;
  }
}

void draw_bcd_clock(DateTime dt) {
  // we draw it column for column
  draw_bcd_column(0, dt.hour() / 10, col_a.r, col_a.g, col_a.b);
  draw_bcd_column(1, dt.hour() % 10, col_a.r, col_a.g, col_a.b);

  draw_bcd_column(3, dt.minute() / 10, col_b.r, col_b.g, col_b.b);
  draw_bcd_column(4, dt.minute() % 10, col_b.r, col_b.g, col_b.b);

  draw_bcd_column(6, dt.second() / 10, col_a.r, col_a.g, col_a.b);
  draw_bcd_column(7, dt.second() % 10, col_a.r, col_a.g, col_a.b); 
}

/***************************************************************************/
void draw_analog_clock(DateTime dt) {
  byte pix_idx, minut;
  draw_single_digit_3x6(1, 1, dt.hour() / 10, col_a.r, col_a.g, col_a.b);
  draw_single_digit_3x6(4, 1, dt.hour() % 10, col_b.r, col_b.g, col_b.b);

  minut = 0;
  for (int i=0; minut < dt.minute() + dt.second() % 2; i++) {
    minut = pgm_read_byte(minute_idx+i);
    pix_idx = pgm_read_byte(minute_pix_idx+i);
    if (pix_idx % 8 >= 4) {
      set_pixel(pix_idx, col_a.r, col_a.g, col_a.b, brightness);
    } else {
      set_pixel(pix_idx, col_b.r, col_b.g, col_b.b, brightness);      
    }
  }
}

/***************************************************************************/
void draw_party(DateTime dt) {
  byte r;
  byte g;
  byte b;
  int offset1;
  int offset2;
  int offset3;
  for(int i=0; i<8; i++) {
    for(int j=0; j<8; j++) {
      offset1 = ((fast_counter*5)>>6) % 60;  //dt.second();
      offset2 = ((fast_counter*3)>>6) % 60;  //dt.second();
      offset3 = ((fast_counter*7)>>6) % 60;  //dt.second();
      r = abs(((offset1 + i*3 + j*1) % 60) - 30);
      g = abs(((offset2 + i*2 + j*2 + 30) % 60) - 30);
      b = abs(((offset3 + i*1 + j*3 + 50) % 60) - 30);
      pixels.setPixelColor(j*8+i, pixels.Color(r, g, b));
    }
  }
      r = 30 - abs(((offset1 + 2*3 + 3*1) % 60) - 30);
      g = 30 - abs(((offset2 + 2*2 + 3*2 + 30) % 60) - 30);
      b = 30 - abs(((offset3 + 2*1 + 3*3 + 50) % 60) - 30);
  draw_single_digit_(2, 1, dt.hour() / 10, r, g, b);
      r = 30 - abs(((offset1 + 5*3 + 3*1) % 60) - 30);
      g = 30 - abs(((offset2 + 5*2 + 3*2 + 30) % 60) - 30);
      b = 30 - abs(((offset3 + 5*1 + 3*3 + 50) % 60) - 30);
  draw_single_digit_(4, 1, dt.hour() % 10, r, g, b);
}
/***************************************************************************/
void draw_hex_clock(DateTime dt) {
  float word_time_float;
  unsigned int word_time;
  word_time_float = (float(approx_millis)/1000 + (dt.hour()*60*60+dt.minute()*60+dt.second())) * 65536 / 86400;
  word_time = int(word_time_float);
//  word_time = (dt.hour()*60*60+dt.minute()*60+dt.second()) * 65536 / 86400;
  draw_single_digit_3x5(1, 1, word_time >> 12, col_a.r, col_a.g, col_a.b);
  draw_single_digit_3x5(4, 1, (word_time >> 8) & 0xF, col_b.r, col_b.g, col_b.b);
  for (int x=0; x<8; x++) {
    if (word_time & (0x1 << (7-x))) {
      set_pixel(7*8+x, col_b.r, col_b.g, col_b.b, brightness);
    }
  }
}


/***************************************************************************/
// normal clock with digits from 'font'
void draw_clock_4x4(DateTime dt, RGB col_a, RGB col_b) {
  draw_single_digit_4x4(0, 0, dt.hour() / 10, col_a.r, col_a.g, col_a.b);
  draw_single_digit_4x4(4, 0, dt.hour() % 10, col_b.r, col_b.g, col_b.b);
  draw_single_digit_4x4(0, 4, dt.minute() / 10, col_b.r, col_b.g, col_b.b);
  draw_single_digit_4x4(4, 4, dt.minute() % 10, col_a.r, col_a.g, col_a.b);
//  if (dt.second() % 2 == 0) {
//    set_pixel(7*8, col_a.r,col_a.g,col_a.b, brightness);
//  }
}

/***************************************************************************/

void draw_robot(int eyes, int arms, int legs, RGB col_a, RGB col_b) {
  display_bitmap_(0, 0, BITMAP_ROBOT_BASE1, col_a.r, col_a.g, col_a.b);
  display_bitmap_(0, 0, BITMAP_ROBOT_BASE2, col_b.r, col_b.g, col_b.b);
  display_bitmap_(0, 0, BITMAP_ROBOT_BASE3, 0, 10, 25);

  display_bitmap_(0, 0, arms, 0, 30, 0);
  display_bitmap_(0, 0, eyes, 0, 0, 0);
  display_bitmap_(0, 0, legs, 0, 10, 25);
}


void draw_clock_robot(DateTime dt, RGB col_a, RGB col_b) {
  int arms, eyes, legs;

  pixels.clear();
  draw_bcd_column(0, dt.hour(), col_a.r, col_a.g, col_a.b);
  draw_bcd_column(7, dt.minute(), col_b.r, col_b.g, col_b.b);

  if (dt.second() % 2 == 0) {
    eyes = BITMAP_ROBOT_EYES1;
  } else {
    eyes = BITMAP_ROBOT_EYES2;    
  }
  if (dt.minute() % 3 == 0) {
    arms = BITMAP_ROBOT_ARMS1;
  } else if (dt.minute() % 3 == 1) {
    arms = BITMAP_ROBOT_ARMS2;    
  } else {
    arms = BITMAP_ROBOT_ARMS3;
  }
  if (dt.hour() % 2 == 0) {
    legs = BITMAP_ROBOT_LEGS1;
  } else {
    legs = BITMAP_ROBOT_LEGS2;    
  }
  draw_robot(eyes, arms, legs, col_a, col_b);
  pixels.show();
}


void loop () 
{
    DateTime now = rtc.now(); //get the current date-time
    uint32_t ts = now.getEpoch();
    byte pix_trans;
    if (ts != old_ts) {
       start_approx_millis_time = millis();
    }
    approx_millis = millis() - start_approx_millis_time;
    if ((ts - last_action_ts) > 300) {
      // go back to default program mode
      pgm_mode = PGM_MODE_LO;
    }
    if (old_enc_pos != new_enc_pos) {
      // detection of movement
      old_enc_pos = new_enc_pos;
    }

    last_but_a_val = but_a_val;
    but_a_val = digitalRead(BUT_A_PIN);

    ldr_value = analogRead(LDR_PIN);
    //Serial.println(ldr_value);
    // offset of a different LDR
    brightness_target = 600 - int(float(ldr_value) * 0.9) + brightness_offset;
    if (brightness < max(brightness_target, 10)) {
    //if (brightness < max(1023 - ldr_value, 10)) {
      brightness++;
    } else {
      brightness--;
    }
    //Serial.println(max(600 - int(float(ldr_value) * 0.9) + brightness_offset, 10));
    //brightness = 50;  // for taking pictures only!!!

      // button down = entering time set mode
      if (but_a_val == LOW) {
          last_action_ts = ts;
          but_counter += 1;

//          // straight transition
//          pixels.setPixelColor((but_counter-50)/3, pixels.Color(0,5,0));
//          pixels.show();

          for (int i=0; i<64; i++) {
            pix_trans = pgm_read_byte(fill_pattern+i+FILL_PATTERN_1);
            if (pix_trans <= (but_counter-50)/3) {
              pixels.setPixelColor(i, pixels.Color(0,5,0));
            }
          }
          pixels.show();

          if (but_counter > 191+50) {   // approx 2 second press
            if (clock_mode == MODE_SET_CLOCK) {
              // cancel clock set
              clock_mode = last_clock_mode;
              pgm_mode = PGM_MODE_LO;
              pgm_mode_tm_state = PGM_TM_NONE;
              rot_enc.write(0);
              display_bitmap(BITMAP_NO, 4, 0, 0);
            } else {
              // enter set clock
              last_clock_mode = clock_mode;
              clock_mode = MODE_SET_CLOCK;
              pgm_mode = PGM_MODE_TM;
              pgm_mode_tm_state = PGM_TM_HOUR;
              rot_enc.write(now.hour()*4);
              new_enc_pos = now.hour()*4;
              hour_inc = now.hour();
              minute_inc = 0;
              enc_pos = new_enc_pos;
              display_bitmap(BITMAP_TM, 0, 4, 0);
            }
            wait_button_up();
          }
      } else {
            but_counter = 0;
      }

      switch (pgm_mode) {
        case PGM_MODE_TM:  // set time
          if ((last_but_a_val == LOW) && (but_a_val == HIGH)) {  // wait until the button is released
            DateTime dt;
            switch (pgm_mode_tm_state) {
              case PGM_TM_HOUR:
                pgm_mode_tm_state = PGM_TM_MINUTE;
                // save hour
                dt = DateTime(now.year(), now.month(), now.date(), hour_inc, now.minute(), now.second(), now.dayOfWeek());
                rtc.setDateTime(dt); //Adjust date-time as defined 'dt' above 
                rot_enc.write(now.minute()*4);
                new_enc_pos = now.minute()*4;
                minute_inc = now.minute();
                hour_inc = 0;  // it is set now
                //enc_pos = new_enc_pos;
                break;
              case PGM_TM_MINUTE:
                pgm_mode_tm_state = PGM_TM_NONE;
                clock_mode = last_clock_mode;
                // save minute
                dt = DateTime(now.year(), now.month(), now.date(), now.hour(), minute_inc, now.second(), now.dayOfWeek());
                rtc.setDateTime(dt); //Adjust date-time as defined 'dt' above 

                pgm_mode = PGM_MODE_LO;
                display_bitmap(BITMAP_OK, 0, 4, 0);
                delay(PGM_MODE_SWITCH_DELAY);
                pgm_mode_tm_state = PGM_TM_NONE;
                rot_enc.write(0);

                break;
            }
          }
          // encoder readout
          switch (pgm_mode_tm_state) {
            case PGM_TM_HOUR:
              if (new_enc_pos < 0) {
                new_enc_pos += 24*4;
                rot_enc.write(new_enc_pos);
              }
              if (new_enc_pos >= 24*4) {
                new_enc_pos -= 24*4;
                rot_enc.write(new_enc_pos);
              }
              hour_inc = new_enc_pos / 4;                
              break;
            case PGM_TM_MINUTE:
              if (new_enc_pos < 0) {
                new_enc_pos += 60*4;
                rot_enc.write(new_enc_pos);
              }
              if (new_enc_pos >= 60*4) {
                new_enc_pos -= 60*4;
                rot_enc.write(new_enc_pos);
              }
              minute_inc = new_enc_pos / 4;                 
              break;
          }
          break;
        case PGM_MODE_LO:
          // encoder readout
          if (new_enc_pos < 0) {
            new_enc_pos += 4*NUM_CLOCK_MODES;
            rot_enc.write(new_enc_pos);
          }
          clock_mode = (new_enc_pos / 4) % NUM_CLOCK_MODES;
          // button a readout
          if ((last_but_a_val == LOW) && (but_a_val == HIGH)) {
            write_eeprom();
            // go to color mode
            pgm_mode = PGM_MODE_CO;
            rot_enc.write(col_b.h + 256 * (255-col_b.s) / 64);  // calculate the encoder position back from current colors
            enc_pos = new_enc_pos;
            pos_enc_pos = col_b.h + 256 * (255-col_b.s) / 64;
            neg_enc_pos = -col_a.h - 256 * (255-col_a.s) / 64;
            display_colors();
            delay(PGM_MODE_SWITCH_DELAY);
          }
          break;
//        case PGM_MODE_GM:
//          // encoder readout
//          if (new_enc_pos != enc_pos) {
//            game();
//            wait_button_up();
//            rot_enc.write(0);
//            enc_pos = 0;
//            new_enc_pos = 0;
//          }
//          // button a readout
//          if ((last_but_a_val == LOW) && (but_a_val == HIGH)) {
//            pgm_mode = PGM_MODE_CO;
//            rot_enc.write(0);
//            enc_pos = new_enc_pos;
//            pos_enc_pos = 0;
//            neg_enc_pos = 0;
//            display_color_bitmap(BITMAP_CLR_CO);
//            delay(PGM_MODE_SWITCH_DELAY);
//          }
//          break;
        case PGM_MODE_CO:
          // encoder readout
          if (new_enc_pos < 0) {
            set_colors_cont_a2(-new_enc_pos);
            if (new_enc_pos > neg_enc_pos) {
              // we're rotating back
              rot_enc.write(pos_enc_pos);
              new_enc_pos = pos_enc_pos;
            } else {
              neg_enc_pos = new_enc_pos;
            }
          }
          if (new_enc_pos > 0) {
            set_colors_cont_b2(new_enc_pos);
            if (new_enc_pos < pos_enc_pos) {
              rot_enc.write(neg_enc_pos);
              new_enc_pos = neg_enc_pos;
            } else {
              pos_enc_pos = new_enc_pos;
            }
          }
          // button a readout
          if ((last_but_a_val == LOW) && (but_a_val == HIGH)) {
            write_eeprom();
            pgm_mode = PGM_MODE_BR;
            // calculate the encoder position back from current brightness offset
            rot_enc.write(brightness_offset);  
            display_brightness();
            delay(PGM_MODE_SWITCH_DELAY);
          }
          break;
        case PGM_MODE_BR:
          // encoder readout
          if (new_enc_pos < 0) {
            rot_enc.write(0);
            new_enc_pos = 0;
          }
          if (new_enc_pos > 100*4) {
            rot_enc.write(100*4);
            new_enc_pos = 100*4;
          }
          Serial.println(new_enc_pos);
          brightness_offset = new_enc_pos;
          // button a readout
          if ((last_but_a_val == LOW) && (but_a_val == HIGH)) {
            write_eeprom();
            pgm_mode = PGM_MODE_LO;
            rot_enc.write(clock_mode*4);
            enc_pos = new_enc_pos;
            display_bitmap(BITMAP_LO, 0, 0, 200);
            delay(PGM_MODE_SWITCH_DELAY);
          }
          break;
      }

    // only show stuff when no button is pressed down
    if (but_a_val == HIGH) {
      pixels.clear();
      switch (clock_mode) {
        case MODE_NORMAL:
          draw_normal_clock(now, col_a, col_b, 1, 1, 1, 1, 7*8);
          break;
        case MODE_NORMAL_REV:
          draw_normal_clock(now, col_b, col_a, 1, 1, 1, 1, 7*8);
          break;
        case MODE_NORMAL_ALT:
          draw_normal_clock(now, col_a, col_b, 0, 1, 2, 3, 7*8);
          break;
        case MODE_NORMAL_ALT_REV:
          draw_normal_clock(now, col_b, col_a, 0, 1, 2, 3, 7*8);
          break;
        case MODE_NORMAL_ALT2:
          draw_normal_clock(now, col_a, col_b, 3, 2, 1, 0, 7*8+7);
          break;
        case MODE_NORMAL_ALT2_REV:
          draw_normal_clock(now, col_b, col_a, 3, 2, 1, 0, 7*8+7);
          break;
        case MODE_NORMAL_ALT3:
          draw_normal_clock(now, col_a, col_b, 1, 2, 1, 2, 7*8);
          break;
        case MODE_NORMAL_ALT3_REV:
          draw_normal_clock(now, col_b, col_a, 1, 2, 1, 2, 7*8);
          break;
        case MODE_BCD:
          draw_bcd_clock(now);
          break;
        case MODE_ANALOG:
          draw_analog_clock(now);
          break;
        case MODE_PARTY:
          draw_party(now);
          break;
        case MODE_HEX:
          draw_hex_clock(now);
          break;
        case MODE_GAME_OF_LIFE:
          if (old_ts / 2 != ts / 2) {
            step_game_of_life();
            //step_game_of_life_test();
            //step_game_of_life_majority();
          }
          if (old_ts / 10 != ts / 10) {
            // for majority rule
//            for (int i=0; i<3*64; i++) {
//              game_of_life[0][i] = 0;
//            }

            draw_normal_clock_gol(now, col_a, col_b, 1, 1, 1, 1, 7*8);
          }
          draw_game_of_life(now);
          // draw_normal_clock(now, col_a, col_b, 1, 1, 1, 1, 8*8);
          break;
        case MODE_4X4:
          draw_clock_4x4(now, col_a, col_b);
          break;
        case MODE_WORD:
          draw_clock_word(now, col_a, col_b);
          break;
        case MODE_ROBOT:
          draw_clock_robot(now, col_a, col_b);
          break;
        case MODE_SET_CLOCK:
          // edit clock  
          draw_edit_clock(now, hour_inc, minute_inc);    
          break;
        }
    }

    // day transition
    if ((now.hour() == 23) && (now.minute() == 59) && (now.second() >= 57)) {
      tmp_int = ((((now.second() - 57) % 3) * 900 + approx_millis) * 15 / 1000);
     // x-wing 1=black 2=orange 3=red
      display_big_bitmap(
        tmp_int-8, 0,
        BIG_BITMAP_0, 
        5,5,3,
        50,20,5,
        20,0,0);
    }

    // other hour transitions
    if ((now.hour() != 23) && (now.minute() == 59) && (now.second() >= 58)) {
      tmp_int = (((now.second() % 2) * 1000 + approx_millis) * 12 / 1000) ;  // reversed x-location

      switch(now.hour() % 5) {
        
      case 0:
      if ((approx_millis / 150) % 2 == 0) {
        display_bitmap_(8-tmp_int, 0, BITMAP_PACMAN0, 50, 50, 0);
      } else {
        display_bitmap_(8-tmp_int, 0, BITMAP_PACMAN1, 50, 50, 0);
      }
      break;

      case 1:
      // blinky
      if ((approx_millis / 150) % 2 == 0) {
        display_bitmap_(8-tmp_int, 0, BITMAP_GHOST0, 50, 0, 0);
      } else {
        display_bitmap_(8-tmp_int, 0, BITMAP_GHOST1, 50, 0, 0);
      }
      break;

      case 2:
      // pinky
      if ((approx_millis / 150) % 2 == 0) {
        display_bitmap_(8-tmp_int, 0, BITMAP_GHOST0, 50, 20, 30);
      } else {
        display_bitmap_(8-tmp_int, 0, BITMAP_GHOST1, 50, 20, 30);
      }
      break;

      case 3:
      // inky
      if ((approx_millis / 150) % 2 == 0) {
        display_bitmap_(8-tmp_int, 0, BITMAP_GHOST0, 0, 50, 50);
      } else {
        display_bitmap_(8-tmp_int, 0, BITMAP_GHOST1, 0, 50, 50);
      }
      break;

      case 4:
      // clyde
      if ((approx_millis / 150) % 2 == 0) {
        display_bitmap_(8-tmp_int, 0, BITMAP_GHOST0, 50, 20, 5);
      } else {
        display_bitmap_(8-tmp_int, 0, BITMAP_GHOST1, 50, 20, 5);
      }
      break;
      }
    }
    pixels.show();

    old_ts = ts;
    fast_counter++;
}
