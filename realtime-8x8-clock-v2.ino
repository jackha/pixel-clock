/*

ds3231 realtime clock + 8x8 ws2812 "display" (aka neopixels) = awesome clock!


*/

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

#define NUM_CLOCK_MODES 12  //

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

// hex only
static const unsigned char font4x7[] PROGMEM = {
  0b0110,
  0b1001,
  0b1011,
  0b1101,
  0b1001,
  0b1001,
  0b0110,
  
  0b0100,
  0b1100,
  0b0100,
  0b0100,
  0b0100,
  0b0100,
  0b1110,
  
  0b0110,
  0b1001,
  0b0001,
  0b0010,
  0b0100,
  0b1000,
  0b1111,
  
  0b0110,
  0b1001,
  0b0001,
  0b0110,
  0b0001,
  0b1001,
  0b0110,
  
  0b1001,
  0b1001,
  0b1001,
  0b1111,
  0b0001,
  0b0001,
  0b0001,
  
  0b1111,
  0b1000,
  0b1000,
  0b1110,
  0b0001,
  0b0001,
  0b1110,
  
  0b0111,
  0b1000,
  0b1000,
  0b1110,
  0b1001,
  0b1001,
  0b0110,
  
  0b1111,
  0b0001,
  0b0010,
  0b0100,
  0b1000,
  0b1000,
  0b1000,
  
  0b0110,
  0b1001,
  0b1001,
  0b0110,
  0b1001,
  0b1001,
  0b0110,
  
  0b0110,
  0b1001,
  0b1001,
  0b0111,
  0b0001,
  0b0001,
  0b0110,
  
  0b0110,
  0b1001,
  0b1001,
  0b1111,
  0b1001,
  0b1001,
  0b1001,
  
  0b1110,
  0b1001,
  0b1001,
  0b1110,
  0b1001,
  0b1001,
  0b1110,
  
  0b0110,
  0b1001,
  0b1000,
  0b1000,
  0b1000,
  0b1001,
  0b0110,
  
  0b1100,
  0b1010,
  0b1001,
  0b1001,
  0b1001,
  0b1001,
  0b1100,
  
  0b1111,
  0b1000,
  0b1000,
  0b1110,
  0b1000,
  0b1000,
  0b1111,
  
  0b1111,
  0b1000,
  0b1000,
  0b1111,
  0b1000,
  0b1000,
  0b1000,
};

// hex font
static const unsigned char font3x5[] PROGMEM = {
  0b010,
  0b101,
  0b101,
  0b101,
  0b010,

  0b010,
  0b010,
  0b010,
  0b010,
  0b010,

  0b110,
  0b001,
  0b010,
  0b100,
  0b111,

  0b111,
  0b001,
  0b111,
  0b001,
  0b111,

  0b101,
  0b101,
  0b111,
  0b001,
  0b001,

  0b111,
  0b100,
  0b110,
  0b001,
  0b110,

  0b010,
  0b100,
  0b111,
  0b101,
  0b111,

  0b111,
  0b001,
  0b010,
  0b010,
  0b010,

  0b111,
  0b101,
  0b111,
  0b101,
  0b111,

  0b111,
  0b101,
  0b111,
  0b001,
  0b010,

  0b010,
  0b101,
  0b111,
  0b101,
  0b101,

  0b110,
  0b101,
  0b110,
  0b101,
  0b110,

  0b011,
  0b100,
  0b100,
  0b100,
  0b011,

  0b110,
  0b101,
  0b101,
  0b101,
  0b110,

  0b111,
  0b100,
  0b111,
  0b100,
  0b111,

  0b111,
  0b100,
  0b111,
  0b100,
  0b100,
};

#define BITMAP_TM 0
#define BITMAP_LO 8
#define BITMAP_GM 16
#define BITMAP_OK 24
#define BITMAP_NO 32

static const unsigned char bitmaps[] PROGMEM = {
  // 'set' aka 'time'
  0b00000000,
  0b11011010,
  0b10010011,
  0b11011010,
  0b01010010,
  0b11011001,
  0b00000000,
  0b00000000,
//  0b11101110,
//  0b01000100,
//  0b01001110,
//  0b00000000,
//  0b11110011,
//  0b10101011,
//  0b10101010,
//  0b00000011,

  // lo layout
  0b10000110,
  0b10001001,
  0b10001001,
  0b11100110,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,

  // game
  0b01000110,
  0b10001001,
  0b10101111,
  0b11101001,
  0b00000000,
  0b11110011,
  0b10101010,
  0b10101001,

  // ok
  0b00000000,
  0b01001001,
  0b10101010,
  0b10101100,
  0b10101110,
  0b10101001,
  0b01001001,
  0b00000000,

  // no
  0b00000000,
  0b10010010,
  0b11010101,
  0b10110101,
  0b10010101,
  0b10010010,
  0b00000000,
  0b00000000,
};


// fill patterns for transitions. 8x8 with content 'time of change'
#define FILL_PATTERN_1 0

static const unsigned char fill_pattern[] PROGMEM = {
  0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 
  0x3A, 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x31, 
  0x3B, 0x20, 0x0D, 0x0C, 0x0B, 0x0A, 0x19, 0x30, 
  0x3C, 0x21, 0x0E, 0x03, 0x02, 0x09, 0x18, 0x2F, 
  0x3D, 0x22, 0x0F, 0x04, 0x01, 0x08, 0x17, 0x2E, 
  0x3E, 0x23, 0x10, 0x05, 0x06, 0x07, 0x16, 0x2D, 
  0x3F, 0x24, 0x11, 0x12, 0x13, 0x14, 0x15, 0x2C, 
  0x40, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 
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
  35,  5,  0,   23, 17,  0,
  30, 10,  0,   20, 20,  0,
   0, 40,  0,   20, 20,  0,
   0, 30, 20,    0, 10, 10,
   0,  0, 40,    0, 20, 10,
  20,  0, 30,   20,  0, 10,
  20, 20, 20,   10, 10, 30,
};

#define NUM_COLORS 8

// y*8 + x:
// 0b00yyyxxx
static const unsigned char analog_arm_idx[] PROGMEM = {
  0b00000011, 0b00000100,
  0b00000101, 0b00001110,
  0b00001110, 0b00010111,
  0b00011111, 0b00100111,
  0b00101111, 0b00110110,
  0b00110110, 0b00111101,
  0b00111011, 0b00111100,
  0b00111010, 0b00110001,
  0b00110001, 0b00101000,
  0b00100000, 0b00011000,
  0b00001001, 0b00010000,
  0b00000010, 0b00001001,
};

// actually every 2 seconds, we set 30 pixels
static const unsigned char minute_idx[] PROGMEM = {
  2+6*8, 3+6*8, 4+6*8, 5+6*8,
  1+5*8, 2+5*8, 3+5*8, 4+5*8, 5+5*8, 6+5*8,
  1+4*8, 2+4*8, 3+4*8, 4+4*8, 5+4*8, 6+4*8,
  1+3*8, 2+3*8,               5+3*8, 6+3*8,
  1+2*8, 2+2*8, 3+2*8, 4+2*8, 5+2*8, 6+2*8,
  2+1*8, 3+1*8, 4+1*8, 5+1*8,
};

long enc_pos  = -999;  // rotary encoder position
// for colors
long neg_enc_pos  = -999;  // rotary encoder position
long pos_enc_pos  = -999;  // rotary encoder position
volatile long new_enc_pos = -999;
uint16_t fast_counter = 0;  // just cycles, for use in timing / moving stuff other than seconds

unsigned long start_approx_millis_time = 0;
unsigned long approx_millis = 0;  // approximate millis

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

}

uint32_t old_ts;

void write_eeprom() {
  EEPROM.write(0, col_a.r);
  EEPROM.write(1, col_a.g);
  EEPROM.write(2, col_a.b);

  EEPROM.write(3, col_b.r);
  EEPROM.write(4, col_b.g);
  EEPROM.write(5, col_b.b);

  EEPROM.write(6, clock_mode);
}

void read_eeprom() {
  col_a.r = EEPROM.read(0);
  col_a.g = EEPROM.read(1);
  col_a.b = EEPROM.read(2);

  col_b.r = EEPROM.read(3);
  col_b.g = EEPROM.read(4);
  col_b.b = EEPROM.read(5);

  clock_mode = EEPROM.read(6);  
  if (clock_mode > NUM_CLOCK_MODES) {
    // problably empty eeprom
    set_colors(0);
    clock_mode = MODE_NORMAL;
  }
}

void set_colors(int color_idx) {
  col_a.r = pgm_read_byte(colors+color_idx*6);
  col_a.g = pgm_read_byte(colors+color_idx*6+1);
  col_a.b = pgm_read_byte(colors+color_idx*6+2);

  col_b.r = pgm_read_byte(colors+color_idx*6+3);
  col_b.g = pgm_read_byte(colors+color_idx*6+4);
  col_b.b = pgm_read_byte(colors+color_idx*6+5);
}

void set_colors_cont_a(long int enc) {
  col_a.r = abs(((enc * 587 + 0) % 16000) - 8000) / 200;
  col_a.g = abs(((enc * 389 + 128) % 16000) - 8000) / 200; 
  col_a.b = abs(((enc * 881 + 128) % 16000) - 8000) / 200;
}

void set_colors_cont_b(long int enc) {
  col_b.r = abs(((enc * 293 + 0) % 16000) - 8000) / 200;
  col_b.g = abs(((enc * 331 + 128) % 16000) - 8000) / 200; 
  col_b.b = abs(((enc * 523 + 128) % 16000) - 8000) / 200;
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

void draw_single_digit_(int xx, int yy, int digit, byte r, byte g, byte b) {
  byte digit_row;
  for (int y=0; y<5; y++) {
    digit_row = pgm_read_byte(font+y+digit*5);
    for (int x=0; x<2; x++) {
      if ((digit_row & 0b1) > 0) {
        pixels.setPixelColor(xx + 1 - x + (yy + y) * 8, pixels.Color(r, g, b));
        //set_pixel(xx + 1 - x + (yy + y) * 8, r, g, b, brightness);
      }
      digit_row = digit_row >> 1;
    }
  }
}

// draw single digit on x, y location with 4x7 font
void draw_single_digit_4x7(int xx, int yy, int digit, byte r, byte g, byte b) {
  byte digit_row;
  for (int y=0; y<7; y++) {
    digit_row = pgm_read_byte(font4x7+y+digit*7);
    for (int x=0; x<4; x++) {
      if ((digit_row & 0b1) > 0) {
        set_pixel(xx + 3 - x + (yy + y) * 8, r, g, b, brightness);
      }
      digit_row = digit_row >> 1;
    }
  }
}

// draw single digit on x, y location with 4x7 font
void draw_single_digit_3x5(int xx, int yy, int digit, byte r, byte g, byte b) {
  byte digit_row;
  for (int y=0; y<5; y++) {
    digit_row = pgm_read_byte(font3x5+y+digit*5);
    for (int x=0; x<3; x++) {
      if ((digit_row & 0b1) > 0) {
        set_pixel(xx + 2 - x + (yy + y) * 8, r, g, b, brightness);
      }
      digit_row = digit_row >> 1;
    }
  }
}

void wait_button_up() {
    // wait button up
    while (but_a_val == LOW) {
        but_a_val = digitalRead(BUT_A_PIN);
    }
    last_but_a_val = but_a_val;
}

/***************************************************************************/
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
// for every hour we have 2 pixels lit
void draw_analog_clock(DateTime dt) {
  int pixel0_idx = pgm_read_byte(analog_arm_idx+(dt.hour()%12)*2);
  int pixel1_idx = pgm_read_byte(analog_arm_idx+(dt.hour()%12)*2+1);
  RGB col_hour = col_a;
  RGB col_minute = col_b;
  if (dt.hour() >= 12) {
    col_hour = col_b;
    col_minute = col_a;
  }
    set_pixel(pixel0_idx, col_hour.r, col_hour.g, col_hour.b, brightness);
    set_pixel(pixel1_idx, col_hour.r, col_hour.g, col_hour.b, brightness);
    for (int i=0; i<dt.minute() / 2; i++) {
      set_pixel(pgm_read_byte(minute_idx+i), col_minute.r, col_minute.g, col_minute.b, brightness);
    }
  if (dt.second() % 2 == 0) {
    set_pixel(4*8+3, col_hour.r, col_hour.g, col_hour.b, brightness);
    set_pixel(4*8+4, col_hour.r, col_hour.g, col_hour.b, brightness);
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
    // quit
    if ((last_but_a_val == HIGH) && (but_a_val == LOW)) {
      //finished = true;
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
    if (player_y == 0) {
      finished = true;
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

void loop () 
{
    DateTime now = rtc.now(); //get the current date-time
    uint32_t ts = now.getEpoch();
    byte pix_trans;
    if (ts != old_ts) {
       start_approx_millis_time = millis();
    }
    approx_millis = millis() - start_approx_millis_time;

    last_but_a_val = but_a_val;
    but_a_val = digitalRead(BUT_A_PIN);

    ldr_value = analogRead(LDR_PIN);
    if (brightness < max(1023 - ldr_value, 10)) {
      brightness++;
    } else {
      brightness--;
    }

      // button down = entering time set mode
      if (but_a_val == LOW) {
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
        case PGM_MODE_TM:
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
            pgm_mode = PGM_MODE_GM;
            rot_enc.write(0);
            enc_pos = new_enc_pos;
            last_clock_mode = clock_mode;
            display_bitmap(BITMAP_GM, 0, 0, 4);
            delay(PGM_MODE_SWITCH_DELAY);
          }
          break;
        case PGM_MODE_GM:
          // encoder readout
          if (new_enc_pos != enc_pos) {
            game();
            wait_button_up();
            rot_enc.write(0);
            enc_pos = 0;
            new_enc_pos = 0;
          }
          // button a readout
          if ((last_but_a_val == LOW) && (but_a_val == HIGH)) {
            pgm_mode = PGM_MODE_CO;
            rot_enc.write(0);
            enc_pos = new_enc_pos;
            pos_enc_pos = 0;
            neg_enc_pos = 0;
            display_color_bitmap(BITMAP_CLR_CO);
            delay(PGM_MODE_SWITCH_DELAY);
          }
          break;
        case PGM_MODE_CO:
          // encoder readout
          if (new_enc_pos < 0) {
            set_colors_cont_b(-new_enc_pos);
            if (new_enc_pos > neg_enc_pos) {
              // we're rotating back
              rot_enc.write(pos_enc_pos);
              new_enc_pos = pos_enc_pos;
            } else {
              neg_enc_pos = new_enc_pos;
            }
          }
          if (new_enc_pos > 0) {
            set_colors_cont_a(new_enc_pos);
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
            pgm_mode = PGM_MODE_LO;
            rot_enc.write(clock_mode*4);
            enc_pos = new_enc_pos;
            display_bitmap(BITMAP_LO, 0, 0, 4);
            delay(PGM_MODE_SWITCH_DELAY);
          }
          break;
      }
    
//    enc_pos = new_enc_pos;
    
//
//    if (old_ts == 0 || old_ts != ts) {
//    	old_ts = ts;
//    }

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
        case MODE_SET_CLOCK:
          // edit clock  
          draw_edit_clock(now, hour_inc, minute_inc);    
          break;
        }
    }

    pixels.show();

    old_ts = ts;
    fast_counter++;
}
