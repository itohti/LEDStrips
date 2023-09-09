#include <FastLED.h>
#include "queue.h"

#define NUM_LEDS 150

CRGB leds[NUM_LEDS];

int main() {
  init();
  Serial.begin(9600);
  delay(150);
  FastLED.addLeds<WS2812, 2, GRB>(leds, NUM_LEDS);
  int position = 0;
  int velocity = 130;
  int acceleration = 1;
  for (int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB(255, 0, 255);
  }
  FastLED.show();
  while (1) {
    pulse(&position, &velocity, &acceleration);
  }
}

// this is a simple oscillation.
void pulse(int *position, int *velocity, int *acceleration){
  *position += *velocity;
  if (*position > 0){
    fadeToBlackBy(leds, NUM_LEDS, 1);
    FastLED.show();
    *velocity -= *acceleration;
  }
  else{
    for (int i = 0; i < NUM_LEDS; i++){
      leds[i] = blend(leds[i], CRGB(255, 0, 255), 1);
    }
    FastLED.show();
    *velocity += *acceleration;
  }
}

// this gives the led lights a snake like pattern.
void snake(int amountOfLEDs) {
  Queue *previousLEDs = create_queue(amountOfLEDs);
  for (int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB(255, 0, 255);
    FastLED.show();
    enqueue(previousLEDs, i);
    delay(10);
    if (previousLEDs->count == previousLEDs->capacity){
      int index = dequeue(previousLEDs);
      leds[index] = CRGB(0, 0, 0);
      FastLED.show();
      delay(10);
    }
  }
  // sets the trailing leds left over off.
  while(!(isEmpty(previousLEDs))){
    int index = dequeue(previousLEDs);
    leds[index] = CRGB(0, 0, 0);
    FastLED.show();
    delay(10);
  }
  // when the function ends free the malloced variables and wait for while(1) loop to call on another function or the same one.
  free(previousLEDs->array);
  free(previousLEDs);
}

// will skip x amount of leds and light the next one.
void dots(int amountToSkip){
  for (int i = 0; i < NUM_LEDS; i += (amountToSkip+1)){
    if (i > NUM_LEDS){
      continue;
    }
    leds[i] = CRGB(255, 0, 255);
  }
  FastLED.show();
}

// a very special mode ;)
void specialMode(){

}
