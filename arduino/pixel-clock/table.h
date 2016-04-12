/* other arrays than bitmaps and fonts */


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

// pixel indices for every minute.
static const unsigned char minute_idx[] PROGMEM = {
  3, 5, 7, 9, 11,
  13, 15, 17, 19, 21,
  23, 25, 27, 30,

  33, 35, 37, 39, 41,
  43, 45, 47, 49, 51,
  53, 55, 57, 60,
};

static const unsigned char minute_pix_idx[] PROGMEM = {
  4+0*8, 5+0*8, 6+0*8, 7+0*8, 7+1*8,  
  7+2*8, 7+3*8, 7+4*8, 7+5*8, 7+6*8, 
  7+7*8, 6+7*8, 5+7*8, 4+7*8,  
  
  3+7*8, 2+7*8, 1+7*8, 0+7*8, 0+6*8, 
  0+5*8, 0+4*8, 0+3*8, 0+2*8, 0+1*8, 
  0+0*8, 1+0*8, 2+0*8, 3+0*8, 3+0*8, 
};
