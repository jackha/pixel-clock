/* common function you can use */

void write_eeprom() {
  EEPROM.write(0, col_a.r);
  EEPROM.write(1, col_a.g);
  EEPROM.write(2, col_a.b);

  EEPROM.write(3, col_b.r);
  EEPROM.write(4, col_b.g);
  EEPROM.write(5, col_b.b);

  EEPROM.write(6, clock_mode);

  EEPROM.write(7, col_a.h);
  EEPROM.write(8, col_a.s);
  EEPROM.write(9, col_a.l);

  EEPROM.write(10, col_b.h);
  EEPROM.write(11, col_b.s);
  EEPROM.write(12, col_b.l);

  EEPROM.write(13, brightness_offset >> 8);
  EEPROM.write(14, brightness_offset && 255);
}

void read_eeprom() {
  col_a.r = EEPROM.read(0);
  col_a.g = EEPROM.read(1);
  col_a.b = EEPROM.read(2);

  col_b.r = EEPROM.read(3);
  col_b.g = EEPROM.read(4);
  col_b.b = EEPROM.read(5);

  clock_mode = EEPROM.read(6);  

  col_a.h = EEPROM.read(7);
  col_a.s = EEPROM.read(8);
  col_a.l = EEPROM.read(9);

  col_b.h = EEPROM.read(10);
  col_b.s = EEPROM.read(11);
  col_b.l = EEPROM.read(12);

  brightness_offset = EEPROM.read(13) << 8 + EEPROM.read(14);
  if ((brightness_offset < 0) || (brightness_offset > 400)) {
    brightness_offset = 100;  // probably first time
  }

  if (clock_mode > NUM_CLOCK_MODES) {
    // problably empty eeprom
    set_colors(0);
    clock_mode = MODE_NORMAL;
  }

  col_a.l = 26;
  col_b.l = 26;
}

void set_colors(int color_idx) {
  col_a.r = pgm_read_byte(colors+color_idx*6);
  col_a.g = pgm_read_byte(colors+color_idx*6+1);
  col_a.b = pgm_read_byte(colors+color_idx*6+2);

  col_b.r = pgm_read_byte(colors+color_idx*6+3);
  col_b.g = pgm_read_byte(colors+color_idx*6+4);
  col_b.b = pgm_read_byte(colors+color_idx*6+5);

  col_a.h = 0;
  col_a.s = 255;
  col_a.l = 26;

  col_b.h = 0;
  col_b.s = 255;
  col_b.l = 26;
}

void set_colors_cont_a2(long int enc) {
  col_a.h = enc % 256;
  col_a.s = 255 - 64 * ((enc / 256) % 4);
  long unsigned int col = hsl_to_rgb(float(col_a.h) / 255, float(col_a.s)/255, float(col_a.l)/255);
  col_a.r = col >> 16;
  col_a.g = (col >> 8) & 0xff;
  col_a.b = col & 0xff;
}
void set_colors_cont_b2(long int enc) {
  col_b.h = enc % 256;
  col_b.s = 255 - 64 * ((enc / 256) % 4);
  long unsigned int col = hsl_to_rgb(float(col_b.h) / 255, float(col_b.s)/255, float(col_b.l)/255);
  col_b.r = col >> 16;
  col_b.g = (col >> 8) & 0xff;
  col_b.b = col & 0xff;
  col_b.h = enc % 256;
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

// draw single digit on x, y location with 3x5 font
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

// draw single digit on x, y location with 3x5 font
// doubling the 4th row to make it 3x6 ^^
void draw_single_digit_3x6(int xx, int yy, int digit, byte r, byte g, byte b) {
  byte digit_row;
  for (int y=0; y<6; y++) {
    if (y < 2) {
      digit_row = pgm_read_byte(font3x5+y+digit*5);
    } else {
      digit_row = pgm_read_byte(font3x5+y-1+digit*5);    
    }
    for (int x=0; x<3; x++) {
      if ((digit_row & 0b1) > 0) {
        set_pixel(xx + 2 - x + (yy + y) * 8, r, g, b, brightness);
      }
      digit_row = digit_row >> 1;
    }
  }
}

// draw single digit on x, y location with 4x4 font
void draw_single_digit_4x4(int xx, int yy, int digit, byte r, byte g, byte b) {
  byte digit_row;
  for (int y=0; y<4; y++) {
    digit_row = pgm_read_byte(font4x4+y+digit*4);
    for (int x=0; x<4; x++) {
      if ((digit_row & 0b1) > 0) {
        set_pixel(xx + 3 - x + (yy + y) * 8, r, g, b, brightness);
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

// helper function to use on r, g and b
float temp_to_col(float temp_1, float temp_2, float temp_col)
{
  float result;
  if (6 * temp_col < 1) {
    result = (temp_2 + (temp_1 - temp_2) * 6 * temp_col);
  } else if (2 * temp_col < 1) {
    result = temp_1;
  } else if (3 * temp_col < 2) {
    result = temp_2 + (temp_1 - temp_2) * (0.666666 - temp_col) * 6;
  } else {
    result = temp_2;
  }
  return result;    

}

// convert hue / saturation / luminance to rgb. h/s/l is 0..1
// from: http://www.niwa.nu/2013/05/math-behind-colorspace-conversions-rgb-hsl/
// rgb is coded as bytes in the return value 
// r = (result >> 16), g = (result >> 8) & 0xff, b = result & 0xff
unsigned long hsl_to_rgb(float h, float s, float l) {
  float temp_1, temp_2, temp_r, temp_g, temp_b;
  float r, g, b;
  // 2
  if (l < 0.5) {
    temp_1 = l * (1.0 + s);
  } else {
    temp_1 = l + s - l * s;
  }
  //Serial.println(temp_1);
  // 3
  temp_2 = 2 * l - temp_1;
  //Serial.println(temp_2);
  // 5
  temp_r = h + 0.333333;
  if (temp_r > 1) {
    temp_r -= 1;
  }
  temp_g = h;  
  temp_b = h - 0.333333;
  if (temp_b < 0) {
    temp_b += 1;
  }
  //Serial.println(temp_r);
  //Serial.println(temp_g);
  //Serial.println(temp_b);

  // 6
  r = temp_to_col(temp_1, temp_2, temp_r);
  g = temp_to_col(temp_1, temp_2, temp_g);
  b = temp_to_col(temp_1, temp_2, temp_b);

  //Serial.println(r);
  //Serial.println(g);
  //Serial.println(b);

  return long(255*r) * 256 * 256 + long(255*g) * 256 + long(255*b);
}

/***************************************************************************/

void display_tetromino(int location_offset_x, int location_offset_y, int tetromino, int rotation, int brightness) {
  byte digit_row;
  byte xx, yy;
  byte r, g, b;
  r = pgm_read_byte(tetris_colors + 3 * tetromino + 0);
  g = pgm_read_byte(tetris_colors + 3 * tetromino + 1);
  b = pgm_read_byte(tetris_colors + 3 * tetromino + 2);
  for (int y=0; y<4; y++) {
    digit_row = pgm_read_byte(tetris_blocks+tetromino*16+rotation*4+y);
    for (int x=0; x<4; x++) {
      if ((digit_row & 0b1) > 0) {
        xx = 3 - x + location_offset_x;
        yy = y + location_offset_y;
        if ((xx >= 0) && (xx < 8) && (yy >= 0) && (yy < 8)) {
          //pixels.setPixelColor(xx + yy * 8, pixels.Color(r, g, b));
          set_pixel(xx + yy * 8, r, g, b, brightness);
        }
      }
      digit_row = digit_row >> 1;
    }
  }
}

void display_bitmap_(int location_offset_x, int location_offset_y, int bitmap_offset, byte r, byte g, byte b) {
  byte digit_row;
  byte xx, yy;
  for (int y=0; y<8; y++) {
    digit_row = pgm_read_byte(bitmaps+bitmap_offset+y);
    for (int x=0; x<8; x++) {
      if ((digit_row & 0b1) > 0) {
        xx = 7 - x + location_offset_x;
        yy = y + location_offset_y;
        if ((xx >= 0) && (xx < 8) && (yy >= 0) && (yy < 8)) {
          //pixels.setPixelColor(xx + yy * 8, pixels.Color(r, g, b));
          set_pixel(xx + yy * 8, r, g, b, brightness);
        }
      }
      digit_row = digit_row >> 1;
    }
  }
}

void display_bitmap(int bitmap_offset, byte r, byte g, byte b) {
  pixels.clear();
  display_bitmap_(0, 0, bitmap_offset, r, g, b);
  pixels.show();
}

/* standard brighness image */
void display_brightness() {
  pixels.clear();
  for (int y=0; y<8; y++) {
    for (int x=0; x<8; x++) {
      set_pixel(y*8+x, 2+x*5, 2+x*5, 2+x*5, brightness);
    }
  }
  pixels.show();
}

/* display color mode*/
void display_colors() {
  byte r,g,b;
  long unsigned int col;
  pixels.clear();  
  for (int y=0; y<8; y++) {
    for (int x=0; x<8; x++) {
      col = hsl_to_rgb(float(x) / 8, 1 - float(y) / 8, 0.1);
      r = col >> 16;
      g = (col >> 8) & 0xff;
      b = col & 0xff;
      set_pixel(y*8+x, r, g, b, brightness);  
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

