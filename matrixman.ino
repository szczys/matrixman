#include "matrixman-gamefile.h"
#include <SmartMatrix_32x32.h>

SmartMatrix matrix;

/*------------------Display Specific Funcions----------------------------*/
void initDisplay(void) {
  matrix.begin();
  matrix.setBrightness(128);
}

void displayClear(uint8_t color) {
    matrix.fillScreen({colors[color][0],colors[color][1],colors[color][2]});
    displayLatch();
}

void displayGameOver(void) {
    matrix.setScrollOffsetFromEdge(13);
    matrix.setScrollMode(wrapForward);
    matrix.scrollText("Game Over", 1);
    while(matrix.getScrollStatus() > 0) { }
}

void displayPixel(uint8_t x, uint8_t y, char color) {
    matrix.drawPixel(x,y-2,{colors[color][0],colors[color][1],colors[color][2]});
}

void displayClose(void) {

}

void displayLatch(void) {
    matrix.swapBuffers();
}

/*---------------------Control Specific Funcitons ----------------------*/
#define JOYUP      24
#define JOYDOWN    25
#define JOYLEFT    26
#define JOYRIGHT   27
#define JOYBUT     28

void initControl(void) {
  pinMode(JOYUP, INPUT_PULLUP);
  pinMode(JOYDOWN, INPUT_PULLUP);
  pinMode(JOYLEFT, INPUT_PULLUP);
  pinMode(JOYRIGHT, INPUT_PULLUP);
  pinMode(JOYBUT, INPUT_PULLUP);
}

uint8_t getControl(void) {
  if (digitalRead(JOYUP) == LOW) { return UP; }
  if (digitalRead(JOYDOWN) == LOW) { return DOWN; }
  if (digitalRead(JOYLEFT) == LOW) { return LEFT; }
  if (digitalRead(JOYRIGHT) == LOW) { return RIGHT; }
  if (digitalRead(JOYBUT) == LOW) { return BUTTON; }
  /*--------------Need to implement this for teensy --------------------
    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            SDL_Keycode keyPressed = event.key.keysym.sym;

            switch (keyPressed)
            {
                case SDLK_ESCAPE:
                    return ESCAPE;
                case SDLK_UP:
                    return UP;
                case SDLK_DOWN:
                    return DOWN;
                case SDLK_LEFT:
                    return LEFT;
                case SDLK_RIGHT:
                    return RIGHT;
            }
        }
    }
  --------------------------------------------------------------------*/
    
    return NOINPUT;
    
}

void controlDelayMs(uint16_t ms) {
  delay(ms); 
}

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  //Everything is handled in the matrixman.c loop
  playMatrixman();
}
