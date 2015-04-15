#include <stdio.h>
#include <SDL2/SDL.h>
#include "board.h"

//SDL2 variables
void* nullptr;
SDL_Window *win;
SDL_Renderer *ren;

#define TRUE 1
#define FALSE 0

//Player Variables
struct Player { 
    uint8_t x;          //Position on the game surface, 0 is left
    uint8_t y;          //Position on the game surface, 0 is top
    uint8_t tarX;       //Target X coord. for enemy
    uint8_t tarY;       //Target Y coord. for enemy
    int8_t speed;       //Currently unused
    uint8_t travelDir;  //Uses directional defines below
    uint8_t color;      //Uses color defines below
    uint8_t inPlay;
};

struct Player myGuy;
struct Player enemy1;
struct Player enemy2;
struct Player enemy3;
struct Player enemy4;

uint8_t enemyMode;

//enemyMode types
#define SCATTER 0
#define CHASE 1

static const uint16_t behaviors[] = {
    //In milliseconds
    7000, 20000, 7000, 20000, 5000, 20000, 10, 0 //trailing zero is a hack
};

//Directions of travel
#define UP      0
#define DOWN    1
#define LEFT    2
#define RIGHT   3

//Enemy Data
#define REDX 27
#define REDY 0
#define PINKX 4
#define PINKY 0
#define ORANGEX 2
#define ORANGEY 35
#define CYANX 29
#define CYANY 35

//Color definitions
#define BLUE 0
#define YELLOW 1
#define RED 2
#define PINK 3
#define ORANGE 4
#define CYAN 5
#define BLACK 6

//Color values
static const uint8_t colors[][3] = {
    { 0, 0, 255 },      //Blue
    { 255, 255, 0 },    //Yellow
    { 255, 0, 0 },      //Red
    { 255, 153, 204 },        //Pink
    { 255, 102, 0 },        //Orange
    { 0, 255, 255 },    //Cyan
    { 0, 0, 0 }         //Black
};

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
}

void displayClear(uint8_t color) {
    SDL_SetRenderDrawColor(ren, colors[color][0], colors[color][1], colors[color][2], 255);
    SDL_RenderClear(ren);
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

    switch (pawn->travelDir) {
        case UP:
            testY--;
            break;
        case DOWN:
            testY++;
            break;
        case LEFT:
            testX--;
            break;
        case RIGHT:
            testX++;
            break;
    }

    //is next space unoccupied?
    if (canMove(testX, testY)) {
        //erase player at current spot
        displayPixel(pawn->x, pawn->y, BLACK);
        //Tunnel Tests
        if ((testY == 17) && (testX == 1)) { testX = 29; }
        if ((testY == 17) && (testX == 30)) { testX = 2; }
        //increment player position
        pawn->x = testX;
        pawn->y = testY;
        //redraw player at new spot
        displayPixel(pawn->x, pawn->y, pawn->color);
        SDL_RenderPresent(ren);
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

void playerRoute(struct Player *pawn, uint8_t nextDir) {
    if (nextDir == pawn->travelDir) return;

    uint8_t testX = pawn->x;
    uint8_t testY = pawn->y;
    switch (nextDir) {
        case UP:
            testY--;
            break;
        case DOWN:
            testY++;
            break;
        case LEFT:
            testX--;
            break;
        case RIGHT:
            testX++;
            break;
    }
    
    if (canMove(testX, testY)) { pawn->travelDir = nextDir; }   
}

void routeChoice(struct Player *pawn) {
    //Does the pawn have a choice of routes right now?
    uint8_t testX = pawn->x;
    uint8_t testY = pawn->y;

    //Test for four intersections where turning upward is forbidden
    if (((testX == 14) || (testX == 17)) 
        && ((testY == 14) || (testY == 27))
        && pawn->travelDir != DOWN) { printf("no turning up\n"); return; }

    //Set 3 distances then choose the shortest
    uint16_t route1, route2, route3;
    //Set arbitrarily high distance numbers
    route1 = 6000;
    route2 = 6000;
    route3 = 6000;

    switch(pawn->travelDir) {
        case UP:
            if (canMove(testX-1, testY)) { route1 = getDistance(testX-1, testY, pawn->tarX, pawn->tarY); }
            if (canMove(testX+1, testY)) { route2 = getDistance(testX+1, testY, pawn->tarX, pawn->tarY); }
            if (canMove(testX, testY-1)) { route3 = getDistance(testX, testY-1, pawn->tarX, pawn->tarY); }
            if ((route1 < route2) && (route1 < route3)) {
                pawn->travelDir = LEFT;
            }
            else if ((route2 < route1) && (route2 < route3)) {
                pawn->travelDir = RIGHT;
            }
            break;
        case DOWN:
            if (canMove(testX-1, testY)) { route1 = getDistance(testX-1, testY, pawn->tarX, pawn->tarY); }
            if (canMove(testX+1, testY)) { route2 = getDistance(testX+1, testY, pawn->tarX, pawn->tarY); }
            if (canMove(testX, testY+1)) { route3 = getDistance(testX, testY+1, pawn->tarX, pawn->tarY); }
            if ((route1 < route2) && (route1 < route3)) {
                pawn->travelDir = LEFT;
            }
            else if ((route2 < route1) && (route2 < route3)) {
                pawn->travelDir = RIGHT;
            }
            break;
        case LEFT:
            if (canMove(testX, testY-1)) { route1 = getDistance(testX, testY-1, pawn->tarX, pawn->tarY); }
            if (canMove(testX, testY+1)) { route2 = getDistance(testX, testY+1, pawn->tarX, pawn->tarY); }
            if (canMove(testX-1, testY)) { route3 = getDistance(testX-1, testY, pawn->tarX, pawn->tarY); }
            if ((route1 < route2) && (route1 < route3)) {
                pawn->travelDir = UP;   //TODO: apparently moving up is forbidden in original AI logic
            }
            else if ((route2 < route1) && (route2 < route3)) {
                pawn->travelDir = DOWN;
            }
            break;
        case RIGHT:
            if (canMove(testX, testY-1)) { route1 = getDistance(testX, testY-1, pawn->tarX, pawn->tarY); }
            if (canMove(testX, testY+1)) { route2 = getDistance(testX, testY+1, pawn->tarX, pawn->tarY); }
            if (canMove(testX+1, testY)) { route3 = getDistance(testX+1, testY, pawn->tarX, pawn->tarY); }
            if ((route1 < route2) && (route1 < route3)) {
                pawn->travelDir = UP;   //TODO: apparently moving up is forbidden in original AI logic
            }
            else if ((route2 < route1) && (route2 < route3)) {
                pawn->travelDir = DOWN;
            }
            break;
    }
}

void setTargets(struct Player *player, struct Player *pawn1, struct Player *pawn2, struct Player *pawn3, struct Player *pawn4) {
    if (enemyMode == SCATTER) { return; }

    /*--------------- Enemy1 ------------------*/
    pawn1->tarX = player->x;
    pawn1->tarY = player->y;

    /*--------------- Enemy2 ------------------*/
    switch (player->travelDir) {
        case UP:
            if (player->y < 4) { pawn2->tarY = 0; }
            else { pawn2->tarY = player->y - 4; }
            //Account for original game overflow bug
            if (player->x < 4) { pawn2->tarX = 0; }
            else { pawn2->tarX = player->x - 4; }
            break;
        case DOWN:
            if (player->y > 31) { pawn2->tarY = 35; }
            else { pawn2->tarY = player->y + 4; }
            break;
        case LEFT:
            if (player->x < 4) { pawn2->tarX = 0; }
            else { pawn2->tarX = player->x - 4; }
            break;
        case RIGHT:
            if (player->x > 27) { pawn2->tarX = 31; }
            else { pawn2->tarX = player->x + 4; }
            break;
    }

    /*--------------- Enemy3 ------------------*/
    int8_t tempNum;

    //setX
    tempNum = player->x - (pawn1->x - player->x);
    if (tempNum < 0) { tempNum = 0; }    
    if (tempNum > 31) { tempNum = 31; }
    pawn3->tarX = (uint8_t)tempNum;

    //setY    
    tempNum = player->y - (pawn1->y - player->y);
    if (tempNum < 0) { tempNum = 0; }    
    if (tempNum > 35) { tempNum = 35; }
    pawn3->tarY = (uint8_t)tempNum;

    /*--------------- Enemy4 ------------------*/
    if (getDistance(pawn4->x, pawn4->y, player->x, player->y) > 64) {
        pawn4->tarX = player->x;
        pawn4->tarY = player->y;
    }
    else {
        pawn4->tarX = ORANGEX;
        pawn4->tarY = ORANGEY;
    }
}

void checkDots(void) {
    //TODO: Add dot counters for all enemies (enemy2 is always zero)
    if (enemy2.inPlay == FALSE) {
        displayPixel(enemy2.x, enemy2.y, BLACK); //erase current locaiton
        enemy2.x = 18;
        enemy2.y = 14;
        displayPixel(enemy2.x, enemy2.y, enemy2.color); //Draw new locaiton
        enemy2.inPlay = TRUE;
        enemy2.travelDir = LEFT; //TODO: shouldn't need to reset direction here
    }
}

void setupPlayers(void) {
    //Set Player values
    myGuy.x = 15;
    myGuy.y = 26;
    myGuy.speed = 10; //Currently unused
    myGuy.travelDir = LEFT;
    myGuy.color = YELLOW;
    myGuy.tarX = ORANGEX;
    myGuy.tarY = ORANGEY;
    
    //Set Enemy values
    enemy1.x = 15;
    enemy1.y = 14;
    enemy1.speed = 10; //Currently unused
    enemy1.travelDir = LEFT;
    enemy1.color = RED;
    enemy1.tarX = REDX;
    enemy1.tarY = REDY;
    enemy1.inPlay = TRUE;

    enemy2.x = 17;
    enemy2.y = 16;
    enemy2.speed = 10; //Currently unused
    enemy2.travelDir = LEFT;
    enemy2.color = PINK;
    enemy2.tarX = PINKX;
    enemy2.tarY = PINKY;
    enemy2.inPlay = FALSE;

    enemy3.x = 17;
    enemy3.y = 17;
    enemy3.speed = 10; //Currently unused
    enemy3.travelDir = LEFT;
    enemy3.color = CYAN;
    enemy3.tarX = CYANX;
    enemy3.tarY = CYANY;
    enemy3.inPlay = FALSE;

    enemy4.x = 14;
    enemy4.y = 17;
    enemy4.speed = 10; //Currently unused
    enemy4.travelDir = LEFT;
    enemy4.color = ORANGE;
    enemy4.tarX = ORANGEX;
    enemy4.tarY = ORANGEY;
    enemy4.inPlay = FALSE;

    enemyMode = SCATTER;
}

void changeBehavior(uint8_t mode) {
    switch(mode) {
        case SCATTER:
            //Change Targets
            enemy1.tarX = REDX;
            enemy1.tarY = REDY;
            enemy2.tarX = PINKX;
            enemy2.tarY = PINKY;
            enemy3.tarX = CYANX;
            enemy3.tarY = CYANY;
            enemy4.tarX = ORANGEX;
            enemy4.tarY = ORANGEY;
            enemyMode = SCATTER;
            break;
        case CHASE:
            enemyMode = CHASE;
            break;
    }
}

int main(int argn, char **argv)
{
    printf("Hello world!\n");

    setupPlayers(); //set initial values for player and enemies
    
    initDisplay();
    
    uint16_t i;
    for (i = 2; i < 34; i++) {
        printf("Loop %d\n",i);
        for (uint16_t j = 0; j<32; j++) {
            if (board[i] & (1<<(31-j))) {    //Invert the x (big endian)
                displayPixel(j, i, BLUE); 
            }
        }        
    }

    //Draw the player
    displayPixel(myGuy.x, myGuy.y, myGuy.color);
    SDL_RenderPresent(ren);
    
    SDL_Event event;
    uint8_t gameRunning = 1;
    uint16_t ticks = 0;
    uint16_t behaviorTicks = 0;
    uint8_t behaviorIndex = 0;

    uint8_t nextDir = RIGHT;

    while (gameRunning)
    {
        
        //TODO: This is all input code which need to change when ported
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                gameRunning = 0;
            }
            if (event.type == SDL_KEYDOWN) {
                SDL_Keycode keyPressed = event.key.keysym.sym;
      
                switch (keyPressed)
                {
                    case SDLK_ESCAPE:
                        gameRunning = 0;
                        break;
                    case SDLK_UP:
                        printf("Up Key Pressed\n");
                        nextDir = UP;
                        break;
                    case SDLK_DOWN:
                        printf("Down Key Pressed\n");
                        nextDir = DOWN;
                        break;
                    case SDLK_LEFT:
                        printf("Left Key Pressed\n");
                        nextDir = LEFT;
                        break;
                    case SDLK_RIGHT:
                        printf("Right Key Pressed\n");
                        nextDir = RIGHT;
                        break;
                }
            }
         
        }



        /* This animates the game */

        //Switch Modes
        if (behaviorTicks++ > behaviors[behaviorIndex]) {
            if (behaviors[behaviorIndex] > 0) {
                //Checking for 0 lets us run final behavior forever
                behaviorIndex++;
                behaviorTicks = 0;
                
                //TODO: We're about to behavior switch, all enemys should change direction
                
                if (behaviorIndex % 2) { changeBehavior(CHASE); }
                else { changeBehavior(SCATTER); }
            }
        }

        //Move the players        
        if (ticks++ > 150) {
            setTargets(&myGuy, &enemy1, &enemy2, &enemy3, &enemy4);
            routeChoice(&enemy1); //This is for enemy movement
            routeChoice(&enemy2);
            routeChoice(&enemy3);
            routeChoice(&enemy4);

            movePlayer(&enemy1);
            movePlayer(&enemy2);
            movePlayer(&enemy3);
            movePlayer(&enemy4);

            playerRoute(&myGuy, nextDir);        //see if player wanted direction change
            movePlayer(&myGuy); //move player
            

            //TODO: Reset counter (this should be interrupts when in hardware
            ticks = 0;
        }
        
        //Enemy dot counters
        checkDots();
        
        SDL_Delay(1);
        /* End of game animation */


   }

    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
