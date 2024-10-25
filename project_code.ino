// libraries

#include <FastLED.h>
#include <LiquidCrystal.h>

// LED strip macros

#define LED_PIN     10

#define NUM_LEDS 24

// LEDS defines # of leds in strip
// start defines first visible led (keep in mind for left handed strips this counts from the RIGHT); starts at 0
#define STRIP_1_LEDS 22
#define STRIP_1_START 0
#define STRIP_2_LEDS 22
#define STRIP_2_START 0
#define STRIP_3_LEDS 22
#define STRIP_3_START 0
#define STRIP_4_LEDS 22
#define STRIP_4_START 0
#define STRIP_5_LEDS 22
#define STRIP_5_START 0

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

// game macros

#define DICE_ROLL_TIME 3
#define DICE_DISPLAY_TIME 3

// game variables

int player1pos = 0; // !!! STARTS AT 0
int player2pos = 0; // !!! STARTS AT 0

int player1hold = 0;
int player2hold = 0;

int dicedisplay = 0;

int gamestate = 0;
/*
 * 0 -> waiting player 1
 * 1 -> dice player 1
 * 2 -> dice display 1
 * 3 -> moving player 1
 * 4 -> waiting player 2
 * 5 -> dice player 2
 * 6 -> dice display 2
 * 7 -> moving player 2
 * 8 -> game end!!!
 */

int tick = 0;

// miscellaneous

#define UPDATES_PER_SECOND 100

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
  ledoffset = stripledoffsets[strip] + (pos % 5) * 4 + playerposoffset;
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
  lcd.print("> Chutes & Ladders <");
  lcd.setCursor(0, 1);
  switch (gamestate) {
    case 0:
      lcd.print("Player 1's Turn!");
      lcd.setCursor(0, 2);
      lcd.print("(Press the button!)");
      break;
    case 1:
      lcd.print("Player 1 Rolling...");
      lcd.setCursor(random(LCD_COLUMNS), 2);
      lcd.print(String(dicedisplay));
      break;
    case 2:
      lcd.print("PLAYER 1 ROLLED A ");
      lcd.print(String(dicedisplay));
      lcd.print("!");
      lcd.setCursor(0, 2);
      clear_lcd_row();
      break;
    case 3:
      lcd.print("Player 2's Turn!");
      break;
    case 4:
      lcd.print("Player 2 Rolling...");
      break;
    case 5:
      lcd.print("Player 2 Moving...");
      break;
  }
  lcd.setCursor(0, 3);
  clear_lcd_row();
  lcd.setCursor(0, 3);
  lcd.print("P1 ");
  lcd.print(String(player1pos+1));
  if (player2pos > 8) {
    lcd.setCursor(LCD_COLUMNS-5,3);
  } else {
    lcd.setCursor(LCD_COLUMNS-4,3);
  }
  lcd.print(String(player2pos+1));
  lcd.print(" P2");
  
  FastLED.show();

  FastLED.delay(1000 / UPDATES_PER_SECOND);

  tick++;
}

void loop() {
  gamestate = 0;
  while (true) {
    if (digitalRead(PLAYER_1_PIN) == HIGH) {
      gamestate = 1;
      break;
    }
    game_loop();
  }
  tick = 0;
  while (tick < UPDATES_PER_SECOND * DICE_ROLL_TIME) {
    dicedisplay = random(6)+1;
    game_loop();
  }
  gamestate = 2;
  tick = 0;
  while (tick < UPDATES_PER_SECOND * DICE_DISPLAY_TIME) {
    game_loop();
  }
  gamestate = 3;
  tick = 0;
}
