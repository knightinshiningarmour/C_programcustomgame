#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <raymath.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define MAX_ENEMIES 3
#define MAX_SHOP_ITEMS 5
#define MAX_CURRENCY 2000
#define MAX_ARROWS 3

const int windwidth = 1200;
const int windheight = 800;
const int mapwidth = windwidth * 3;
const int mapheight = -windheight;

typedef enum{
    ITEM_NONE,
    KEY,
    ARMOR,
    SWORD
}ItemType;

struct GameAssets 
{ //store game assets
    Image images[50];
    Music music[10];
    Sound sound[20];
    Texture2D texture[50];
    float src_idlex[5];
    float src_idley[5];
    float src_runningx[7];
    float src_runningy[7];
    float src_runningwidth[7];
    float src_jumpingx[6];
    float src_jumpingy[6];
    float src_fallingx[4];
    float src_fallingy[4];
    float src_jumpingwidth[6];
    float src_jumpingheight[6];
    float src_attackingx[6];
    float src_attackingy[6];
    float src_attackingwidth[6];
    float src_shieldx[7];
    float src_shieldy[7];
    float src_shieldwidth[7];
    float src_dyingx[5];
    float src_dyingy[5];
    float src_dyingwidth[5];

    int frameCount;                // Total frames in this animation
    float frameSpeed; 
    int texturecount;
    int imagecount;
    int musiccount;
    int soundcount;
    int potionleftinshop[4];
    int potionprice[4];
    int shopkeycount;
    int shopkeyprice;

    int coincurrentframe;
    int coinindex;
    bool doorkeyinserted[4];
    int doorkeycircledrawn[4];
    int doorkeyinsertedcount;
};

struct Playerinfo 
{
    Rectangle rect;
    Rectangle rect2;
    Rectangle playerhitboxx;
    Rectangle playerhitboxyup;
    Rectangle playerhitboxydown;
    Color colour;
    Vector2 Position;
    Vector2 direction;
    float width;
    float height;
    float velocityY;
    float speed;
    bool isJumping;
    bool isfalling;
    bool isidling;
    bool isrunning;
    bool onplatform;
    bool onground;
    bool onshield;
    bool attack;
    bool dead;
    bool arrowshot;
    bool enemywithcoin[MAX_ENEMIES];
    bool enemywithkey[MAX_ENEMIES];
    bool deadenemyremoved;
    bool chestrec;
    bool hugeobs;
    bool portal; //portal door to switch map
    bool key;
    bool keyclaimed;
    int cheststate;
    int chestanimationframe;
    float chestanimationtimer;

    float playerdamage;
    int animationstate;
    int currentframe;
    int animationindex;
    float animationtimer;
    int count;
    int hitpoints;
    int currenthp;
    int facedirection;
    int currency;
    int accumulatedcurrency;
    float alivetimer;
    float alivetimercompare;
    float deadtimer;

    int potionbought[4];
    int potionorder[4];
    int inventoryrow2n3available[2][4];
    int row2n3invencount;
    int rowspacing;
    int potioncount;
    int activepotiontype;
    int potionused;
    int playerdefense;
    bool defenseboostshdact;
    float jumpboost;
    bool jumpboostshdact;
    bool jumpboostactivated;
    bool attackboostshdact;
    bool entereddoor;
    bool won;
    float potioneffect;

    int keycount;
    int successfulparry;
    int damagetaken;
    int enemykilled;
    float timelapsed;
};

struct ShopItem {
    const char* name;
    int price;
    const char* description;
    void (*effect)(struct Playerinfo* player); // Function pointer for item effect
};

typedef enum 
{
    MENU,
    LOADSAVES,
    OPTIONS,
    QUIT,
    PLAYING,
    INVENTORY,
    SHOP,
    PAUSE,
    GAMEOVER,
} Gamestate;

typedef struct Arrow{
    Vector2 position;
    int direction;
    bool active;
    bool movingDown;
    bool arrowonground;
    bool arrowonblock;
    bool reflected;

    float speedX;
    float speedY;
    float arrowtimer;
    float rotation;
    float arrowongroundtimer;
    float arrowonblocktimer;
    float reflectedtimer;
}Arrow;


struct Playerinfo *blocksarray = NULL;
struct Playerinfo enemies[MAX_ENEMIES];
Arrow arrows[MAX_ARROWS];

//function prototypes
void drawbackground(struct GameAssets* assets, Camera2D* camera, int x, float scalefactor);
void drawobstacles(int blockcount, struct GameAssets* assets, struct Playerinfo* player, Gamestate* currentGameState);
int calculatemovementplayer(struct Playerinfo* player, int* maxplatform, struct GameAssets* assets, Gamestate currentgamestate);
void updatecamera(Camera2D* camera, struct Playerinfo* player);
void keepobjectwithinscreen(struct Playerinfo* object, struct GameAssets* assets);
void LoadAnimationDataplayer(struct GameAssets* assets);
void iterateanimationplayer(Gamestate* currentGameState, struct GameAssets* assets, struct Playerinfo* player, int* currentframecount, int* facedirection);
void enemymovement(struct Playerinfo* enemy, struct Playerinfo* player, int enemyonblock[MAX_ENEMIES], int enemyIndex, struct GameAssets* assets);
void enemyanimations(struct Playerinfo* enemy, struct GameAssets* assets, bool coinenemy, bool enemywithkey, struct Playerinfo* player);
void checkPlayerAttackCollision(struct Playerinfo* player, struct Playerinfo enemies[MAX_ENEMIES], int facedirection);
void iteratearrowanimation(Arrow* arrow, Texture2D texture, struct GameAssets* assets, Rectangle ground, Rectangle playerHitbox, int blockCount, struct Playerinfo* player, int playerlastframedirection);
void Unloadresources(struct GameAssets* assets);
void aligntextcentre(int x, int y, int fontsize, const char* text, Color color);
void rotatetextcentre(const char* text, Vector2 position, int fontSize, float rotation, Color color);
void shop(struct GameAssets* assets, struct Playerinfo* player, int* currentGameState, int* currentmusic, int destx, int desty, int scalefactor);
void drawtrees(struct GameAssets* assets, int i, int destx, int desty, int scalefactor);
void savegamedata(struct Playerinfo* Playerdata, struct GameAssets* assets, Gamestate* currentGameState, int* currentmusic, float musicVolume, int enemycount, struct Playerinfo enemies[MAX_ENEMIES], Camera2D* camera);
void loadgamedata(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, int* enemycount, struct Playerinfo enemies[MAX_ENEMIES], Camera2D* camera);
void handleMenuState(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, Camera2D* camera, Gamestate* previousgamestate);
void handleLoadSaves(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, Camera2D* camera, bool *gamedataloaded, int* enemycount, struct Playerinfo enemies[MAX_ENEMIES], int* playerlastframedirection, int blockcount);
void handleOptionsState(struct GameAssets* assets, Gamestate* currentGameState, int* currentmusic, float* musicVolume, Gamestate* previousgamestate);
void handlePlayingState(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, Camera2D* camera, int* blockcount, int* playerlastframedirection, int* playercurrentframe, 
    int enemyonblock[MAX_ENEMIES], struct Playerinfo enemies[MAX_ENEMIES], int* enemycount, int* currentmusic, bool* gamedataloaded,float* musicVolume, Gamestate* previousgamestate);
void drawbuttonsplayingstate(struct GameAssets* assets, Camera2D* camera, Gamestate* currentGameState, struct Playerinfo* Playerdata);
void handleInventorystate(struct GameAssets* assets, struct Playerinfo* player, Gamestate* currentGameState, Gamestate* previousgamestate, int* currentmusic, float* musicVolume);
void handlePauseState(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, Gamestate* previousgamestate, int* currentmusic, float* musicVolume, int* enemycount, struct Playerinfo enemies[MAX_ENEMIES], Camera2D* camera, bool* gamedataloaded);
void playerenemyhpbar(struct Playerinfo* player, struct Playerinfo enemies[MAX_ENEMIES], struct GameAssets* assets, int enemycount, Camera2D* camera);
void shopInteraction(struct Playerinfo* player, int* playerCurrency);
void handleshopstate(struct GameAssets* assets, struct Playerinfo* player, int* currentGameState, int* currentmusic, float* musicVolume);
void shopstateanimation(struct GameAssets* assets);
void drawInventory2n3row(struct GameAssets* assets, struct Playerinfo* player, Rectangle playerinvenboxes[12]);
void addItemToInventory(struct Playerinfo* player, ItemType itemType);
void handleGameOverState(struct GameAssets* assets, struct Playerinfo* player, Gamestate* currentGameState, int* currentmusic, float* musicVolume, Camera2D* camera, struct Playerinfo enemies[MAX_ENEMIES], int* enemycount);
void gameoverOverview(struct Playerinfo* Playerdata, struct GameAssets* assets, int enemycount, struct Playerinfo enemies[MAX_ENEMIES], char action);
void lockeddoor(struct GameAssets* assets, struct Playerinfo* player, Gamestate* currentGameState, int* blockcount);

void handleGameState(Gamestate* currentGameState, Gamestate* previousgamestate, Camera2D* camera, struct GameAssets* assets, struct Playerinfo* Playerdata, int* blockcount, 
                        int* playerlastframedirection, int* playercurrentframe, int enemyonblock[MAX_ENEMIES], 
                        struct Playerinfo enemies[MAX_ENEMIES], int* enemycount, int* currentmusic){

    switch (*currentGameState){
        static float musicVolume = 0.5f;
        static bool gamedataloaded = false; 

        case MENU:
            handleMenuState(assets, Playerdata, currentGameState, currentmusic, &musicVolume, camera, previousgamestate);
            break;
        case LOADSAVES: //load saved file or new game
            handleLoadSaves(assets, Playerdata, currentGameState, currentmusic, &musicVolume, camera, &gamedataloaded, enemycount, enemies, playerlastframedirection, *blockcount);
            break;
        case OPTIONS: 
            handleOptionsState(assets, currentGameState, currentmusic, &musicVolume, previousgamestate);
            break;
        case QUIT:
            Unloadresources(assets); 
            CloseAudioDevice();
            CloseWindow();
            exit(0);
            break;
        case PLAYING:
            handlePlayingState(assets, Playerdata, currentGameState, camera, blockcount, playerlastframedirection, playercurrentframe, 
                               enemyonblock, enemies, enemycount, currentmusic, &gamedataloaded, &musicVolume, previousgamestate);
            break;
        case INVENTORY:
            handleInventorystate(assets, Playerdata, currentGameState, previousgamestate, currentmusic, &musicVolume);
            break;
        case SHOP:
            handleshopstate(assets, Playerdata, (int*)currentGameState, currentmusic, &musicVolume);
            break;
        case PAUSE: 
            handlePauseState(assets, Playerdata, currentGameState, previousgamestate, currentmusic, &musicVolume, enemycount, enemies, camera, &gamedataloaded);
            break;
        case GAMEOVER:
            handleGameOverState(assets, Playerdata, currentGameState, currentmusic, &musicVolume, camera, enemies, enemycount);
            break;
    }
}

void initializeGameState(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, int* lastframedirection){
    // Initialize player data
    Playerdata->Position = (Vector2){1000, windheight - 500};
    Playerdata->width = 50;
    Playerdata->height = 50;
    Playerdata->currenthp = 100;
    Playerdata->hitpoints = 100; //hp
    Playerdata->currency = 2000;
    Playerdata->accumulatedcurrency = 0;
    Playerdata->onground = false;
    Playerdata->onplatform = false;
    Playerdata->isJumping = false;
    Playerdata->isfalling = false;
    Playerdata->attack = false;
    Playerdata->onshield = false;
    Playerdata->dead = false;
    Playerdata->entereddoor = false;
    Playerdata->won = false;
    Playerdata->deadtimer = 0.0f;
    Playerdata->playerdamage = 1; //attack
    Playerdata->playerdefense = 5; //defense
    Playerdata->animationstate = 0;
    Playerdata->potioncount = 0;
    Playerdata->potioneffect = 0.0f;
    Playerdata->activepotiontype = -1;
    Playerdata->potionused = 0;
    Playerdata->successfulparry = 0;
    Playerdata->damagetaken = 0;
    Playerdata->keycount = 0;
    Playerdata->timelapsed = 0.0f;
    assets->doorkeyinsertedcount = 0;
    assets->shopkeycount = 2;
    assets->shopkeyprice = 500;

    Playerdata->jumpboost = -700.0f; //jump
    Playerdata->jumpboostshdact = false;
    Playerdata->jumpboostactivated = false;
    Playerdata->attackboostshdact = false;
    Playerdata->defenseboostshdact = false;

    for (int i = 0; i<4; i++){
        assets->doorkeyinserted[i] = false;
        assets->doorkeycircledrawn[i] = 0;
        Playerdata->potionbought[i] = 0;
        Playerdata->potionorder[i] = 0;
        Playerdata->row2n3invencount = 0;
        Playerdata->rowspacing = 0;

        if (i < 2){
            for (int j = 0; j < 4; j++){
                Playerdata->inventoryrow2n3available[i][j] = 0;
            }
        }
    }
    *currentGameState = MENU;
    *currentmusic = -1; 
    *musicVolume = 0.5f;
    *lastframedirection = 1;

    //potion data
    for (int i=0; i<4; i++){
        assets->potionleftinshop[i] = 5;
    }
    assets->potionprice[0] = 150;
    assets->potionprice[1] = 80;
    assets->potionprice[2] = 100;
    assets->potionprice[3] = 100;
    LoadAnimationDataplayer(assets);

    for (int i = 0; i < MAX_ARROWS; i++) {
        arrows[i].active = false;
        arrows[i].position = (Vector2){0, 0};
        arrows[i].direction = 1;
        arrows[i].speedX = 160.0f;
        arrows[i].speedY = 45.0f;
        arrows[i].rotation = 0.0f;
        arrows[i].arrowtimer = 0.0f;
        arrows[i].movingDown = false;
        arrows[i].arrowonground = false;
        arrows[i].arrowonblock = false;
        arrows[i].reflected = false;
        arrows[i].arrowongroundtimer = 0.0f;
        arrows[i].arrowonblocktimer = 0.0f;
        arrows[i].reflectedtimer = 0.0f;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].Position = (Vector2){400 + i * 700, 700 - 700*i};
        enemies[i].width = 105;
        enemies[i].height = 140;
        enemies[i].attack = false;
        enemies[i].dead = false;
        enemies[i].deadenemyremoved = false;
        enemies[i].isrunning = true;
        enemies[i].currentframe = 0;
        enemies[i].animationindex = 0;
        enemies[i].animationtimer = 0.0f;
        enemies[i].facedirection = -1;
        enemies[i].hitpoints = 75;
        enemies[i].deadtimer = 0.0f;
        if (i % 2 == 0){
            enemies[i].enemywithcoin[i] = true; 
            enemies[i].enemywithkey[i] = false;
        }else if (i % 2 == 1){
            enemies[i].enemywithcoin[i] = false; 
            enemies[i].enemywithkey[i] = true;
        }else if (i % 3 == 0){
            enemies[i].enemywithcoin[i] = true;   
            enemies[i].enemywithkey[i] = true;
        }
    }
}

void handleMenuState(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, Camera2D* camera, Gamestate* previousgamestate){
    static bool settingsLoaded = false; 
    static bool hoverplayed = false;
    *previousgamestate = MENU;   

    if (*currentmusic != 1) { 
        StopMusicStream(assets->music[*currentmusic]);
        PlayMusicStream(assets->music[1]);
        SetMusicVolume(assets->music[*currentmusic], *musicVolume);
        *currentmusic = 1;
    }

    if (!settingsLoaded) {
        FILE *file = fopen("settings.txt", "r");
        if (file) {
            if (fscanf(file, "MusicVolume=%f\n", musicVolume) == 1) {
                SetMusicVolume(assets->music[*currentmusic], *musicVolume);
            }
            fclose(file);
        } else {
            printf("Warning: settings.txt not found. Using default settings.\n");
        }
        settingsLoaded = true;
    }
    Vector2 mousePos = GetMousePosition();
    Rectangle playButtonsrc = {97, 1, 46, 14};
    Rectangle optionButtonsrc = {193, 1, 46, 14};
    Rectangle exitButtonsrc = {481, 1, 46, 14};
    Rectangle playButtondest = {windwidth/2 - 200, 250, 400, 100};
    Rectangle optionsButtondest = {windwidth/2 - 200, 400, 400, 100};
    Rectangle exitButtondest = {windwidth/2 - 200, 550, 400, 100};

    bool isbuttonhovered = false;
    if (CheckCollisionPointRec(mousePos, playButtondest)) {
        playButtonsrc.x += 48;
        isbuttonhovered = true;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            *currentGameState = LOADSAVES;
        }
    }else if (CheckCollisionPointRec(mousePos, optionsButtondest)) {
        optionButtonsrc.x += 48;
        isbuttonhovered = true;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            *currentGameState = OPTIONS; 
        }
    }else if (CheckCollisionPointRec(mousePos, exitButtondest)) {
        exitButtonsrc.x += 48;
        isbuttonhovered = true;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            *currentGameState = QUIT; 
        }
    }else {
        isbuttonhovered = false;
    }

    if (isbuttonhovered && !hoverplayed){
        PlaySound(assets->sound[0]);
        hoverplayed = true;
    }
    if (!isbuttonhovered){ //reset the hovered state so the sound can repeat
        hoverplayed = false;
    }
    
    for (int i = 0; i < (mapwidth/windwidth); i++) {
        Rectangle skysrc = {0, 0, assets->texture[6].width, assets->texture[6].height};
        Rectangle skydest = {i * assets->texture[6].width, 0, assets->texture[6].width, assets->texture[6].height/2};
        DrawTexturePro(assets->texture[6], skysrc, skydest, (Vector2){0, 0}, 0, WHITE);
    }
    DrawTexturePro(assets->texture[20], playButtonsrc, playButtondest, (Vector2){0, 0}, 0, WHITE);
    DrawTexturePro(assets->texture[20], optionButtonsrc, optionsButtondest, (Vector2){0, 0}, 0, WHITE);
    DrawTexturePro(assets->texture[20], exitButtonsrc, exitButtondest, (Vector2){0, 0}, 0, WHITE);
    drawbackground(assets, camera, 4, 0.7f);
    shop(assets, Playerdata, (int*)currentGameState, currentmusic, 840, 380, 3);
    drawtrees(assets, 3, 0, 310, 4);
    drawtrees(assets, 5, 60, 650, 3);

    static int currentframe = 0;
    static int facedirection = 1;
    Playerdata->animationstate = 0; 
    Playerdata->Position.x = windwidth/2 - Playerdata->width/2;
    Playerdata->Position.y = 250 - Playerdata->height; 
    Playerdata->width *= 3;
    Playerdata->height *= 3;  
    iterateanimationplayer(NULL, assets, Playerdata, &currentframe, &facedirection);
    
    for (int i = 0; i < 2; i++) {
        enemies[i].Position.x = 200 + i * 100; // Spread enemies horizontally
        enemies[i].Position.y = windheight - 240; // Position near the bottom of the screen
        enemies[i].animationstate = 0; // Set to idle animation state
        enemies[i].facedirection = (i % 2 == 0) ? 1 : -1; // Alternate facing directions
        enemyanimations(&enemies[i], assets, NULL, NULL, Playerdata);
    }

    //reset back to the wanted position
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i] = (struct Playerinfo){
            .Position = {400 + i * 700, 700 - 700*i},
            .width = 105,
            .height = 140,
            .attack = false,
            .dead = false,
            .isrunning = true,
            .currentframe = 0,
            .animationindex = 0,
            .animationtimer = 0.0f,
            .facedirection = -1,
            .hitpoints = 75,
            .deadtimer = 0.0f
        };
    }
}

void handleLoadSaves(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, Camera2D* camera, bool *gamedataloaded, int* enemycount, struct Playerinfo enemies[MAX_ENEMIES], int* playerlastframedirection, int blockcount){
    
    Rectangle savesbgsrc = {0, 0, 120, 140};
    Rectangle savesbgdest = {600, 400, 800, windheight + 40};
    Vector2 savesbgorigin = {savesbgdest.width/2, savesbgdest.height/2};
    Rectangle loadButton = {450, 250, 300, 100};
    Rectangle newGameButton = {450, 400, 300, 100};
    Rectangle backButton = {450, 550, 300, 100};
    Rectangle Hoveredbuttonsrc = {0, 4, assets->texture[26].width, 25};
    Vector2 mousePos = GetMousePosition();
    Rectangle enemyskelsrc = {0, 0, assets->texture[19].width/5, assets->texture[19].height/2};
    Rectangle enemyskeldestright = {950, windheight/2 - (enemyskelsrc.height/5), enemyskelsrc.width/2.5, enemyskelsrc.height/2.5};
    Vector2 enemyskelorigin = {0, 0};

    DrawTexturePro(assets->texture[25], savesbgsrc, savesbgdest, savesbgorigin, 0, WHITE);
    DrawTexturePro(assets->texture[19], enemyskelsrc, enemyskeldestright, enemyskelorigin, 0, WHITE);
    enemyskelsrc.x += enemyskelsrc.width;
    enemyskelsrc.width = -fabs(enemyskelsrc.width); // Flip horizontally
    Rectangle enemyskeldestleft = {savesbgdest.x - 600, enemyskeldestright.y, fabs(enemyskelsrc.width) / 2.5, enemyskelsrc.height / 2.5};
    DrawTexturePro(assets->texture[19], enemyskelsrc, enemyskeldestleft, enemyskelorigin, 0, WHITE);
    
    aligntextcentre(200 + savesbgdest.width/2, 150, 50, "LOAD GAME DATA", BLACK);  
    const char* textload[] = {"LOAD GAME", "NEW GAME", "BACK TO MENU"};
    bool ishovered = false;
    static bool hoverdrawn = false;

    for (int i = 0; i < 3; i++){
        Rectangle emptybuttonsrc = {0, 2, assets->texture[23].width, 27};
        Rectangle emptybuttondest = {450, 250 + i * 150, 300, 100};
        DrawTexturePro(assets->texture[23], emptybuttonsrc, emptybuttondest, (Vector2){0, 0}, 0.0f, WHITE);
        aligntextcentre(emptybuttondest.x + emptybuttondest.width / 2, emptybuttondest.y + emptybuttondest.height / 2, 30, textload[i], BLACK);
    }

    if (CheckCollisionPointRec(mousePos, newGameButton))
    {
        DrawTexturePro(assets->texture[26], Hoveredbuttonsrc, newGameButton, (Vector2){0, 0}, 0.0f, WHITE);
        ishovered = true;
        aligntextcentre(newGameButton.x + newGameButton.width / 2, newGameButton.y + newGameButton.height / 2, 30, "NEW GAME", BLACK);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            initializeGameState(assets, Playerdata, currentGameState, currentmusic, musicVolume, playerlastframedirection);
            Playerdata->alivetimer = 0.0f;
            Playerdata->alivetimercompare = 0.0f;
            *currentGameState = PLAYING; 
            *gamedataloaded = true;
        } 
    }else if (CheckCollisionPointRec(mousePos, backButton)){
        DrawTexturePro(assets->texture[26], Hoveredbuttonsrc, backButton, (Vector2){0, 0}, 0.0f, WHITE);
        ishovered = true;
        aligntextcentre(backButton.x + backButton.width / 2, backButton.y + backButton.height / 2, 30, "BACK TO MENU", BLACK);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            *currentGameState = MENU;
        } 
    }else if (CheckCollisionPointRec(mousePos, loadButton)){
        DrawTexturePro(assets->texture[26], Hoveredbuttonsrc, loadButton, (Vector2){0, 0}, 0.0f, WHITE);
        ishovered = true;
        aligntextcentre(loadButton.x + loadButton.width / 2, loadButton.y + loadButton.height / 2, 30, "LOAD GAME", BLACK);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            initializeGameState(assets, Playerdata, currentGameState, currentmusic, musicVolume, playerlastframedirection);
            loadgamedata(assets, Playerdata, currentGameState, currentmusic, musicVolume, enemycount, enemies, camera);
            printf("Alive timer: %f\n", Playerdata->alivetimer);
            printf("Playerposition: %f, %f\n", Playerdata->Position.x, Playerdata->Position.y);
            *currentGameState = PLAYING;
        }
    }else{
        ishovered = false;
    }

    if (ishovered && !hoverdrawn){
        PlaySound(assets->sound[0]);
        hoverdrawn = true;}
    if (!ishovered){ 
        hoverdrawn = false;}
}

void handleOptionsState(struct GameAssets* assets, Gamestate* currentGameState, int* currentmusic, float* musicVolume, Gamestate* previousgamestate){ //26
    Rectangle optiontabsrc = {0, 0, 120, 140};
    Rectangle optiontabdest = {600, 370, 900, windheight + 80};
    Vector2 optiontaborigin = {optiontabdest.width/2, optiontabdest.height/2};
    Vector2 mousePos = GetMousePosition();
    Rectangle volumebuttondest = {300, 200, 250, 80};                
    Rectangle volumeSlider = {windwidth / 2, 215, 200, 40}; 
    Rectangle homebuttonsrc = {65, 34, 91, 97};
    Rectangle homebuttondest = {1030, 50, 120, 120};
    static bool loadedmusicvol = false;

    if (*previousgamestate != OPTIONS && loadedmusicvol == false) { 
        FILE *file = fopen("settings.txt", "r");
        if (file) {
            if (fscanf(file, "MusicVolume=%f\n", musicVolume) == 1) {
                SetMusicVolume(assets->music[*currentmusic], *musicVolume);
            }
            fclose(file);
        }
        loadedmusicvol = true;
    }

    if (mousePos.x >= 1030 && mousePos.x <= 1150 && mousePos.y >= 50 && mousePos.y <= 170){
        homebuttonsrc.x = 181;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            *currentGameState = MENU;
        }
    }
    DrawTexturePro(assets->texture[31], homebuttonsrc, homebuttondest, (Vector2){0, 0}, 0, WHITE);
    DrawTexturePro(assets->texture[22], optiontabsrc, optiontabdest, optiontaborigin, 0, WHITE);
    DrawTexturePro(assets->texture[23], (Rectangle){0, 0, assets->texture[23].width, assets->texture[23].height},
                    volumebuttondest, (Vector2){0, 0}, 0.0f, WHITE );
    DrawRectangleRec(volumeSlider, LIGHTGRAY);
    DrawRectangle(volumeSlider.x, volumeSlider.y, *musicVolume * 200, volumeSlider.height, DARKBLUE);
    aligntextcentre(volumebuttondest.x + volumebuttondest.width/2, 
                    volumebuttondest.y + volumebuttondest.height/2, 30, "Music Volume", BLACK);
    Rectangle togglebar = {(volumeSlider.x + *musicVolume*200)-2, volumeSlider.y - 5, 4, volumeSlider.height + 10}; 
    DrawRectangleRec(togglebar, DARKGRAY);

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, volumeSlider)) {
        *musicVolume = (mousePos.x - volumeSlider.x) / volumeSlider.width;
        if (*musicVolume < 0.0f){
            *musicVolume = 0.0f;}
        if (*musicVolume > 1.0f) {
            *musicVolume = 1.0f;}
        SetMusicVolume(assets->music[*currentmusic], *musicVolume); // Adjust music volume
    }
    char volumeText[10];
    sprintf(volumeText, "%.0f", *musicVolume * 100);
    DrawText(volumeText, volumeSlider.x + volumeSlider.width + 20, volumeSlider.y+volumeSlider.height/2-10, 20, BLACK);

    const char* controls[] = {"Space", "A/D", "Left Mouse", "Shift", "P", "E", "I"};
    const char* functions[] = {"Jump", "Left/Right", "Attack", "Parry", "Pause", "Interact", "Inventory"};
    int FontSize = 20; 
    int emptybuttonWidth = 150; 
    int emptybuttonHeight = 50; 
    int controlsStartY = volumebuttondest.y + volumebuttondest.height + 40; // Starting Y position for controls
    int buttonSpacing = 12;
    int buttonGap = 20; 
    static float settingssavedtimer = 0.0f;
    
    // Draw "Controls" title
    DrawText("Controls:", 310, controlsStartY - 30, FontSize + 5, BLACK);

    int displaybutton = 7;
    for (int i = 0; i < displaybutton; i++) {
        Rectangle keyButtonRect = {300, controlsStartY + (i + 1) * (emptybuttonHeight + buttonSpacing) - 60, emptybuttonWidth,emptybuttonHeight};
        Rectangle functionButtonRect = {keyButtonRect.x + keyButtonRect.width + buttonGap, controlsStartY + (i + 1) * (emptybuttonHeight + buttonSpacing) - 60, 
                                        emptybuttonWidth + 50, emptybuttonHeight};
        DrawTexturePro(assets->texture[23], (Rectangle){0, 0, assets->texture[23].width, assets->texture[23].height},
                        keyButtonRect, (Vector2){0, 0}, 0.0f, WHITE);
        DrawTexturePro(assets->texture[23],(Rectangle){0, 0, assets->texture[23].width, assets->texture[23].height},
                        functionButtonRect, (Vector2){0, 0}, 0.0f, WHITE);
        aligntextcentre(keyButtonRect.x + keyButtonRect.width / 2, keyButtonRect.y + keyButtonRect.height / 2, FontSize, controls[i], BLACK);
        aligntextcentre(functionButtonRect.x + functionButtonRect.width / 2, functionButtonRect.y + functionButtonRect.height / 2, FontSize, functions[i], BLACK);
    }

    Rectangle saveButton = {windwidth / 2 + 140, controlsStartY + 250, 200, 50};
    Rectangle backButton = {saveButton.x, saveButton.y + 80, 200, 50};
    DrawRectangleRec(saveButton, LIGHTGRAY);
    aligntextcentre(saveButton.x + saveButton.width / 2, saveButton.y + saveButton.height / 2, 20, "Save Settings", BLACK);
    DrawRectangleRec(backButton, LIGHTGRAY);
    aligntextcentre(backButton.x + backButton.width / 2, backButton.y + backButton.height / 2, 20, "Back", BLACK);
    DrawRectangleLines(saveButton.x, saveButton.y, saveButton.width, saveButton.height, BLACK);
    DrawRectangleLines(backButton.x, backButton.y, backButton.width, backButton.height, BLACK);
    
    if (CheckCollisionPointRec(mousePos, saveButton)){
        DrawRectangleRec(saveButton, BLUE);
        aligntextcentre(saveButton.x + saveButton.width / 2, saveButton.y + saveButton.height / 2, 20, "Save Settings", WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            FILE *file = fopen("settings.txt", "w");
            if (file) {
                fprintf(file, "MusicVolume=%.2f\n", *musicVolume);
                fclose(file);
                settingssavedtimer = 3.0f;
            }
        }
    }
    if (settingssavedtimer > 0.0f) {
        DrawText("Settings Saved!", saveButton.x - 20, saveButton.y + 140, 40, GREEN);
        settingssavedtimer -= GetFrameTime();
    }

    if (CheckCollisionPointRec(mousePos, backButton)){
        DrawRectangleRec(backButton, BLUE);
        aligntextcentre(backButton.x + backButton.width / 2, backButton.y + backButton.height / 2, 20, "Back", WHITE);
            if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) && *previousgamestate == MENU){
                *currentGameState = MENU; 
                settingssavedtimer = 0.0f;
                loadedmusicvol = false;
            }
            else if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) && *previousgamestate == PLAYING){
                *currentGameState = PLAYING; 
                settingssavedtimer = 0.0f;
                loadedmusicvol = false;
            }
            else if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) && *previousgamestate == PAUSE){
                *currentGameState = PAUSE; 
                settingssavedtimer = 0.0f;
                loadedmusicvol = false;
            }
            else if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) && *previousgamestate == INVENTORY){
                *currentGameState = INVENTORY; 
                settingssavedtimer = 0.0f;
                loadedmusicvol = false;
            }
    }
}

void handlePlayingState(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, Camera2D* camera, int* blockcount, int* playerlastframedirection, int* playercurrentframe, 
                        int enemyonblock[MAX_ENEMIES], struct Playerinfo enemies[MAX_ENEMIES], int* enemycount, int* currentmusic, bool* gamedataloaded, float* musicVolume, Gamestate* previousgamestate)
{
    static float cointimer = 0.0f;
    float dt = GetFrameTime(); 
    bool playerpostracked;
    *previousgamestate = PLAYING;

    Playerdata->alivetimer += dt;
    cointimer += dt;
    if (cointimer >= 1.0f) {
        cointimer = 0.0f;
        if (Playerdata->currency + 2 <= MAX_CURRENCY)
            Playerdata->currency += 2;
            Playerdata->accumulatedcurrency += 2;
    }

    if (*currentmusic != 0) { 
        StopMusicStream(assets->music[*currentmusic]);
        *currentmusic = 0;
        PlayMusicStream(assets->music[*currentmusic]);
        SetMusicVolume(assets->music[*currentmusic], *musicVolume);
    }
    BeginMode2D(*camera);
    for (int j = 1; j < 5; j++){
        if (j == 4){
            drawbackground(assets, camera, j, 0.7);
        }
        else{
            drawbackground(assets, camera, j, 0.7);
        }
    }

    drawobstacles(*blockcount, assets, Playerdata, currentGameState);
    shop(assets, Playerdata, (int*)currentGameState, currentmusic, 15, 350 - (128*0.7) - (assets->texture[24].height * 3) + 60, 3);
    playerenemyhpbar(Playerdata, enemies, assets, *enemycount, camera);
    drawbuttonsplayingstate(assets, camera, currentGameState, Playerdata);   
    //lockeddoor(assets, Playerdata, currentGameState, blockcount);
    *playerlastframedirection = calculatemovementplayer(Playerdata, blockcount, assets, *currentGameState);
    updatecamera(camera, Playerdata);
    keepobjectwithinscreen(Playerdata, assets);
    iterateanimationplayer(currentGameState, assets, Playerdata, playercurrentframe, playerlastframedirection);
    
    Texture2D arrowtexture = assets->texture[18];
    Rectangle ground = {0, windheight - assets->texture[13].height * 0.7, mapwidth, assets->texture[13].height * 0.7};
    Rectangle playerHitbox = {Playerdata->Position.x, Playerdata->Position.y, Playerdata->width - 30, Playerdata->height};
    if (*playerlastframedirection >0 && Playerdata->onshield){
        playerHitbox.x += 40;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemymovement(&enemies[i], Playerdata, enemyonblock, i, assets);
        enemyanimations(&enemies[i], assets, enemies[i].enemywithcoin[i], enemies[i].enemywithkey[i], Playerdata);

        if (enemies[i].attack && enemies[i].animationindex == 9 && !enemies[i].arrowshot){
            for (int j = 0; j < MAX_ARROWS; j++){ //only allow three arrows to be active at a time
                if (!arrows[j].active) {
                    arrows[j].active = true;
                    arrows[j].direction = enemies[i].facedirection;
                    if (enemies[i].facedirection > 0) {
                        arrows[j].position = (Vector2){enemies[i].Position.x + enemies[i].width, enemies[i].Position.y + enemies[i].height / 2};
                    } else {
                        arrows[j].position = (Vector2){enemies[i].Position.x, enemies[i].Position.y + enemies[i].height / 2};
                    }
                    enemies[i].arrowshot = true;
                    break; // Activate only one arrow per attack
                }
            }
        }

        if (!enemies[i].attack || enemies[i].animationindex != 9) {
            enemies[i].arrowshot = false;
        }
    }
    for (int i = 0; i < MAX_ARROWS; i++){
        if (arrows[i].active) {
            iteratearrowanimation(&arrows[i], arrowtexture, assets, ground, playerHitbox, *blockcount, Playerdata, *playerlastframedirection);
        }
    }

    checkPlayerAttackCollision(Playerdata, enemies, *playerlastframedirection);
    EndMode2D();

    if (IsKeyPressed(KEY_P)){
        *currentGameState = PAUSE;
    }
    if(IsKeyPressed(KEY_I)){
        *currentGameState = INVENTORY;
    }
}

void handleInventorystate(struct GameAssets* assets, struct Playerinfo* player, Gamestate* currentGameState, Gamestate* previousgamestate, int* currentmusic, float* musicVolume){
    Vector2 mousepos = GetMousePosition();
    Rectangle playerinvensrc = {0, 0, assets->texture[30].width, assets->texture[30].height};
    Rectangle invenbg = {3740, 3695, 582, 180};
    Rectangle invenbgdest = {100, 100, 1000, 300};
    Rectangle playeridlesrc = {21, 0, 50, assets->texture[1].height};
    Rectangle playeridledest = {50, 400, 250, 300};
    Rectangle homebuttonsrc = {53, 30, 110, 103};
    Rectangle homebuttondest = {1040, 40, 110, 100};
    Rectangle invenvbordersrc = {16, 14, 482, 486};
    Rectangle invenborderdest = {376, 250, 447, 336};
    Rectangle rocksrc = {33, 29, 33, 19};
    Rectangle rockdest = {60, 699, 220, 100};
    Rectangle resumebuttonsrc = {65, 386, 90, 95};
    Rectangle resumebuttondest = {930, 45, 95, 95};
    Rectangle settingsbuttonsrc = {66, 270, 90, 95};
    Rectangle settingsbuttondest = {50, resumebuttondest.y, 100, 100};
    static Rectangle hoverborderdest = {401, 269, 99, 99};
    char potionCountText[10];
    static bool potionavailable[4] = {false, false, false, false};
    static bool potionuseconfirmation = false;
    static float timelapsed = 0.0f;
    static bool canusepotion = false;
    static bool morethanonepotion = false;
    static bool secondenter = false;
    float dt = GetFrameTime();
    static float potioneffectlapsed = 0.0f;
    Rectangle playerinvenboxes[12];
    
    ClearBackground((Color){206, 250, 255, 0});
    DrawTexturePro(assets->texture[33], invenvbordersrc, invenborderdest, (Vector2){0,0}, 0, WHITE);
    DrawTexturePro(assets->texture[35], rocksrc, rockdest, (Vector2){0,0}, 0, WHITE);

    if (CheckCollisionPointRec(mousepos, homebuttondest)){
        homebuttonsrc.x = 173;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            *currentGameState = MENU;
            hoverborderdest.x = 401;
            hoverborderdest.y = 269;
        }
    }

    if (mousepos.x >= resumebuttondest.x && mousepos.x <= resumebuttondest.x + resumebuttondest.width
        && mousepos.y >= resumebuttondest.y && mousepos.y <= resumebuttondest.y + resumebuttondest.height){
        resumebuttonsrc.x = 181;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            *currentGameState = PLAYING;
            hoverborderdest.x = 401;
            hoverborderdest.y = 269;
        }
    }

    if (mousepos.x >= settingsbuttondest.x && mousepos.x <= settingsbuttondest.x + settingsbuttondest.width
        && mousepos.y >= settingsbuttondest.y && mousepos.y <= settingsbuttondest.y + settingsbuttondest.height){
        settingsbuttonsrc.x = 182;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            *currentGameState = OPTIONS;
            *previousgamestate = INVENTORY;
        }
    }

    if (IsKeyPressed(KEY_LEFT) && hoverborderdest.x > 401){
        hoverborderdest.x -= 100;
    }else if (IsKeyPressed(KEY_RIGHT) && hoverborderdest.x < 700){
        hoverborderdest.x += 100;
    }else if (IsKeyPressed(KEY_UP) && hoverborderdest.y > 269){
        hoverborderdest.y -= 100;
    }else if (IsKeyPressed(KEY_DOWN) && hoverborderdest.y < 468){
        hoverborderdest.y += 100;
    }

    DrawTexturePro(assets->texture[31], homebuttonsrc, homebuttondest, (Vector2){0, 0}, 0.0f, WHITE);
    DrawTexturePro(assets->texture[34], invenbg, invenbgdest, (Vector2){0, 0}, 0.0f, WHITE);
    DrawTexturePro(assets->texture[1], playeridlesrc, playeridledest, (Vector2){0, 0}, 0.0f, WHITE);
    DrawText("Inventory", 435, invenbgdest.y + 20, 70, BLUE);
    printf("%.2f, %.2f\n", mousepos.x, mousepos.y);
    DrawTexturePro(assets->texture[31], settingsbuttonsrc, settingsbuttondest, (Vector2){0, 0}, 0, WHITE);
    DrawTexturePro(assets->texture[31], resumebuttonsrc, resumebuttondest, (Vector2){0, 0}, 0, WHITE); 

    int y = 0;
    for (int i = 0; i < 12; i++) {
        int x = i % 4;
        if (i % 4 == 0 && i != 0) {
            y++;
        }
        Rectangle playerinvendest = {invenborderdest.x + 24 + (x * 100), invenborderdest.y + 18 + (y * 100), 100, 100};
        DrawTexturePro(assets->texture[30], playerinvensrc, playerinvendest, (Vector2){0, 0}, 0.0f, WHITE);
        playerinvenboxes[i] = playerinvendest;
    
        if (i < player->potioncount){
            int potiontype = player->potionorder[i];
            if (player->potionbought[potiontype] > 0) {
                potionavailable[potiontype] = true;
    
                Rectangle potionsrc = {potiontype * (assets->texture[29].width / 4), 0, assets->texture[29].width / 4,assets->texture[29].height / 2};
                Rectangle potiondest = {playerinvendest.x + 20, playerinvendest.y + 10, 60, 80};
                DrawTexturePro(assets->texture[29], potionsrc, potiondest, (Vector2){0, 0}, 0.0f, WHITE);
                sprintf(potionCountText, "x%d", player->potionbought[potiontype]);
                DrawText(potionCountText, potiondest.x + 40, potiondest.y + 50, 40, BLACK);
            }
        }
    }
    drawInventory2n3row(assets, player, playerinvenboxes);

    DrawRectangleLinesEx(hoverborderdest, 3, GOLD);
    char potiondesc[4][30] = {"Heal 30Hp", "Jump Height x1.5 for 20s", "+10 damage for 20s", "+5 defense for 20s"};
    int potionnum = (hoverborderdest.x - 401) / 100;

    if (hoverborderdest.y >= 369 && hoverborderdest.y <= 468){
        if (player->inventoryrow2n3available[0][potionnum] == 1){
            aligntextcentre(900, 650, 30, "Key. Collect this to unlock the", BLUE);
            aligntextcentre(900, 690, 30, "door located at the Eastern. ", BLUE);
        }else if (player->inventoryrow2n3available[0][potionnum] == 2){
            aligntextcentre(900, 650, 30, "Plated Armour. Wear this to", BLUE);
            aligntextcentre(900, 690, 30, "triumph in battles. ", BLUE);
        }else if (player->inventoryrow2n3available[0][potionnum] == 3){
            aligntextcentre(900, 650, 30, "Sharpened Sword. Believed to be", BLUE);
            aligntextcentre(900, 690, 30, "made by the strongest blacksmith. ", BLUE);
        }
    }
    if (potionnum < player->potioncount) {
        int type = player->potionorder[potionnum];
        if (player->potionbought[type] != 0) {
            if (hoverborderdest.y < 270){
                aligntextcentre(1000, 650, 30, potiondesc[type], BLUE);
            }
            if (IsKeyPressed(KEY_ENTER) && !potionuseconfirmation) {
                potionuseconfirmation = true;
                secondenter = false;
                timelapsed = 0.0f;
            }
        }
    }
    
    if (potionuseconfirmation) {
        if (timelapsed <= 3.0f) {
            aligntextcentre(500, 700, 30, "Enter again to use the Potion", GREEN);
            timelapsed += dt;
            if (IsKeyReleased(KEY_ENTER)) {
                secondenter = true;
            }
        } else {
            potionuseconfirmation = false;
            secondenter = false;
            timelapsed = 0.0f;
        }
    }

    if (morethanonepotion){
        potioneffectlapsed += dt;
        if (potioneffectlapsed <= 3.0f){
            aligntextcentre(600, 760, 30, "You cannot use two buff potions at the same time", GREEN);}
        else{
            potioneffectlapsed = 0.0f;
            morethanonepotion = false;
        }
    }

    if (IsKeyPressed(KEY_ENTER) && secondenter){
        secondenter = false;
        canusepotion = false;
        int potiontype = player->potionorder[potionnum];
        switch(potiontype){
            case 0: 
                canusepotion = true;
                if (player->currenthp <= 70){
                    player->currenthp += 30;}
                else{
                    player->currenthp = 100;}
                break;
            case 1:
                if (!player->jumpboostshdact){
                    if (player->attackboostshdact || player->defenseboostshdact){
                        player->jumpboostshdact = false;
                        canusepotion = false;
                        morethanonepotion = true;
                    }else{
                        player->jumpboostshdact = true;
                        canusepotion = true;
                    }
                }
                break;
            case 2:
                if (!player->attackboostshdact){
                    if (player->jumpboostshdact || player->defenseboostshdact){
                        player->attackboostshdact = false;
                        canusepotion = false;
                        morethanonepotion = true;
                    }else{
                        player->attackboostshdact = true;
                        canusepotion = true;
                    }
                }
                break;
            case 3:
                if (!player->defenseboostshdact){
                    if (player->attackboostshdact || player->jumpboostshdact){
                        player->defenseboostshdact = false;
                        canusepotion = false;
                        morethanonepotion = true;
                    }else{
                        player->defenseboostshdact = true;
                        canusepotion = true;
                    }
                }  
                break;
        }

        if (canusepotion){
            player->potionbought[potiontype]--;
            player->activepotiontype = potiontype;
            player->potionused++;
        }
        
        potionuseconfirmation = false;
    }
}

void handleshopstate(struct GameAssets* assets, struct Playerinfo* player, int* currentGameState, int* currentmusic, float* musicVolume){
    Rectangle shopbgsrc = {0, 0, 1024, 1024};
    Rectangle shopbgdest = {0, 0, windwidth, windheight};
    Vector2 shopbgorigin = {0, 0};
    Rectangle shopInventorysrc = {36, 62, 652, 328};
    Rectangle shopInventorydest = {100, 150, 1000, 600};
    Vector2 shopInventoryorigin = {0, 0};
    Rectangle shopinvenextsrc = {33, 94, 100, 264};
    Rectangle shopinvenextdest = {shopInventorydest.x-60, shopInventorydest.y+59, shopInventorydest.width*0.1, 490};
    Rectangle potionspos[4]; //array to store the potions' coordinates
    Vector2 mousePos = GetMousePosition();
    Rectangle emptybuttonsrc = {0, 2, assets->texture[23].width, 27};
    Rectangle shopbackbuttondest = {900, 40, 250, 80};
    Rectangle playerhpsrc = {337 - (48 * ((100 - player->currenthp)/7)), 1, 46, 14};
    Rectangle playerhpdest = {50, 50, 200, 70};
    Rectangle descriptionrecdest = {65, 325, 325, 470};
    Rectangle keysrc = {0, 0, assets->texture[36].width, assets->texture[36].height};
    Rectangle keydest = {450, 325, 50, 40};

    //static int potionindexbought[4];
    //int potioncount;
    float dt = GetFrameTime();
    static float timelapsed = 0.0f;
    static bool notenufmoney = false;
    char potionleft[10];
    Color potioncolor[4] = {RED, GREEN, BLUE, GOLD};
    char potiondesc[16][100] = {"CRIMSON ELIXIR", "Brewed from ember petals", "Restores +30 health", "\"Feel the burn, heal fast!\"",
                                "VERDANT TONIC", "Forest herbs & zest", "Jump height x1.5 (30s)", "\"Run like wind, strike fast!\"",
                                "SAPPHIRE DRAUGHT", "Moonwater and starroot", "+30 damage boost", "\"Clarity, focus, magic.\"",
                                "GOLDEN BULWARK", "Dragon scales with gold", "+5 defense", "\"Will forged in gold stays\""
                                };
    char currencytext[30];
    float potiontexturewidth = assets->texture[29].width / 4;
    int lineHeight = descriptionrecdest.height / 9;
    printf("\n%.2f, %.2f", mousePos.x, mousePos.y);

    sprintf(currencytext, "Currently available money: %d", player->currency);
    DrawTexturePro(assets->texture[11], shopbgsrc, shopbgdest, shopbgorigin, 0, WHITE);
    //DrawRectangleRec(shopbgdest, (Color){242, 255, 196, 100});
    DrawTexturePro(assets->texture[28], shopInventorysrc, shopInventorydest, shopInventoryorigin, 0, WHITE);
    DrawTexturePro(assets->texture[28], shopinvenextsrc, shopinvenextdest, (Vector2){0, 0}, 0, WHITE);
    DrawTexturePro(assets->texture[23], emptybuttonsrc, shopbackbuttondest, (Vector2){0, 0}, 0.0f, WHITE);
    DrawTexturePro(assets->texture[21], playerhpsrc, playerhpdest, (Vector2){0, 0}, 0.0f, WHITE);
    aligntextcentre(shopbackbuttondest.x + shopbackbuttondest.width / 2, shopbackbuttondest.y + shopbackbuttondest.height / 2, 50, "Back", BLACK);   
    aligntextcentre(360, 765, 30, currencytext, BLUE);
    shopstateanimation(assets);

    if (*currentmusic != 2) { 
        *currentmusic = 2;
        StopMusicStream(assets->music[*currentmusic]);
        PlayMusicStream(assets->music[2]);
        SetMusicVolume(assets->music[*currentmusic], *musicVolume * 2);
    }

    if (notenufmoney){
        timelapsed += dt;
        DrawText("Not enough coins!!", 500, 80, 40, GREEN);
        if (timelapsed >= 2.0f){
            timelapsed = 0.0f;
            notenufmoney = false;
        }
    }

    for (int i=0; i<4; i++){
        Rectangle potionsrc = {0 + (i * potiontexturewidth), 0, potiontexturewidth, assets->texture[29].height/2};
        Rectangle potionsdest = {448 + (i * potionsrc.width/6), 205, potionsrc.width/8 - 5, potionsrc.height/8};
        potionspos[i] = (Rectangle){potionsdest.x, potionsdest.y, potionsdest.width, potionsdest.height};
        DrawTexturePro(assets->texture[29], potionsrc, potionsdest, (Vector2){0, 0}, 0.0f, WHITE);
    }
   
    DrawTexturePro(assets->texture[36], keysrc, keydest, (Vector2){0, 0}, 0.0f, WHITE);
    if (assets->shopkeycount != 0){
        sprintf(potionleft, "x%d", assets->shopkeycount);
        DrawRectangleRec((Rectangle){460, 365, 40, 30}, (Color){255, 255, 153, 120});
        DrawText(potionleft, 465, 365, 30, BLACK);
    }else{
        Vector2 soldtextpos = {keydest.x + keydest.width / 2, keydest.y + keydest.height / 2};
        DrawRectangleRec((Rectangle){439, 306, 70, 87}, (Color){180, 180, 180, 180});
        rotatetextcentre("SOLD OUT", soldtextpos, 20, -45, WHITE);
    }
    if (mousePos.x >= keydest.x && mousePos.x <= keydest.x + keydest.width
        && mousePos.y >= keydest.y && mousePos.y <= keydest.y + keydest.height){
        DrawRectangleRec(keydest, (Color){255, 255, 255, 80});
        aligntextcentre(descriptionrecdest.x + descriptionrecdest.width/2, 450, 50, "Key", PURPLE);
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            if (player->currency >= assets->shopkeyprice){
                addItemToInventory(player, 1);
                player->currency -= assets->shopkeyprice;
                assets->shopkeycount--;
            }else{
                notenufmoney = true;
            }
        }
    }

    for (int i=0; i<4; i++){
        char potionlefttext[20];

        if (assets->potionleftinshop[i] != 0){
            sprintf(potionlefttext, "x%d", assets->potionleftinshop[i]);
            DrawRectangleRec((Rectangle){448 + (i * potiontexturewidth/6) + 18, 260, 40, 30}, (Color){255, 255, 153, 120}); //potiondest.y + 55
            DrawText(potionlefttext, 448 + (i * potiontexturewidth/6) + 24, 260, 30, BLACK);
        }else{
            Vector2 soldtextpos = {478 + (90 * i), 250};
            DrawRectangleRec((Rectangle){439 + (92*i), 200, 70, 87}, (Color){180, 180, 180, 180});
            rotatetextcentre("SOLD OUT", soldtextpos, 20, -45, WHITE);
        }

        if (CheckCollisionPointRec(mousePos, potionspos[i])){
            DrawRectangleRec(potionspos[i], (Color){255, 255, 255, 80});
            DrawRectangle(65, 330, 325, 5, DARKBROWN);
            for (int j=0; j<4; j++){
                int descIndex = i * 4 + j;
                int lineY = 10 + descriptionrecdest.y + j * lineHeight;
                if (descIndex % 4 == 0){
                    aligntextcentre(descriptionrecdest.x + descriptionrecdest.width/2, lineY- 35, 31, potiondesc[descIndex], potioncolor[i]);
                }else{
                    aligntextcentre(descriptionrecdest.x + descriptionrecdest.width/2, lineY - 30, 24, potiondesc[descIndex], ORANGE);
                }
            }
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && assets->potionleftinshop[i] != 0)
            {
                //printf("Potion %d clicked!\n", i+1);
                //can probably add a purchase confirmation page
                if (player->currency >= assets->potionprice[i]){
                    if (player->potionbought[i] == 0) {
                        player->potionorder[player->potioncount++] = i;
                    }
                    player->potionbought[i]++;
                    assets->potionleftinshop[i]--;
                    player->currency -= assets->potionprice[i];
                }else{
                    notenufmoney = true;
                }
            }
        }

    }

    if (CheckCollisionPointRec(GetMousePosition(), shopbackbuttondest)){
        DrawTexturePro(assets->texture[26], (Rectangle){0, 4, assets->texture[26].width, 25}, shopbackbuttondest, (Vector2){0, 0}, 0.0f, WHITE);
        aligntextcentre(shopbackbuttondest.x + shopbackbuttondest.width / 2, shopbackbuttondest.y + shopbackbuttondest.height / 2, 50, "Back", BLACK);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            *currentGameState = PLAYING;
        }
    }
}

void handlePauseState(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, Gamestate* previousgamestate, int* currentmusic, float* musicVolume, int* enemycount, struct Playerinfo enemies[MAX_ENEMIES], Camera2D* camera, bool* gamedataloaded){
    Rectangle pausebgsrc = {0, 0, 120, 140};
    Rectangle pausebgdest = {600, 400, 800, windheight + 40};
    Vector2 pausebgorigin = {pausebgdest.width/2, pausebgdest.height/2};
    Rectangle hoveredbuttonsrc = {0, 4, assets->texture[26].width, 25};
    Vector2 mousePos = GetMousePosition();
    Rectangle charactersrc = {assets->src_shieldx[4], assets->src_shieldy[4], assets->src_shieldwidth[4], 58};
    Rectangle characterdest = {0, windheight/2 - (charactersrc.height*4)/2, charactersrc.width * 4, charactersrc.height * 4};
    Vector2 characterorigin = {0, 0};
    Rectangle pauseenemyskelsrc = {2418, 1055, 600, assets->texture[15].height - 1085};
    Rectangle pauseenemyskeldestright = {940, windheight/2 - (pauseenemyskelsrc.height/2.5)/2, pauseenemyskelsrc.width/2.5, pauseenemyskelsrc.height/2.5};
    Vector2 pauseenemyskelorigin = {0, 0};
    Rectangle settingsbuttonsrc = {67, 270, 90, 96};
    Rectangle settingsbuttondest = {1050, 50, 100, 100};
    Rectangle homebuttonsrc = {65, 34, 91, 97};
    Rectangle homebuttondest = {50, 50, 100, 100};

    const char* text[] = {"RESUME", "SAVE GAME", "BACK TO MENU"};
    static float savetimer = 0.0f;
    static bool confirmedsave = false;
    static bool unsavedchanges = true;
    static bool showsavechangeswarning = false;
    static bool hoverdrawn = false;
    bool ishovered = false;

    if (mousePos.x >= homebuttondest.x && mousePos.x <= homebuttondest.x + homebuttondest.width
        && mousePos.y >= homebuttondest.y && mousePos.y <= homebuttondest.y +homebuttondest.height){
            homebuttonsrc.x = 181;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                *currentGameState = MENU;
            }
        }
    if (mousePos.x >= settingsbuttondest.x && mousePos.x <= settingsbuttondest.x + settingsbuttondest.width
        && mousePos.y >= settingsbuttondest.y && mousePos.y <= settingsbuttondest.y +settingsbuttondest.height){
            settingsbuttonsrc.x = 182;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                *currentGameState = OPTIONS;
                *previousgamestate = PAUSE;
            }
        }

    DrawTexturePro(assets->texture[25], pausebgsrc, pausebgdest, pausebgorigin, 0, WHITE);
    DrawTexturePro(assets->texture[5], charactersrc, characterdest, characterorigin, 0, WHITE);
    aligntextcentre(200 + pausebgdest.width/2, 150, 50, "PAUSED", BLACK);   
    DrawTexturePro(assets->texture[19], pauseenemyskelsrc, pauseenemyskeldestright, pauseenemyskelorigin, 0, WHITE);
    DrawTexturePro(assets->texture[31], settingsbuttonsrc, settingsbuttondest, (Vector2){0,0}, 0.0f, WHITE);
    DrawTexturePro(assets->texture[31], homebuttonsrc, homebuttondest, (Vector2){0,0}, 0, WHITE);

    for (int i = 0; i < 3; i++){
        Rectangle emptybuttonsrc = {0, 2, assets->texture[23].width, 27};
        Rectangle emptybuttondest = {450, 250 + i * 150, 300, 100};
        DrawTexturePro(assets->texture[23], emptybuttonsrc, emptybuttondest, (Vector2){0, 0}, 0.0f, WHITE);
        aligntextcentre(emptybuttondest.x + emptybuttondest.width / 2, emptybuttondest.y + emptybuttondest.height / 2, 30, text[i], BLACK);
    }

    Rectangle resumeButton = {450, 250, 300, 100};
    Rectangle saveButton = {450, 400, 300, 100};
    Rectangle backButton = {450, 550, 300, 100};
    

    if (CheckCollisionPointRec(mousePos, resumeButton)){
        DrawTexturePro(assets->texture[26], hoveredbuttonsrc, resumeButton, (Vector2){0, 0}, 0.0f, WHITE);
        aligntextcentre(resumeButton.x + resumeButton.width / 2, resumeButton.y + resumeButton.height / 2, 30, text[0], BLACK);
        ishovered = true; //boolean to check if the mouse is hovering over the button
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            *currentGameState = PLAYING; // Resume the game
            savetimer = 0.0f;
            unsavedchanges = true;
            showsavechangeswarning = false;
        }
    }else if (CheckCollisionPointRec(mousePos, saveButton)){
        DrawTexturePro(assets->texture[26], hoveredbuttonsrc, saveButton, (Vector2){0, 0}, 0.0f, WHITE);
        aligntextcentre(saveButton.x + saveButton.width / 2, saveButton.y + saveButton.height / 2, 30, text[1], BLACK);
        ishovered = true;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            confirmedsave = true;
        }
    }else if (CheckCollisionPointRec(mousePos, backButton)){
            DrawTexturePro(assets->texture[26], hoveredbuttonsrc, backButton, (Vector2){0, 0}, 0.0f, WHITE);
            aligntextcentre(backButton.x + backButton.width / 2, backButton.y + backButton.height / 2, 30, text[2], BLACK);
            ishovered = true;
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (!unsavedchanges){ //if the user has already saved the game
                    *currentGameState = MENU; // Return to the main menu
                    *gamedataloaded = false;
                    savetimer = 0.0f;
                    unsavedchanges = true;
                }else{
                    showsavechangeswarning = true; //the player has not saved the game
                    unsavedchanges = true;
                }
            }
    }else{
        ishovered = false;
    }
    if (ishovered && !hoverdrawn){
        PlaySound(assets->sound[0]);
        hoverdrawn = true;}
    if(!ishovered){ 
        hoverdrawn = false;}
    if (confirmedsave) {
        if (showsavechangeswarning){
            showsavechangeswarning = false;
        }
        DrawTexturePro(assets->texture[23], (Rectangle){0, 0, assets->texture[23].width, assets->texture[23].height},(Rectangle){280, saveButton.y - 10, 640, saveButton.height+10}, (Vector2){0, 0}, 0.0f, WHITE);
        aligntextcentre(605, saveButton.y - 10 + saveButton.height / 2, 20, "Confirm Save? The previous data will be overwritten.", BLACK);
        
        Rectangle yesButton = {310, saveButton.y + 90, 280, 40};
        Rectangle noButton = {590, saveButton.y + 90, 280, 40};

        DrawRectangleRec(yesButton, BEIGE);
        DrawRectangleRec(noButton, BEIGE);
        DrawRectangleLines(yesButton.x, yesButton.y, yesButton.width, yesButton.height, BLACK);
        DrawRectangleLines(noButton.x, noButton.y, noButton.width, noButton.height, BLACK);
        aligntextcentre(yesButton.x + yesButton.width / 2, yesButton.y + yesButton.height / 2, 30, "Yes", WHITE);
        aligntextcentre(noButton.x + noButton.width / 2, noButton.y + noButton.height / 2, 30, "No", WHITE);

        if (CheckCollisionPointRec(mousePos, yesButton)){
            DrawRectangleRec(yesButton, LIGHTGRAY);
            aligntextcentre(yesButton.x + yesButton.width / 2, yesButton.y + yesButton.height / 2, 30, "Yes", BLACK);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { //player has saved the game
                confirmedsave = false;
                savegamedata(Playerdata, assets, currentGameState, currentmusic, *musicVolume, *enemycount, enemies, camera);
                unsavedchanges = false;
                showsavechangeswarning = false;
                savetimer = 3.0f;
            }
        }
        else if (CheckCollisionPointRec(mousePos, noButton)){
            DrawRectangleRec(noButton, LIGHTGRAY);
            aligntextcentre(noButton.x + noButton.width / 2, noButton.y + noButton.height / 2, 30, "No", BLACK);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                confirmedsave = false; //so that the prompt disappears
                savetimer = 0.0f;
                unsavedchanges = true;
                showsavechangeswarning = false;
            }
        }
        else if (CheckCollisionPointRec(mousePos, backButton) || CheckCollisionPointRec(mousePos, resumeButton)){
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                confirmedsave = false; //so that the prompt disappears
                if (unsavedchanges){
                    showsavechangeswarning = true; //the player has not saved the game
                }else{
                    savetimer = 0.0f;
                    unsavedchanges = true;
                }
            }
        }
    }
    if (savetimer > 0.0f) {
        aligntextcentre(200 + pausebgdest.width/2, 700, 50, "Game Saved!", GREEN); 
        savetimer -= GetFrameTime();
    }

    if (showsavechangeswarning){
        if (confirmedsave){
            confirmedsave = false;
        }
        Rectangle yesButtonback = {310, backButton.y + 90, 280, 40};
        Rectangle noButtonback = {590, backButton.y + 90, 280, 40};
        DrawTexturePro(assets->texture[23], (Rectangle){0, 0, assets->texture[23].width, assets->texture[23].height},(Rectangle){280, backButton.y - 10, 640, backButton.height+10}, (Vector2){0, 0}, 0.0f, WHITE);
        aligntextcentre(605, backButton.y - 10 + backButton.height / 2, 20, "Unsaved changes! Are you sure you want to quit?", BLACK);
        
        DrawRectangleRec(yesButtonback, BEIGE);
        DrawRectangleRec(noButtonback, BEIGE);
        DrawRectangleLines(yesButtonback.x, yesButtonback.y, yesButtonback.width, yesButtonback.height, BLACK);
        DrawRectangleLines(noButtonback.x, noButtonback.y, noButtonback.width, noButtonback.height, BLACK);
        aligntextcentre(yesButtonback.x + yesButtonback.width / 2, yesButtonback.y + yesButtonback.height / 2, 30, "Yes", WHITE);
        aligntextcentre(noButtonback.x + noButtonback.width / 2, noButtonback.y + noButtonback.height / 2, 30, "No", WHITE);

        if (CheckCollisionPointRec(mousePos, yesButtonback)){
            DrawRectangleRec(yesButtonback, LIGHTGRAY);
            aligntextcentre(yesButtonback.x + yesButtonback.width / 2, yesButtonback.y + yesButtonback.height / 2, 30, "Yes", BLACK);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { //player has saved the game
                unsavedchanges = true;
                showsavechangeswarning = false;
                *currentGameState = MENU;
            }
        }
        else if (CheckCollisionPointRec(mousePos, noButtonback)){
            DrawRectangleRec(noButtonback, LIGHTGRAY);
            aligntextcentre(noButtonback.x + noButtonback.width / 2, noButtonback.y + noButtonback.height / 2, 30, "No", BLACK);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                unsavedchanges = true; //so that the prompt disappears
                showsavechangeswarning = false;
            }
        }
    }
}

void handleGameOverState(struct GameAssets* assets, struct Playerinfo* player, Gamestate* currentGameState, int* currentmusic, float* musicVolume, Camera2D* camera, struct Playerinfo enemies[MAX_ENEMIES], int* enemycount){
    Rectangle homebuttonsrc = {65, 34, 91, 97};
    Rectangle homebuttondest = {50, 50, 100, 100};
    Rectangle playerdeadsrc = {assets->src_dyingx[4], assets->src_dyingy[4], assets->src_dyingwidth[4], assets->texture[40].height - assets->src_dyingy[4]};
    Rectangle playerdeaddest = {windwidth/2 - 500, windheight - (128 * 0.8) - 80, playerdeadsrc.width * 3.5, 80};
    Rectangle playeridlesrc = {assets->src_idlex[3], assets->src_idley[3], 50, 61};
    Rectangle playeridledest = {windwidth/2 - 500, windheight - (128*0.8) - 180, 140, 180};
    Vector2 mousePos = GetMousePosition();

    float dt = GetFrameTime();
    static int playerlastframedirection = 1;
    static int currentframe = 0;
    static int displayingtextindex = 0;
    static float timelapsed = 0.0f;
    static bool gameoverviewloaded = false;
    char text[100];

    if (!gameoverviewloaded){
        gameoverOverview(player, assets, *enemycount, enemies, 'w');
        gameoverOverview(player, assets, *enemycount, enemies, 'r');
        gameoverviewloaded = true;
    }
    if (*currentmusic != 3 && !player->won) { 
        *currentmusic = 3;
        StopMusicStream(assets->music[*currentmusic]);
        PlayMusicStream(assets->music[3]);
        SetMusicVolume(assets->music[*currentmusic], *musicVolume * 1.5);
    }

    for (int j = 1; j < 5; j++){
        drawbackground(assets, camera, j, 0.8);
    }

    if (mousePos.x >= homebuttondest.x && mousePos.x <= homebuttondest.x + homebuttondest.width
        && mousePos.y >= homebuttondest.y && mousePos.y <= homebuttondest.y +homebuttondest.height){
        homebuttonsrc.x = 181;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            initializeGameState(assets, player, currentGameState, currentmusic, musicVolume, &playerlastframedirection);
            *currentGameState = MENU;
            gameoverviewloaded = false;
            currentframe = 0;
            timelapsed = 0.0f;
        }
    }else if (IsKeyPressed(KEY_ENTER)){
        initializeGameState(assets, player, currentGameState, currentmusic, musicVolume, &playerlastframedirection);
        *currentGameState = MENU;
        gameoverviewloaded = false;
    }

    drawtrees(assets, 3, 200, windheight-(128*0.8)-(100 * 5) + 2, 5);
    drawtrees(assets, 6, 500, windheight-(128*0.8) + 2, 6);
    DrawTexturePro(assets->texture[31], homebuttonsrc, homebuttondest, (Vector2){0,0}, 0, WHITE);
    
    if (player->won){
        DrawTexturePro(assets->texture[1], playeridlesrc, playeridledest, (Vector2){0, 0}, 0.0f, WHITE);
        aligntextcentre(500+(27 * 6)/2, windheight - (128 * 0.8) - (28 * 6) + 40, 22, "Victorious", BEIGE);
    }else{
        DrawTexturePro(assets->texture[40], playerdeadsrc, playerdeaddest, (Vector2){0, 0}, 0.0f, (Color){213, 213, 213, 255});
        aligntextcentre(500+(27 * 6)/2, windheight - (128 * 0.8) - (28 * 6) + 40, 22, "In Memories", BEIGE);
    }

    for (int i = 0; i<3; i++){
        drawtrees(assets, 1, 500 + (60*i), windheight-(128*0.8) + 2, 4);
        drawtrees(assets, 4, 1000 + (60*i), windheight-(128*0.8) + 2, 4);
        if (i <= 2){
            drawtrees(assets, 4, 320 + (60*i), windheight-(128*0.8) + 2, 3);
            drawtrees(assets, 5, 270 - (80 * i), 600 - (30 * i), 4);
        }
    }

    switch (displayingtextindex){
        case 0:
            if (player->alivetimer >= player->alivetimercompare){
                sprintf(text, "Your current playthrough alive time is: %.2fs.\n\n\nThis is your all time best.\n\n\nPress -> or <- to continue.", player->alivetimer);
            }else if (player->alivetimer < player->alivetimercompare){
                sprintf(text, "Your current playthrough alive time is: %.2fs.\n\n\nYour all time best is: %.2fs.\n\n\nPress -> or <- to continue.", player->alivetimer, player->alivetimercompare);
            }
            break;

        case 1:
            if (player->accumulatedcurrency >= 2000){
                sprintf(text, "You have accumulated a total of %d coins.\n\n\nYou are truly a one-of-a-kind money collector.\n\n\nPress -> or <- to continue.", player->accumulatedcurrency);
            }else if (player->accumulatedcurrency < 2000 && !player->won){
                sprintf(text, "You have accumulated a total of %d coins.\n\n\nYou're truly one of a kind... at missing money.\n\n\nPress -> or <- to continue.", player->accumulatedcurrency);
            }else if (player->accumulatedcurrency < 2000 && player->won){
                sprintf(text, "You have accumulated a total of %d coins.\n\n\nYou're truly one of a kind... who doesn't need money.\n\n\nPress -> or <- to continue.", player->accumulatedcurrency);
            }
            break;

        case 2:
            if (player->potionused >= 5 && !player->won){
                sprintf(text, "You have used a total of %d potions.\n\n\nPotion count: embarrassing. Were you bathing in them?\n\n\nPress -> or <- to continue.", player->potionused);
            }else if (player->potionused >= 5 && player->won){
                sprintf(text, "You have used a total of %d potions.\n\n\nPotion count: impressive. You are a true alchemist.\n\n\nPress -> or <- to continue.", player->potionused);
            }else if (player->potionused < 5){
                sprintf(text, "You have used a total of %d potions.\n\n\nMinimal potion use? You're built different.\n\n\nPress -> or <- to continue.", player->potionused);
            }
            break;

        case 3:
            if (player->enemykilled >= MAX_ENEMIES - 1){
                sprintf(text, "Your kill count in this run: %d.\n\n\nSlayer stats unlocked: that kill count speaks for itself.\n\n\nPress -> or <- to continue.", player->enemykilled);
            }else if (player->enemykilled < MAX_ENEMIES - 1 && !player->won){
                sprintf(text, "Your kill count in this run: %d.\n\n\nEnemies killed: barely. Did you miss them on purpose?\n\n\nPress -> or <- to continue.", player->enemykilled);
            }else if (player->enemykilled < MAX_ENEMIES - 1 && player->won){
                sprintf(text, "Your kill count in this run: %d.\n\n\nEnemies killed: not bad. You got your priorities right.\n\n\nPress -> or <- to continue.", player->enemykilled);
            }
            break;

        case 4:
            if (player->damagetaken >= 300){
                sprintf(text, "Damage taken: %d. The more you took, the stronger you became.\n\n\n You took everything like a boss.\n\n\nPress -> or <- to continue.", player->damagetaken);
            }else if (player->damagetaken < 300 && !player->won){
                sprintf(text, "Damage taken: %d. You barely took any damage but still die.\n\n\nAre you playing without a shield?\n\n\nPress -> or <- to continue.", player->damagetaken);
            }else if (player->damagetaken < 300 && player->won){
                sprintf(text, "Damage taken: %d. You barely took any damage and won.\n\n\nYou are a true ninja.\n\n\nPress -> or <- to continue.", player->damagetaken);
            }
            break;

        case 5:
            if (player->successfulparry >= 10){
                sprintf(text, "Successful parry this run: %d. Perfect!\n\n\nYou read their move and countered like a master.\n\n\nPress -> or <- to continue.", player->successfulparry);
            }else if (player->successfulparry < 10 && !player->won){
                sprintf(text, "Successful parry this run: %d.\n\n\nMissed that parry by a mile? Better luck next time!\n\n\nPress -> or <- to continue.", player->successfulparry);
            }else if (player->successfulparry < 10 && player->won){
                sprintf(text, "Successful parry this run: %d.\n\n\nYou don't need parry, parry needs you.\n\n\nPress -> or <- to continue.", player->successfulparry);
            }
            break;
    }

    if (IsKeyPressed(KEY_RIGHT)){
        displayingtextindex++;
        if (displayingtextindex > 5){
            displayingtextindex = 0;
        }
    }else if (IsKeyPressed(KEY_LEFT)){
        displayingtextindex--;
        if (displayingtextindex < 0){
            displayingtextindex = 5;
        }
    }
    DrawRectangleRec((Rectangle){220, 30, windwidth - 230, 150}, (Color){210, 210, 210, 160});
    aligntextcentre(windwidth/2 + 100, windheight/2 - 340, 30, text, BLACK);
}


void aligntextcentre(int x, int y, int fontsize, const char* text, Color color) {
    int textWidth = MeasureText(text, fontsize);
    DrawText(text, x - textWidth / 2, y - fontsize/2, fontsize, color);
}

void rotatetextcentre(const char* text, Vector2 position, int fontSize, float rotation, Color color){
    Vector2 origin = {0, 0}; 
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1);
    origin.x = textSize.x / 2;
    origin.y = textSize.y / 2;
    DrawTextPro(GetFontDefault(), text, position, origin, rotation, fontSize, 1, color);
}

void drawtrees(struct GameAssets* assets, int i, int destx, int desty, int scalefactor){
    float dt = GetFrameTime();
    static float leafOffset = -80.0f;
    static float timer = 0.0f; 
    static float leafcooldown = 0.0f;    
    
    if (i == 1){    // grass1
        Rectangle grasswithflowerssrc = {263, 234, 20, 22};
        Rectangle grasswithflowersdest = {destx, desty - (grasswithflowerssrc.height*scalefactor), grasswithflowerssrc.width*scalefactor, grasswithflowerssrc.height*scalefactor};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[10], grasswithflowerssrc, grasswithflowersdest, origin, 0, WHITE);
    }else if (i == 2){
        Rectangle tree2src = {17, 12 , 73, assets->texture[9].height - tree2src.y};
        Rectangle tree2dest = {destx, desty, tree2src.width*scalefactor, tree2src.height*scalefactor};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[9], tree2src, tree2dest, origin, 0, WHITE);
    }else if (i == 3){
        Rectangle pinktreesrc = {245, 12, 72, 100};
        Rectangle pinktreedest = {destx, desty, pinktreesrc.width*scalefactor, pinktreesrc.height*scalefactor};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[10], pinktreesrc, pinktreedest, origin, 0, WHITE);
    }else if (i == 4){
        Rectangle grasssrc2 = {215, 235, 24, 21};
        Rectangle grassdest2 = {destx, desty - (grasssrc2.height*scalefactor), grasssrc2.width*scalefactor, grasssrc2.height*scalefactor};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[10], grasssrc2, grassdest2, origin, 0, WHITE);
    }else if (i == 5){ //falling leaves
        timer += dt;
        if (leafcooldown > 0.0f){
            leafcooldown -= dt;
            return;
        }
        if (timer >= 0.1f && leafcooldown <= 0.0f){
            leafOffset += 1.0f;
            timer = 0.0f;      
        }
        if (leafOffset >= windheight - (128*0.8) - desty){ 
            leafOffset = -80.0f;   
            leafcooldown = 3.0f;   
            return;
        }
        Rectangle fallingleavessrc = {249, 214, 25, 15};
        Rectangle fallingleavesdest = {destx - leafOffset + 40, desty + leafOffset - (fallingleavessrc.height*scalefactor), fallingleavessrc.width*scalefactor, fallingleavessrc.height*scalefactor};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[10], fallingleavessrc, fallingleavesdest, origin, 0, WHITE);
    }else if (i == 6){
        Rectangle noticeboardsrc = {228, 180, 22, 28};
        Rectangle noticeboarddest = {destx, desty - (noticeboardsrc.height*scalefactor), 30 + noticeboardsrc.width*scalefactor, noticeboardsrc.height*scalefactor};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[10], noticeboardsrc, noticeboarddest, origin, 0, WHITE);
    }
}

void Unloadresources(struct GameAssets* assets){
    if (blocksarray != NULL) {
        free(blocksarray);
        blocksarray = NULL;
    }

    printf("%d images to be unloaded....", (assets->imagecount));
    for (int i=0; i<assets->imagecount; i++){
        UnloadImage(assets->images[i]);
    }

    printf("%d musics to be unloaded....", (assets->musiccount));
    for (int i=0; i<assets->musiccount; i++){
        UnloadMusicStream(assets->music[i]);
    }

    printf("%d sounds to be unloaded....", (assets->soundcount));
    for (int i=0; i<assets->soundcount; i++){
        UnloadSound(assets->sound[i]);
    }
    
    printf("%d textures to be unloaded....", (assets->texturecount));
    for (int i=0; i<assets->texturecount; i++){
        if (assets->texture[i].id > 0){ 
            UnloadTexture(assets->texture[i]);
        }
    }
}

void shop(struct GameAssets* assets, struct Playerinfo* player, int* currentGameState, int* currentmusic, int destx, int desty, int scalefactor){
    static int currentframe = 0; 
    static float frametimer = 0.0f; 
    const int totalframes = 6; 
    const float frameDuration = 0.2f; 

    frametimer += GetFrameTime();
    if (frametimer >= frameDuration){
        frametimer = 0.0f;
        currentframe = (currentframe + 1) % totalframes;
    }
    Rectangle shopsrc = {currentframe * (assets->texture[24].width / totalframes), 18, 
                        assets->texture[24].width / totalframes, assets->texture[24].height - 18};
    Rectangle shopdest = {destx, desty, shopsrc.width * scalefactor, shopsrc.height * scalefactor};
    Rectangle shoplampsrc = {0, 0, assets->texture[43].width, assets->texture[43].height};
    Rectangle shoplampdest = {shopdest.x + shopdest.width + 3, shopdest.y + shopdest.height - (assets->texture[43].height * scalefactor) - 42, assets->texture[43].width * scalefactor, 40 + assets->texture[43].height *scalefactor};
    Vector2 origin = {0, 0};
    DrawTexturePro(assets->texture[24], shopsrc, shopdest, origin, 0, WHITE);
    DrawTexturePro(assets->texture[43], shoplampsrc, shoplampdest, origin, 0, WHITE);

    if (player->Position.x + player->width/2 >= destx && player->Position.x + player->width/2 <= destx + shopdest.width && 
        player->Position.y + player->height >= desty && player->Position.y <= desty + shopdest.height) {
        DrawTexturePro(assets->texture[27], shopsrc, shopdest, origin, 0, WHITE);
        DrawRectangleRounded((Rectangle){destx + 30, desty + shopdest.height / 2 - 20, 290, 40}, 20, 0, (Color){255, 255, 255, 100});
        aligntextcentre(destx + shopdest.width / 2, desty + shopdest.height / 2, 30, "Press E to enter", WHITE);
        if (IsKeyPressed(KEY_E)) {
            *currentGameState = SHOP; 
            *currentmusic = 1;
        }
    }
}

void addItemToInventory(struct Playerinfo* player, ItemType itemType){
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 4; col++) {
            if (player->inventoryrow2n3available[row][col] == 0) {
                player->inventoryrow2n3available[row][col] = itemType;
                if (itemType == KEY){
                    player->keycount++;
                }
                return;
            }else if (player->inventoryrow2n3available[row][col] == itemType) {
                if (itemType == KEY){
                    player->keycount++;
                } 
                return;
            }
        }
    }

    if (player->rowspacing >= 2) {
        printf("Inventory is full! Cannot add more items.\n");
        return;
    }

    // Add item to the current slot
    player->inventoryrow2n3available[player->rowspacing][player->row2n3invencount] = itemType;
    player->row2n3invencount++;

    // Move to the next row if the current row is full
    if (player->row2n3invencount >= 4) {
        player->row2n3invencount = 0;
        player->rowspacing++;
    }
}

void drawInventory2n3row(struct GameAssets* assets, struct Playerinfo* player, Rectangle playerinvenboxes[12]) {
    int y = 0; 
    int startIndex = 5; 
    char keyCountText[10];

    for (int i = 4; i < 12; i++) {
        int x = i % 4; 
        if (i % 4 == 0 && i != 0){
            y++; 
        }
        Rectangle slotRect = playerinvenboxes[i];

        switch (player->inventoryrow2n3available[y-1][x]){
            case KEY:
                if (player->keycount > 0){
                    Rectangle keysrc = {0, 0, assets->texture[36].width, assets->texture[36].height};
                    Rectangle keydest = {slotRect.x + 20, slotRect.y + 20, 65, 45};
                    DrawTexturePro(assets->texture[36], keysrc, keydest, (Vector2){0, 0}, 0.0f, WHITE);
                }
                break;
            case ARMOR:
                break;
            case SWORD:
                break;
            default:
                break;
        }
                //20,10,60,80
        switch (player->inventoryrow2n3available[y-1][x]){
            case KEY:
            if (player->keycount > 0){
                sprintf(keyCountText, "x%d", player->keycount);
                DrawText(keyCountText, slotRect.x + 55, slotRect.y + 60, 40, BLACK);
                break;
            }
            case ARMOR:
                break;
            case SWORD:
                break;
            default:
                break;
        }
    }
}

Camera2D Camerasettings(struct Playerinfo* player){
    Camera2D camera;
    //camera.offset = (Vector2){windwidth/2, windheight/2};
   // camera.target = (Vector2){player->Position.x + player->width / 2, player->Position.y + player->height / 2};
    camera.rotation = 0.0f;
    camera.zoom = 1;
    return camera;
}

void updatecamera(Camera2D* camera, struct Playerinfo* player) {
    // Boundaries in which the camera stops moving
    const float cameraLeftBound = windwidth/2;
    const float cameraRightBound = mapwidth - windwidth/2;
    const float cameraTopBound = mapheight + windheight/2;
    const float cameraBottomBound = windheight - windheight/2;

    // Set the camera target to follow the player
    camera->target = (Vector2){player->Position.x + player->width/2,player->Position.y + player->height/2};

    if (camera->target.x < cameraLeftBound) {
        camera->target.x = cameraLeftBound;
    }
    if (camera->target.x > cameraRightBound) {
        camera->target.x = cameraRightBound;
    }
    if (camera->target.y < cameraTopBound) {
        camera->target.y = cameraTopBound;
    }
    if (camera->target.y > cameraBottomBound) {
        camera->target.y = cameraBottomBound;
    }

    camera->offset = (Vector2){windwidth / 2, windheight / 2};
}

void drawbackground (struct GameAssets* assets, Camera2D* camera, int x, float scalefactor){
    switch (x){
        case 1:
            Rectangle skysrc;
            Rectangle skydest;
            for (int i = 0; i < (mapwidth/((assets->texture[6].width+200)*scalefactor)); i++) {
                if (i==0){
                    skysrc = (Rectangle){0, 0, assets->texture[6].width, assets->texture[6].height};
                    skydest = (Rectangle){i * (skysrc.width*scalefactor), -800, (float)(assets->texture[6].width+200)*scalefactor, (float)skysrc.height * scalefactor + 270};  
                }else{
                    skysrc = (Rectangle){0, 0, 1300, assets->texture[6].height};
                    skydest = (Rectangle){i * (assets->texture[6].width*scalefactor), -800, (float)(assets->texture[6].width+200)*scalefactor, (float)skysrc.height * scalefactor + 270};
                }
                Vector2 origin = {0, 0};
                DrawTexturePro(assets->texture[6], skysrc, skydest, origin, 0, WHITE);
                printf("Skydest.y = %.2f\n", skydest.y);
            }
            break;
        case 2:
            for (int i = 0; i < (mapwidth/((assets->texture[7].width + 500)*scalefactor)); i++) {
                Rectangle mountainsrc = {0, 0, assets->texture[7].width, assets->texture[7].height};
                Rectangle mountaindest = {i * (mountainsrc.width+500)*scalefactor, windheight - (128*0.7) - (mountainsrc.height*scalefactor) - 300, (mountainsrc.width + 500)*scalefactor, mountainsrc.height*scalefactor + 300}; // Adjust position and size
                Vector2 origin = {0, 0};
                DrawTexturePro(assets->texture[7], mountainsrc, mountaindest, origin, 0, WHITE);
            }
            break;
        case 3:
            for (int i = 0; i < (mapwidth/((assets->texture[8].width + 500)*scalefactor)); i++) {
                Rectangle mountainsrc2 = {0, 0, assets->texture[8].width, assets->texture[8].height};
                Rectangle mountaindest2 = {i * (mountainsrc2.width+500)*scalefactor, windheight - (128*0.7) - (mountainsrc2.height*scalefactor)  - 300, (mountainsrc2.width + 500)*scalefactor, mountainsrc2.height*scalefactor + 300}; // Adjust position and size
                Vector2 origin = {0, 0};
                DrawTexturePro(assets->texture[8], mountainsrc2, mountaindest2, origin, 0, WHITE);
            }
            break;
        case 4:
            for (int i = 0; i<(mapwidth/(128*scalefactor)); i++){
                Rectangle groundtilessrc = {0, 0, 128, 128};
                Rectangle groundtilesdest = {i * (groundtilessrc.width*scalefactor), windheight - (groundtilessrc.height*scalefactor), (float)(groundtilessrc.width * scalefactor), (float)(groundtilessrc.height * scalefactor)};
                Vector2 origin = {0, 0};
                DrawTexturePro(assets->texture[13], groundtilessrc, groundtilesdest, origin, 0, WHITE);
            }
            break;
            
    }
    
    /*for (int i=0; i<3; i++){ //nightsky
        Rectangle mountainsrc = {112, 32, 64, 32};
        Rectangle mountaindest = {500, 300, 800, 200};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[16], mountainsrc, mountaindest, origin, 0, WHITE);
    }*/
    /*for (int i=0; i<3; i++){ //forest
        Rectangle mountainsrc = {0, 0, assets->texture[11].width, assets->texture[11].height};
        Rectangle mountaindest = {(-windwidth)/2 + (i * windwidth), windheight-mountainsrc.height + 400, windwidth, windheight};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[11], mountainsrc, mountaindest, origin, 0, WHITE);
    }*/


    ///trees, mountain, sky
    //drawtrees(assets, 2, 0, windheight/3, 2); //
}


/// can add potion timer at the side
void drawbuttonsplayingstate(struct GameAssets* assets, Camera2D* camera, Gamestate* currentGameState, struct Playerinfo* Playerdata){
    Vector2 mousePos = GetMousePosition();
    Rectangle currencyrec = {camera->target.x-570, camera->target.y-290, 200, 30};
    Rectangle currencybar = {currencyrec.x, currencyrec.y, 0.1*Playerdata->currency, currencyrec.height};
    Rectangle moneybagsrc = {0, 10, assets->texture[32].width/8, 21};
    Rectangle moneybagdest = {currencyrec.x - 5, currencyrec.y - 8, 55, 46};
    Rectangle pausebuttonsrc = {65, 153, 90, 95};
    Rectangle pausebuttondest = {camera->target.x + 400, camera->target.y - 350, 76, 76};
    Rectangle pausebuttondestcollision = {1000, 50, pausebuttondest.width, pausebuttondest.height};
    Rectangle settingsbuttonsrc = {67, 270, 90, 96};
    Rectangle settingsbuttondest = {pausebuttondest.x + 90, pausebuttondest.y, 76, 76};
    Rectangle settingsbuttondestcollision = {pausebuttondestcollision.x + 90, pausebuttondestcollision.y, 70, 70};

    float dt = GetFrameTime();

    DrawRectangleRec(currencyrec, WHITE);
    DrawRectangleRec(currencybar, GOLD);
    DrawTexturePro(assets->texture[32], moneybagsrc, moneybagdest, (Vector2){0,0}, 0, WHITE);
    char currencyText[30];
    sprintf(currencyText, "Coins: %d/2000", Playerdata->currency);
    aligntextcentre(currencyrec.x + currencyrec.width / 2, currencyrec.y + currencyrec.height / 2, 20, currencyText, BLACK);

    if (mousePos.x >= pausebuttondestcollision.x && mousePos.x <= pausebuttondestcollision.x + pausebuttondestcollision.width
        && mousePos.y >= pausebuttondestcollision.y && mousePos.y <= pausebuttondestcollision.y + pausebuttondestcollision.height){
        pausebuttonsrc.x = 182;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            *currentGameState = PAUSE; 
        }
    }
    if (mousePos.x >= settingsbuttondestcollision.x && mousePos.x <= settingsbuttondestcollision.x + settingsbuttondestcollision.width
        && mousePos.y >= settingsbuttondestcollision.y && mousePos.y <= settingsbuttondestcollision.y + settingsbuttondestcollision.height){
        settingsbuttonsrc.x = 182;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            *currentGameState = OPTIONS; 
        }
    }

    DrawTexturePro(assets->texture[31], pausebuttonsrc, pausebuttondest, (Vector2){0, 0}, 0.0f, WHITE);
    DrawTexturePro(assets->texture[31], settingsbuttonsrc, settingsbuttondest, (Vector2){0,0}, 0.0f, WHITE);

    if ((Playerdata->jumpboostshdact || Playerdata->attackboostshdact || Playerdata->defenseboostshdact) && Playerdata->potioneffect < 20.0f){
        Rectangle potionsrc = {Playerdata->activepotiontype * (assets->texture[29].width / 4), 0, assets->texture[29].width / 4,assets->texture[29].height / 2};
        Rectangle potiondest = {camera->target.x + 490, camera->target.y -240, 60, 80};
        float potioneffecttime = 20.0f; 
        float remainingtime = potioneffecttime - Playerdata->potioneffect;
        float startangle = 270.0f; 
        float endangle = startangle + (remainingtime / potioneffecttime) * 360.0f;
        float timerradius = 50.0f; 
        char timerText[20];
        Vector2 timerPosition = {camera->target.x + 520, camera->target.y - 200};

        DrawCircleSector(timerPosition, timerradius, 0, 360, 80, (Color){0, 0, 0, 50});
        DrawTexturePro(assets->texture[29], potionsrc, potiondest, (Vector2){0,0}, 0, WHITE);
        DrawCircleSector(timerPosition, timerradius, startangle, endangle, 80, (Color){0, 0, 0, 50});  
        sprintf(timerText, "%.0fs", remainingtime);
        DrawText(timerText, timerPosition.x - MeasureText(timerText, 20) / 2, timerPosition.y - 10, 20, WHITE);
    }

    if (Playerdata->attackboostshdact){
        Playerdata->potioneffect += dt;
        if (Playerdata->potioneffect < 20.0f){
            Playerdata->playerdamage *= 1.3;
        }else{
            Playerdata->playerdamage = 1;
            Playerdata->attackboostshdact = false;
            Playerdata->potioneffect = 0.0f;
        }
    }else if (Playerdata->defenseboostshdact){
        Playerdata->potioneffect += dt;
        if (Playerdata->potioneffect < 10.0f){
            Playerdata->playerdefense = 10;             /////////// change the defense value
        }else{
            Playerdata->playerdefense = 0;
            Playerdata->defenseboostshdact = false;
            Playerdata->potioneffect = 0.0f;
        }
    }
}

void shopstateanimation(struct GameAssets* assets){
    static int framescount = 0;
    static int currentframecount = 0;
    int frametimer = 10;
    currentframecount++;

    if (currentframecount % (frametimer) == 1) {
        framescount++;
    }

    if (framescount >= 8) {
        framescount = 0;
    }

    Rectangle moneybagsrc = {framescount * (assets->texture[32].width/8), 10, assets->texture[32].width/8, 21};
    Rectangle moneybagdest = {30, 720, 100, 70};
    DrawTexturePro(assets->texture[32], moneybagsrc, moneybagdest, (Vector2){0,0}, 0, WHITE);
}

void collisionplayerblocks(char axis, struct Playerinfo* object, int* maxplatform, int* facedirection) {
    //static int blockindex = -1;
    Rectangle player = {object->Position.x, object->Position.y, object->width, object->height};

    // **Hitboxes for Different Collisions**
    Rectangle feetHitbox = {object->Position.x + object->width * 0.4, 
                           object->Position.y + object->height - 4, 
                           object->width * 0.3, 4};  // offset the position by 4 pixels as the feet is only 4 pixels tall

    Rectangle headHitbox = {object->Position.x + object->width * 0.2, 
                              object->Position.y, 
                              object->width * 0.6, 4}; 

    Rectangle leftHitbox = {object->Position.x, 
                            object->Position.y + object->height * 0.2, 
                            4, object->height * 0.6}; 

    Rectangle rightHitbox = {object->Position.x + object->width - 4, 
                             object->Position.y + object->height * 0.2, 
                             4, object->height * 0.6}; 

    if (*facedirection == -1) {  // Assuming -1 means facing left
        feetHitbox.x -= 12; // Adjust slightly when facing left
    }

    object->onplatform = false;
    for (int i = 0; i < *maxplatform; i++) {
        // Feet collision with the platform
        if (axis == 'y' && CheckCollisionRecs(feetHitbox, blocksarray[i].rect)){
            object->onplatform = true;
            //blockindex = i; 

            if (object->velocityY >= 0) { // Falling down
                object->Position.y = blocksarray[i].rect.y - object->height;
                object->velocityY = 0;
                object->isfalling = false;
                object->isJumping = false;
            }
        }
        // Head collision with the platform
        if (axis == 'y' && CheckCollisionRecs(headHitbox, blocksarray[i].rect)) {
            if (object->velocityY < 0) { // Moving upwards
                object->Position.y = blocksarray[i].rect.y + blocksarray[i].rect.height;
                object->velocityY = 0;
                object->isfalling = true;
            }
        }

        // Left side collision
        if (axis == 'x' && CheckCollisionRecs(leftHitbox, blocksarray[i].rect)) {
            if (object->direction.x < 0) { 
                object->Position.x = blocksarray[i].rect.x + blocksarray[i].rect.width;
            }
        }

        // Right side collision
        if (axis == 'x' && CheckCollisionRecs(rightHitbox, blocksarray[i].rect)) {
            if (object->direction.x > 0) {
                object->Position.x = blocksarray[i].rect.x - object->width;
            }
        }
    }

    if (object->onplatform && axis == 'y'){
        object->onground = false;
    }
    //printf("DEBUG for playerblock: onplatform=%d, onground=%d, isfalling=%d\n", object->onplatform, object->onground, object->isfalling);

}

int calculatemovementplayer(struct Playerinfo *player, int* maxplatform, struct GameAssets *assets, Gamestate currentgamestate) {
    static int facedirection = 1;
    float dt = GetFrameTime();
    float speed = 300.0f;
    float gravity = 1000.0f;
    static float timelapsed = 0.0f;

    if (currentgamestate != PLAYING){
        return facedirection;
    }

    if(player->currenthp <= 0){ 
        return facedirection;
    }

    if (IsKeyDown(KEY_LEFT_SHIFT) && !player->isJumping && !player->isfalling){ //ensure that the character will stop when hes holding shield
        player->onshield = true;
        player->animationstate = 5;
        player->speed = 0;
    } else {
        player->onshield = false;
    }

    if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !player->isJumping) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && player->isfalling)) {
        player->attack = true;
        //player->animationstate = 4; // Attack animation state
    }

    if (player->attack){
        player->animationstate = 4;
    }

    if (!IsKeyPressed(KEY_SPACE) && !IsKeyDown(KEY_A) && !IsKeyDown(KEY_D) && !player->attack && !player->onshield){ 
        player->animationstate = 0; 
    }

    if (player->jumpboostshdact){
        player->potioneffect += dt;
    }
    if (IsKeyPressed(KEY_SPACE) && !player->isJumping){
        player->onground = false;
        player->onplatform = false;
        if (player->jumpboostshdact){ 
            if (!player->jumpboostactivated){
                player->jumpboost = -700 * 1.5; //x1.5 jump height
                player->jumpboostactivated = true;
            }
            if (player->potioneffect >= 20.0f){
                player->jumpboost = -700.0f;
                player->potioneffect = 0.0f;
                player->jumpboostshdact = false;
                player->jumpboostactivated = false;
            }
        }else{
            player->jumpboost = -700.0f; // Normal jump height
        }
        player->velocityY = player->jumpboost;
        player->isJumping = true;
        player->isfalling = false;
        player->animationstate = 2;  // Jumping state
        player->isrunning = false;
    }
    if (player->isJumping && !player->attack) {
        player->animationstate = 2;  // Ensure jump state stays
    }

    player->direction.x = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A));

    if (player->direction.x > 0) {
        facedirection = 1;
    } else if (player->direction.x < 0) {
        facedirection = -1;
    }

    player->Position.x += dt * speed * player->direction.x;
    collisionplayerblocks('x', player, maxplatform, &facedirection);

    if ((!player->onplatform && !player->onground) && (!player->attack) &&!player->onshield) {  
        player->isrunning = false;
        player->velocityY += gravity * dt;

        if (player->velocityY > 0) {
            player->isfalling = true;
            player->animationstate = 3; //falling animation
        } else {
            player->isfalling = false;
        }
    }

    if (!player->isJumping && !player->onground && !player->onplatform) {
        player->isfalling = true;
    } else if (player->velocityY == 0) {
        player->isfalling = false;
    }
    
    player->Position.y += player->velocityY * dt;
    collisionplayerblocks('y', player, maxplatform, &facedirection);


    keepobjectwithinscreen(player, assets);
    //printf("DEBUG: isfalling=%d, isjumping=%d, velocityY=%.2f\n", 
        //player->isfalling, player->isJumping, player->velocityY);
    if (player->animationstate == 2) { // If currently in a jump animation, let it finish before changing state
        return facedirection;
    }

    if ((player->onplatform||player->onground) && !player->attack && !player->onshield && !player->isJumping && !player->isfalling) {
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_D)) 
        {
            if (IsKeyDown(KEY_A) && IsKeyDown(KEY_D)) //make sure that when both keys pressed at the same time character doesn't move
            {
                player->animationstate = 0;
                player->isrunning = false;
            }
            else if (IsKeyDown(KEY_A) || IsKeyDown(KEY_D))
            {
                player->animationstate = 1; // Running
                player->isrunning = true;
            }
        } 
        else
        {
            player->animationstate = 0; // Idle
            player->isrunning = false;
        }
    }
    //printf("DEBUG for playerblock: onplatform=%d, onground=%d, isfalling=%d, animationstate=%d\n", player->onplatform, player->onground, player->isfalling, player->animationstate);
    if (player->direction.x > 0) {
         facedirection = 1;
    } else if (player->direction.x < 0) {
        facedirection = -1;
    }
    return facedirection;
}

void LoadAnimationDataplayer(struct GameAssets* assets) {
    FILE *file = fopen("shieldgirlpos.txt", "r");
    if (file == NULL) {
        printf("Error: Cannot open file for reading!\n");
    }

    char line[100];  // Buffer for reading lines

    while (fgets(line, sizeof(line), file)) 
    {

        if (strstr(line, "idle animations:")) 
        {
            fscanf(file, "srcx = %f, %f, %f, %f\n", &assets->src_idlex[0], &assets->src_idlex[1], &assets->src_idlex[2], &assets->src_idlex[3]);
            fscanf(file, "srcy = %f, %f, %f, %f\n", &assets->src_idley[0], &assets->src_idley[1], &assets->src_idley[2], &assets->src_idley[3]);
        }

        if (strstr(line, "walking animations:")) 
        {
            fscanf(file, "srcx = %f, %f, %f, %f, %f, %f, %f\n", &assets->src_runningx[0], &assets->src_runningx[1], 
                &assets->src_runningx[2], &assets->src_runningx[3], &assets->src_runningx[4], &assets->src_runningx[5], &assets->src_runningx[6]);
            fscanf(file, "width = %f, %f, %f, %f, %f, %f, %f\n", &assets->src_runningwidth[0], &assets->src_runningwidth[1], 
                &assets->src_runningwidth[2], &assets->src_runningwidth[3], &assets->src_runningwidth[4], &assets->src_runningwidth[5], &assets->src_runningwidth[6]);
            fscanf(file, "srcy = %f, %f, %f, %f, %f, %f, %f\n", &assets->src_runningy[0], &assets->src_runningy[1], 
                &assets->src_runningy[2], &assets->src_runningy[3], &assets->src_runningy[4], &assets->src_runningy[5], &assets->src_runningy[6]);
        } 

        if (strstr(line, "falling animations:")) 
        {
            fscanf(file, "srcx = %f, %f, %f\n", &assets->src_fallingx[0], &assets->src_fallingx[1], &assets->src_fallingx[2]);
            fscanf(file, "srcy = %f, %f, %f\n", &assets->src_fallingy[0], &assets->src_fallingy[1], &assets->src_fallingy[2]);
        }

        if (strstr(line, "jumping animations:")) 
        {
            fscanf(file, "srcx = %f, %f, %f, %f, %f, %f\n", &assets->src_jumpingx[0], &assets->src_jumpingx[1], 
                    &assets->src_jumpingx[2], &assets->src_jumpingx[3], &assets->src_jumpingx[4], &assets->src_jumpingx[5]);
            fscanf(file, "width = %f, %f, %f, %f, %f, %f\n", &assets->src_jumpingwidth[0], &assets->src_jumpingwidth[1], 
                    &assets->src_jumpingwidth[2], &assets->src_jumpingwidth[3], &assets->src_jumpingwidth[4], &assets->src_jumpingwidth[5]);
            fscanf(file, "srcy = %f, %f, %f, %f, %f, %f\n", &assets->src_jumpingy[0], &assets->src_jumpingy[1], 
                    &assets->src_jumpingy[2], &assets->src_jumpingy[3], &assets->src_jumpingy[4], &assets->src_jumpingy[5]);
            fscanf(file, "height = %f, %f, %f, %f, %f, %f\n", &assets->src_jumpingheight[0], &assets->src_jumpingheight[1], 
                    &assets->src_jumpingheight[2], &assets->src_jumpingheight[3], &assets->src_jumpingheight[4], &assets->src_jumpingheight[5]);
        }

        if (strstr(line, "attacking animations:")) 
        {
            fscanf(file, "srcx = %f, %f, %f, %f, %f, %f\n", &assets->src_attackingx[0], &assets->src_attackingx[1], 
                    &assets->src_attackingx[2], &assets->src_attackingx[3], &assets->src_attackingx[4], &assets->src_attackingx[5]);
            fscanf(file, "srcy = %f, %f, %f, %f, %f, %f\n", &assets->src_attackingy[0], &assets->src_attackingy[1], 
                    &assets->src_attackingy[2], &assets->src_attackingy[3], &assets->src_attackingy[4], &assets->src_attackingy[5]);
            fscanf(file, "width = %f, %f, %f, %f, %f, %f\n", &assets->src_attackingwidth[0], &assets->src_attackingwidth[1], 
                    &assets->src_attackingwidth[2], &assets->src_attackingwidth[3], &assets->src_attackingwidth[4], &assets->src_attackingwidth[5]);
        }

        if (strstr(line, "shield animations:")) 
        {
            fscanf(file, "srcx = %f, %f, %f, %f, %f, %f, %f\n", &assets->src_shieldx[0], &assets->src_shieldx[1], &assets->src_shieldx[2], 
                    &assets->src_shieldx[3], &assets->src_shieldx[4], &assets->src_shieldx[5], &assets->src_shieldx[6]);
            fscanf(file, "srcy = %f, %f, %f, %f, %f, %f, %f\n", &assets->src_shieldy[0], &assets->src_shieldy[1], 
                    &assets->src_shieldy[2], &assets->src_shieldy[3], &assets->src_shieldy[4], &assets->src_shieldy[5], &assets->src_shieldy[6]);
            fscanf(file, "width = %f, %f, %f, %f, %f, %f, %f\n", &assets->src_shieldwidth[0], &assets->src_shieldwidth[1], &assets->src_shieldwidth[2],
                    &assets->src_shieldwidth[3], &assets->src_shieldwidth[4], &assets->src_shieldwidth[5], &assets->src_shieldwidth[6]);
        } 

        if (strstr(line, "dying animations:")) 
        {
            fscanf(file, "srcx = %f, %f, %f, %f, %f\n", &assets->src_dyingx[0], &assets->src_dyingx[1], &assets->src_dyingx[2], 
                    &assets->src_dyingx[3], &assets->src_dyingx[4]);
            fscanf(file, "srcy = %f, %f, %f, %f, %f\n", &assets->src_dyingy[0], &assets->src_dyingy[1], &assets->src_dyingy[2], 
                    &assets->src_dyingy[3], &assets->src_dyingy[4]);
            fscanf(file, "width = %f, %f, %f, %f, %f\n", &assets->src_dyingwidth[0], &assets->src_dyingwidth[1], &assets->src_dyingwidth[2],
                    &assets->src_dyingwidth[3], &assets->src_dyingwidth[4]);
        } 
    }
    fclose(file);
}

void iterateanimationplayer(Gamestate* currentGameState, struct GameAssets* assets, struct Playerinfo* player, int* currentframecount, int* facedirection) {
    int framescount = 0;
    int frametimer = 0;
    Texture2D texture;


    if (player->animationstate == 0) //idle
    {
        framescount = 4; 
        frametimer = 15; 
    } 
    else if (player->animationstate == 1) //walking
    {
        framescount = 7; 
        frametimer = 12;
    } 
    else if (player->animationstate == 2) //jumping
    {
        framescount = 6; 
        frametimer = 13;
    }
    else if (player->animationstate == 3) //jumping
    {
        framescount = 3; 
        frametimer = 5;
    }

    else if (player->animationstate == 4){ //attacking
        framescount = 6;
        frametimer = 8;
    }

    else if (player->animationstate == 5){ //shield
        framescount = 7;
        frametimer = 4;
    }
    
    else if (player->animationstate == 6){ //dying
        player->deadtimer += GetFrameTime();
        framescount = 5;
        frametimer = 30;
    }

    if (player->deadtimer > 5.0f){
        *currentGameState = GAMEOVER;
    }
    (*currentframecount)++;

    if (*currentframecount % (frametimer) == 1) {
        (player->animationindex)++;
        *currentframecount = 2;
    }

    if (player->animationindex >= framescount){
        if (player->animationstate == 4) { // If attacking, reset attack state after animation completes
            player->animationindex = 0;
            player->attack = false; // Reset attack flag
        }
        else if (player->animationstate == 5) 
        {
            player->animationindex = 6;
        }else if (player->animationstate == 6 && player->deadtimer <= 8.0f){
            player->animationindex = 4;
        }else 
        {
            player->animationindex = 0;
            if (player->animationstate == 2 && player->isJumping) //let the character jumping frame to iterate till it finishes jumping
            {
                player->animationstate = 2;
                player->animationindex = 3;
            }
        }

    }

    //printf("AnimationState: %d, isFalling: %d, isRunning: %d, onShield: %d\n", 
        //player->animationstate, player->isfalling, player->isrunning, player->onshield);

    Rectangle sourceRect, destRect;

    if (player->animationstate == 0) {  // Idle Animation
        texture = assets->texture[1];
        sourceRect = (Rectangle){assets->src_idlex[player->animationindex], assets->src_idley[player->animationindex], 50, 61};
    }else if (player->animationstate == 1) // Running Animation
    {  
        texture = assets->texture[0];
        sourceRect = (Rectangle){assets->src_runningx[player->animationindex], assets->src_runningy[player->animationindex], assets->src_runningwidth[player->animationindex],
                                 assets->texture[0].height - assets->src_runningy[player->animationindex]};
    }else if (player->animationstate == 2) // Jumping Animation
    {  
        texture = assets->texture[3];
        sourceRect = (Rectangle){assets->src_jumpingx[player->animationindex], assets->src_jumpingy[player->animationindex], assets->src_jumpingwidth[player->animationindex], assets->src_jumpingheight[player->animationindex]};
    }else if (player->animationstate == 3) // falling Animation
    {
        texture = assets->texture[2];
        sourceRect = (Rectangle){assets->src_fallingx[player->animationindex], assets->src_fallingy[player->animationindex], 50, 57};
    }else if (player->animationstate == 4){
        player->attack = true;
        texture = assets->texture[4];
        sourceRect = (Rectangle){assets->src_attackingx[player->animationindex], assets->src_attackingy[player->animationindex], 
                      assets->src_attackingwidth[player->animationindex], assets->texture[4].height - assets->src_attackingy[player->animationindex]};
    
        //printf("Attack Animation Frame: %d, Attack Flag: %d\n", *i, player->attack); // Debugging print
        if (player->animationindex == 5){
            player->animationindex = 0;
            player->attack = false; //think abt how to make the fifth frame iterate finish 
        }
    }else if (player->animationstate == 5) // falling Animation
    {
        texture = assets->texture[5];
        sourceRect = (Rectangle){assets->src_shieldx[player->animationindex], assets->src_shieldy[player->animationindex], assets->src_shieldwidth[player->animationindex], 
                                 assets->texture[5].height - assets->src_shieldy[player->animationindex]};
    }
    player->width = 100;
    player->height = 100;
    destRect = (Rectangle){player->Position.x, player->Position.y, player->width, player->height};

    if (player->animationstate == 6) // dying Animation
    {
        texture = assets->texture[40];
        sourceRect = (Rectangle){assets->src_dyingx[player->animationindex], assets->src_dyingy[player->animationindex], assets->src_dyingwidth[player->animationindex], 
                                 assets->texture[40].height - assets->src_dyingy[player->animationindex]};
        
        player->width += 10.0f * player->animationindex;
        player->height -= 10.0f * player->animationindex;
        destRect = (Rectangle){player->Position.x, player->Position.y + (10.0f * player->animationindex), player->width, player->height};
    }

    if (*facedirection < 0) {
        sourceRect.width = -fabs(sourceRect.width);
        destRect.width = fabs(destRect.width);
        destRect.x = player->Position.x; 
    }else{
        sourceRect.width = fabs(sourceRect.width);
        destRect.width = fabs(destRect.width);
        destRect.x = player->Position.x;  
    }

    Vector2 origin = {0, 0};
    DrawTexturePro(texture, sourceRect, destRect, origin, 0.0f, WHITE);

}

void savegamedata(struct Playerinfo* Playerdata, struct GameAssets* assets, Gamestate* currentGameState, int* currentmusic, float musicVolume, int enemycount, struct Playerinfo enemies[MAX_ENEMIES], Camera2D* camera){
    FILE *file = fopen("savegame.txt", "w");
    if (file) {
        fprintf(file, "PlayerPositionX=%.2f\n", Playerdata->Position.x);
        fprintf(file, "PlayerPositionY=%.2f\n", Playerdata->Position.y);
        fprintf(file, "Playeranimationstate=%d\n", Playerdata->animationstate);
        fprintf(file, "PlayerIsJumping=%d\n", Playerdata->isJumping);
        fprintf(file, "PlayerIsFalling=%d\n", Playerdata->isfalling);
        fprintf(file, "PlayerVelocityY=%.2f\n", Playerdata->velocityY);
        fprintf(file, "PlayerOnGround=%d\n", Playerdata->onground);
        fprintf(file, "PlayerOnPlatform=%d\n", Playerdata->onplatform);
        fprintf(file, "PlayerHealth=%d\n", Playerdata->currenthp);
        fprintf(file, "PlayerMaxHealth=%d\n", Playerdata->hitpoints);
        fprintf(file, "Playerdead=%d\n", Playerdata->dead);
        fprintf(file, "Playerdeadtimer=%.2f\n", Playerdata->deadtimer);
        fprintf(file, "Playeralivetimer=%.2f\n", Playerdata->alivetimer);
        fprintf(file, "Playerattackdamage=%d\n", Playerdata->playerdamage);
        fprintf(file, "Playerdefense=%d\n", Playerdata->playerdefense);
        fprintf(file, "Playerjumpheight=%d\n", Playerdata->jumpboost);
        fprintf(file, "PlayerCurrency=%d\n", Playerdata->currency);
        fprintf(file, "Playeraccumulatedcurrency=%d\n", Playerdata->accumulatedcurrency);
        fprintf(file, "MusicVolume=%.2f\n", musicVolume);
        fprintf(file, "EnemyCount=%d\n", enemycount);
        for (int i = 0; i < enemycount; i++) {
            fprintf(file, "Enemy[%d]PositionX=%.2f\n", i, enemies[i].Position.x);
            fprintf(file, "Enemy[%d]PositionY=%.2f\n", i, enemies[i].Position.y);
            fprintf(file, "Enemy[%d]Health=%d\n", i, enemies[i].hitpoints);
            fprintf(file, "Enemy[%d]Dead=%d\n", i, enemies[i].dead);
            fprintf(file, "Enemy[%d]isremoved=%d\n", i, enemies[i].deadenemyremoved);
        }
        fprintf(file, "CameraTargetX=%.2f\n", camera->target.x);
        fprintf(file, "CameraTargetY=%.2f\n", camera->target.y);
        fprintf(file, "CameraZoom=%.2f\n", camera->zoom);
        for (int i = 0; i < 4; i++){
            fprintf(file, "Potion[%d]price: %d\n", i+1, assets->potionprice[i]);
            fprintf(file, "Potion[%d]bought: %d\n", i+1, Playerdata->potionbought[i]);
            fprintf(file, "Potion[%d]remaining: %d\n", i+1, assets->potionleftinshop[i]);
        }
        fprintf(file, "Potionused=%d\n", Playerdata->potionused);

        for (int i = 2; i<=3; i++){
            for (int j = 0; j < 4; j++){
                fprintf(file, "Playerinvenrow%d[%d]=%d\n", i, j, Playerdata->inventoryrow2n3available[i-2][j]);
            }
        }
        fprintf(file, "Jumpboostactive=%d\n", Playerdata->jumpboostshdact);
        fprintf(file, "Attackboostactive=%d\n", Playerdata->attackboostshdact);
        fprintf(file, "Defenseboostactive=%d\n", Playerdata->defenseboostshdact);
        fprintf(file, "PotionCount=%d\n", Playerdata->potioncount); 
        for (int i = 0; i < Playerdata->potioncount; i++){
            fprintf(file, "PotionOrder[%d]=%d\n", i, Playerdata->potionorder[i]);
            fprintf(file, "PotionBought[%d]=%d\n", i, Playerdata->potionbought[Playerdata->potionorder[i]]);
        }
        fprintf(file, "Potioneffecttimer=%.2f\n", Playerdata->potioneffect); 
        fprintf(file, "Potionactivetype=%d\n", Playerdata->activepotiontype);
        fprintf(file, "Numberofkeys=%d\n", Playerdata->keycount);
        fprintf(file, "Doorkeyinserted=%d\n", assets->doorkeyinsertedcount);
        for (int i = 0; i<4; i++){
            fprintf(file, "Doorkey[%d]inserted=%d\n", i + 1, assets->doorkeyinserted[i]);
            fprintf(file, "Doorkey[%d]circledrawn=%d\n", i + 1, assets->doorkeycircledrawn[i]);
        }
        fprintf(file, "Shopkeycount=%d\n", assets->shopkeycount);
        fprintf(file, "Playersuccessfulparry=%d\n", Playerdata->successfulparry);
        fprintf(file, "Playerdamagetaken=%d\n", Playerdata->damagetaken);

        fclose(file);
    }
}

void loadgamedata(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, int* enemycount, struct Playerinfo enemies[MAX_ENEMIES], Camera2D* camera) {
    FILE *file = fopen("savegame.txt", "r");
    char line[100];
    int enemyIndex, valueInt, potionnum, doorkeynum;
    float valueFloat;
    if (!file){
        printf("Error: No save file found. Starting a new game.\n");
        return;
    }

    while (fgets(line, sizeof(line), file)){
        if (strstr(line, "PlayerPositionX=")){
            sscanf(line, "PlayerPositionX=%f", &Playerdata->Position.x);
        }else if (strstr(line, "PlayerPositionY=")){
            sscanf(line, "PlayerPositionY=%f", &Playerdata->Position.y);
        }else if (strstr(line, "Playeranimationstate=")){
            sscanf(line, "Playeranimationstate=%d", &Playerdata->animationstate);
        }else if (strstr(line, "PlayerIsJumping=")){
            sscanf(line, "PlayerIsJumping=%d", &Playerdata->isJumping);
        }else if (strstr(line, "PlayerIsFalling=")){
            sscanf(line, "PlayerIsFalling=%d", &Playerdata->isfalling);
        }else if (strstr(line, "PlayerVelocityY=")){
            sscanf(line, "PlayerVelocityY=%f", &Playerdata->velocityY);
        }else if (strstr(line, "PlayerOnGround=")){
            sscanf(line, "PlayerOnGround=%d", &Playerdata->onground);
        }else if (strstr(line, "PlayerOnPlatform=")){
            sscanf(line, "PlayerOnPlatform=%d", &Playerdata->onplatform);
        }else if (strstr(line, "PlayerHealth=")){
            sscanf(line, "PlayerHealth=%d", &Playerdata->currenthp);
        }else if (strstr(line, "PlayerMaxHealth=")){
            sscanf(line, "PlayerMaxHealth=%d", &Playerdata->hitpoints);
        }else if (strstr(line, "Playerdead=")){
            sscanf(line, "Playerdead=%d", &Playerdata->dead);
        }else if (strstr(line, "Playerdeadtimer=")){
            sscanf(line, "Playerdeadtimer=%d", &Playerdata->deadtimer);
        }else if (strstr(line, "Playeralivetimer=")){
            sscanf(line, "Playeralivetimer=%.2f", &Playerdata->alivetimer);
        }else if (strstr(line, "Playerattackdamage=")){
            sscanf(line, "Playerattackdamage=%d", &Playerdata->playerdamage);
        }else if (strstr(line, "Playerdefense=")){
            sscanf(line, "Playerdefense=%d", &Playerdata->playerdefense);
        }else if (strstr(line, "Playerjumpheight=")){
            sscanf(line, "Playerjumpheight=%d", &Playerdata->jumpboost);
        }else if (strstr(line, "PlayerCurrency=")){
            sscanf(line, "PlayerCurrency=%d", &Playerdata->currency);
        }else if (strstr(line, "Playeraccumulatedcurrency=")){
            sscanf(line, "Playeraccumulatedcurrency=%d", &Playerdata->accumulatedcurrency);
        }else if (strstr(line, "MusicVolume=")){
            sscanf(line, "MusicVolume=%f", musicVolume);
        }else if (strstr(line, "EnemyCount=")){
            sscanf(line, "EnemyCount=%d", enemycount);
        }else if (sscanf(line, "Enemy[%d]PositionX=%f", &enemyIndex, &valueFloat) == 2 && enemyIndex < MAX_ENEMIES){
            enemies[enemyIndex].Position.x = valueFloat;
        }else if (sscanf(line, "Enemy[%d]PositionY=%f", &enemyIndex, &valueFloat) == 2 && enemyIndex < MAX_ENEMIES){
            enemies[enemyIndex].Position.y = valueFloat;
        }else if (sscanf(line, "Enemy[%d]Health=%d", &enemyIndex, &valueInt) == 2 && enemyIndex < MAX_ENEMIES){
            enemies[enemyIndex].hitpoints = valueInt;
        }else if (sscanf(line, "Enemy[%d]Dead=%d", &enemyIndex, &valueInt) == 2 && enemyIndex < MAX_ENEMIES){
            enemies[enemyIndex].dead = valueInt;
        }else if(sscanf(line, "Enemy[%d]isremoved=%d", &enemyIndex, &valueInt) == 2 && enemyIndex < MAX_ENEMIES){
            enemies[enemyIndex].deadenemyremoved = valueInt;
        }else if (strstr(line, "CameraTargetX=")){
            sscanf(line, "CameraTargetX=%f", &camera->target.x);
        }else if (strstr(line, "CameraTargetY=")){
            sscanf(line, "CameraTargetY=%f", &camera->target.y);
        }else if (strstr(line, "CameraZoom=")){
            sscanf(line, "CameraZoom=%f", &camera->zoom);
        }

        for (int i = 0; i < 4; i++) {
            char potionPriceKey[30], potionBoughtKey[30], potionLeftKey[30];
            sprintf(potionPriceKey, "Potion[%d]price:", i + 1);
            sprintf(potionBoughtKey, "Potion[%d]bought:", i + 1);
            sprintf(potionLeftKey, "Potion[%d]remaining:", i + 1);

            if (strstr(line, potionPriceKey)){
                sscanf(line, "Potion[%d]price: %d", &i, &assets->potionprice[i]);
            } else if (strstr(line, potionBoughtKey)){
                sscanf(line, "Potion[%d]bought: %d", &i, &Playerdata->potionbought[i]);
            } else if (strstr(line, potionLeftKey)){
                sscanf(line, "Potion[%d]remaining: %d", &i, &assets->potionleftinshop[i]);
            }
        }
        if (strstr(line, "Potionused=")){
            sscanf(line, "Potionused=%d", &Playerdata->potionused);
        }
        for (int i = 2; i<=3; i++){
            for (int j = 0; j < 4; j++){
                char inventoryKey[30];
                sprintf(inventoryKey, "Playerinvenrow%d[%d]", i, j);
                if (strstr(line, inventoryKey)){
                    sscanf(line, "Playerinvenrow%d[%d]=%d", &i, &j, &Playerdata->inventoryrow2n3available[i-2][j]);
                }
            }
        }
/////////somehow when load gamedata the character is floating
        if (strstr(line, "Jumpboostactive=")){
            sscanf(line, "Jumpboostactive=%d", &Playerdata->jumpboostshdact);
        } else if (strstr(line, "Attackboostactive=")){
            sscanf(line, "Attackboostactive=%d", &Playerdata->attackboostshdact);
        } else if (strstr(line, "Defenseboostshdact=")){
            sscanf(line, "Defenseboostactive=%d", &Playerdata->defenseboostshdact);
        }
        if (strstr(line, "PotionCount=")){
            sscanf(line, "PotionCount=%d", &Playerdata->potioncount);
        }
        for (int i = 0; i < Playerdata->potioncount; i++){
            if (strstr(line, "PotionOrder")){
                int index, order;
                sscanf(line, "PotionOrder[%d]=%d", &index, &order);
                Playerdata->potionorder[index] = order;
            }
            if (strstr(line, "PotionBought")){
                int index, bought;
                sscanf(line, "PotionBought[%d]=%d", &index, &bought);
                Playerdata->potionbought[Playerdata->potionorder[index]] = bought;
            }
        } 
        if (strstr(line, "Potioneffecttimer=")){
            sscanf(line, "Potioneffecttimer=%f", &Playerdata->potioneffect);
        }else if (strstr(line, "Potionactivetype=")){
            sscanf(line, "Potionactivetype=%d", &Playerdata->activepotiontype);
        }else if (strstr(line, "Numberofkeys=")){
            sscanf(line, "Numberofkeys=%d", &Playerdata->keycount);
        }else if (strstr(line, "Doorkeyinserted=")){
            sscanf(line, "Doorkeyinserted=%d", &assets->doorkeyinsertedcount);
        }else if (sscanf(line, "Doorkey[%d]inserted=%d", &doorkeynum, &valueInt) == 2 && doorkeynum <= 4){
            assets->doorkeyinserted[doorkeynum - 1] = valueInt;
        }else if (sscanf(line, "Doorkey[%d]circledrawn=%d", &doorkeynum, &valueInt) == 2 && doorkeynum <= 4){
            assets->doorkeycircledrawn[doorkeynum - 1] = valueInt;
        }else if(strstr(line, "Shopkeycount=")){
            sscanf(line, "Shopkeycount=%d", &assets->shopkeycount);
        }else if(strstr(line, "Playersuccessfulparry=")){
            sscanf(line, "Playersuccessfulparry=%d", &Playerdata->successfulparry);
        }else if(strstr(line, "Playerdamagetaken=")){
            sscanf(line, "Playerdamagetaken=%d", &Playerdata->damagetaken);
        }
    }
    fclose(file);
    printf("Game data loaded successfully.\n");    
}

void gameoverOverview(struct Playerinfo* Playerdata, struct GameAssets* assets, int enemycount, struct Playerinfo enemies[MAX_ENEMIES], char action){
    
    if (action == 'w'){
        FILE *fileread = fopen("gameoverview.txt", "r"); //read the longest alive time
        if (fileread) {
            char line[50];
            while (fgets(line, sizeof(line), fileread)){
                if (strstr(line, "Playerlongestalivetime=")) {
                    sscanf(line, "Playerlongestalivetime=%.2f", &Playerdata->alivetimercompare);
                }
            }
            fclose(fileread);}

        FILE *file = fopen("gameoverview.txt", "w");
        for (int i = 0; i < enemycount; i++){
            if (enemies[i].dead == 1){
                Playerdata->enemykilled++;
            }
        }
        if (Playerdata->alivetimer > Playerdata->alivetimercompare){
            Playerdata->alivetimercompare = Playerdata->alivetimer;
        }

        if (file){
            fprintf(file, "Playerlongestalivetime=%.2f\n", Playerdata->alivetimercompare);
            fprintf(file, "Playeralivetimer=%.2f\n", Playerdata->alivetimer);
            fprintf(file, "Playeraccumulatedcurrency=%d\n", Playerdata->accumulatedcurrency);
            fprintf(file, "Playerpotionused=%d\n", Playerdata->potionused);
            fprintf(file, "Enemykilled=%d\n", Playerdata->enemykilled);
            fprintf(file, "Playerdamagetaken=%d\n", Playerdata->damagetaken);
            fprintf(file, "Playersuccessfulparry=%d\n", Playerdata->successfulparry);
            fclose(file);
        }
    }else if (action == 'r'){
        FILE *file = fopen("gameoverview.txt", "r");
        if (file){
            char line[100];
            while (fgets(line, sizeof(line), file)){
                if (strstr(line, "Playerlongestalivetime=")){
                    sscanf(line, "Playerlongestalivetime=%.2f", &Playerdata->alivetimercompare);
                }else if (strstr(line, "Playeralivetimer=")){
                    sscanf(line, "Playeralivetimer=%.2f", &Playerdata->alivetimer);
                }else if (strstr(line, "Playeraccumulatedcurrency=")){
                    sscanf(line, "Playeraccumulatedcurrency=%d", &Playerdata->accumulatedcurrency);
                }else if (strstr(line, "Playerpotionused=")){
                    sscanf(line, "Playerpotionused=%d", &Playerdata->potionused);
                }else if (strstr(line, "Enemykilled=")){
                    sscanf(line, "Enemykilled=%d", &Playerdata->enemykilled);
                }else if (strstr(line, "Playerdamagetaken=")){
                    sscanf(line, "Playerdamagetaken=%d", &Playerdata->damagetaken);
                }else if (strstr(line, "Playersuccessfulparry=")){
                    sscanf(line, "Playersuccessfulparry=%d", &Playerdata->successfulparry);
                }
            }
            fclose(file);
        }
    }
}

int loadmap(const char* filename, struct GameAssets* assets){
    if (blocksarray != NULL) {
        free(blocksarray);
        blocksarray = NULL;
    }
    
    FILE* Fileread = fopen(filename, "r");
    if (!Fileread) {
        printf("Failed to open map file!\n");
        return 0;
    }

    int blockwidth = 100;
    int blockheight = 50;
    char line[256]; 
    int row = 0;
    int i = 0;

    blocksarray = malloc(sizeof(struct Playerinfo) * 1000); 
    if (blocksarray == NULL) {
        printf("Memory allocation for blocksarray failed!\n");
        fclose(Fileread);
        return 0;
    }

    while (fgets(line, sizeof(line), Fileread)) {
        for (int col = 0; line[col] != '\0' && line[col] != '\n'; col++) {
            if (line[col] == '1') {
                blocksarray[i].rect = (Rectangle){col * blockwidth, row * blockheight, blockwidth, blockheight};
                blocksarray[i].chestrec = false;
                blocksarray[i].hugeobs = false;
                i++;
            } else if (line[col] == '2') {
                blocksarray[i].rect = (Rectangle){col * blockwidth, row * blockheight, blockwidth, blockheight};
                blocksarray[i].chestrec = true;
                blocksarray[i].key = false;
                blocksarray[i].hugeobs = false;
                blocksarray[i].cheststate = 0;
                blocksarray[i].chestanimationframe = 0;
                blocksarray[i].chestanimationtimer = 0.0f;
                i++;
            } else if (line[col] == '3') {
                blocksarray[i].rect = (Rectangle){col * blockwidth, row * blockheight, blockwidth, blockheight};
                blocksarray[i].chestrec = true;
                blocksarray[i].hugeobs = false;
                blocksarray[i].key = true;
                blocksarray[i].keyclaimed = false;
                blocksarray[i].cheststate = 0;
                blocksarray[i].chestanimationframe = 0;
                blocksarray[i].chestanimationtimer = 0.0f;
                i++;
            }else if (line[col] == '4') {
                blocksarray[i].rect = (Rectangle){col * blockwidth - 30, windheight - (blockheight*9) - (128*0.7) + 5, blockwidth * 5, blockheight * 9};
                blocksarray[i].chestrec = false;
                blocksarray[i].hugeobs = true;
                i++;
            }else if(line[col] == '5') {
                blocksarray[i].portal = true;
                blocksarray[i].chestrec = false;
                blocksarray[i].hugeobs = false;
                i++;
            }
        }
        row++;
    }
    fclose(Fileread);
    return i;
}

void drawobstacles(int blockcount, struct GameAssets* assets, struct Playerinfo* player, Gamestate* currentGameState) {
    //Rectangle platformssourceRect = {240, 48, 63, 30};
    Rectangle platformsrc = {0, 448, 96, 32};
    Rectangle bigplatformsrc = {0, 0, 96, 96};
    Rectangle keysrc = {0, 0, assets->texture[36].width, assets->texture[36].height};
    Rectangle keydest = {0, 0, 65, 45};
    Vector2 origin = {0, 0};
    Texture2D chesttexture;
    static float keyRiseTime = 0.0f;
    static bool keyshdrise = false;
    static Vector2 chestlocation = {0, 0};

    lockeddoor(assets, player, currentGameState, &blockcount);

    if (keyshdrise && keyRiseTime < 4.0f) {
        keyRiseTime += GetFrameTime();
        if (keyRiseTime <= 2.5f){
            chestlocation.y -= keyRiseTime;
        }else{
            chestlocation.y -= 0;
        }
        keydest.x = chestlocation.x;
        keydest.y = chestlocation.y;
        DrawTexturePro(assets->texture[36], keysrc, keydest, origin, 0, WHITE);

    }else {
        keyRiseTime = 0.0f;
        keyshdrise = false;
        chestlocation.x = 0;
        chestlocation.y = 0;
    }

    for (int i = 0; i < blockcount; i++) {
        if (blocksarray[i].hugeobs){//12
            DrawTexturePro(assets->texture[42], bigplatformsrc, blocksarray[i].rect, origin, 0, WHITE);
        }else{
            DrawTexturePro(assets->texture[42], platformsrc, blocksarray[i].rect, origin, 0, WHITE);
        }

        if (blocksarray[i].chestrec){
            chesttexture = assets->texture[38];
            Rectangle chestdest = {blocksarray[i].rect.x, blocksarray[i].rect.y - (assets->texture[38].height / 8) - 68, 130, 100};

            // Check if the player is near the chest
            if (player->Position.x + player->width > blocksarray[i].rect.x &&
                player->Position.x < blocksarray[i].rect.x + blocksarray[i].rect.width &&
                player->Position.y + player->height > blocksarray[i].rect.y - 50 &&
                player->Position.y < blocksarray[i].rect.y + blocksarray[i].rect.height && blocksarray[i].cheststate != 2) {
                DrawRectangleRounded((Rectangle){chestdest.x - 40, chestdest.y - 30, chestdest.width + 70, 40}, 20, 0, (Color){255, 255, 255, 100});
                aligntextcentre(chestdest.x + chestdest.width / 2, chestdest.y - 10, 30, "Press E to Open", WHITE);

                if (IsKeyPressed(KEY_E) && blocksarray[i].cheststate == 0) {
                    blocksarray[i].cheststate = 1; 
                    blocksarray[i].chestanimationframe = 0;
                    blocksarray[i].chestanimationtimer = 0.0f;
                    SetSoundVolume(assets->sound[1], 0.6f);
                    PlaySound(assets->sound[1]);
                }
            }


            if (blocksarray[i].cheststate == 1){
                blocksarray[i].chestanimationtimer += GetFrameTime();
                if (blocksarray[i].key){
                    keyshdrise = true;
                    PlaySound(assets->sound[2]);
                    chestlocation = (Vector2){blocksarray[i].rect.x + 16, blocksarray[i].rect.y - 53};
                }

                if (blocksarray[i].chestanimationtimer >= 0.1f) { 
                    blocksarray[i].chestanimationframe++;
                    blocksarray[i].chestanimationtimer = 0.0f;

                    if (blocksarray[i].chestanimationframe >= 9) { 
                        blocksarray[i].cheststate = 2; 
                        blocksarray[i].chestanimationframe = 9;
                    }
                }
            }

            if (blocksarray[i].cheststate == 2) {
                if (blocksarray[i].key && !blocksarray[i].keyclaimed) {
                    addItemToInventory(player, 1);
                    blocksarray[i].keyclaimed = true;
                }
            }

            int frameindex = blocksarray[i].chestanimationframe % 5;
            int rowindex = blocksarray[i].chestanimationframe / 5; 
            Rectangle chestsrc = {frameindex * (assets->texture[38].width / 5), rowindex * (assets->texture[38].height / 8), assets->texture[38].width / 5, assets->texture[38].height / 8 };
            DrawTexturePro(chesttexture, chestsrc, chestdest, origin, 0, WHITE);
        }
    }
}

void keepobjectwithinscreen(struct Playerinfo* object, struct GameAssets* assets){
    if (object->Position.x < 0){
        (*object).Position.x = 0;
    }
    if (object->Position.x > (mapwidth - object->width)){
        object->Position.x = mapwidth-object->width;
    }
    if (object->Position.y < mapheight){
        object->Position.y = mapheight;
        object->velocityY = 0;
    }
    if (object->Position.y > windheight - object->height- assets->texture[13].height*0.7){
        object->Position.y = windheight - object->height - assets->texture[13].height*0.7;
        object->velocityY = 0;
        object->isJumping = false;
        object->isfalling = false;
        object->onground = true;

        if (!object->attack && !object->isJumping){
            if (object->currenthp <= 0){
                object->animationstate = 6;
            }else if (!object->isrunning && !object->onshield) {
                object->animationstate = 0; // Idle
            }else if (object->isrunning && !object->onshield) {
                object->animationstate = 1; // Running
            }
        }
        /*if (!object->isrunning && !object->onshield){
            object->animationstate = 0;
        }

        else if (object->isrunning && !object->onshield){
            object->animationstate = 1;
        }

        if (object->onground && !object->onshield) //this block of statements is for the animation change when the player is on the ground level
        {
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_D)) {
                if (IsKeyDown(KEY_A) && IsKeyDown(KEY_D)) //make sure that when both keys pressed at the same time character doesn't move
                {
                    object->animationstate = 0;
                    object->isrunning = false;
                }
                else {
                    object->animationstate = 1; // Running
                    object->isrunning = true;
                }
            } 

            else if (IsKeyPressed(KEY_SPACE) && !object->isJumping) {
                object->animationstate = 2; // Idle
                object->isrunning = false;
                object->isJumping = true;
            }
            else {
                object->animationstate  = 0;
            }
        }*/
    }else{
        object->onground = false;
    }
}

//mighht not need
void randomenemypos(int* maxplatform, int enemyonblock[MAX_ENEMIES]){
    for (int i = 0; i < MAX_ENEMIES; i++) {
        int randomnum;
        int duplicate;
        do {
            duplicate = 0;
            randomnum = GetRandomValue(0, *maxplatform - 1);
            for (int j = 0; j < i; j++) {
                if (abs(enemyonblock[j] - randomnum) < 3) { // Ensure at least 2 tiles away
                    duplicate = 1; // Mark as duplicate
                    break; // Break out of the for loop back to do-while loop again
                }
            }
        } while (duplicate); // Repeat if duplicate is found

        enemyonblock[i] = randomnum; // Save the number
    }
}

void lockeddoor(struct GameAssets* assets, struct Playerinfo* player, Gamestate* currentGameState, int* blockcount){
    Vector2 mousepos = GetMousePosition();
    Rectangle lockeddoorsrc = {0, 40, 332, 332};
    Rectangle lockeddoordest = {2500, windheight-240-(128*0.7), 240, 240};
    Rectangle lockeddoordest2 = {1500, windheight-240-(128*0.7), 240, 240};
    Vector2 origin = {0, 0};

    static bool notenoughkeywarning = false;
    static bool keybeinginsertedwarning = false;
    static float timelapsed = 0.0f;
    bool keybeinginserted = false;
    float dt = GetFrameTime();

    if (notenoughkeywarning){
        timelapsed += dt;
        aligntextcentre(lockeddoordest.x + 400, lockeddoordest.y + 30, 40, "Not enough keys!", GREEN);
        if (timelapsed > 3.0f){
            notenoughkeywarning = false;
        }
    }

    if (assets->doorkeyinserted[assets->doorkeyinsertedcount - 1] && assets->doorkeycircledrawn[assets->doorkeyinsertedcount - 1] < 3){
        player->timelapsed += dt;
        keybeinginserted = true;
        if (player->timelapsed > 1.0f){
            player->timelapsed = 0.0f;
            assets->doorkeycircledrawn[assets->doorkeyinsertedcount - 1]++;
        }
    }else if (assets->doorkeyinserted[assets->doorkeyinsertedcount - 1] && assets->doorkeycircledrawn[assets->doorkeyinsertedcount - 1] >= 3){
        keybeinginserted = false;
        player->timelapsed = 0.0f;
    }else{
        player->timelapsed = 0.0f;
    }

    if (assets->doorkeyinserted[3] && assets->doorkeycircledrawn[assets->doorkeyinsertedcount - 1] >= 3){
        player->won = true;
    }

    if (player->Position.x + player->width - 10 >= lockeddoordest.x && player->Position.x <= lockeddoordest.x + lockeddoordest.width
        && player->Position.y >= lockeddoordest.y && player->Position.y <= lockeddoordest.y + lockeddoordest.height){
            lockeddoorsrc.y += 668;
            if (!player->won){
                DrawRectangleRec((Rectangle){lockeddoordest.x - 70, lockeddoordest.y - 120, lockeddoordest.width + 150, 80}, (Color){255, 255, 255, 100});
                aligntextcentre(lockeddoordest.x + lockeddoordest.width/2 + 5, lockeddoordest.y - 80, 25, "Press E to Insert Key", ORANGE);
            }else{
                DrawRectangleRec((Rectangle){lockeddoordest.x - 70, lockeddoordest.y - 140, lockeddoordest.width + 150, 100}, (Color){255, 255, 255, 100});
                aligntextcentre(lockeddoordest.x + lockeddoordest.width/2 + 5, lockeddoordest.y - 120, 25, "You have unlocked the door!", ORANGE);
                aligntextcentre(lockeddoordest.x + lockeddoordest.width/2 + 5, lockeddoordest.y - 85, 25, "Feel free to continue explore", ORANGE);
                aligntextcentre(lockeddoordest.x + lockeddoordest.width/2 + 5, lockeddoordest.y - 50, 25, "or enter when you are ready!", ORANGE);
            }
        if (IsKeyPressed(KEY_E)){
            if (!keybeinginserted && !player->won){
                if (player->keycount > 0){
                    assets->doorkeyinserted[assets->doorkeyinsertedcount++] = true;
                    player->keycount--;
                }else if (player->keycount <= 0 && !notenoughkeywarning){
                    notenoughkeywarning = true;
                }
            }else if (player->won){
                *currentGameState = GAMEOVER;
            }
        }
    }

    if (player->Position.x + player->width - 10 >= lockeddoordest2.x && player->Position.x <= lockeddoordest2.x + lockeddoordest2.width
        && player->Position.y >= lockeddoordest2.y && player->Position.y <= lockeddoordest2.y + lockeddoordest2.height){
            lockeddoorsrc.y += 668;
            if (!player->entereddoor){
                DrawRectangleRec((Rectangle){lockeddoordest.x - 70, lockeddoordest.y - 120, lockeddoordest.width + 150, 80}, (Color){255, 255, 255, 100});
                aligntextcentre(lockeddoordest.x + lockeddoordest.width/2 + 5, lockeddoordest.y - 80, 25, "Press E to Enter Portal", ORANGE);
                if (IsKeyPressed(KEY_E)){
                    player->entereddoor = true;
                    *blockcount = loadmap("map2.txt", assets);
                }
            }else{
                DrawRectangleRec((Rectangle){lockeddoordest.x - 70, lockeddoordest.y - 120, lockeddoordest.width + 150, 80}, (Color){255, 255, 255, 100});
                aligntextcentre(lockeddoordest.x + lockeddoordest.width/2 + 5, lockeddoordest.y - 80, 25, "Press E to Enter Portal", ORANGE);
                if (IsKeyPressed(KEY_E)){
                    player->entereddoor = false;
                    *blockcount = loadmap("map.txt", assets);
                }
            }
        }

    DrawTexturePro(assets->texture[41], lockeddoorsrc, lockeddoordest, origin, 0, WHITE);
    DrawTexturePro(assets->texture[41], lockeddoorsrc, lockeddoordest2, origin, 0, WHITE);

    for (int i = 0; i < 4; i++){
        switch (i){
            case 0:
                if (assets->doorkeyinserted[i]){
                    for (int j = 0; j<assets->doorkeycircledrawn[assets->doorkeyinsertedcount - 1]; j++){
                        DrawCircle(lockeddoordest.x + 61 + (22*j), 579, 7, BLUE);
                    }
                }
                break;
            
            case 1:
                if (assets->doorkeyinserted[i]){
                    for (int j = 0; j<assets->doorkeycircledrawn[assets->doorkeyinsertedcount - 1]; j++){
                        DrawCircle(lockeddoordest.x + lockeddoordest.width - 60 - (22*j), 579, 7, BLUE);
                    }
                }
                break;

            case 2:
                if (assets->doorkeyinserted[i]){
                    for (int j = 0; j<assets->doorkeycircledrawn[assets->doorkeyinsertedcount - 1]; j++){
                        DrawCircle(lockeddoordest.x + 61 + (22*j), 651, 7, BLUE);
                    }
                }
                break;

            case 3:
                if (assets->doorkeyinserted[i]){
                    for (int j = 0; j<assets->doorkeycircledrawn[assets->doorkeyinsertedcount - 1]; j++){
                        DrawCircle(lockeddoordest.x + lockeddoordest.width - 60 - (22*j), 651, 7, BLUE);
                    }
                }
                break;
        }
    }
    //printf("%.2f, %.2f\n", mousepos.x, mousepos.y);
}

void enemyanimations(struct Playerinfo* enemy, struct GameAssets* assets, bool coinenemy, bool enemywithkey, struct Playerinfo* player){ 
    int framecount = 0;
    int frametimer = 0;
    int coinframecount = 6;
    int coinframetimer = 40;
    float dt = GetFrameTime();
    Texture2D texture;

    if (enemy->animationstate == 0){
        framecount = 10;
        frametimer = 30;
    }
    if (enemy->animationstate == 1){
        framecount = 10;
        frametimer = 30;
    } else if (enemy->animationstate == 2){
        framecount = 10;
        frametimer = 8;
    } else if (enemy->animationstate == 3){
        framecount = 5;
        frametimer = 10;
    }
    enemy->currentframe++;
    assets->coincurrentframe++;

    if (enemy->deadenemyremoved){
        return;
    }

    if (assets->coinindex >= coinframecount){
        assets->coincurrentframe = 0;
        assets->coinindex = 0; 
    }

    if (assets->coincurrentframe % coinframetimer == 1 && assets->coinindex < coinframecount){
        assets->coinindex++;
    }

    if (enemy->animationstate == 2 && enemy->animationindex == 0) { //delay the attack animations, takes time to recharge
        enemy->animationtimer += dt;
        if (enemy->animationtimer < 2.0f) {
            enemy->animationindex = 0;
        } else {
            enemy->animationtimer = 0.0f;  // Reset timer after delay
            enemy->animationindex++;  
        }
    } else if (enemy->animationstate == 3 && enemy->animationindex == 5){
        enemy->animationindex = 5;
        enemy->deadtimer += dt; // Increment dead timer
        if (enemy->deadtimer >= 2.0f) {
            if (!enemy->deadenemyremoved){
                enemy->deadenemyremoved = true;
            }
            if (coinenemy){
                if (player->currency + 300 <= MAX_CURRENCY){
                    player->currency += 300;}
                else{
                    player->currency = MAX_CURRENCY;
                }
            }
            if (enemywithkey){
                addItemToInventory(player, 1);
            }
            return;
        }
    }
    
    else if (enemy->currentframe % frametimer == 1){
        enemy->animationindex++;
        enemy->currentframe = 2;
    }

    if (enemy->animationindex > framecount){
        if (enemy->dead == true){

        }
        else {
        enemy->animationindex = 0;
        }
    }

    Rectangle sourceRect, destRect;
    if (enemy->animationstate == 0){
        texture = assets->texture[19];
        int frameIndex = enemy->animationindex % 5;  // 0-4 for first row, 0-4 again for second row
        int rowIndex = (enemy->animationindex < 5) ? 0 : 1; // First 5 use row 0, next 5 use row 1
        float frameWidth = texture.width / 5;

        sourceRect = (Rectangle){(frameWidth * (frameIndex)) + 10, rowIndex * (assets->texture[16].height / 2), 
                                frameWidth, assets->texture[16].height / 2 - 45};

        destRect = (Rectangle){enemy->Position.x, enemy->Position.y + 10, 105, 140};
    }
    if (enemy->animationstate == 1)
    {
        texture = assets->texture[16];
        int frameIndex = enemy->animationindex % 5;  // 0-4 for first row, 0-4 again for second row
        int rowIndex = (enemy->animationindex < 5) ? 0 : 1; // First 5 use row 0, next 5 use row 1
        float frameWidth = texture.width / 5;

        sourceRect = (Rectangle){(frameWidth * (frameIndex)) + 10, rowIndex * (assets->texture[16].height / 2), 
                                frameWidth, assets->texture[16].height / 2 - 45};

        destRect = (Rectangle){enemy->Position.x, enemy->Position.y + 10, 105, 140}; 
    } else if (enemy->animationstate == 2)
    {
        texture = assets->texture[15];
        int frameIndex = enemy->animationindex % 5;  // 0-4 for first row, 0-4 again for second row
        int rowIndex = (enemy->animationindex < 5) ? 0 : 1; // First 5 use row 0, next 5 use row 1
        float frameWidth = texture.width / 5;

        sourceRect = (Rectangle){(frameWidth * (frameIndex)) + 10, rowIndex * (assets->texture[15].height / 2), 
                                frameWidth, assets->texture[15].height / 2 - 45};

        destRect = (Rectangle){enemy->Position.x, enemy->Position.y + 10, 105, 140};
    } else if (enemy->animationstate == 3){
        texture = assets->texture[17];
        int frameIndex = enemy->animationindex % 5;
        int rowIndex = (enemy->animationindex < 5) ? 0 : 1;
        float frameWidth = texture.width / 5;

        sourceRect = (Rectangle){(frameWidth * (frameIndex)), rowIndex * (assets->texture[15].height / 2), frameWidth, 1010};
        destRect = (Rectangle){enemy->Position.x, enemy->Position.y + 10, 105, 140};
    }

    if (enemy->facedirection > 0) {
        sourceRect.x += sourceRect.width;
        sourceRect.width = -fabs(sourceRect.width);
        destRect.width = fabs(destRect.width);
    }

    Vector2 origin = {0, 0};
    DrawTexturePro(texture, sourceRect, destRect, origin, 0, WHITE);

    Rectangle coinsrc = {assets->coinindex * (assets->texture[37].width/6), 0, assets->texture[37].width/6, assets->texture[37].height};
    Rectangle coindest = {enemy->Position.x + 15, enemy->Position.y - 50, 80, 70};
    Rectangle keysrc = {0, 0, assets->texture[36].width, assets->texture[36].height};
    Rectangle keydest = {enemy->Position.x + 25, enemy->Position.y - 20, 50, 30};

    if (coinenemy){
        if (enemy->deadtimer != 0){
            coindest.y += 120;
            coindest.width += 30;
            coindest.height += 30;
        }
        DrawTexturePro(assets->texture[37], coinsrc, coindest, origin, 0, WHITE);
    }

    if (enemywithkey){
        if (enemy->deadtimer != 0){
            keydest.y += 120;
            keydest.width += 10;
            keydest.height += 10;
        }
        DrawTexturePro(assets->texture[36], keysrc, keydest, origin, 0, WHITE);
    }
    //printf("Animation State: %d, Frame: %d\n", enemy->animationstate, enemy->animationindex);
    //printf("%.2f, %.2f\n", destRect.width, destRect.height);
}

//have to randomise their positions on blovksarray
void enemymovement(struct Playerinfo* enemy, struct Playerinfo* player, int enemyonblock[MAX_ENEMIES], int enemyIndex, struct GameAssets* assets) {
    float dt = GetFrameTime();
    float speed = 50.0f;
    float gravity = 800.0f;

    if (!enemy->onplatform) {
        enemy->velocityY += gravity * dt; 
    } else {
        enemy->velocityY = 0; 
    }

    enemy->Position.y += enemy->velocityY * dt;
    Rectangle feetHitbox = {enemy->Position.x + enemy->width * 0.2f, enemy->Position.y + enemy->height+10, enemy->width * 0.6f, 10};
    bool touchingPlatform = false;
    for (int i = 0; i < *enemyonblock; i++) {
        if (CheckCollisionRecs(feetHitbox, blocksarray[i].rect)) {
            touchingPlatform = true;
            enemy->Position.y = blocksarray[i].rect.y - enemy->height - feetHitbox.height; 
            enemy->onplatform = true;
            break;
        }
    }

    if (!touchingPlatform) {
        enemy->onplatform = false;
    }


    if (fabs(enemy->Position.x - player->Position.x) <= 300 && !enemy->dead) {
        if (enemy->Position.y - player->Position.y <= 100 && enemy->Position.y - player->Position.y >= -100) {
            enemy->animationstate = 2; 
            enemy->attack = true;
            speed = 0;

            if (enemy->Position.x < player->Position.x) { 
                enemy->facedirection = 1;
            } else {
                enemy->facedirection = -1;
            }
        } else {
            enemy->animationstate = 1;
            enemy->attack = false;
        }
    } else {
        enemy->animationstate = 1; 
        enemy->attack = false;
    }

    if (!enemy->attack && !enemy->dead) {
        if (enemy->Position.x + enemy->width / 2 <= blocksarray[enemyonblock[enemyIndex]].rect.x + 10) {
            enemy->facedirection = 1;
        } else if (enemy->Position.x + enemy->width / 2 >= blocksarray[enemyonblock[enemyIndex]].rect.x + blocksarray[enemyonblock[enemyIndex]].rect.width) {
            enemy->facedirection = -1;
        }
    }

    if (enemy->dead) {
        if (enemy->animationindex > 5) {
            enemy->animationindex = 0;
        }
        enemy->animationstate = 3;
        speed = 0;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemy != &enemies[i] && !enemies[i].dead) {
            if (CheckCollisionRecs((Rectangle){enemy->Position.x + 5, enemy->Position.y, enemy->width - 10, enemy->height},
                                   (Rectangle){enemies[i].Position.x, enemies[i].Position.y, enemies[i].width, enemies[i].height})) {
                speed = 0;
            }
        }
    }

    if (enemy->Position.y + enemy->height >= windheight - assets->texture[13].height * 0.7 - 8) {
        enemy->Position.y = windheight - assets->texture[13].height * 0.7 - enemy->height - 8;
        enemy->velocityY = 0;
        enemy->isJumping = false;
        enemy->isfalling = false;
        enemy->onground = true;
    }
    enemy->Position.x += dt * speed * enemy->facedirection;
}

void iteratearrowanimation(Arrow* arrow, Texture2D texture, struct GameAssets* assets, Rectangle ground, Rectangle playerHitbox, int blockCount, struct Playerinfo* player, int playerlastframedirection){
    
    float dt = GetFrameTime();
    float rotationfactor;
    bool arrowdoesdamage = true;
    arrow->arrowtimer += dt;

    if (!arrow->active){
        return;
    }

    if (arrow->arrowtimer >= 0.3f) {
        arrow->movingDown = true; 
    }

    if (arrow->direction < 0){
        arrow->speedX = -fabs(arrow->speedX); 
    }
    arrow->position.x += arrow->speedX * dt;
    if (arrow->movingDown && !arrow->arrowonground && !arrow->arrowonblock){
        arrow->position.y += arrow->speedY * dt;
        if (arrow->rotation >= 20.0f){
            rotationfactor = 0.05;
        }else if (arrow->reflected){
            rotationfactor = 0.4;
        }else{
            rotationfactor = 0.2;
        }
        arrow->rotation += arrow->arrowtimer * rotationfactor;
    } 

    //printf("Playerdefense: %d\n", player->playerdefense);
    Rectangle src = {1, 0, 361, texture.height};
    Rectangle dst = {arrow->position.x, arrow->position.y, src.width / 3.5, src.height / 3.5};
    Vector2 origin = {dst.width / 2.0f, dst.height / 2.0f};
    Rectangle arrowtip = {arrow->position.x + dst.width / 2 - (arrow->rotation * 0.45), arrow->position.y + (arrow->rotation * 0.55) + 3, 4, 4}; //to offset the arrowtip hitbox

    if (arrow->arrowtimer <= 0.1f){ //ensure that the arrow hits the player if the player is right in front
        arrowtip.x -= 70;
        arrowtip.width += 70;
    }

    if (arrow->direction < 0) {
        src.x += src.width;
        src.width = -fabs(src.width);
        dst.width = fabs(dst.width);
        arrowtip.x = arrow->position.x - dst.width / 2 + (arrow->rotation * 0.45);
    }

    if (arrow->reflected && !arrow->arrowonground && !arrow->arrowonblock) {
        arrow->reflectedtimer += dt;
        arrow->speedX = -fabs(arrow->speedX + 100);
        if (arrow->direction < 0){
            arrow->speedX = 30;
        }
    }

    for (int i = 0; i < blockCount; i++) {
        if (CheckCollisionRecs(arrowtip, blocksarray[i].rect)) { //////add timer for arrow on block
            //printf("Arrow collided with block %d\n", i);
            arrow->speedX = 0.0f;
            arrow->speedY = 0.0f;
            arrow->movingDown = false;
            arrowdoesdamage = false;
            arrow->arrowonblock = true;

            arrow->arrowonblocktimer += dt;
            if (arrow->arrowonblocktimer >= 3.0f){
                arrow->arrowonblock = false;
                arrow->arrowonblocktimer = 0.0f;
                arrow->arrowtimer = 0.0f;
                arrow->active = false; 
                arrow->speedX = 140.0f;
                arrow->speedY = 45.0f;
                arrow->rotation = 0.0f;
                return;
            }
        }
    }

    // Check collision with the ground
    if (CheckCollisionRecs(arrowtip, ground)) {
        //printf("Arrow collided with the ground.\n");
        arrow->speedX = 0.0f;
        arrow->speedY = 0.0f;
        arrow->movingDown = false;
        arrow->arrowonground = true;
        arrowdoesdamage = false;

        arrow->arrowongroundtimer += dt;
        if (arrow->arrowongroundtimer >= 3.0f){
            arrow->active = false;
            arrow->arrowongroundtimer = 0.0f; 
            arrow->reflectedtimer = 0.0f;
            arrow->reflected = false;
            arrow->arrowtimer = 0.0f;
            arrow->speedX = 140.0f;
            arrow->speedY = 45.0f;
            arrow->rotation = 0.0f;
            arrow->arrowonground = false;
            return;
        }
    }

    if (CheckCollisionRecs(arrowtip, playerHitbox)) {
        //printf("Arrow collided with the player.\n");
        if (arrowdoesdamage){
            if (player->onshield && (playerlastframedirection != arrow->direction)){
                arrow->reflected = true;
                player->successfulparry++;
            }else{
                player->currenthp -= (17.5 - player->playerdefense);
                player->damagetaken += (17.5 - player->playerdefense); 
                arrow->active = false; 
                arrow->arrowtimer = 0.0f;
                arrow->speedX = 140.0f;
                arrow->speedY = 45.0f;
                arrow->rotation = 0.0f;
                arrow->arrowonground = false;
                return;
            }
        }
    }

    if (player->currenthp <= 0 && !player->dead){
        player->dead = true;
        player->animationstate = 6;
        player->deadtimer = 0.0f;
        player->animationindex = 0;
        player->currentframe = 0;
    }
    DrawRectangleRec(arrowtip, RED);
    DrawTexturePro(texture, src, dst, origin, (arrow->rotation) * arrow->direction, WHITE);
}

void checkPlayerAttackCollision(struct Playerinfo* player, struct Playerinfo enemies[MAX_ENEMIES], int facedirection) {
    Rectangle playerattackHitbox = {};
    float dt = GetFrameTime();
    
    if (!player->attack) {
        return; // No need to check for collisions if the player is not attacking
    }else if (player->dead){
        return;
    }

    if (facedirection > 0){
        playerattackHitbox.x = player->Position.x + player->width;
        playerattackHitbox.y = player->Position.y;
        playerattackHitbox.width = player->width;
        playerattackHitbox.width = player->height;
    }
    else {
        playerattackHitbox.x = player->Position.x - player->width;
        playerattackHitbox.y = player->Position.y;
        playerattackHitbox.width = player->width;
        playerattackHitbox.width = player->height;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].dead && CheckCollisionRecs(playerattackHitbox, (Rectangle){enemies[i].Position.x, enemies[i].Position.y, enemies[i].width, enemies[i].height})) {
            if (player->attack){
                enemies[i].hitpoints -= player->playerdamage;
                printf("Enemy %d hit!\n", i);
                printf("hitpoints left: %d", enemies[i].hitpoints);
                if (enemies[i].hitpoints <= 0) {
                    enemies[i].dead = true; // Mark the enemy as dead
                    printf("Enemy %d dead!\n", i);
                }

                else{
                    return;
                }
            }
            else{
                return;
            }
        }
    }
    //printf("Playerattackdmg: %.2f\n", player->playerdamage);
}

void playerenemyhpbar(struct Playerinfo* player, struct Playerinfo enemies[MAX_ENEMIES], struct GameAssets* assets, int enemyCount, Camera2D* camera) {        
    int playerhpindex = (player->hitpoints - player->currenthp)/12.5;
    if (playerhpindex >= 7){
        playerhpindex = 7;
    }
    /*else if (playerhpdiff < 0) { //if the player is healed
        playerhpindex--;
        if (playerhpindex < 0) {
            playerhpindex = 0; 
        }
    }*/
    Rectangle playerhpsrc = {337 - ((assets->texture[21].width/8) * playerhpindex), 1, assets->texture[21].width/8, 14};
    Rectangle playerhpdest = {camera->target.x - 550, camera->target.y - 350, playerhpsrc.width * 3, playerhpsrc.height * 3};
    Vector2 origin = {0, 0};

    //printf("\nplayerposition: %.2f", player->Position.y);
    //printf("\nhpdest: %.2f", playerhpdest.x);
    DrawTexturePro(assets->texture[21], playerhpsrc, playerhpdest, origin, 0, WHITE);
}


int main()
{
    InitWindow(windwidth, windheight, "Gravity game");
    InitAudioDevice();

    struct GameAssets assets = {0};
    {
    assets.images[assets.imagecount++] = LoadImage("Images/background.png"); ///the icon
    assets.images[assets.imagecount++] = LoadImage("Images/tilecompleteset.png");  //
    assets.images[assets.imagecount++] = LoadImage("Images/Walking_KG_2.png"); //2
    assets.images[assets.imagecount++] = LoadImage("Images/Idle_KG_2.png");
    assets.images[assets.imagecount++] = LoadImage("Images/Fall_KG_2.gif");
    assets.images[assets.imagecount++] = LoadImage("Images/Jump_KG_2.gif"); //5
    assets.images[assets.imagecount++] = LoadImage("Images/Attack_KG_2.png");
    assets.images[assets.imagecount++] = LoadImage("Images/Shield_Up_KG_1.png"); //7
    assets.images[assets.imagecount++] = LoadImage("Images/skybg.png");
    assets.images[assets.imagecount++] = LoadImage("Images/mountains.png"); //9
    assets.images[assets.imagecount++] = LoadImage("Images/mountain2.png");
    assets.images[assets.imagecount++] = LoadImage("Images/tree2.png");
    assets.images[assets.imagecount++] = LoadImage("Images/shopbgimg.png"); //12
    assets.images[assets.imagecount++] = LoadImage("Images/Tile1.png"); //13
    assets.images[assets.imagecount++] = LoadImage("Images/groundtiles.png");
    assets.images[assets.imagecount++] = LoadImage("enemies/arrow.png");
    assets.images[assets.imagecount++] = LoadImage("enemies/arrowskelattack.png");
    assets.images[assets.imagecount++] = LoadImage("enemies/arrowskelwalk.png"); //17
    assets.images[assets.imagecount++] = LoadImage("enemies/arrowskeldead.png");
    assets.images[assets.imagecount++] = LoadImage("enemies/arrow.png"); //19
    assets.images[assets.imagecount++] = LoadImage("enemies/arrowskelidle.png");
    assets.images[assets.imagecount++] = LoadImage("Images/MenuSprites.png");
    assets.images[assets.imagecount++] = LoadImage("Images/HPSprites.png"); //22
    assets.images[assets.imagecount++] = LoadImage("Images/setting_menu.png");
    assets.images[assets.imagecount++] = LoadImage("Images/buttonempty.png");
    assets.images[assets.imagecount++] = LoadImage("Images/shop.png");
    assets.images[assets.imagecount++] = LoadImage("Images/empty_menu.png"); //26
    assets.images[assets.imagecount++] = LoadImage("Images/buttonemptyhovered.png");
    assets.images[assets.imagecount++] = LoadImage("Images/shopborder.png");
    assets.images[assets.imagecount++] = LoadImage("Images/shopinventory.png"); //29
    assets.images[assets.imagecount++] = LoadImage("Images/potions.png"); //30
    assets.images[assets.imagecount++] = LoadImage("Images/playerinventory.png"); //31
    assets.images[assets.imagecount++] = LoadImage("Images/buttons.png");
    assets.images[assets.imagecount++] = LoadImage("Images/Moneybag.png"); //33
    assets.images[assets.imagecount++] = LoadImage("Images/inventoryborder.png");
    assets.images[assets.imagecount++] = LoadImage("Images/inventorytitle.png");
    assets.images[assets.imagecount++] = LoadImage("Images/Key.png"); //36
    assets.images[assets.imagecount++] = LoadImage("Images/coin.png");
    assets.images[assets.imagecount++] = LoadImage("Images/Chests.png");
    assets.images[assets.imagecount++] = LoadImage("Images/Chestsbordered.png"); //39
    assets.images[assets.imagecount++] = LoadImage("Images/Dying_KG_1.png");
    assets.images[assets.imagecount++] = LoadImage("Images/door.png");
    assets.images[assets.imagecount++] = LoadImage("Images/tileset2.png");
    assets.images[assets.imagecount++] = LoadImage("Images/lamp.png"); //43

    assets.music[assets.musiccount++] = LoadMusicStream("Music/playing_music.mp3");
    assets.music[assets.musiccount++] = LoadMusicStream("Music/menu_music.mp3");
    assets.music[assets.musiccount++] = LoadMusicStream("Music/shop_music.mp3"); //2
    assets.music[assets.musiccount++] = LoadMusicStream("Music/gameover_music.mp3");

    assets.sound[assets.soundcount++] = LoadSound("Sound/Menu_Hover_sound.mp3");
    assets.sound[assets.soundcount++] = LoadSound("Sound/chest_sound.mp3");
    assets.sound[assets.soundcount++] = LoadSound("Sound/foundkey_sound.mp3");
    //assets.sound[assets.soundcount++] = LoadSound("Music/Menu_Hover.mp3");

    ImageFormat(&assets.images[0], PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    SetWindowIcon(assets.images[0]); 
    PlayMusicStream(assets.music[0]);
    SetTargetFPS(60);

    //can use a for loop afterwards
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[2]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[3]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[4]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[5]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[6]); //4
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[7]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[8]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[9]); //7
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[10]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[11]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[1]); //10
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[12]); //11
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[13]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[14]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[15]); //14 arrow.png
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[16]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[17]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[18]); //17
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[19]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[20]); //19 skelidle
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[21]); 
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[22]); //21 HpSprites
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[23]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[24]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[25]); //24 shop
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[26]); //25 empty menu
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[27]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[28]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[29]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[30]); //29 potions
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[31]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[32]); //31 player inventory
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[33]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[34]); //33
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[35]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[0]); //35
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[36]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[37]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[38]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[39]); //39
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[40]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[41]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[42]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[43]);
}
    
    struct Playerinfo Playerdata = {0};
    Camera2D camera = Camerasettings(&Playerdata);
    Gamestate currentGameState = MENU;
    Gamestate previousgamestate;
    int currentmusic, enemyonblock[MAX_ENEMIES]; 
    int blockcount;
    
    blockcount = loadmap("map.txt", &assets);
    int playerlastframedirection = 1;
    int playercurrentframe = 0;
    int enemycount = MAX_ENEMIES;
    float musicVolume;
    randomenemypos(&blockcount, enemyonblock);
    initializeGameState(&assets, &Playerdata, &currentGameState, &currentmusic, &musicVolume, &playerlastframedirection);

    while (!WindowShouldClose())
    {
        UpdateMusicStream(assets.music[currentmusic]);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        handleGameState(&currentGameState, &previousgamestate, &camera, &assets, &Playerdata, &blockcount, 
                        &playerlastframedirection, &playercurrentframe, 
                        enemyonblock, enemies, &enemycount, &currentmusic);
    
        EndDrawing();
        //printf("Player Position: (%.2f, %.2f)\n", Playerdata.Position.x, Playerdata.Position.y + Playerdata.height);
    }

    FILE* file = fopen("settings.txt", "r");
    if (file){
        fscanf(file, "MusicVolume=%f\n", &musicVolume);
        fclose(file);}
    savegamedata(&Playerdata, &assets, &currentGameState, &currentmusic, musicVolume, enemycount, enemies, &camera);
    Unloadresources(&assets); 
    CloseAudioDevice();
    CloseWindow();
    return 0;
}