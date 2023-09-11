/* Created by Izdar Tohti */
#include <FastLED.h>
#include <IRremote.h>
#include "queue.h"
#define NUM_LEDS 300

/* IR remote commands */
#define IR_POWER_BUTTON 64
#define IR_BRIGHTNESS_UP 92
#define IR_BRIGHTNESS_DOWN 93
#define IR_RED_BUTTON 88
#define IR_GREEN_BUTTON 89
#define IR_BLUE_BUTTON 69 // hehe funny number
#define IR_WHITE_BUTTON 68
#define IR_SWITCH_MODES 65
#define IR_ORANGE_BUTTON 84
#define IR_MAGENTA_BUTTON 30
#define IR_INCREMENT_FUNCTION 72
#define IR_DECREMENT_FUNCTION 76
#define IR_SET_BPM_MODE 31
#define IR_SET_BPM 27

CRGB leds[NUM_LEDS];
const byte RECV_PIN = 11;


int main() {
  init();
  Serial.begin(9600);
  IrReceiver.begin(RECV_PIN);
  pinMode(13, OUTPUT);
  FastLED.addLeds<WS2812, 2, GRB>(leds, NUM_LEDS);

  char STATE = 0; // 0000 3 bit start bit 2 bit bpm set 1 bit idk 0 bit idk
  int deltaColor = 0;
  int amountChanged = 0;

  byte amountSkip = 1;
  byte amountOfLEDs = 10;

  byte limit = 0;

  uint16_t elapsed_seconds = 0;
  uint16_t previous_seconds = 0;
  byte seconds = 0;
  int bpm = 125;

  int position = 0;
  int velocity = 130;
  int acceleration = 1;
  CRGB setColor = CRGB(255, 255, 255);
  byte mode = 0;
  while (1) {
    if (IrReceiver.decode()){
      byte command = IrReceiver.decodedIRData.command;
      Serial.println(command);
      decode_command(command, &STATE, &setColor, &mode, &amountSkip, &amountOfLEDs, &bpm);
      IrReceiver.resume();
    }
    if (get_bit(STATE, 3)){
      switch(mode){
        case 0:
          FastLED.clearData();
          dots(amountSkip, setColor);
          break;
        case 1:
          FastLED.clearData();
          dots_with_bpm(bpm, &amountSkip, setColor, &limit);
          break;
        case 2:
          snake(amountOfLEDs, setColor);
          break;
        case 3:
          pulse(&position, &velocity, &acceleration, setColor);
          break;
        case 4:
          rainbow(50, 5);
          break;
        case 5:
          rainbow(50, 5);
          snake(amountOfLEDs, setColor);
          break;
        case 6:
          blends(&amountChanged, &deltaColor);
          break;
      }
    }
    else{
      turn_off_lights();
    }
  }
}

void decode_command(byte command, char *STATE, CRGB *setColor, byte *mode, byte *amountSkip, byte *amountOfLEDs, int *bpm){
  switch(command){
    case IR_POWER_BUTTON:
      toggle_bit(STATE, 3);
      break;
    case IR_BRIGHTNESS_UP:
      if ((FastLED.getBrightness() + 255/10) < 255){
      FastLED.setBrightness(FastLED.getBrightness() + 255/10);
      }
      else{
        FastLED.setBrightness(255);
      }
      break;
    case IR_BRIGHTNESS_DOWN:
      if ((FastLED.getBrightness() - 255/10) > 0){
      FastLED.setBrightness(FastLED.getBrightness() - 255/10);
      }
      else{
        FastLED.setBrightness(0);
      }
      break;
    case IR_RED_BUTTON:
      *setColor = CRGB::Red;
      break;
    case IR_GREEN_BUTTON:
      *setColor = CRGB::Green;
      break;
    case IR_BLUE_BUTTON:
      *setColor = CRGB::Blue;
      break;
    case IR_WHITE_BUTTON:
      *setColor = CRGB::White;
      break;
    case IR_ORANGE_BUTTON:
      *setColor = CRGB::OrangeRed;
      break;
    case IR_MAGENTA_BUTTON:
      *setColor = CRGB::Magenta;
      break;
    case IR_SWITCH_MODES:
      *mode += 1;
      *mode %= 6; // CHANGE THE MOD VALUE WHEN ADDING MORE MODES.
    case IR_INCREMENT_FUNCTION:
      if (*mode == 0){
        if (*amountSkip > 0 && *amountSkip < 26){
          *amountSkip += 1;
        }
      }
      else if (*mode == 1){
        if (*bpm < 240){
          *bpm += 10;
        }
      }
      else if (*mode == 2 || *mode == 5){
        if (*amountOfLEDs < 40){
          *amountOfLEDs += 10;
        }
      }
      break;
    case IR_DECREMENT_FUNCTION:
      if (*mode == 0){
        if (*amountSkip > 0 && *amountSkip < 26){
          *amountSkip -= 1;
        }
      }
      else if (*mode == 1){
        if (*bpm > 30){
          *bpm -= 10;
        }
      }
      else if (*mode == 2 || *mode == 5){
        if (!(*amountOfLEDs - 10 > 50)){
          *amountOfLEDs -= 10;
        }
      }
      break;
    case IR_SET_BPM_MODE:
      add_bit(STATE, 2);
      remove_bit(STATE, 3);
      break;
  }
}

byte get_bit(char STATE, byte index){
  char mask = 1 << index;
  return STATE & mask;
}

void toggle_bit(char *STATE, byte index){
  char mask = 1 << index;
  *STATE ^= mask;
}

void add_bit(char *STATE, byte index){
  char mask = 1 << index;
  *STATE |= mask;
}

void remove_bit(char *STATE, byte index){
  char mask = 1 << index;
  *STATE &= ~mask;
}

void turn_off_lights(){
  for (int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB::Black;
    if (IrReceiver.isIdle()){
      FastLED.show();
    }
  }
}

void dots_with_bpm(int bpm, byte *amountSkip, CRGB color, byte *limit){
  uint8_t beat = beatsin8(bpm, 200, 256);
  if (beat == 255 && *limit == 0){
    *amountSkip += 1;
    *amountSkip %= 20;
    *limit += 1;
  }

  if (beat == 200){
    *limit = 0;
  }
  dots(*amountSkip, color);
}

void blends(int *amountChanged, int *deltaColor){
  CRGB *colors = malloc(sizeof(CRGB) * 9);
  colors[0] = CRGB::Purple;
  colors[1] = CRGB::Aqua;
  colors[2] = CRGB(245, 122, 138);
  colors[3] = CRGB::Green;
  colors[4] = CRGB::Blue;
  colors[5] = CRGB::Red;
  colors[6] = CRGB::FloralWhite;
  colors[7] = CRGB::FairyLightNCC;
  colors[8] = CRGB::Brown;

  for (int i = 0; i < NUM_LEDS; i++){
    leds[i] = blend(leds[i], colors[*deltaColor], 1);
  }
  if (IrReceiver.isIdle()){
    FastLED.show();
  }

  if (*amountChanged == 255){
    *deltaColor += 1;
    *deltaColor %= 9;
    *amountChanged = 0;
  }

  *amountChanged += 1;
  free(colors);
}

// this is a simple oscillation.
void pulse(int *position, int *velocity, int *acceleration, CRGB color){
  *position += *velocity;
  if (*position > 0){
    fadeToBlackBy(leds, NUM_LEDS, 1);
    if (IrReceiver.isIdle()){
      FastLED.show();
    }
    *velocity -= *acceleration;
  }
  else{
    for (int i = 0; i < NUM_LEDS; i++){
      leds[i] = blend(leds[i], color, 1);
    }
    if (IrReceiver.isIdle()){
      FastLED.show();
    }
    *velocity += *acceleration;
  }
}

// this gives the led lights a snake like pattern.
void snake(int amountOfLEDs, CRGB color) {
  Queue *previousLEDs = create_queue(amountOfLEDs);
  for (int i = 0; i < NUM_LEDS; i++){
    leds[i] = color;
    if (IrReceiver.isIdle()){
      FastLED.show();
    }
    enqueue(previousLEDs, i);
    if (previousLEDs->count == previousLEDs->capacity){
      int index = dequeue(previousLEDs);
      leds[index] = CRGB(0, 0, 0);
      if (IrReceiver.isIdle()){
        FastLED.show();
      }
    }
  }
  // sets the trailing leds left over off.
  while(!(isEmpty(previousLEDs))){
    int index = dequeue(previousLEDs);
    leds[index] = CRGB(0, 0, 0);
    if (IrReceiver.isIdle()){
      FastLED.show();
    }
  }
  // when the function ends free the malloced variables and wait for while(1) loop to call on another function or the same one.
  free(previousLEDs->array);
  free(previousLEDs);
}

// will skip x amount of leds and light the next one.
void dots(int amountToSkip, CRGB color){
  for (int i = 0; i < NUM_LEDS; i += (amountToSkip+1)){
    if (i > NUM_LEDS){
      continue;
    }
    leds[i] = color;
  }
  if (IrReceiver.isIdle()){
      FastLED.show();
  }
}

// basic rainbow pattern ;)
void rainbow(int speed, int deltaHue){
  uint8_t startHue = beat8(speed, 255);

  fill_rainbow(leds, NUM_LEDS, startHue, deltaHue);
  if (IrReceiver.isIdle()){
    FastLED.show();
  }
}
