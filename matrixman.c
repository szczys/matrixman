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
uint16_t level;         //Which level is currently running (zero index)
uint8_t nextDir;        //Stores the newest direction input from user
uint8_t powerPixelColor;    //Used to facilitate flashing of the powerPixels
uint8_t lives;          //Remaining extra lives
uint16_t behaviorTicks; //Timer for switch from scatter to chase
uint8_t behaviorIndex;  //Index that tracks timer values for mode changes

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
/* Speed is indexed as follows: [(level * 5) + <index>]
    <index>: Player, Enemy, PlayerFright, EnemyFright, EnemyTunnel */
const uint16_t speed[] = { 125, 133, 111, 200, 250 };

//Index values for the speed array
#define SPEEDPLAYER         0
#define SPEEDENEMY          1
#define SPEEDPLAYERFRIGHT   2
#define SPEEDENEMYFRIGHT    3
#define SPEEDENEMYTUNNEL    4

//PowerPixel rows and columns
#define PP1COL    3
#define PP2COL    28
#define PP1ROW    6
#define PP2ROW    26

/*---- Prototypes ----*/
void enterHouse(Player *pawn);
void changeBehavior(uint8_t mode);
void changeSpeed(Player *pawn, uint8_t index);
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
    dotTimer = 0;   //Reset timer

    if (enemy1.inPlay == FALSE) {
        enemy1.dotCount += 1;
        return;
    }
    if (enemy2.inPlay == FALSE) {
        enemy2.dotCount += 1;
        return;
    }
    if (enemy3.inPlay == FALSE) {
        enemy3.dotCount += 1;
        return;
    }
    if (enemy4.inPlay == FALSE) {
        enemy4.dotCount += 1;
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
        if (testY == 17) {
            if (testX == 1) { testX = 29; } //Warp left to right
            else if (testX == 30) { testX = 2; } //Warp right to left
            else if ((pawn->id) && (pawn->travelDir == LEFT)) {
                if (testX == 7) { pawn->speedMode = SPEEDENEMYTUNNEL; } //Slow down
                if (testX == 23) {
                    //Speed Up
                    if (enemyMode != FRIGHT) { pawn->speedMode = SPEEDENEMY; }
                    else { pawn->speedMode = SPEEDENEMYFRIGHT; }
                }
            }
            else if ((pawn->id) && (pawn->travelDir == RIGHT)) {
                if (testX == 24) {pawn->speedMode = SPEEDENEMYTUNNEL; } //Slow down
                if (testX == 8) {
                    //Speed Up
                    if (enemyMode != FRIGHT) { pawn->speedMode = SPEEDENEMY; }
                    else { pawn->speedMode = SPEEDENEMYFRIGHT; }
                }
            }
        }
        //increment player position
        pawn->x = testX;
        pawn->y = testY;
        //redraw player at new spot
        displayPixel(pawn->x, pawn->y, pawn->color);
        //gobble the dot
        if ((pawn == &myGuy) && isPixel(pawn->x,pawn->y)) {
            dotTracker[pawn->y] &= ~(1<<(31-pawn->x));  //Remove dot from the board
            gobbleCount();  //Increment dotCounts
            //There is a speed hit for each dot
            pawn->speed += 17;  // ~1/60th of a second
            if (isPowerPixel(pawn->x, pawn->y)) {
                //Additional speed hit for PowerPixels
                pawn->speed += 33;  // ~ 2/60th of a second
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
        && pawn->travelDir != DOWN) { return; }

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

void setTarget(uint8_t index) {
    if (enemyMode != CHASE) { return; }
    int8_t tempNum;
    switch (index) {
        case 1:
            /*--------------- Enemy1 ------------------*/
            enemy1.tarX = myGuy.x;
            enemy1.tarY = myGuy.y;
            break;

        case 2:
            /*--------------- Enemy2 ------------------*/
            switch (myGuy.travelDir) {
                case UP:
                    if (myGuy.y < 4) { enemy2.tarY = 0; }
                    else { enemy2.tarY = myGuy.y - 4; }
                    //Account for original game overflow bug
                    if (myGuy.x < 4) { enemy2.tarX = 0; }
                    else { enemy2.tarX = myGuy.x - 4; }
                    break;
                case DOWN:
                    if (myGuy.y > 31) { enemy2.tarY = 35; }
                    else { enemy2.tarY = myGuy.y + 4; }
                    break;
                case LEFT:
                    if (myGuy.x < 4) { enemy2.tarX = 0; }
                    else { myGuy.tarX = enemy2.x - 4; }
                    break;
                case RIGHT:
                    if (myGuy.x > 27) { enemy2.tarX = 31; }
                    else { enemy2.tarX = myGuy.x + 4; }
                    break;
            }
            break;

        case 3:
            /*--------------- Enemy3 ------------------*/

            //setX
            tempNum = myGuy.x - (enemy1.x - myGuy.x);
            if (tempNum < 0) { tempNum = 0; }
            if (tempNum > 31) { tempNum = 31; }
            enemy3.tarX = (uint8_t)tempNum;

            //setY
            tempNum = myGuy.y - (enemy1.y - myGuy.y);
            if (tempNum < 0) { tempNum = 0; }
            if (tempNum > 35) { tempNum = 35; }
            enemy3.tarY = (uint8_t)tempNum;
            break;

        case 4:
            /*--------------- Enemy4 ------------------*/
            if (getDistance(enemy4.x, enemy4.y, myGuy.x, myGuy.y) > 64) {
                enemy4.tarX = myGuy.x;
                enemy4.tarY = myGuy.y;
            }
            else {
                enemy4.tarX = scatterX[enemy4.id];
                enemy4.tarY = scatterY[enemy4.id];
            }
            break;
    }
}

void checkDots(Player *pawn, uint8_t force) {
    if ((force == TRUE) || ((pawn->inPlay == FALSE) && (pawn->dotCount >= pawn->dotLimit))) {
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
    if (newId) { changeSpeed(pawn, SPEEDENEMY); }
    else { changeSpeed(pawn, SPEEDPLAYER); }
    pawn->travelDir = LEFT;
    pawn->color = playerColor[pawn->id];
    pawn->tarX = scatterX[pawn->id];
    pawn->tarY = scatterY[pawn->id];
    pawn->dotCount = 0;
    pawn->dotLimit = newDotLimit;
    pawn->inPlay = FALSE;
}

void setupPlayerAfterDeath(Player *pawn) {
    pawn->x = startingX[pawn->id];
    pawn->y = startingY[pawn->id];
    if (pawn->id) { changeSpeed(pawn, SPEEDENEMY); }
    else { changeSpeed(pawn, SPEEDPLAYER); }
    pawn->travelDir = LEFT;
    pawn->color = playerColor[pawn->id];
    pawn->tarX = scatterX[pawn->id];
    pawn->tarY = scatterY[pawn->id];
    pawn->inPlay = FALSE;
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

void changeSpeed(Player *pawn, uint8_t index) {
    //multiplying by 5 offsets single-level array for diff levels
    pawn->speed = speed[(level * 5) + index];
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
        if (enemy1.color != GREEN) { enemy1.color = playerColor[enemy1.id]; }
        if (enemy2.color != GREEN) { enemy2.color = playerColor[enemy2.id]; }
        if (enemy3.color != GREEN) { enemy3.color = playerColor[enemy3.id]; }
        if (enemy4.color != GREEN) { enemy4.color = playerColor[enemy4.id]; }
    }

    switch(mode) {
        case SCATTER:
            //Change Speed
            myGuy.speedMode = SPEEDPLAYER;
            enemy1.speedMode = SPEEDENEMY;
            enemy2.speedMode = SPEEDENEMY;
            enemy3.speedMode = SPEEDENEMY;
            enemy4.speedMode = SPEEDENEMY;
            //Change Targets
            setScatterTar(&enemy1);
            setScatterTar(&enemy2);
            setScatterTar(&enemy3);
            setScatterTar(&enemy4);
            enemyMode = SCATTER;
            break;
        case CHASE:
            //Change Speed
            myGuy.speedMode = SPEEDPLAYER;
            enemy1.speedMode = SPEEDENEMY;
            enemy2.speedMode = SPEEDENEMY;
            enemy3.speedMode = SPEEDENEMY;
            enemy4.speedMode = SPEEDENEMY;
            enemyMode = CHASE;
            break;
        case FRIGHT:
            //TODO: Fright timer should change as levels increase
            frightTimer = 6000;
            if (enemyMode != FRIGHT) { lastBehavior = enemyMode; }
            enemyMode = FRIGHT;
            //Change speeds
            myGuy.speedMode = SPEEDPLAYERFRIGHT;
            enemy1.speedMode = SPEEDENEMYFRIGHT;
            enemy2.speedMode = SPEEDENEMYFRIGHT;
            enemy3.speedMode = SPEEDENEMYFRIGHT;
            enemy4.speedMode = SPEEDENEMYFRIGHT;
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

void expiredDotTimer(void) {
    //Too long since a dot was eaten, release an enemy if you canMove
    dotTimer = 0;   //Reset timer
    printf("dotTimer expired.\n");

    if (enemy1.inPlay == FALSE) { checkDots(&enemy1, TRUE); return; }
    if (enemy2.inPlay == FALSE) { checkDots(&enemy2, TRUE); return; }
    if (enemy3.inPlay == FALSE) { checkDots(&enemy3, TRUE); return; }
    if (enemy4.inPlay == FALSE) { checkDots(&enemy4, TRUE); return; }
}

void enemyTick(Player *pawn) {
    if (--pawn->speed == 0) {
        setTarget(pawn->id);
        routeChoice(pawn);
        movePlayer(pawn);
        checkEaten();
        changeSpeed(pawn, pawn->speedMode);
    }
}

void playerTick(Player * pawn) {
    if (--pawn->speed == 0) {
        playerRoute(&myGuy, nextDir);
        /*NOTE: changeSpeed must be called before
            movePlayer because the dot eaten timer
            penalty is added to the speed counter */
        changeSpeed(pawn, pawn->speedMode);
        movePlayer(&myGuy);
        checkEaten();
    }
}

void setupLevel(void) {
    //Call this to redraw level
    //All current setting for dots and players are used

    //Draw the board
    for (uint16_t i = 2; i < 34; i++) {
        for (uint16_t j = 0; j<32; j++) {
            if (board[i] & (1<<(31-j))) {    //Invert the x (big endian)
                displayPixel(j, i, BLUE); 
            }
        }
    }

    //Draw the dots
    for (uint16_t i = 2; i < 34; i++) {
        for (uint16_t j = 0; j<32; j++) {
            if (dotTracker[i] & (1<<(31-j))) {    //Invert the x (big endian)
                if (isPowerPixel(j,i)) { displayPixel(j, i, WHITE); }
                else { displayPixel(j, i, GREY); }
            }
        }
    }

    //Draw the player
    displayPixel(myGuy.x, myGuy.y, myGuy.color);

    drawLives();

    displayLatch(); //Redraws display (if necessary)
}

void setupDefaults(void) {
    //TODO: may want to make level a parameter
    level = 0;
    lives = 2;

    //set initial values for player and enemies
    setupPlayer(&myGuy,0,0);
    setupPlayer(&enemy1,1,0);
    enemy1.inPlay = TRUE; //Enemy1 always starts inPlay
    setupPlayer(&enemy2,2,0);
    setupPlayer(&enemy3,3,30);
    setupPlayer(&enemy4,4,60);
    enemyMode = SCATTER;
}

void deathRestart(void) {
    //Reset behavioral timer and index
    behaviorIndex = 0;
    behaviorTicks = 0;

    //set initial values for player and enemies
    setupPlayerAfterDeath(&myGuy);
    setupPlayerAfterDeath(&enemy1);
    enemy1.inPlay = TRUE; //Enemy1 always starts inPlay
    setupPlayerAfterDeath(&enemy2);
    setupPlayerAfterDeath(&enemy3);
    setupPlayerAfterDeath(&enemy4);
    enemyMode = SCATTER;
}

void refreshDotTracker(void) {
    //Get Dot-tracking array ready
    for (uint8_t i=0; i<36; i++) {
        dotTracker[i] = 0x00000000;
    }
    //Copy dots from the static array
    for (uint16_t i = 2; i < 34; i++) { dotTracker[i] = dots[i]; }
}

void drawLives(void) {
    for (int8_t i=0; i<lives; i++) {
        displayPixel(0, 33-(i*2), YELLOW);
    }
}

int main(int argn, char **argv)
{
    //TODO: Level change: Update dot counters by level
    /*TODO: Level change: Player and enemy speed changes
        100% = 10/second = 100ms ticks
        17ms speed penalty per dot gobbled
        50ms speed penalty for power pellet
        Level1: player  80%, 90% (fright)   125ms, 111ms
        Level1: enemy   75%, 50% (fright)   133ms, 200ms
    */
    //TODO: PowerPixel blink
    //TODO: Implement extra lives
    //TODO: Implement global dot counter (when life lost; 7/17/32 dots)
    //TODO: bonus food
    /*TODO: When enemy in fright mode is eaten and makes it back to house:
        1) It should emerge as a danger to player (can be re-eaten)
        2) Does it go back into SCATTER/CHASE mode?
        3) Retreating should be at highest speed
    */
    //TODO: Incremental score for eating enemies
    //TODO: BUG enemies in retreat hunt again without going home if free when mode changes

    setupDefaults();
    initDisplay();
    refreshDotTracker();
    setupLevel();       //show everything on the display

    uint8_t programRunning = TRUE;
    gameRunning = TRUE;
    behaviorTicks = 0;
    behaviorIndex = 0;
    nextDir = RIGHT;
    dotTimer = 0;

    while (programRunning)
    {

        //TODO: This is all input code which need to change when ported

        /*---- User Input ----*/
        uint8_t control = getControl();
        switch (control) {
            case NOINPUT:
                break;
            case ESCAPE:
                gameRunning = FALSE;
                programRunning = FALSE;
                continue;
            case BUTTON:
                if (gameRunning = FALSE) {
                    //Restart the game
                }
                break;
            default:
                nextDir = control; 
        }

        if (gameRunning) {

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

            //decrement counter and check for trigger
            //set target or take user input
            //choose route
            //movePiece
            //checkEaten

            enemyTick(&enemy1);
            enemyTick(&enemy2);
            enemyTick(&enemy3);
            enemyTick(&enemy4);

            playerTick(&myGuy);

            //Enemy dot counters
            checkDots(&enemy1, FALSE);
            checkDots(&enemy2, FALSE);
            checkDots(&enemy3, FALSE);
            checkDots(&enemy4, FALSE);

            if (dotTimer++ >= 4000) {
                //TODO: this time limit should change at lvl5+
                expiredDotTimer();
            }

            controlDelayMs(1);
            /* End of game animation */
        }

        if ((lives > 0) && (gameRunning == FALSE)) {
            --lives;
            //TODO: Pause after a life is lost
            displayClear(BLACK);
            //TODO: These defaults shouldn't reset dot-counters, etc.
            deathRestart();
            setupLevel();
            gameRunning = TRUE;
        }
   }

    displayClose();

    return 0;
}
