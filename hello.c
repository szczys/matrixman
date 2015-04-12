#include <stdio.h>
#include <SDL2/SDL.h>

int main(int argn, char **argv)
{
    printf("Hello world!\n");

    void* nullptr = NULL;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, 960, 540, SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
    SDL_RenderClear(ren);
    SDL_Rect r;
    r.x = 50;
    r.y = 50;
    r.w = 50;
    r.h = 50;

    SDL_SetRenderDrawColor(ren, 0, 0, 255, 255);
    SDL_RenderFillRect(ren, &r);

    SDL_RenderPresent(ren);

    //SDL_RenderClear(ren);
    //SDL_RenderPresent(ren);
    
    SDL_Delay(2000);

    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
