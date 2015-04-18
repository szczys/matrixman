//Color definitions
#define BLUE    0
#define YELLOW  1
#define RED     2
#define PINK    3
#define ORANGE  4
#define CYAN    5
#define BLACK   6
#define GREY    7
#define WHITE   8
#define LAVENDAR  9
#define GREEN   10

//Color values
static const uint8_t colors[][3] = {
    { 0, 0, 255 },      //Blue
    { 255, 255, 0 },    //Yellow
    { 255, 0, 0 },      //Red
    { 255, 153, 204 },  //Pink
    { 255, 102, 0 },    //Orange
    { 0, 255, 255 },    //Cyan
    { 0, 0, 0 },        //Black
    { 64, 64, 64 },     //Grey
    { 255, 255, 255 },  //White
    { 196, 64, 255},    //Lavendar
    { 0, 255, 0}        //Green
};

/*---- Prototypes ----*/
void initDisplay(void);
void displayClear(uint8_t color);
void displayPixel(uint8_t x, uint8_t y, char color);
void displayClose(void);
void displayLatch(void);
/*--------------------*/
