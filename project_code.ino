// libraries

#include <FastLED.h>
#include <LiquidCrystal.h>

// LED strip macros

#define LED_PIN     10

#define NUM_LEDS 8
#define MAX_SPACES 2

#define BRIGHTNESS  20

#define LED_TYPE    WS2812B

#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

#define PLAYER1_COLOR CRGB::Red
#define PLAYER2_COLOR CRGB::Blue
#define EMPTY_COLOR CHSV(0,0,0)

#define PLAYER1_PIN 1
#define PLAYER2_PIN 2

// LCD macros

const int rs = 12, en = 11, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
#define LCD_ROWS 4
#define LCD_COLUMNS 20
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// game variables

int player1pos = 0;
int player2pos = 0;

int player1hold = 0;
int player2hold = 0;

// miscellaneous

#define UPDATES_PER_SECOND 100

void setup() {

  delay( 3000 ); // power-up safety delay

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );

  FastLED.setBrightness(  BRIGHTNESS );

  lcd.begin(LCD_COLUMNS, LCD_ROWS);

  randomSeed(analogRead(0));

  pinMode(PLAYER1_PIN, INPUT);
  pinMode(PLAYER2_PIN, INPUT);

}

void loop() {
  for ( int i = 0; i < NUM_LEDS; ++i) {
    if (i % 4 == 0) {
      if (floor(i / 4) == player1pos) {
        leds[i] = PLAYER1_COLOR;
      } else {
        leds[i] = EMPTY_COLOR;
      }
    } else if (i % 4 == 3) {
      if (floor(i / 4) == player2pos) {
        leds[i] = PLAYER2_COLOR;
      } else {
        leds[i] = EMPTY_COLOR;
      }
    } else {
      leds[i] = EMPTY_COLOR;
    }
  }
  int player1button = digitalRead(PLAYER1_PIN);
  int player2button = digitalRead(PLAYER2_PIN);
  if (player1button == LOW) {
    if (player1hold) {

    } else {
      player1pos ++;
      player1pos %= MAX_SPACES;
      player1hold = true;
    }
  } else {
    player1hold = false;
  }
  if (player2button == LOW) {
    if (player2hold) {

    } else {
      player2pos ++;
      player2pos %= MAX_SPACES;
      player2hold = true;
    }
  } else {
    player2hold = false;
  }

  FastLED.show();

  FastLED.delay(1000 / UPDATES_PER_SECOND);

  lcd.setCursor(0, 0);
  lcd.print("> Chutes & Ladders <");
  lcd.setCursor(0, 1);
  lcd.print("Player 1 position: " + String(player1pos));
  lcd.setCursor(0, 2);
  lcd.print("Player 2 position: " + String(player2pos));
  lcd.setCursor(0, 3);
  lcd.print("####################");
}
