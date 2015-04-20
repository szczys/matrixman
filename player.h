typedef struct PlayerTAG {
    uint8_t id;         //Index used to find stored values
    uint8_t x;          //Position on the game surface, 0 is left
    uint8_t y;          //Position on the game surface, 0 is top
    uint8_t tarX;       //Target X coord. for enemy
    uint8_t tarY;       //Target Y coord. for enemy
    int16_t speed;      //Countdown how freqeuntly to move
    uint8_t travelDir;  //Uses directional defines below
    uint8_t color;      //Uses color defines below
    uint8_t inPlay;     //On the hunt = TRUE, in reserve = FALSE
    uint8_t dotCount;   /*For player tracks level completion
                          For enemy decides when to go inPlay*/
    uint8_t dotLimit;   //How many dots before this enemy is inPlay
    uint8_t speedMode;  //Index used to look up player speed
} Player;
