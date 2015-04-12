#include <stdio.h>
#include <SDL2/SDL.h>

//SDL2 variables
void* nullptr;
SDL_Window *win;
SDL_Renderer *ren;

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

void displayPixel(char x, char y, char r, char g, char b) {
    SDL_Rect rect;
    //Eventually these rectangles will just be LED pixels so this is arbitrary
    rect.x = x;
    rect.y = y;
    rect.w = 10;
    rect.h = 10;

    SDL_SetRenderDrawColor(ren, r, g, b, 255);
    SDL_RenderFillRect(ren, &rect);
}

int main(int argn, char **argv)
{
    printf("Hello world!\n");

    initDisplay();
    uint16_t i;
    for (i = 0; i < 32; i++) {
        printf("Loop %d\n",i);
        displayClear(255, 0, 0);
        displayPixel(i*10, 0, 0, 0, 255);
        SDL_RenderPresent(ren);

        SDL_Delay(2000);
    }
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
