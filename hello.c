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
    int8_t speed;
    uint8_t dirIsHor; // non-zero if moving in a horizontal direction
};

struct Player myGuy;

//Enemy Data
#define REDX 27
#define REDY 0
#define PINKX 4
#define PINKY 0
#define ORANGEX 2
#define ORANGEY 31
#define BLUEX 29
#define BLUEY 31

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

//Function returns 1 if next move is not a collision with the board
uint8_t canMove(uint8_t nextX, uint8_t nextY) {
    if (board[nextY] & (1<<(31-nextX))) {
        return 0;
    }
    else return 1;
}

void movePlayer(struct Player *pawn) {
    uint8_t testX = pawn->x;
    uint8_t testY = pawn->y;

    if (pawn->dirIsHor) {
        if (pawn->speed > 0) { testX++; }
        else { testX--; }
    }
    else {
        if (pawn->speed > 0) { testY++; }
        else { testY--; }
    }

    //is next space unoccupied?
    if (canMove(testX, testY)) {
        //erase player at current spot
        displayPixel(pawn->x, pawn->y, 0, 0, 0);
        //increment player position
        pawn->x = testX;
        pawn->y = testY; 
        //redraw player at new spot
        displayPixel(pawn->x, pawn->y, 255, 255, 0);
        SDL_RenderPresent(ren);
    }
    //TODO: Remove this else statement (just for testing)
    else {
        if (pawn->speed < 0) pawn->speed = 10;
        else pawn->speed = -10;
    }
}

uint16_t getDistance(uint8_t x, uint8_t y, uint8_t targetX, uint8_t targetY) {
    //Takes point and a target point and returns squared distance between them

    uint8_t hor, vert;
    if (x < targetX) { hor = targetX-x; }
    else { hor = x-targetX; }
    if (y < targetY) { vert = targetY-y; }
    else { vert = y-targetY; }

    return (hor * hor) + (vert * vert);
}

void routeChoice(struct Player *pawn) {
    //Does the pawn have a choice of routes right now?
    uint8_t testX = pawn->x;
    uint8_t testY = pawn->y;

    //Set 3 distances then choose the shortest
    uint16_t route1, route2, route3;
    route1 = 6000;
    route2 = 6000;
    route3 = 6000;

    if (pawn->dirIsHor) {
        if (canMove(testX, testY-1)) { route1 = getDistance(testX, testY-1, REDX, REDY); }
        if (canMove(testX, testY+1)) { route2 = getDistance(testX, testY+1, REDX, REDY); }
        if (pawn->speed > 0) { 
            if (canMove(testX+1, testY)) { route3 = getDistance(testX+1, testY, REDX, REDY); }
        }
        else { 
            if (canMove(testX-1, testY)) { route3 = getDistance(testX-1, testY, REDX, REDY); }
        }

        if ((route1 < route2) && (route1 < route3)) {
            pawn->dirIsHor = 0;
            pawn->speed = -10;
        }
        else if ((route2 < route1) && (route2 < route3)) {
            pawn->dirIsHor = 0;
            pawn->speed = +10;
        }
    }
    else {
        if (canMove(testX-1, testY)) { route1 = getDistance(testX-1, testY, REDX, REDY); }
        if (canMove(testX+1, testY)) { route2 = getDistance(testX+1, testY, REDX, REDY); }
        if (pawn->speed > 0) {
            if (canMove(testX, testY+1)) { route3 = getDistance(testX, testY+1, REDX, REDY); }
        }
        else {
            if (canMove(testX, testY-1)) { route3 = getDistance(testX, testY-1, REDX, REDY); }
        }

        if ((route1 < route2) && (route1 < route3)) {
            pawn->dirIsHor = 1;
            pawn->speed = -10;
        }
        else if ((route2 < route1) && (route2 < route3)) {
            pawn->dirIsHor = 1;
            pawn->speed = +10;
        }
    }
}

int main(int argn, char **argv)
{
    printf("Hello world!\n");

    myGuy.x = 15;
    myGuy.y = 23;
    myGuy.speed = 10; //Currently only used for determining -/+ of movement
    myGuy.dirIsHor = 1;
    
    initDisplay();
    
    uint16_t i;
    for (i = 0; i < 32; i++) {
        printf("Loop %d\n",i);
        for (uint16_t j = 0; j<32; j++) {
            if (board[i] & (1<<(31-j))) {    //Invert the x (big endian)
                displayPixel(j, i, 0, 0, 255); 
            }
        }        
    }

    //Draw the player
    displayPixel(myGuy.x, myGuy.y, 255, 255, 0);
    SDL_RenderPresent(ren);
    

    

    while(1) {
        //TODO: Take input from human for dir changes
        SDL_Delay(250);
        //pause

        routeChoice(&myGuy);
        //move player
        movePlayer(&myGuy);
        
        
    }

    SDL_RenderPresent(ren);
    SDL_Delay(10000);

    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
