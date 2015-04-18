//Directions of travel
#define UP      0
#define DOWN    1
#define LEFT    2
#define RIGHT   3
//Miscellaneous
#define ESCAPE  4
#define NOINPUT 5

/*---- Prototypes ----*/
void initControl(void);
uint8_t getControl(void);
void controlDelayMs(uint16_t ms);
