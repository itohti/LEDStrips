/* Created by Izdar Tohti */
#include <FastLED.h>
#include <IRremote.h>
#include "queue.h"
#define NUM_LEDS 150

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

CRGB leds[NUM_LEDS];
const byte RECV_PIN = 11;


int main() {
  init();
  Serial.begin(9600);
  IrReceiver.begin(RECV_PIN);
  pinMode(13, OUTPUT);
  FastLED.addLeds<WS2812, 2, GRB>(leds, NUM_LEDS);

  char STATE = 0; // 0000 3 bit start bit 2 bit idk 1 bit idk 0 bit idk
  byte deltaColor = 0;
  byte amountChanged = 0;

  int position = 0;
  int velocity = 130;
  int acceleration = 1;
  CRGB setColor = CRGB(255, 255, 255);
  byte mode = 0;
  /* uint16_t sec = seconds16() super useful if you're going to be programming music.  */
  while (1) {
    if (IrReceiver.decode()){
      byte command = IrReceiver.decodedIRData.command;
      Serial.println(command);
      decode_command(command, &STATE, &setColor, &mode);
      IrReceiver.resume();
    }
    if (get_bit(STATE, 3)){
      switch(mode){
        case 0:
          FastLED.clearData();
          dots(1, setColor);
          break;
        case 1:
          snake(10, setColor);
          break;
        case 2:
          pulse(&position, &velocity, &acceleration, setColor);
          break;
        case 3:
          rainbow(50, 5);
          break;
        case 4:
          rainbow(50, 5);
          snake(10, setColor);
          break;
        case 5:
          blends(&amountChanged, &deltaColor);
          break;
      }
    }
    else{
      turn_off_lights();
    }
  }
}

void decode_command(byte command, char *STATE, CRGB *setColor, byte *mode){
  switch(command){
    case IR_POWER_BUTTON:
      toggle_start(STATE);
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
  }
}

byte get_bit(char STATE, byte index){
  char mask = 1 << index;
  return STATE & mask;
}

void toggle_start(char *STATE){
  char mask = 1 << 3;
  *STATE ^= mask;
}

void turn_off_lights(){
  for (int i = 0; i < 150; i++){
    leds[i] = CRGB::Black;
    if (IrReceiver.isIdle()){
      FastLED.show();
    }
  }
}

void blends(byte *amountChanged, byte *deltaColor){
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
    blend(leds[i], colors[*deltaColor], 1);
  }

  if (*amountChanged == 255){
    *deltaColor += 1;
    *deltaColor %= 9;
  }

  *amountChanged++;

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
    delay(10);
    if (previousLEDs->count == previousLEDs->capacity){
      int index = dequeue(previousLEDs);
      leds[index] = CRGB(0, 0, 0);
      if (IrReceiver.isIdle()){
        FastLED.show();
      }
      delay(10);
    }
  }
  // sets the trailing leds left over off.
  while(!(isEmpty(previousLEDs))){
    int index = dequeue(previousLEDs);
    leds[index] = CRGB(0, 0, 0);
    if (IrReceiver.isIdle()){
      FastLED.show();
    }
    delay(10);
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
