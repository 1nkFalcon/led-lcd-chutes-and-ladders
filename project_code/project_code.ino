// libraries

#include <FastLED.h>
#include <LiquidCrystal.h>

// timings (in seconds). prooooobably wont behave as expected if you set any of these to "zero", so i wouldn't recommend it.

#define MAIN_ANIMATION 0.4
#define DICE_ROLL_TIME 1
#define DICE_DISPLAY_TIME 1
#define CHUTE_LADDER_DISPLAY_TIME 1
#define END_DISPLAY_TIME 10

// LED strip macros

#define LED_PIN     10

// LEDS defines # of leds in strip
// start defines first visible led (keep in mind for left handed strips this counts from the RIGHT); starts at 0
#define STRIP_1_LEDS 21
#define STRIP_1_START 0
#define STRIP_2_LEDS 22
#define STRIP_2_START 0
#define STRIP_3_LEDS 22
#define STRIP_3_START 1
#define STRIP_4_LEDS 22
#define STRIP_4_START 0
#define STRIP_5_LEDS 23
#define STRIP_5_START 1

#define NUM_LEDS STRIP_1_LEDS+STRIP_2_LEDS+STRIP_3_LEDS+STRIP_4_LEDS+STRIP_5_LEDS

#define MAX_SPACES 25

#define BRIGHTNESS  20

#define LED_TYPE    WS2812B

#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

#define PLAYER_1_COLOR CRGB::Red
#define PLAYER_2_COLOR CRGB::Blue
#define EMPTY_COLOR CHSV(0,0,0)

// LCD macros

const int rs = 12, en = 11, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
#define LCD_ROWS 4
#define LCD_COLUMNS 20
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// player control pins

#define PLAYER_1_PIN 2
#define PLAYER_2_PIN 3

// game space definitions
// all ladders. space ON, then space to move TO. SUBTRACT 1 FROM SPACE ON BOARD (so if label on physical board says 12, put 11).
#define LADDER_COUNT 4
int LADDERS[][2] = {
  {2,7},
  {3,14},
  {12,20},
  {15,24}
};
#define CHUTE_COUNT 3
int CHUTES[][2] = {
  {10,1},
  {16,4},
  {21,17},
  {23,7}
};

// game variables

int player1pos = 0; // !!! STARTS AT 0
int player2pos = 0; // !!! STARTS AT 0

int player1hold = 0;
int player2hold = 0;

int dicedisplay = 0;

int gamestate = 0;
/*
 * 0 -> waiting
 * 1 -> dice roll
 * 2 -> dice display
 * 3 -> moving
 * 4 -> ladder
 * 5 -> chute
 * 6 -> game end!!!
 */

int currentplayer = 1;
int currentplayerpin;
int *currentplayerpos; // ooo pointers scary!

int tick = 0;
int animation = 0;

// miscellaneous

#define UPDATES_PER_SECOND 10

void setup() {

  delay( 3000 ); // power-up safety delay

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );

  FastLED.setBrightness(  BRIGHTNESS );

  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  Serial.begin(9600);

  randomSeed(analogRead(0));

  pinMode(PLAYER_1_PIN, INPUT);
  pinMode(PLAYER_2_PIN, INPUT);
}

int get_led_index(int pos, int playerposoffset) { // get the index for the led based on player position
  int stripoffset = 0;
  int ledoffset = 0;
  int strip = pos / 5;
  int stripledcounts[] = {STRIP_1_LEDS,STRIP_2_LEDS,STRIP_3_LEDS,STRIP_4_LEDS,STRIP_5_LEDS};
  int stripledoffsets[] = {STRIP_1_START,STRIP_2_START,STRIP_3_START,STRIP_4_START,STRIP_5_START};
  for (int i = 0; i < strip; ++i) {
    stripoffset += stripledcounts[i];
  }
  if (strip % 2 == 1) {
    playerposoffset = 3 - playerposoffset;
  }
  int stripled = (pos % 5) * 4 + playerposoffset;
  if (stripled >= (stripledcounts[strip]/2)) {
    stripled++;
  }
  ledoffset = stripledoffsets[strip] + stripled;
  return stripoffset + ledoffset;
}

void clear_lcd_row() {
  for (int i=0; i<LCD_COLUMNS; ++i) {
    lcd.print(" ");
  }
}

void game_loop() {
  // threading isnt possible, so this function is all of the stuff that updates EVERY TICK

  // updates the LED screen
  
  // math for determining which LED to have on for each player
  int player1led = get_led_index(player1pos,0);
  int player2led = get_led_index(player2pos,3);
  
  // updating each LED (probably a better way of doing this but this should be sufficient)
  for ( int i = 0; i < NUM_LEDS; ++i) {
    if (i == player1led) {
      leds[i] = PLAYER_1_COLOR;
    } else if (i == player2led) {
      leds[i] = PLAYER_2_COLOR;
    } else {
      leds[i] = EMPTY_COLOR;
    }
  }

  // screen
  
  lcd.setCursor(0, 0);
  if (animation % round(UPDATES_PER_SECOND * 2 * MAIN_ANIMATION) < round(UPDATES_PER_SECOND * MAIN_ANIMATION)) {
    lcd.print("< Chutes & Ladders >");
  } else {
    lcd.print("> Chutes & Ladders <");
  }
  lcd.setCursor(0, 1);
  switch (gamestate) {
    case 0: // waiting
      lcd.print("Player ");
      lcd.print(currentplayer);
      lcd.print("'s Turn!    ");
      lcd.setCursor(0, 2);
      lcd.print("(Press the button!)");
      break;
    case 1: // rolling
      lcd.print("Player ");
      lcd.print(currentplayer);
      lcd.print(" Rolling...");
      lcd.setCursor(random(LCD_COLUMNS), 2);
      lcd.print(String(dicedisplay));
      break;
    case 2: // display
      lcd.print("PLAYER ");
      lcd.print(currentplayer);
      lcd.print(" ROLLED A ");
      lcd.print(String(dicedisplay));
      lcd.print("!");
      lcd.setCursor(0, 2);
      clear_lcd_row();
      break;
    case 3:
      lcd.print("PLAYER ");
      lcd.print(currentplayer);
      lcd.print(" MOVING...  ");
      lcd.setCursor(0, 2);
      clear_lcd_row();
      break;
    case 4: // ladder
      lcd.print("PLAYER ");
      lcd.print(currentplayer);
      lcd.print(" LANDED ON A");
      lcd.setCursor(0, 2);
      lcd.print("LADDER! :)          ");
      break;
    case 5:// chute
      lcd.print("PLAYER ");
      lcd.print(currentplayer);
      lcd.print(" LANDED ON A");
      lcd.setCursor(0, 2);
      lcd.print("CHUTE! :(           ");
      break;
    case 6: // game end!!!
      lcd.print("PLAYER ");
      lcd.print(currentplayer);
      lcd.print(" HAS WON!!   ");
      lcd.setCursor(0, 2);
      clear_lcd_row();
      break;
  }
  lcd.setCursor(0, 3);
  lcd.print("P1 ");
  lcd.print(String(player1pos+1));
  if (player1pos+1 <= 9) {
    lcd.print(" ");
  }
  for (int i = 0; i < LCD_COLUMNS-10; i++) {
    lcd.print(" ");
  }
  if (player2pos <= 9) {
    lcd.print(" ");
  }
  lcd.print(String(player2pos+1));
  lcd.print(" P2");
  
  FastLED.show();

  FastLED.delay(1000 / UPDATES_PER_SECOND);

  tick++;
  animation++;
}

void loop() {
  if (currentplayer == 1) {
    currentplayerpos =  &player1pos;
    currentplayerpin = PLAYER_1_PIN;
  } else {
    currentplayerpos =  &player2pos;
    currentplayerpin = PLAYER_2_PIN;
  }
  // await input
  gamestate = 0;
  while (true) {
    if (digitalRead(currentplayerpin) == HIGH) {
      gamestate = 1;
      break;
    }
    game_loop();
  }
  tick = 0;
  // dice roll
  while (tick < UPDATES_PER_SECOND * DICE_ROLL_TIME) {
    dicedisplay = random(6)+1;
    game_loop();
  }
  gamestate = 2;
  tick = 0;
  // dice display
  while (tick < UPDATES_PER_SECOND * DICE_DISPLAY_TIME) {
    game_loop();
  }
  gamestate = 3;
  int movedirection = 1;
  tick = 0;
  // move player
  while (tick < UPDATES_PER_SECOND * dicedisplay) {
    if (((tick + 1) / UPDATES_PER_SECOND) > (tick / UPDATES_PER_SECOND)) {
      *currentplayerpos+=movedirection;
      if (*currentplayerpos >= MAX_SPACES-1) {
        movedirection = -1;
      }
    }
    game_loop();
  }
  // check ladders player
  int moveto = 0;
  for (int i = 0; i < LADDER_COUNT; i++) {
    if (LADDERS[i][0] == *currentplayerpos) {
      gamestate = 4;
      moveto = LADDERS[i][1];
      break;
    }
  }
  // check chutes player
  for (int i = 0; i < CHUTE_COUNT; i++) {
    if (CHUTES[i][0] == *currentplayerpos) {
      gamestate = 5;
      moveto = CHUTES[i][1];
      break;
    }
  }
  if (gamestate == 4 || gamestate == 5) {
    tick = 0;
    while (tick < UPDATES_PER_SECOND * CHUTE_LADDER_DISPLAY_TIME) {
      game_loop();
    }
    *currentplayerpos = moveto;
  }
  // check for win
  if (*currentplayerpos == MAX_SPACES-1) {
    gamestate = 6;
    tick = 0;
    while (tick < UPDATES_PER_SECOND * END_DISPLAY_TIME) {
      game_loop();
    }
    currentplayer = 1;
    player1pos = 0;
    player2pos = 0;
    return;
  }
  currentplayer = 3 - currentplayer;
}
