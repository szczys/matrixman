#include <stdio.h>
#include <SDL2/SDL.h>
#include "control.h"

SDL_Event event;

void initControl(void) {

}

uint8_t getControl(void) {
    if (SDL_PollEvent(&event)) {
        /*
        if (event.type == SDL_QUIT) {
            gameRunning = 0;
        }
        */
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
                case SDLK_SPACE:
                    return BUTTON;
            }
        }
    }
    return NOINPUT;
}

void controlDelayMs(uint16_t ms) {
    SDL_Delay(ms);
}
