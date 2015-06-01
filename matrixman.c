#include "matrixman-gamefile.h"

#include <stdio.h>
#include <SDL2/SDL.h>

SDL_Event event;

//SDL2 variables
void* nullptr;
SDL_Window *win;
SDL_Renderer *ren;

void initDisplay(void) {

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
    }

    win = SDL_CreateWindow("Matrixman", 100, 100, 10*32, 10*32, SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
    }

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
    }
    
    displayClear(BLACK);
}

void displayClear(uint8_t color) {
    SDL_SetRenderDrawColor(ren, colors[color][0], colors[color][1], colors[color][2], 255);
    SDL_RenderClear(ren);
}

void displayGameOver(void) {
    //Display GameOver Message Here
    printf("Game Over\n");
}

void displayPixel(uint8_t x, uint8_t y, char color) {
    SDL_Rect rect;
    //TODO: Eventually these rectangles will just be LED pixels so this number 10 is arbitrary
    rect.x = x*10;
    rect.y = (y-2)*10;
    rect.w = 10;
    rect.h = 10;

    SDL_SetRenderDrawColor(ren, colors[color][0], colors[color][1], colors[color][2], 255);
    SDL_RenderFillRect(ren, &rect);
}

void displayClose(void) {
    SDL_DestroyWindow(win);
    SDL_Quit();
}

void displayLatch(void) {
    SDL_RenderPresent(ren);
}

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

//Everything is handled in the matrixman.c loop
int main(void) {
    playMatrixman();
}
