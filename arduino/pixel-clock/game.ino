// unfinished frogger

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

