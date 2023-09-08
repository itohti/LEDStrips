#include <FastLED.h>
#include "queue.h"

/* PROTOTYPE FUNCTIONS */
void snake();

CRGB leds[150];

int main() {
  init();
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  FastLED.addLeds<WS2812, 2, GRB>(leds, 150);
  while (1) {
    digitalWrite(13, 0);
    snake(10);
    digitalWrite(13, 1); // arduino will blink when function finishes.
    delay(100);
  }
}

// this gives the led lights a snake like pattern.
void snake(int amountOfLEDs) {
  Queue *previousLEDs = create_queue(amountOfLEDs);
  for (int i = 0; i < 150; i++){
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
