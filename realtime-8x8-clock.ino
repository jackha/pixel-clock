/*

ds3231 realtime clock + 8x8 ws2812 "display" (aka neopixels) = awesome clock!


*/

// Date and time functions using RX8025 RTC connected via I2C and Wire lib

#include <Wire.h>
#include "Sodaq_DS3231.h"
//#include "pitches.h"  // from: toneMelody
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <Encoder.h>  // rotary encoder

Encoder rot_enc(6, 5);  // rotary encoder: D5, D6
#define BUT_A_PIN 8  // button next to rotary encoder
#define BUT_B_PIN 7  // button on rotary encoder

#define LDR_PIN A0

#define NEOPIXEL_PIN 10  // we plug an 8x8 onto it
#define NEOPIXEL_NUM 64

#define MODE_NORMAL 0  // use the 2x5 font
#define MODE_BCD 1 // binary coded digital 
#define MODE_SET_CLOCK 2 // go to this mode temporary and then go back to the old mode 

#define NUM_CLOCK_MODES 2  //

// every time you press button A, the program mode advances
#define PGM_MODE_TM 0  // time
#define PGM_MODE_LO 1  // layout
#define PGM_MODE_GM 2  // game
#define PGM_MODE_CO 3  // color

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
};

struct RGB col_a; 
struct RGB col_b; 

// 2x5 font, numbers only
static const unsigned char font[] PROGMEM = {
    0b11, 
    0b11, 
    0b11, 
    0b11, 
    0b11,
    
    0b01, 
    0b01, 
    0b01,
    0b01,
    0b01,
    
    0b10, 
    0b01, 
    0b11,
    0b10,
    0b11,
    
    0b11, 
    0b01, 
    0b11,
    0b01,
    0b11,
    
    0b10, 
    0b10, 
    0b11,
    0b01,
    0b01,
    
    0b11, 
    0b10, 
    0b11,
    0b01,
    0b11,
    
    0b01, 
    0b10, 
    0b11,
    0b11,
    0b11,
    
    0b11, 
    0b01, 
    0b01,
    0b01,
    0b01,
    
    0b11, 
    0b11, 
    0b00,
    0b11,
    0b11,
    
    0b11, 
    0b11, 
    0b11,
    0b01,
    0b10
};

#define BITMAP_TM 0
#define BITMAP_LO 8
#define BITMAP_GM 16

static const unsigned char bitmaps[] PROGMEM = {
  0b11101110,
  0b01000100,
  0b01001110,
  0b00000000,
  0b11110011,
  0b10101011,
  0b10101010,
  0b00000011,

  0b10000110,
  0b10001001,
  0b10001001,
  0b11100110,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,

  0b01000110,
  0b10001001,
  0b10101111,
  0b11101001,
  0b00000000,
  0b11110011,
  0b10101010,
  0b10101001,
};

#define BITMAP_CLR_CO 0

static const unsigned char bitmaps_co[] PROGMEM = {
  0x00, 0x00, 0x00,  0x40, 0x00, 0x00,  0x40, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x20,  0x00, 0x00, 0x20,  0x00, 0x00, 0x00,  
  0x40, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x40, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x20,  0x00, 0x00, 0x00,  0x00, 0x00, 0x20,  
  0x40, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x40, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x20,  0x00, 0x00, 0x00,  0x00, 0x00, 0x20,  
  0x40, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x40, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x20,  0x00, 0x00, 0x20,  0x00, 0x00, 0x00,  
  0x40, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x40, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x20,  0x00, 0x00, 0x20,  0x00, 0x00, 0x00,  
  0x40, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x40, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x20,  0x00, 0x00, 0x00,  0x00, 0x00, 0x20,  
  0x40, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x40, 0x00,  0x00, 0x40, 0x00,  0x00, 0x40, 0x00,  0x00, 0x00, 0x20,  0x00, 0x00, 0x00,  0x00, 0x00, 0x20,  
  0x00, 0x00, 0x00,  0x40, 0x00, 0x00,  0x40, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x20,  0x00, 0x00, 0x00,  0x00, 0x00, 0x20,  
};

static const unsigned char colors[] PROGMEM = {
  40,  0,  0,   25, 15,  0,
  30, 10,  0,   20, 20,  0,
   0, 40,  0,   20, 20,  0,
   0, 30, 20,    0, 10, 10,
   0,  0, 40,    0, 20, 10,
  20,  0, 30,   20,  0, 10,
  20, 20, 20,   10, 10, 30,
};

#define NUM_COLORS 7


long enc_pos  = -999;  // rotary encoder position
volatile long new_enc_pos = -999;
uint16_t fast_counter = 0;  // just cycles, for use in timing / moving stuff other than seconds

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEOPIXEL_NUM, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);


char weekDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
//year, month, date, hour, min, sec and week-day(starts from 0 and goes to 6)
//writing any non-existent time-data may interfere with normal operation of the RTC.
//Take care of week-day also.
DateTime dt(2016, 2, 10, 15, 18, 0, 5);


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
int pgm_mode = PGM_MODE_TM;

int pgm_mode_tm_state = PGM_TM_NONE;
int hour_inc = 0;  // increase/decrease hour because we want to change it - is auto modded (%)
int minute_inc = 0;

int last_but_a_val = HIGH;
int last_but_b_val = HIGH;
int but_a_val = HIGH;
int but_b_val = HIGH;

int color_idx = 0;

uint16_t ldr_value;
uint16_t brightness;

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

  // test tones
//    // iterate over the notes of the melody:
//  for (int thisNote = 0; thisNote < 8; thisNote++) {
//
//    // to calculate the note duration, take one second
//    // divided by the note type.
//    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
//    int noteDuration = 1000 / noteDurations[thisNote];
//    tone(9, melody[thisNote], noteDuration);
//
//    // to distinguish the notes, set a minimum time between them.
//    // the note's duration + 30% seems to work well:
//    int pauseBetweenNotes = noteDuration * 1.30;
//    delay(pauseBetweenNotes);
//    // stop the tone playing:
//    noTone(9);
//  }
    pinMode(BUT_A_PIN, INPUT_PULLUP);
    pinMode(BUT_B_PIN, INPUT_PULLUP);

    pinMode(LDR_PIN, INPUT_PULLUP);

    Serial.begin(57600);
    Wire.begin();
    rtc.begin();

    //rtc.setDateTime(dt); //Adjust date-time as defined 'dt' above 
    // test
    pixels.begin();
      pixels.setPixelColor(0, pixels.Color(0, 0, 5 ));  // test
      pixels.show();

  // set some variables
  color_idx = 0;
  set_colors(color_idx);
//  col_a.r = 40;
//  col_a.g = 0;
//  col_a.b = 0;
//
//  col_b.r = 20;
//  col_b.g = 20;
//  col_b.b = 0;

  // prevent readouts on start
  new_enc_pos = rot_enc.read();
  enc_pos = new_enc_pos;
}

uint32_t old_ts;

void set_colors(int color_idx) {
  col_a.r = pgm_read_byte(colors+color_idx*6);
  col_a.g = pgm_read_byte(colors+color_idx*6+1);
  col_a.b = pgm_read_byte(colors+color_idx*6+2);

  col_b.r = pgm_read_byte(colors+color_idx*6+3);
  col_b.g = pgm_read_byte(colors+color_idx*6+4);
  col_b.b = pgm_read_byte(colors+color_idx*6+5);
}

void set_pixel(int offset, uint32_t r, uint32_t g, uint32_t b, uint32_t brightness) {
  // brightness 0..1023
  // offset is memory location
  pixels.setPixelColor(offset, pixels.Color((r * brightness) >> 10, (g * brightness) >> 10, (b * brightness) >> 10));
}

// draw single digit on x, y location.
void draw_single_digit(int xx, int yy, int digit, byte r, byte g, byte b) {
  byte digit_row;
  for (int y=0; y<5; y++) {
    digit_row = pgm_read_byte(font+y+digit*5);
    for (int x=0; x<2; x++) {
      if ((digit_row & 0b1) > 0) {
//        pixels.setPixelColor(xx + 1 - x + (yy + y) * 8, pixels.Color(r, g, b));
        set_pixel(xx + 1 - x + (yy + y) * 8, r, g, b, brightness);
      }
      digit_row = digit_row >> 1;
    }
  }
}

void display_bitmap(int offset, byte r, byte g, byte b) {
  byte digit_row;
  pixels.clear();
  for (int y=0; y<8; y++) {
    digit_row = pgm_read_byte(bitmaps+offset+y);
    for (int x=0; x<8; x++) {
      if ((digit_row & 0b1) > 0) {
        pixels.setPixelColor(7 - x + y * 8, pixels.Color(r, g, b));
      }
      digit_row = digit_row >> 1;
    }
  }
  pixels.show();
}


void display_color_bitmap(int offset) {
  byte r, g, b;
  pixels.clear();
  for (int y=0; y<8; y++) {
    for (int x=0; x<8; x++) {
      r = pgm_read_byte(bitmaps_co+y*3*8+x*3);
      g = pgm_read_byte(bitmaps_co+y*3*8+x*3+1);
      b = pgm_read_byte(bitmaps_co+y*3*8+x*3+2);
      pixels.setPixelColor(x + y * 8, pixels.Color(r, g, b));
    }
  }
  pixels.show();
}

// normal clock with digits from 'font'
void draw_normal_clock(DateTime dt) {
  draw_single_digit(0, 1, dt.hour() / 10, col_a.r, col_a.g, col_a.b);
  draw_single_digit(2, 1, dt.hour() % 10, col_b.r, col_b.g, col_b.b);
  draw_single_digit(4, 1, dt.minute() / 10, col_a.r, col_a.g, col_a.b);
  draw_single_digit(6, 1, dt.minute() % 10, col_b.r, col_b.g, col_b.b);
  if (dt.second() % 2 == 0) {
    pixels.setPixelColor(7*8, pixels.Color(col_b.r,col_b.g,col_b.b));
  }
}

void draw_edit_clock(DateTime dt, int hour_inc, int minute_inc) {
  // make it blink
  if ((pgm_mode_tm_state == PGM_TM_MINUTE) || ((fast_counter >> 6) % 2 == 0)) {
    draw_single_digit(0, 1, max(min(dt.hour() + hour_inc, 23), 0) / 10, col_a.r, col_a.g, col_a.b);
    draw_single_digit(2, 1, max(min(dt.hour() + hour_inc, 23), 0) % 10, col_b.r, col_b.g, col_b.b);
  }
  if ((pgm_mode_tm_state == PGM_TM_HOUR) || ((fast_counter >> 6) % 2 == 0)) {
    draw_single_digit(4, 1, max(min(dt.minute() + minute_inc, 59), 0) / 10, col_a.r, col_a.g, col_a.b);
    draw_single_digit(6, 1, max(min(dt.minute() + minute_inc, 59), 0) % 10, col_b.r, col_b.g, col_b.b);
  }
}

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

void game() {
  // frogger-like game
  boolean finished = false;
  int player_x = 3;
  int player_y = 7;
  int c = 0;
  long enc_val = 0;
  int cars[2][8] = {
    {0,0,0,0,0,1,1,0},
    {0,1,1,0,0,0,0,1}
  };
  int trees[3][8] = {
    {1,1,1,0,1,1,0,0},
    {0,1,1,1,0,0,1,1},
    {1,1,0,0,1,1,1,0}
  };
  int lady_frog_coords[3][2] = {  // x, y
    {1, 0},
    {4, 0},
    {6, 0}
  };
  boolean collided = false;

  
  rot_enc.write(3*4);
  while (!finished) {
    pixels.clear();

  for (int lady_frog=0; lady_frog<3; lady_frog++) {
    pixels.setPixelColor(lady_frog_coords[lady_frog][0] + lady_frog_coords[lady_frog][1] * 8, pixels.Color(30,15,20));
  }

  for (int tree=0; tree<3; tree++) {
    for (int x=0; x<8; x++) {
      if (trees[tree][x] == 1) {
        pixels.setPixelColor(tree*8+1*8+x, pixels.Color(10,5,1));
      }
   }
  }

    last_but_a_val = but_a_val;
    last_but_b_val = but_b_val;
    but_a_val = digitalRead(BUT_A_PIN);
    but_b_val = digitalRead(BUT_B_PIN);
    // quit
    if ((last_but_a_val == HIGH) && (but_a_val == LOW)) {
      finished = true;
    }
    if ((last_but_b_val == HIGH) && (but_b_val == LOW)) {
      player_y --;
    }
    enc_val = rot_enc.read();
    if (enc_val < 0) {
      rot_enc.write(0);
      enc_val = 0;
    }
    if (enc_val > 7*4) {
      rot_enc.write(7*4);
      enc_val = 7*4;
    }
    player_x = enc_val / 4;
    // collision detection
    for (int y=0; y<2; y++) {
      for (int x=0; x<8; x++) {
        if (cars[y][x] == 1) {
          if (5*8+y*8+x == player_y*8+player_x) {
            collided = true;
            finished = true;
          }
          pixels.setPixelColor(5*8+y*8+x, pixels.Color(0,0,30));
        }
      }
    }
    // player
    pixels.setPixelColor(player_x + 8*player_y, pixels.Color(0,40,0));

    pixels.show();
    c++;
    delay(50);
  }
  if (collided) {
    pixels.setPixelColor(player_x + 8*player_y, pixels.Color(40,0,0));
    pixels.show();
    delay(1000);
  }
}

//void game() {
//  // frogger-like game
//  boolean finished = false;
//  int c = 0;
//  rot_enc.write(0);
//  while (!finished) {
//    pixels.clear();
//    last_but_a_val = but_a_val;
//    last_but_b_val = but_b_val;
//    but_a_val = digitalRead(BUT_A_PIN);
//    but_b_val = digitalRead(BUT_B_PIN);
//    if ((last_but_a_val == HIGH) && (but_a_val == LOW)) {
//      finished = true;
//    }
//    //pixels.setPixelColor(c % 64, pixels.Color(100,100,100));
//    pixels.show();
//    c++;
//    delay(50);
//  }
//}

void loop () 
{
    DateTime now = rtc.now(); //get the current date-time
    uint32_t ts = now.getEpoch();

    last_but_a_val = but_a_val;
    last_but_b_val = but_b_val;
    but_a_val = digitalRead(BUT_A_PIN);
    but_b_val = digitalRead(BUT_B_PIN);

    ldr_value = analogRead(LDR_PIN);
    if (brightness < max(1023 - ldr_value, 10)) {
      brightness++;
    } else {
      brightness--;
    }

      switch (pgm_mode) {
        case PGM_MODE_TM:
          // button a readout
          if ((last_but_a_val == HIGH) && (but_a_val == LOW)) {
            clock_mode = last_clock_mode;
            if (pgm_mode_tm_state == PGM_TM_NONE) {
              pgm_mode = PGM_MODE_LO;
              display_bitmap(BITMAP_LO, 0, 0, 4);
              delay(PGM_MODE_SWITCH_DELAY);
            }
            pgm_mode_tm_state = PGM_TM_NONE;
          }
          // button b
          if ((last_but_b_val == HIGH) && (but_b_val == LOW)) {
            DateTime dt;
            switch (pgm_mode_tm_state) {
              case PGM_TM_NONE:
                last_clock_mode = clock_mode;
                clock_mode = MODE_SET_CLOCK;
                pgm_mode_tm_state = PGM_TM_HOUR;
                rot_enc.write(0);
                enc_pos = new_enc_pos;
                break;
              case PGM_TM_HOUR:
                pgm_mode_tm_state = PGM_TM_MINUTE;
                // save hour
                dt = DateTime(now.year(), now.month(), now.date(), max(min(now.hour()+hour_inc, 23), 0), now.minute(), now.second(), now.dayOfWeek());
                rtc.setDateTime(dt); //Adjust date-time as defined 'dt' above 
                hour_inc = 0;  // it is set now
                rot_enc.write(0);
                enc_pos = new_enc_pos;
                break;
              case PGM_TM_MINUTE:
                pgm_mode_tm_state = PGM_TM_NONE;
                clock_mode = last_clock_mode;
                // save minute
                dt = DateTime(now.year(), now.month(), now.date(), now.hour(), max(min(now.minute() + minute_inc, 59), 0), now.second(), now.dayOfWeek());
                rtc.setDateTime(dt); //Adjust date-time as defined 'dt' above 
                break;
            }
          }
          // encoder readout
          if (new_enc_pos != enc_pos) {
            switch (pgm_mode_tm_state) {
              case PGM_TM_HOUR:
                // minute
                hour_inc = new_enc_pos / 4;                 
                break;
              case PGM_TM_MINUTE:
                // hour
                minute_inc = new_enc_pos / 4;                 
                break;
            }
          }
          break;
        case PGM_MODE_LO:
          // button a readout
          if ((last_but_a_val == HIGH) && (but_a_val == LOW)) {
            pgm_mode = PGM_MODE_GM;
            rot_enc.write(0);
            enc_pos = new_enc_pos;
            last_clock_mode = clock_mode;
            display_bitmap(BITMAP_GM, 0, 0, 4);
            delay(PGM_MODE_SWITCH_DELAY);
          }
          // encoder readout
          if (new_enc_pos != enc_pos) {
            clock_mode = (new_enc_pos / 4) % NUM_CLOCK_MODES;
          }
          break;
        case PGM_MODE_GM:
          // button a readout
          if ((last_but_a_val == HIGH) && (but_a_val == LOW)) {
            pgm_mode = PGM_MODE_CO;
            rot_enc.write(0);
            enc_pos = new_enc_pos;
            display_color_bitmap(BITMAP_CLR_CO);
            delay(PGM_MODE_SWITCH_DELAY);
          }
          if ((last_but_b_val == HIGH) && (but_b_val == LOW)) {
            game();
          }
          // encoder readout
          if (new_enc_pos != enc_pos) {
          }
          break;
        case PGM_MODE_CO:
          // button a readout
          if ((last_but_a_val == HIGH) && (but_a_val == LOW)) {
            pgm_mode = PGM_MODE_TM;
            rot_enc.write(0);
            enc_pos = new_enc_pos;
            display_bitmap(BITMAP_TM, 0, 0, 4);
            delay(PGM_MODE_SWITCH_DELAY);
          }
          if ((last_but_b_val == HIGH) && (but_b_val == LOW)) {
          }
          // encoder readout
          if (new_enc_pos < 0) {
            new_enc_pos += NUM_COLORS;
            rot_enc.write(new_enc_pos);
          }
          color_idx = (new_enc_pos / 4) % NUM_COLORS;
          set_colors(color_idx);
//          if (new_enc_pos != enc_pos) {
//          }
          break;
      }
    
    enc_pos = new_enc_pos;
    

    if (old_ts == 0 || old_ts != ts) {
    	old_ts = ts;
//    	Serial.print(now.year(), DEC);
//    	Serial.print('/');
//    	Serial.print(now.month(), DEC);
//    	Serial.print('/');
//    	Serial.print(now.date(), DEC);
//    	Serial.print(' ');
//    	Serial.print(now.hour(), DEC);
//    	Serial.print(':');
//    	Serial.print(now.minute(), DEC);
//    	Serial.print(':');
//    	Serial.print(now.second(), DEC);
//    	Serial.print(' ');
//    	Serial.print(weekDay[now.dayOfWeek()]);
//    	Serial.println();
//    	Serial.print("Seconds since Unix Epoch: "); 
//    	Serial.print(ts, DEC);
//    	Serial.println();

//      pixels.setPixelColor(now.second(), pixels.Color((now.minute() % 2) * 5 , ((now.minute() + 1) % 2) * 5, 0 ));
//      pixels.setPixelColor(new_enc_pos, pixels.Color(col_b.r,col_b.g,col_b.b));
    }
      pixels.clear();
      switch (clock_mode) {
        case MODE_NORMAL:
          draw_normal_clock(now);
          break;
        case MODE_BCD:
          draw_bcd_clock(now);
          break;
        case MODE_SET_CLOCK:
          // edit clock  
          draw_edit_clock(now, hour_inc, minute_inc);    
          break;
        }

//      // ldr brightness testing
//      for (uint16_t i=0; i<8; i++) {
//        if ((1023 - ldr_value) / 4 / 16 / 2 > i) {
//          pixels.setPixelColor(i, pixels.Color(0,0,2));
//        }
//      }
      pixels.show();

    //delay(1);
    fast_counter++;
}
