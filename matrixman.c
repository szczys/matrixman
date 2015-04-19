#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "board.h"
#include "dots.h"
#include "player.h"
#include "display.h"
#include "control.h"

uint32_t dotTracker[36];

#define TRUE 1
#define FALSE 0

//Player Variables
Player myGuy;
Player enemy1;
Player enemy2;
Player enemy3;
Player enemy4;

uint8_t enemyMode;      //SCATTER, CHASE, or FRIGHT
uint8_t gameRunning;    //TRUE unles game over: FALSE
uint16_t frightTimer;   //Counts down to end of FRIGHT mode
uint8_t lastBehavior;   //Saves mode before entering FRIGHT
uint16_t dotTimer;      //Countdown release enemies if dots not eaten

//enemyMode types
#define SCATTER 0
#define CHASE 1
#define FRIGHT 2

static const uint16_t behaviors[] = {
    //In milliseconds
    7000, 20000, 7000, 20000, 5000, 20000, 10, 0 //trailing zero is a hack
    //TODO: add more behaviors for advancing levels (seriously that's never going to happen)
};

//Enemy Data
const uint8_t playerColor[5] = { YELLOW, RED, PINK, CYAN, ORANGE };
const uint8_t startingX[5] = { 15, 15, 17, 17, 14 };
const uint8_t startingY[5] = { 26, 14, 16, 17, 17 };
    //Player doesn't have scatter so 0 index is retreat coordinates
const uint8_t scatterX[5] = { 15, 27, 4, 2, 29 };
const uint8_t scatterY[5] = { 14, 0, 0, 35, 35 };

//PowerPixel rows and columns
#define PP1COL    3
#define PP2COL    28
#define PP1ROW    6
#define PP2ROW    26

/*---- Prototypes ----*/
void enterHouse(Player *pawn);
void changeBehavior(uint8_t mode);
/*--------------------*/


//Function returns 1 if next move is not a collision with the board
uint8_t canMove(uint8_t nextX, uint8_t nextY) {
    if (board[nextY] & (1<<(31-nextX))) {
        return FALSE;
    }
    else return TRUE;
}

void gobbleCount(void) {
    myGuy.dotCount += 1;
    printf("myGuy.dotCount: %d\n",myGuy.dotCount);
    
    if (enemy1.inPlay == FALSE) {
        enemy1.dotCount += 1;
        printf("enemy1.dotCount: %d\n", enemy1.dotCount);
        return;
    }
    if (enemy2.inPlay == FALSE) {
        enemy2.dotCount += 1;
        printf("enemy2.dotCount: %d\n", enemy2.dotCount);
        return;
    }
    if (enemy3.inPlay == FALSE) {
        enemy3.dotCount += 1;
        printf("enemy3.dotCount: %d\n", enemy3.dotCount);
        return;
    }
    if (enemy4.inPlay == FALSE) {
        enemy4.dotCount += 1;
        printf("enemy4.dotCount: %d\n", enemy4.dotCount);
        return;
    }
}

uint8_t isPixel(uint8_t x, uint8_t y) {
    if (dotTracker[y] & 1<<(31-x)) { return TRUE; }
    return FALSE;
}

uint8_t isPowerPixel(uint8_t x, uint8_t y) {
    if ((x == PP1COL) || (x == PP2COL)) {
        if ((y == PP1ROW) || (y == PP2ROW)) {
            return TRUE;
        }
    }
    return FALSE;
}

void movePlayer(Player *pawn) {
    uint8_t testX = pawn->x;
    uint8_t testY = pawn->y;

    if ((pawn->color == GREEN) && (pawn->x == scatterX[0]) && (pawn->y == scatterY[0])) {
        printf("Back Home\n");
        //Gobbled enemy has made it home, put it in the house
        displayPixel(pawn->x, pawn->y, BLACK);
        enterHouse(pawn);
        displayPixel(pawn->x, pawn->y, pawn->color);
        displayLatch(); //redraws display (if necessary)
        return;
    }
    else {
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
    }

    //is next space unoccupied?
    if (canMove(testX, testY)) {
        //erase player at current spot (redraw dot if necessary)
        if (isPixel(pawn->x, pawn->y)) { 
            if (isPowerPixel(pawn->x, pawn->y)) { displayPixel(pawn->x, pawn->y, WHITE); }
            else { displayPixel(pawn->x, pawn->y, GREY); }
        }
        else { displayPixel(pawn->x, pawn->y, BLACK); }
        //Tunnel Tests
        if ((testY == 17) && (testX == 1)) { testX = 29; }
        if ((testY == 17) && (testX == 30)) { testX = 2; }
        //increment player position
        pawn->x = testX;
        pawn->y = testY;
        //redraw player at new spot
        displayPixel(pawn->x, pawn->y, pawn->color);
        //gobble the dot
        if ((pawn == &myGuy) && isPixel(pawn->x,pawn->y)) {
            dotTracker[pawn->y] &= ~(1<<(31-pawn->x));  //Remove dot from the board
            gobbleCount();  //Increment dotCounts
            if (isPowerPixel(pawn->x, pawn->y)) {
                //Switch to Fright mode
                changeBehavior(FRIGHT);
            }
        }
    }
    displayLatch(); //Redraw display (if necessary);
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

void playerRoute(Player *pawn, uint8_t nextDir) {
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

void routeChoice(Player *pawn) {
    //This function is only used for enemies. NEVER for the player
    //TODO: This function works but seems overly complex
    
    //Does the pawn have a choice of routes right now?
    uint8_t testX = pawn->x;
    uint8_t testY = pawn->y;

    //Test for four intersections where turning upward is forbidden
    if (((testX == 14) || (testX == 17)) 
        && ((testY == 14) || (testY == 26))
        && pawn->travelDir != DOWN) { printf("no turning up\n"); return; }

    //Set 3 distances then choose the shortest
    uint16_t route1, route2, route3;
    //Set arbitrarily high distance numbers
    route1 = 6000;
    route2 = 6000;
    route3 = 6000;
    
    /*TODO: Fix this dirty hack 
        This whole block is ineloquent
        Check for "GREEN" color is workaround for retreating in FRIGHT mode
    */
    if ((enemyMode == FRIGHT) && (pawn->color != GREEN)) {
        //Enemies choose route randomly in this mode
        uint8_t findingPath = TRUE;
        while(findingPath) {
            switch(rand()%4) {
                case UP:
                    if ((pawn->travelDir != DOWN) &&
                            canMove(testX,testY-1))
                    {
                        pawn->travelDir = UP;
                        findingPath = FALSE;
                    }
                    break;
                case DOWN:
                    if ((pawn->travelDir != UP) &&
                            canMove(testX,testY+1))
                    {
                        pawn->travelDir = DOWN;
                        findingPath = FALSE;
                    }
                    break;
                case LEFT:
                    if ((pawn->travelDir != RIGHT) &&
                            canMove(testX-1,testY))
                    {
                        pawn->travelDir = LEFT;
                        findingPath = FALSE;
                    }
                    break;
                case RIGHT:
                    if ((pawn->travelDir != LEFT) &&
                            canMove(testX+1,testY))
                    {
                        pawn->travelDir = RIGHT;
                        findingPath = FALSE;
                    }
                    break;
            }
        }
        return;
    }
    /*---end of dirty hack*/ 

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
                pawn->travelDir = UP;
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
                pawn->travelDir = UP;
            }
            else if ((route2 < route1) && (route2 < route3)) {
                pawn->travelDir = DOWN;
            }
            break;
    }
}

void setTargets(Player *player, Player *pawn1, Player *pawn2, Player *pawn3, Player *pawn4) {
    if (enemyMode != CHASE) { return; }

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
        pawn4->tarX = scatterX[pawn4->id];
        pawn4->tarY = scatterY[pawn4->id];
    }
}

void checkDots(Player *pawn) {
    if ((pawn->inPlay == FALSE) && (pawn->dotCount >= pawn->dotLimit)) {
        displayPixel(pawn->x, pawn->y, BLACK); //erase current locaiton
        pawn->x = 18;
        pawn->y = 14;
        displayPixel(pawn->x, pawn->y, pawn->color); //Draw new locaiton
        displayLatch(); //Redraw display (if necessary)
        pawn->inPlay = TRUE;
        pawn->travelDir = LEFT; //TODO: shouldn't need to reset direction here
    }
}

void setupPlayer(Player *pawn, uint8_t newId, uint8_t newDotLimit) {
    pawn->id = newId;
    pawn->x = startingX[pawn->id];
    pawn->y = startingY[pawn->id];
    pawn->speed = 10; //Currently unused
    pawn->travelDir = LEFT;
    pawn->color = playerColor[pawn->id];
    pawn->tarX = scatterX[pawn->id];
    pawn->tarY = scatterY[pawn->id];
    pawn->dotCount = 0;
    pawn->dotLimit = newDotLimit;
}

void reverseDir(Player *pawn) {
    switch(pawn->travelDir) {
        case UP:
            pawn->travelDir = DOWN;
            break;
        case DOWN:
            pawn->travelDir = UP;
            break;
        case LEFT:
            pawn->travelDir = RIGHT;
            break;
        case RIGHT:
            pawn->travelDir = LEFT;
            break;
    }
}

void setScatterTar(Player *pawn) {
    pawn->tarX = scatterX[pawn->id];
    pawn->tarY = scatterY[pawn->id];
}

void changeBehavior(uint8_t mode) {
    //Enemies should reverse current direction when modes change
    //Unless coming out of FRIGHT mode
    if (enemyMode != FRIGHT) {
        reverseDir(&enemy1);
        reverseDir(&enemy2);
        reverseDir(&enemy3);
        reverseDir(&enemy4);
    }
    else {
        //No longer frightened, revive natural color
        enemy1.color = playerColor[enemy1.id];
        enemy2.color = playerColor[enemy2.id];
        enemy3.color = playerColor[enemy3.id];
        enemy4.color = playerColor[enemy4.id];
    }
    
    switch(mode) {
        case SCATTER:
            //Change Targets
            setScatterTar(&enemy1);
            setScatterTar(&enemy2);
            setScatterTar(&enemy3);
            setScatterTar(&enemy4);
            enemyMode = SCATTER;
            break;
        case CHASE:
            enemyMode = CHASE;
            break;
        case FRIGHT:
            //TODO: Fright timer should change as levels increase
            frightTimer = 6000;
            lastBehavior = enemyMode;
            enemyMode = FRIGHT;
            //Fix colors
            enemy1.color = LAVENDAR;
            enemy2.color = LAVENDAR;
            enemy3.color = LAVENDAR;
            enemy4.color = LAVENDAR;
            break;
    }
}

uint8_t wasEaten(Player *player, Player *pawn) {
    if ((player->x == pawn->x) && (player->y == pawn->y)) { return TRUE; }
    return FALSE;
}

void performRetreat(Player *pawn) {
    /*TODO: Each player should have enemyMode setting
        and it should be checked here */
    pawn->color = GREEN;
    pawn->tarX = scatterX[0];
    pawn->tarY = scatterY[0];
}

void enterHouse(Player *pawn) {
    /*TODO: Each player should have enemyMode setting
        and it should be changed here */
    pawn->color = playerColor[pawn->id];
    pawn->x = scatterX[0];
    pawn->y = scatterY[0]+2;
    pawn->tarX = scatterX[pawn->id];
    pawn->tarY = scatterY[pawn->id];
    pawn->inPlay = FALSE;
}

void checkEaten(void) {
    if (enemyMode != FRIGHT) {
        if (wasEaten(&myGuy, &enemy1) ||
            wasEaten(&myGuy, &enemy2) ||
            wasEaten(&myGuy, &enemy3) ||
            wasEaten(&myGuy, &enemy4))
        {
            //Game over
            printf("Game Over\nScore: %d\n\n", myGuy.dotCount * 10);
            gameRunning = FALSE;
        }
    }
    else {
        //Enemies should change color and go home when eaten
        if (wasEaten(&myGuy, &enemy1)) { performRetreat(&enemy1); }
        if (wasEaten(&myGuy, &enemy2)) { performRetreat(&enemy2); }
        if (wasEaten(&myGuy, &enemy3)) { performRetreat(&enemy3); }
        if (wasEaten(&myGuy, &enemy4)) { performRetreat(&enemy4); }
        
    }
}

void flashEnemy(Player *pawn, uint8_t color) {
    if ((pawn->color == WHITE) || (pawn->color == LAVENDAR)) {
        pawn->color = color;
        displayPixel(pawn->x, pawn->y, pawn->color);
        displayLatch(); //redraws display (if necessary)
    }
}

int main(int argn, char **argv)
{
    //TODO: Level change: Update dot counters by level
    //TODO: Level change: Player and enemy speed changes
    //TODO: Speed change for enemies in tunnel
    //TODO: PowerPixel blink
    //TODO: Implement timed enemy release: 4 seconds; 3 seconds lvl5+
    //TODO: Implement extra lives
    //TODO: Implement global dot counter (when life lost; 7/17/32 dots)
    //TODO: bonus food
    /*TODO: When enemy in fright mode is eaten and makes it back to house:
        1) It should emerge as a danger to player (can be re-eaten)
        2) Does it go back into SCATTER/CHASE mode?
    */

    printf("Hello world!\n");

    //set initial values for player and enemies
    setupPlayer(&myGuy,0,0);
    setupPlayer(&enemy1,1,0);
    enemy1.inPlay = TRUE; //Enemy1 always starts inPlay
    setupPlayer(&enemy2,2,0);
    setupPlayer(&enemy3,3,30);
    setupPlayer(&enemy4,4,60);
    enemyMode = SCATTER;

    initDisplay();

    //Draw the board
    for (uint16_t i = 2; i < 34; i++) {
        for (uint16_t j = 0; j<32; j++) {
            if (board[i] & (1<<(31-j))) {    //Invert the x (big endian)
                displayPixel(j, i, BLUE); 
            }
        }
    }

    //Get Dot-tracking array ready
    for (uint8_t i=0; i<36; i++) {
        dotTracker[i] = 0x00000000;
    }
    //Draw the dots
    for (uint16_t i = 2; i < 34; i++) {
        //Copy the dots to the dotTracker
        dotTracker[i] = dots[i];
        for (uint16_t j = 0; j<32; j++) {
            if (dots[i] & (1<<(31-j))) {    //Invert the x (big endian)
                displayPixel(j, i, GREY); 
            }
        }        
    }
    
    //Draw PowerPixels
    displayPixel(PP1COL, PP1ROW, WHITE);
    displayPixel(PP1COL, PP2ROW, WHITE);
    displayPixel(PP2COL, PP1ROW, WHITE);
    displayPixel(PP2COL, PP2ROW, WHITE);

    //Draw the player
    displayPixel(myGuy.x, myGuy.y, myGuy.color);
    displayLatch(); //Redraws display (if necessary)
    
    gameRunning = TRUE;
    uint16_t ticks = 0;
    uint16_t behaviorTicks = 0;
    uint8_t behaviorIndex = 0;
    uint8_t nextDir = RIGHT;
    dotTimer = 0;

    while (gameRunning)
    {

        //TODO: This is all input code which need to change when ported

        /*---- User Input ----*/
        uint8_t control = getControl();
        switch (control) {
            case NOINPUT:
                break;
            case ESCAPE:
                gameRunning = 0;
                continue;
            default:
                nextDir = control; 
        }



        /* This animates the game */

        //Switch Modes
        if (enemyMode == FRIGHT) {
            --frightTimer;
            /*-- This Block Flashes the enemies coming out of FRIGHT mode --*/
            if (frightTimer <= 1800) {
                if (frightTimer%200 == 0) {
                    uint8_t flashColor;
                    if ((frightTimer/200)%2) {
                        //1800 1400 1000 600 200
                        flashColor = WHITE;
                    }
                    else {
                        //1600 1200 800 4000 0
                        flashColor = LAVENDAR;
                    }
                    flashEnemy(&enemy1, flashColor);
                    flashEnemy(&enemy2, flashColor);
                    flashEnemy(&enemy3, flashColor);
                    flashEnemy(&enemy4, flashColor);
                }
            }
            //Leave fright mode when timer expires
            if (frightTimer == 0) { changeBehavior(lastBehavior); }
        }
        //Switch between SCATTER and CHASE depending on level paramaters
        //This is an else statement so that the timer doesn't run during FRIGHT mode
        else if (behaviorTicks++ > behaviors[behaviorIndex]) {
            if (behaviors[behaviorIndex] > 0) {
                //Checking for 0 lets us run final behavior forever
                behaviorIndex++;
                behaviorTicks = 0;

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

            checkEaten();   //Did an enemy enter the player's square?

            playerRoute(&myGuy, nextDir);        //see if player wanted direction change
            movePlayer(&myGuy); //move player

            checkEaten();   //Did the player enter an enemy's square?

            //TODO: Reset counter (this should be interrupts when in hardware
            ticks = 0;
        }

        //Enemy dot counters
        checkDots(&enemy1);
        checkDots(&enemy2);
        checkDots(&enemy3);
        checkDots(&enemy4);

        controlDelayMs(1);
        /* End of game animation */
   }

    displayClose();

    return 0;
}
