/* game of life specific */


/***************************************************************************/

void set_gol(int buf, int x, int y, byte r, byte g, byte b) {
  game_of_life[buf][(x+y*8)*3] = r; 
  game_of_life[buf][(x+y*8)*3+1] = g; 
  game_of_life[buf][(x+y*8)*3+2] = b; 
}

void init_game_of_life() {
  for (int i=0; i<3*64; i++) {
    game_of_life[0][i] = 0;
    game_of_life[1][i] = 0;
  }
  set_gol(0, 0, 0, 40, 0, 0);
  set_gol(0, 0, 1, 40, 0, 0);
  set_gol(0, 0, 2, 40, 0, 0);
  set_gol(0, 1, 2, 40, 0, 0);
  set_gol(0, 2, 1, 40, 0, 0);
}

// return if cell is alive
boolean gol_alive(int buf, int x, int y) {
  if (game_of_life[buf][(y*8+x)*3+0] > 0) {
    return true; 
  }
  if (game_of_life[buf][(y*8+x)*3+1] > 0) {
    return true; 
  }
  if (game_of_life[buf][(y*8+x)*3+2] > 0) {
    return true; 
  }
  return false;
}

// return number of neighbors
byte gol_neighbors(int buf, int x, int y) {
  int result = 0;
  if (gol_alive(buf, (x+1) % 8, y)) {
    result += 1;
  }
  if (gol_alive(buf, (x+1) % 8, (y+1) % 8)) {
    result += 1;
  }
  if (gol_alive(buf, (x) % 8, (y+1) % 8)) {
    result += 1;
  }
  if (gol_alive(buf, (x+7) % 8, (y+1) % 8)) {
    result += 1;
  }
  if (gol_alive(buf, (x+7) % 8, (y) % 8)) {
    result += 1;
  }
  if (gol_alive(buf, (x+7) % 8, (y+7) % 8)) {
    result += 1;
  }
  if (gol_alive(buf, (x) % 8, (y+7) % 8)) {
    result += 1;
  }
  if (gol_alive(buf, (x+1) % 8, (y+7) % 8)) {
    result += 1;
  }
  return result;
}


/*
Any live cell with fewer than two live neighbours dies, as if caused by under-population.
Any live cell with two or three live neighbours lives on to the next generation.
Any live cell with more than three live neighbours dies, as if by over-population.
Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
*/
void step_game_of_life() {
  byte num_neighbors;
//  byte r, g, b;
  // clear buffer 1
  for (int i=0; i<3*64; i++) {
    game_of_life[1][i] = 0;
  }
  // calc buffer 1
  for (int y=0; y<8; y++) {
    for (int x=0; x<8; x++) {
      num_neighbors = gol_neighbors(0, x, y);
      if (gol_alive(0, x, y)) {
//        r = game_of_life[0][(y*8+x)*3+0];
//        g = game_of_life[0][(y*8+x)*3+1];
//        b = game_of_life[0][(y*8+x)*3+2];
        if (num_neighbors < 2) {
          // die
        } else if ((num_neighbors == 2) || (num_neighbors == 3)) {
          set_gol(1, x, y, col_a.r / 2, col_a.g / 2, col_a.b / 2);
        } else if (num_neighbors > 3) {
          // die
        }
      } else {
        if (num_neighbors == 3) {
          set_gol(1, x, y, col_b.r / 2, col_b.g / 2, col_b.b / 2);
        }
      }
    }
  }
  // copy buffer 1 -> 0
  for (int i=0; i<3*64; i++) {
    game_of_life[0][i] = game_of_life[1][i];
  }
}

// Standard Gerard Vichniac voting rule, also known as "Majority".
void step_game_of_life_majority() {
  byte num_neighbors;
//  byte r, g, b;
  // clear buffer 1
  for (int i=0; i<3*64; i++) {
    game_of_life[1][i] = 0;
  }
  // calc buffer 1
  for (int y=0; y<8; y++) {
    for (int x=0; x<8; x++) {
      num_neighbors = gol_neighbors(0, x, y);
      if (gol_alive(0, x, y)) {
        if (num_neighbors >= 4) {
          set_gol(1, x, y, col_a.r / 2, col_a.g / 2, col_a.b / 2);
        }
      } else {
        if (num_neighbors >= 5) {
          set_gol(1, x, y, col_b.r / 2, col_b.g / 2, col_b.b / 2);
        }
      }
    }
  }
  // copy buffer 1 -> 0
  for (int i=0; i<3*64; i++) {
    game_of_life[0][i] = game_of_life[1][i];
  }
}

void step_game_of_life_test() {
  byte num_neighbors;
//  byte r, g, b;
  // clear buffer 1
  for (int i=0; i<3*64; i++) {
    game_of_life[1][i] = 0;
  }
  // calc buffer 1
  for (int y=0; y<8; y++) {
    for (int x=0; x<8; x++) {
      num_neighbors = gol_neighbors(0, x, y);
      if (gol_alive(0, x, y)) {
        if (num_neighbors == 1) {
          //set_gol(1, x, y, col_a.r / 2, col_a.g / 2, col_a.b / 2);
        }
      } else {
        if (num_neighbors == 1) {
          set_gol(1, x, y, col_b.r / 2, col_b.g / 2, col_b.b / 2);
        }
      }
    }
  }
  // copy buffer 1 -> 0
  for (int i=0; i<3*64; i++) {
    game_of_life[0][i] = game_of_life[1][i];
  }
}

void draw_game_of_life(DateTime dt) {
  for (int i=0; i<64; i++) {
    set_pixel(i, game_of_life[0][i*3], game_of_life[0][i*3+1], game_of_life[0][i*3+2], brightness);  
  }
}

void draw_single_digit_gol(int xx, int yy, int digit, byte r, byte g, byte b) {
  byte digit_row;
  for (int y=0; y<5; y++) {
    digit_row = pgm_read_byte(font+y+digit*5);
    for (int x=0; x<2; x++) {
      if ((digit_row & 0b1) > 0) {
        set_gol(0, xx + 1 - x, (yy + y), r, g, b);
        //set_pixel(xx + 1 - x + (yy + y) * 8, r, g, b, brightness);
      }
      digit_row = digit_row >> 1;
    }
  }
}

void draw_normal_clock_gol(DateTime dt, RGB col_a, RGB col_b, int y0, int y1, int y2, int y3, int dot_location) {
  draw_single_digit_gol(0, y0, dt.hour() / 10, col_a.r, col_a.g, col_a.b);
  draw_single_digit_gol(2, y1, dt.hour() % 10, col_b.r, col_b.g, col_b.b);
  draw_single_digit_gol(4, y2, dt.minute() / 10, col_a.r, col_a.g, col_a.b);
  draw_single_digit_gol(6, y3, dt.minute() % 10, col_b.r, col_b.g, col_b.b);
//  if (dt.second() % 2 == 0) {
//    game_of_life[0set_pixel(dot_location, col_b.r,col_b.g,col_b.b, brightness);
//  }
}

