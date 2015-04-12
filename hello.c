#include <stdio.h>
#include <SDL2/SDL.h>
#include "board.h"

//SDL2 variables
void* nullptr;
SDL_Window *win;
SDL_Renderer *ren;

//Player Variables
struct Player { 
    uint8_t x;
    uint8_t y;
    uint8_t speed;
    uint8_t dirIsHor; // non-zero if moving in a horizontal direction
};

struct Player myGuy;


void initDisplay(void) {

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
    }

    win = SDL_CreateWindow("Hello World!", 100, 100, 10*32, 10*32, SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
    }

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
    }
}

void displayClear(char r, char g, char b) {
    SDL_SetRenderDrawColor(ren, r, g, b, 255);
    SDL_RenderClear(ren);
}

void displayPixel(uint8_t x, uint8_t y, char r, char g, char b) {
    SDL_Rect rect;
    //Eventually these rectangles will just be LED pixels so this is arbitrary
    rect.x = x*10;
    rect.y = y*10;
    rect.w = 10;
    rect.h = 10;

    SDL_SetRenderDrawColor(ren, r, g, b, 255);
    SDL_RenderFillRect(ren, &rect);
}

int main(int argn, char **argv)
{
    printf("Hello world!\n");

    myGuy.x = 15;
    myGuy.y = 23;
    myGuy.speed = 10; //Unused for now
    myGuy.dirIsHor = 1;
    
    initDisplay();
    uint16_t i;
    for (i = 0; i < 32; i++) {
        printf("Loop %d\n",i);
        for (uint16_t j = 0; j<32; j++) {
            if (board[i] & (1<<j)) {
                displayPixel(31-j, i, 0, 0, 255); //Invert the x (big endian)
            }
        }        
    }

    //Draw the player
    displayPixel(myGuy.x, myGuy.y, 255, 255, 0);

    //pause
    
    //move player

    //detect collision

    //redraw player

    SDL_RenderPresent(ren);
    SDL_Delay(10000);

    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
