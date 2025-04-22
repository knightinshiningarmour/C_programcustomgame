#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <raymath.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define MAX_ENEMIES 3
#define MAX_SHOP_ITEMS 5
#define MAX_CURRENCY 1000

const int windwidth = 1200;
const int windheight = 800;
const int mapwidth = windwidth * 3;
const int mapheight = -windheight * 5;

struct GameAssets 
{ //store game assets
    Image images[40];
    Music music[10];
    Sound sound[20];
    Texture2D texture[40];
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

    int frameCount;                // Total frames in this animation
    float frameSpeed; 
    int texturecount;
    int imagecount;
    int musiccount;
    int soundcount;
};

struct Playerinfo 
{
    Rectangle rect;
    Rectangle playerhitboxx;
    Rectangle playerhitboxyup;
    Rectangle playerhitboxydown;
    Color colour;
    Vector2 Position;
    Vector2 direction;
    float width;
    float height;
    float velocityY;
    bool isJumping;
    bool isfalling;
    bool isidling;
    bool isrunning;
    bool onplatform;
    bool onground;
    bool onshield;
    bool attack;
    bool dead;
    int animationstate;
    int currentframe;
    int animationindex;
    float animationtimer;
    int count;
    int hitpoints;
    int currenthp;
    int facedirection;
    int currency;
    float deadtimer;
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

struct Playerinfo *blocksarray = NULL;
struct Playerinfo enemies[MAX_ENEMIES];

//function prototypes
void drawbackground(struct GameAssets* assets, Camera2D* camera, int x, float scalefactor);
void drawobstacles(int* maxplatform, struct GameAssets* assets);
int calculatemovementplayer(struct Playerinfo* player, int* maxplatform, struct GameAssets* assets);
void updatecamera(Camera2D* camera, struct Playerinfo* player);
void keepobjectwithinscreen(struct Playerinfo* object, struct GameAssets* assets);
void LoadAnimationDataplayer(struct GameAssets* assets);
void iterateanimationplayer(struct GameAssets* assets, struct Playerinfo* player, int* currentframecount, int* facedirection, int* i);
void enemymovement(struct Playerinfo* enemy, struct Playerinfo* player, int enemyonblock[MAX_ENEMIES], int enemyIndex, struct GameAssets* assets);
void enemyanimations(struct Playerinfo* enemy, struct GameAssets* assets);
void checkPlayerAttackCollision(struct Playerinfo* player, struct Playerinfo enemies[MAX_ENEMIES], int facedirection);
void removeDeadEnemies(struct Playerinfo enemies[MAX_ENEMIES], int* enemyCount);
void iteratearrowanimation(int facedirection, Texture2D texture, struct GameAssets* assets, Vector2* arrowPos, 
                            Vector2 enemypos, Rectangle rec, Rectangle playerHitbox, int blockCount, struct Playerinfo* player, bool* arrowActive);
void Unloadresources(struct GameAssets* assets);
void aligntextcentre(int x, int y, int fontsize, const char* text, Color color);
void shop(struct GameAssets* assets, struct Playerinfo* player, int* currentGameState, int* currentmusic, int destx, int desty, int scalefactor);
void drawtrees(struct GameAssets* assets, int i, int destx, int desty, int scalefactor);
void savegamedata(struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float musicVolume, int enemycount, struct Playerinfo enemies[MAX_ENEMIES], Camera2D* camera);
void loadgamedata(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, int* enemycount, struct Playerinfo enemies[MAX_ENEMIES], Camera2D* camera);
void handleMenuState(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, Camera2D* camera);
void handleLoadSaves(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, Camera2D* camera, bool *gamedataloaded);
void handleOptionsState(struct GameAssets* assets, Gamestate* currentGameState, int* currentmusic, float* musicVolume);
void handlePlayingState(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, Camera2D* camera, int* blockcount, int* playerlastframedirection, int* playercurrentframe, int* playeranimationindex, 
    int enemyonblock[MAX_ENEMIES], struct Playerinfo enemies[MAX_ENEMIES], int* enemycount, int* currentmusic, bool* gamedataloaded, float* musicVolume);
void drawbuttonsplayingstate(struct GameAssets* assets, Camera2D* camera, Gamestate* currentGameState, struct Playerinfo* Playerdata);
void handleInventorystate(struct GameAssets* assets, struct Playerinfo* player, int* currentGameState, int* currentmusic, float* musicVolume);
void handlePauseState(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, int* enemycount, struct Playerinfo enemies[MAX_ENEMIES], Camera2D* camera, bool* gamedataloaded);
void playerenemyhpbar(struct Playerinfo* player, struct Playerinfo enemies[MAX_ENEMIES], struct GameAssets* assets, int enemycount, Camera2D* camera);
void shopInteraction(struct Playerinfo* player, int* playerCurrency);
void handleshopstate(struct GameAssets* assets, struct Playerinfo* player, int* currentGameState, int* currentmusic);



void handleGameState(Gamestate* currentGameState, Camera2D* camera, struct GameAssets* assets, struct Playerinfo* Playerdata, int* blockcount, 
                        int* playerlastframedirection, int* playercurrentframe, int* playeranimationindex, int enemyonblock[MAX_ENEMIES], 
                        struct Playerinfo enemies[MAX_ENEMIES], int* enemycount, int* currentmusic){

    switch (*currentGameState){
        static float musicVolume = 0.5f;
        static bool gamedataloaded = false; 

        case MENU:
            handleMenuState(assets, Playerdata, currentGameState, currentmusic, &musicVolume, camera);
            break;
        case LOADSAVES: //load saved file or new game
            handleLoadSaves(assets, Playerdata, currentGameState, currentmusic, &musicVolume, camera, &gamedataloaded);
            break;
        case OPTIONS: 
            handleOptionsState(assets, currentGameState, currentmusic, &musicVolume);
            break;
        case QUIT:
            Unloadresources(assets); 
            CloseAudioDevice();
            CloseWindow();
            exit(0);
            break;
        case PLAYING:
            handlePlayingState(assets, Playerdata, currentGameState, camera, blockcount, playerlastframedirection, playercurrentframe, 
                               playeranimationindex, enemyonblock, enemies, enemycount, currentmusic, &gamedataloaded, &musicVolume);
            break;
        case INVENTORY:
            handleInventorystate(assets, Playerdata, (int*)currentGameState, currentmusic, &musicVolume);
            break;
        case SHOP:
            handleshopstate(assets, Playerdata, (int*)currentGameState, currentmusic);
            break;
        case PAUSE: 
            handlePauseState(assets, Playerdata, currentGameState, currentmusic, &musicVolume, enemycount, enemies, camera, &gamedataloaded);
            break;
        case GAMEOVER:
            DrawText("Game Over. Press ENTER to Restart", windwidth / 2 - 150, windheight / 2, 20, BLACK);
            if (IsKeyPressed(KEY_ENTER))
            {*currentGameState = MENU;}
            break;
    }
}

void initializeGameState(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume){
    // Initialize player data
    Playerdata->Position = (Vector2){windwidth / 2 - 50, windheight - 100};
    Playerdata->width = 50;
    Playerdata->height = 50;
    Playerdata->currenthp = 100;
    Playerdata->hitpoints = 100;
    Playerdata->currency = 0;
    Playerdata->isJumping = false;
    Playerdata->isfalling = false;
    Playerdata->attack = false;
    Playerdata->onshield = false;
    Playerdata->animationstate = 0;
    *currentGameState = MENU;
    *currentmusic = -1; 
    *musicVolume = 0.5f;
    LoadAnimationDataplayer(assets);
}

void handleMenuState(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, Camera2D* camera){
    static bool settingsLoaded = false; 
    static bool hoverplayed = false;   

    if (*currentmusic != 1) { 
        StopMusicStream(assets->music[*currentmusic]);
        PlayMusicStream(assets->music[1]);
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

    static int currentframe = 0;
    static int animationindex = 0;
    static int facedirection = 1;
    Playerdata->animationstate = 0; 
    Playerdata->Position.x = windwidth/2 - Playerdata->width/2;
    Playerdata->Position.y = 250 - Playerdata->height; 
    Playerdata->width *= 3;
    Playerdata->height *= 3;  
    iterateanimationplayer(assets, Playerdata, &currentframe, &facedirection, &animationindex);
    
    for (int i = 0; i < 2; i++) {
        enemies[i].Position.x = 200 + i * 100; // Spread enemies horizontally
        enemies[i].Position.y = windheight - 240; // Position near the bottom of the screen
        enemies[i].animationstate = 0; // Set to idle animation state
        enemies[i].facedirection = (i % 2 == 0) ? 1 : -1; // Alternate facing directions
        enemyanimations(&enemies[i], assets);
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

void handleLoadSaves(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, Camera2D* camera, bool *gamedataloaded){
    
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
            Playerdata->Position.x = windwidth / 2 - Playerdata->width / 2;
            Playerdata->Position.y = windheight - Playerdata->height;
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

void handleOptionsState(struct GameAssets* assets, Gamestate* currentGameState, int* currentmusic, float* musicVolume){ //26
    Rectangle optiontabsrc = {0, 0, 120, 140};
    Rectangle optiontabdest = {600, 370, 900, windheight + 80};
    Vector2 optiontaborigin = {optiontabdest.width/2, optiontabdest.height/2};
    Vector2 mousePos = GetMousePosition();
    Rectangle volumebuttondest = {300, 220, 250, 80};                
    Rectangle volumeSlider = {windwidth / 2, 235, 200, 40}; 

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

    const char* controls[] = {"Space", "A/D", "Left Mouse", "Shift", "P", "E"};
    const char* functions[] = {"Jump", "Left/Right", "Attack", "Parry", "Pause", "Interact"};
    int FontSize = 20; 
    int emptybuttonWidth = 150; 
    int emptybuttonHeight = 50; 
    int controlsStartY = volumebuttondest.y + volumebuttondest.height + 40; // Starting Y position for controls
    int buttonSpacing = 15;
    int buttonGap = 20; 
    static float settingssavedtimer = 0.0f;
    
    // Draw "Controls" title
    DrawText("Controls:", 310, controlsStartY - 5, FontSize + 5, BLACK);

    int displaybutton = 6;
    for (int i = 0; i < displaybutton; i++) {
        Rectangle keyButtonRect = {300, controlsStartY + (i + 1) * (emptybuttonHeight + buttonSpacing) - 30, emptybuttonWidth,emptybuttonHeight};
        Rectangle functionButtonRect = {keyButtonRect.x + keyButtonRect.width + buttonGap, controlsStartY + (i + 1) * (emptybuttonHeight + buttonSpacing) - 30, 
                                        emptybuttonWidth + 50, emptybuttonHeight};
        DrawTexturePro(assets->texture[23], (Rectangle){0, 0, assets->texture[23].width, assets->texture[23].height},
                        keyButtonRect, (Vector2){0, 0}, 0.0f, WHITE );
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
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                *currentGameState = MENU; 
                settingssavedtimer = 0.0f;
            }
    }
}

void handlePlayingState(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, Camera2D* camera, int* blockcount, int* playerlastframedirection, int* playercurrentframe, int* playeranimationindex, 
                        int enemyonblock[MAX_ENEMIES], struct Playerinfo enemies[MAX_ENEMIES], int* enemycount, int* currentmusic, bool* gamedataloaded, float* musicVolume)
{
    static float cointimer = 0.0f;
    float dt = GetFrameTime(); 
    bool playerpostracked;

    cointimer += dt;
    if (cointimer >= 1.0f) {
        cointimer = 0.0f;
        Playerdata->currency += 2;
    }

    if (*currentmusic != 0) { 
        StopMusicStream(assets->music[*currentmusic]);
        PlayMusicStream(assets->music[0]);
        *currentmusic = 0;
    }
    BeginMode2D(*camera);
    if (!*gamedataloaded) {
        *gamedataloaded = true;
        loadgamedata(assets, Playerdata, currentGameState, currentmusic, musicVolume, enemycount, enemies, camera);
    }
    for (int j = 1; j < 5; j++){
        if (j == 4){
            drawbackground(assets, camera, j, 0.7);
        }
        else{
            drawbackground(assets, camera, j, 0.7);
        }
    }

    drawbuttonsplayingstate(assets, camera, currentGameState, Playerdata);
    drawobstacles(blockcount, assets);
    shop(assets, Playerdata, (int*)currentGameState, currentmusic, 840, 380, 3);
    //shopInteraction(Playerdata, &playercurrency);
    *playerlastframedirection = calculatemovementplayer(Playerdata, blockcount, assets);
    playerenemyhpbar(Playerdata, enemies, assets, *enemycount, camera);
    updatecamera(camera, Playerdata);
    keepobjectwithinscreen(Playerdata, assets);
    iterateanimationplayer(assets, Playerdata, playercurrentframe, playerlastframedirection, playeranimationindex);

    static bool arrowActive = false; 
    static Vector2 arrowPos = {0, 0}; 
    static int arrowFacedirection = 1;
    Texture2D arrowtexture = assets->texture[18];
    Rectangle ground = {0, windheight - assets->texture[13].height * 0.7, mapwidth, assets->texture[13].height * 0.7};
    Rectangle playerHitbox = {Playerdata->Position.x, Playerdata->Position.y, Playerdata->width - 30, Playerdata->height};

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemymovement(&enemies[i], Playerdata, enemyonblock, i, assets);
        enemyanimations(&enemies[i], assets);

        if (enemies[i].attack && enemies[i].animationindex == 9) { 
            if (!arrowActive) {
                arrowActive = true; 
                if (enemies[i].facedirection > 0) {
                    arrowPos = (Vector2){enemies[i].Position.x + enemies[i].width, enemies[i].Position.y + enemies[i].height / 2}; 
                } else {
                    arrowPos = (Vector2){enemies[i].Position.x, enemies[i].Position.y + enemies[i].height / 2}; // Reset arrow position
                }
                arrowFacedirection = enemies[i].facedirection; 
            }
        }
    }

    if (arrowActive) {
        iteratearrowanimation(arrowFacedirection, arrowtexture, assets, &arrowPos, 
                            arrowPos, ground, playerHitbox, *blockcount, Playerdata, &arrowActive);
    }
    
    checkPlayerAttackCollision(Playerdata, enemies, *playerlastframedirection);
    removeDeadEnemies(enemies, enemycount);
    EndMode2D();

    if (IsKeyPressed(KEY_P)){
        *currentGameState = PAUSE;
    }
    if(IsKeyPressed(KEY_I)){
        *currentGameState = INVENTORY;
    }
}

void handleInventorystate(struct GameAssets* assets, struct Playerinfo* player, int* currentGameState, int* currentmusic, float* musicVolume){
    Rectangle playerinvensrc = {0, 0, assets->texture[30].width, assets->texture[30].height};
    Rectangle inventorybg = {0, 0, windwidth, windheight};
    Rectangle invenborder = {80, 50, 1050, 300};
    DrawRectangleRec(inventorybg, (Color){0, 0, 0, 70});
    //DrawRectangleRec(invenborder, (Color){255, 255, 153, 100});

    for (int i=0; i<10; i++){
        Rectangle playerinvendest = {100 + (i * 100), 100, 100, 100};
        DrawTexturePro(assets->texture[30], playerinvensrc, playerinvendest, (Vector2){0, 0}, 0.0f, WHITE);
        //Rectangle playerinvendest2 = {100 + (i * 100), 200, 100, 100};
        //DrawTexturePro(assets->texture[30], playerinvensrc, playerinvendest2, (Vector2){0, 0}, 0.0f, WHITE);
    }
}

void handleshopstate(struct GameAssets* assets, struct Playerinfo* player, int* currentGameState, int* currentmusic){
    Rectangle shopbgsrc = {0, 0, 1024, 1024};
    Rectangle shopbgdest = {0, 0, windwidth, windheight};
    Vector2 shopbgorigin = {0, 0};
    Rectangle shopInventorysrc = {36, 62, 652, 290};
    Rectangle shopInventorydest = {100, 150, 1000, 600};
    Vector2 shopInventoryorigin = {0, 0};
    Rectangle potionspos[4]; //array to store the potions' coordinates
    Vector2 mousePos = GetMousePosition();
    Rectangle emptybuttonsrc = {0, 2, assets->texture[23].width, 27};
    Rectangle shopbackbuttondest = {950, 50, 200, 50};
    Rectangle playerhpsrc = {337 - (48 * ((100 - player->currenthp)/7)), 1, 46, 14};
    Rectangle playerhpdest = {50, 50, 200, 70};
    static int potionbought[4] = {0};
    float potiontexturewidth = assets->texture[29].width / 4;
    DrawTexturePro(assets->texture[11], shopbgsrc, shopbgdest, shopbgorigin, 0, WHITE);
    DrawTexturePro(assets->texture[28], shopInventorysrc, shopInventorydest, shopInventoryorigin, 0, WHITE);
    DrawTexturePro(assets->texture[23], emptybuttonsrc, shopbackbuttondest, (Vector2){0, 0}, 0.0f, WHITE);
    DrawTexturePro(assets->texture[21], playerhpsrc, playerhpdest, (Vector2){0, 0}, 0.0f, WHITE);
    aligntextcentre(shopbackbuttondest.x + shopbackbuttondest.width / 2, shopbackbuttondest.y + shopbackbuttondest.height / 2, 30, "Back", BLACK);
    if (*currentmusic != 2) { 
        float shopmusicVolume = 1.0f;
        *currentmusic = 2;
        StopMusicStream(assets->music[*currentmusic]);
        PlayMusicStream(assets->music[2]);
        SetMusicVolume(assets->music[*currentmusic], shopmusicVolume);
    }

    for (int i=0; i<4; i++){
        Rectangle potionsrc = {0 + (i * potiontexturewidth), 0, potiontexturewidth, assets->texture[29].height/2};
        Rectangle potionsdest = {448 + (i * potionsrc.width/6), 220, potionsrc.width/8 - 3, potionsrc.height/8};
        potionspos[i] = (Rectangle){potionsdest.x, potionsdest.y, potionsdest.width, potionsdest.height};
        DrawTexturePro(assets->texture[29], potionsrc, potionsdest, (Vector2){0, 0}, 0.0f, WHITE);
    }

    for (int i=0; i<4; i++){
        if (CheckCollisionPointRec(mousePos, potionspos[i])){
            DrawRectangleRec(potionspos[i], (Color){255, 255, 255, 100});
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                printf("Potion %d clicked!\n", i+1);
                potionbought[i]++;
            }
        }
        printf("Potion %d bought: %d\n", i+1, potionbought[i]);
    }

    if (CheckCollisionPointRec(GetMousePosition(), shopbackbuttondest)){
        DrawTexturePro(assets->texture[26], (Rectangle){0, 4, assets->texture[26].width, 25}, shopbackbuttondest, (Vector2){0, 0}, 0.0f, WHITE);
        aligntextcentre(shopbackbuttondest.x + shopbackbuttondest.width / 2, shopbackbuttondest.y + shopbackbuttondest.height / 2, 30, "Back", BLACK);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            *currentGameState = PLAYING;
        }
    }
}

void handlePauseState(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, int* enemycount, struct Playerinfo enemies[MAX_ENEMIES], Camera2D* camera, bool* gamedataloaded){
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

    const char* text[] = {"RESUME", "SAVE GAME", "BACK TO MENU"};
    static float savetimer = 0.0f;
    static bool confirmedsave = false;
    static bool hoverdrawn = false;
    bool ishovered = false;

    DrawTexturePro(assets->texture[25], pausebgsrc, pausebgdest, pausebgorigin, 0, WHITE);
    DrawTexturePro(assets->texture[5], charactersrc, characterdest, characterorigin, 0, WHITE);
    aligntextcentre(200 + pausebgdest.width/2, 150, 50, "PAUSED", BLACK);   
    DrawTexturePro(assets->texture[19], pauseenemyskelsrc, pauseenemyskeldestright, pauseenemyskelorigin, 0, WHITE);
    
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
        ishovered = true;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            *currentGameState = PLAYING; // Resume the game
            savetimer = 0.0f;
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
            *currentGameState = MENU; // Return to the main menu
            *gamedataloaded = false;
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
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                confirmedsave = false;
                savegamedata(Playerdata, currentGameState, currentmusic, *musicVolume, *enemycount, enemies, camera);
                savetimer = 3.0f;
            }
        }
        else if (CheckCollisionPointRec(mousePos, noButton)){
            DrawRectangleRec(noButton, LIGHTGRAY);
            aligntextcentre(noButton.x + noButton.width / 2, noButton.y + noButton.height / 2, 30, "No", BLACK);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                confirmedsave = false;
                savetimer = 0.0f;
            }
        }
    }
    if (savetimer > 0.0f) {
        aligntextcentre(200 + pausebgdest.width/2, 700, 50, "Game Saved!", GREEN); 
        savetimer -= GetFrameTime();
    }
}



void aligntextcentre(int x, int y, int fontsize, const char* text, Color color) {
    int textWidth = MeasureText(text, fontsize);
    DrawText(text, x - textWidth / 2, y - fontsize/2, fontsize, color);
}

void drawtrees(struct GameAssets* assets, int i, int destx, int desty, int scalefactor){
    if (i == 1){    // big tree in background.png
        Rectangle tree1src = {201, 7 , 108, 73};
        Rectangle tree1dest = {destx, desty, tree1src.width*scalefactor, tree1src.height*scalefactor};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[8], tree1src, tree1dest, origin, 0, WHITE);
    }

    else if (i == 2){
        Rectangle tree2src = {17, 12 , 73, assets->texture[9].height - tree2src.y};
        Rectangle tree2dest = {destx, desty, tree2src.width*scalefactor, tree2src.height*scalefactor};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[9], tree2src, tree2dest, origin, 0, WHITE);
    }

    else if (i == 3){
        Rectangle pinktreesrc = {245, 12, 72, 100};
        Rectangle pinktreedest = {destx, desty, pinktreesrc.width*scalefactor, pinktreesrc.height*scalefactor};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[10], pinktreesrc, pinktreedest, origin, 0, WHITE);
    }
}

void drawbigobstacles(){

}

void Unloadresources(struct GameAssets* assets){
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
  
    if (blocksarray != NULL) {
        free(blocksarray);
        blocksarray = NULL;
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
    Vector2 origin = {0, 0};
    DrawTexturePro(assets->texture[24], shopsrc, shopdest, origin, 0, WHITE);

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

void drawbackground (struct GameAssets* assets, Camera2D* camera, int x, float scalefactor) {
    float skySpeed = 0.1f;      
    float mountainSpeed1 = 0.3f; 
    float mountainSpeed2 = 0.5f; 

    float skyOffset = -camera->target.x * skySpeed;
    float mountainOffset1 = -camera->target.x * mountainSpeed1;
    float mountainOffset2 = -camera->target.x * mountainSpeed2;

    switch (x){
        case 1:
            for (int i = 0; i < (mapwidth/(assets->texture[6].width*scalefactor)); i++) {
                Rectangle skysrc = {0, 0, assets->texture[6].width, assets->texture[6].height};
                Rectangle skydest = {i * (skysrc.width*scalefactor), -300, (float)skysrc.width * scalefactor, (float)skysrc.height * scalefactor};  
                Vector2 origin = {0, 0};
                DrawTexturePro(assets->texture[6], skysrc, skydest, origin, 0, WHITE);
            }
            break;
        case 2:
            for (int i = 0; i < (mapwidth/(assets->texture[7].width*scalefactor)); i++) {
                Rectangle mountainsrc = {0, 0, assets->texture[7].width, assets->texture[7].height};
                Rectangle mountaindest = {i * mountainsrc.width*scalefactor, windheight - (128*0.7) - (mountainsrc.height*scalefactor), mountainsrc.width*scalefactor, mountainsrc.height*scalefactor}; // Adjust position and size
                Vector2 origin = {0, 0};
                DrawTexturePro(assets->texture[7], mountainsrc, mountaindest, origin, 0, WHITE);
            }
            break;
        case 3:
            for (int i = 0; i < (mapwidth/(assets->texture[8].width*scalefactor)); i++) {
                Rectangle mountainsrc2 = {0, 0, assets->texture[8].width, assets->texture[8].height};
                Rectangle mountaindest2 = {i * mountainsrc2.width*scalefactor, windheight - (128*0.7) - (mountainsrc2.height*scalefactor), mountainsrc2.width*scalefactor, mountainsrc2.height*scalefactor}; // Adjust position and size
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

void drawbuttonsplayingstate(struct GameAssets* assets, Camera2D* camera, Gamestate* currentGameState, struct Playerinfo* Playerdata){
    Vector2 mousePos = GetMousePosition();
    Rectangle currencyrec = {camera->target.x-570, camera->target.y-290, 200, 30};
    Rectangle currencybar = {currencyrec.x, currencyrec.y, 0.2*Playerdata->currency, currencyrec.height};
    Rectangle pausebuttonsrc = {65, 153, 90, 95};
    Rectangle pausebuttondest = {camera->target.x + 400, camera->target.y - 350, 70, 70};
    Rectangle pausebuttondestcollision = {1000, 50, pausebuttondest.width, pausebuttondest.height};
    Rectangle settingsbuttonsrc = {67, 270, 90, 96};
    Rectangle settingsbuttondest = {pausebuttondest.x + 90, pausebuttondest.y, 70, 70};
    
    if (mousePos.x >= pausebuttondestcollision.x && mousePos.x <= pausebuttondestcollision.x + pausebuttondestcollision.width
        && mousePos.y >= pausebuttondestcollision.y && mousePos.y <= pausebuttondestcollision.y + pausebuttondestcollision.height){
        pausebuttonsrc.x = 182;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            *currentGameState = PAUSE; 
            //*currentmusic = 1;
        }
    }
    DrawRectangleRec(currencyrec, WHITE);
    DrawRectangleRec(currencybar, GOLD);
    DrawTexturePro(assets->texture[31], pausebuttonsrc, pausebuttondest, (Vector2){0, 0}, 0.0f, WHITE);
    DrawTexturePro(assets->texture[31], settingsbuttonsrc, settingsbuttondest, (Vector2){0,0}, 0.0f, WHITE);
    char currencyText[30];
    sprintf(currencyText, "Coins: %d/1000", Playerdata->currency);
    aligntextcentre(currencyrec.x + currencyrec.width / 2, currencyrec.y + currencyrec.height / 2, 20, currencyText, BLACK);
    

}

void collisionplayerblocks(char axis, struct Playerinfo* object, int* maxplatform, int* facedirection) {
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

    bool touchingPlatform = false;
    object->onplatform = false;

    for (int i = 0; i < *maxplatform; i++) {
        // Feet collision with the platform
        if (axis == 'y' && CheckCollisionRecs(feetHitbox, blocksarray[i].rect)) {
            touchingPlatform = true;

            if (object->velocityY >= 0) { // Falling down
                object->Position.y = blocksarray[i].rect.y - object->height;
                object->velocityY = 0;
                object->isfalling = false;
                object->onplatform = true;
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

    if (!touchingPlatform) {
        object->onplatform = false;
    }
    
}

int calculatemovementplayer(struct Playerinfo *player, int* maxplatform, struct GameAssets *assets) {
    static int facedirection = 1;
    float dt = GetFrameTime();
    float speed = 300.0f;
    float jumpForce = -500.0f;
    float gravity = 800.0f;

    if (IsKeyDown(KEY_LEFT_SHIFT) && !player->isJumping && !player->isfalling){ //ensure that the character will stop when hes holding shield
        player->onshield = true;
        player->animationstate = 5;
        speed = 0;
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
    if (IsKeyPressed(KEY_SPACE) && !player->isJumping && !player->attack) {
        player->velocityY = jumpForce;
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

    if ((!player->onplatform||!player->onground) && (!player->attack) &&!player->onshield) {  
        player->isrunning = false;
        player->velocityY += gravity * dt;

        if (player->velocityY >= 0) {
            player->isfalling = true;
            player->animationstate = 3; //falling animation
        } else {
            player->isfalling = false;
        }
    }

    if (!player->isJumping && !player->onground && !player->onplatform) {
        player->isfalling = true;
        player->velocityY = 0;
    }
    player->Position.y += player->velocityY * dt;
    collisionplayerblocks('y', player, maxplatform, &facedirection);
    keepobjectwithinscreen(player, assets);

    //printf("DEBUG: isfalling=%d, animationstate=%d, velocityY=%.2f\n", 
        //player->isfalling, player->animationstate, player->velocityY);
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
    }
    fclose(file);
}

void iterateanimationplayer(struct GameAssets* assets, struct Playerinfo* player, int* currentframecount, int* facedirection, int* i) {
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
    (*currentframecount)++;

    if (*currentframecount % (frametimer) == 1) {
        (*i)++;
        *currentframecount = 2;
    }

    if (*i >= framescount) {
        if (player->animationstate == 4) { // If attacking, reset attack state after animation completes
            *i = 0;
            player->attack = false; // Reset attack flag
        }
        else if (player->animationstate == 5) //if shielding, maintain the last frame
        {
            *i = framescount - 1;
        }
        else 
        {
            *i = 0;
            if (player->animationstate == 2 && player->isJumping) //let the character jumping frame to iterate till it finishes jumping
            {
                player->animationstate = 2;
                *i = 3;
            }
        }

    }

    //printf("AnimationState: %d, isFalling: %d, isRunning: %d, onShield: %d\n", 
        //player->animationstate, player->isfalling, player->isrunning, player->onshield);

    Rectangle sourceRect, destRect;

    if (player->animationstate == 0) {  // Idle Animation
        texture = assets->texture[1];
        sourceRect = (Rectangle){assets->src_idlex[*i], assets->src_idley[*i], 50, 61};
    } 
    
    else if (player->animationstate == 1) // Running Animation
    {  
        texture = assets->texture[0];
        sourceRect = (Rectangle){assets->src_runningx[*i], assets->src_runningy[*i], assets->src_runningwidth[*i],
                                 assets->texture[0].height - assets->src_runningy[*i]};
    } 

    else if (player->animationstate == 2) // Jumping Animation
    {  
        texture = assets->texture[3];
        sourceRect = (Rectangle){assets->src_jumpingx[*i], assets->src_jumpingy[*i], assets->src_jumpingwidth[*i], assets->src_jumpingheight[*i]};
    }

    else if (player->animationstate == 3) // falling Animation
    {
        texture = assets->texture[2];
        sourceRect = (Rectangle){assets->src_fallingx[*i], assets->src_fallingy[*i], 50, 57};
    }

    else if (player->animationstate == 4){
        player->attack = true;
        texture = assets->texture[4];
        sourceRect = (Rectangle){assets->src_attackingx[*i], assets->src_attackingy[*i], 
                      assets->src_attackingwidth[*i], assets->texture[4].height - assets->src_attackingy[*i]};
    
        //printf("Attack Animation Frame: %d, Attack Flag: %d\n", *i, player->attack); // Debugging print
        if (*i == 5){
            *i = 0;
            player->attack = false; //think abt how to make the fifth frame iterate finish 
        }
    }

    else if (player->animationstate == 5) // falling Animation
    {
        texture = assets->texture[5];
        sourceRect = (Rectangle){assets->src_shieldx[*i], assets->src_shieldy[*i], assets->src_shieldwidth[*i], 
                                 assets->texture[5].height - assets->src_shieldy[*i]};
    }
    player->width = 100;
    player->height = 100;
    destRect = (Rectangle){player->Position.x, player->Position.y, player->width, player->height};

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

void savegamedata(struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float musicVolume, int enemycount, struct Playerinfo enemies[MAX_ENEMIES], Camera2D* camera) {
    FILE *file = fopen("savegame.txt", "w");
    if (file) {
        fprintf(file, "PlayerPositionX=%.2f\n", Playerdata->Position.x);
        fprintf(file, "PlayerPositionY=%.2f\n", Playerdata->Position.y);
        fprintf(file, "PlayerHealth=%d\n", Playerdata->currenthp);
        fprintf(file, "PlayerMaxHealth=%d\n", Playerdata->hitpoints);
        fprintf(file, "PlayerCurrency=%d\n", Playerdata->currency);
        fprintf(file, "MusicVolume=%.2f\n", musicVolume);
        fprintf(file, "EnemyCount=%d\n", enemycount);
        for (int i = 0; i < enemycount; i++) {
            fprintf(file, "Enemy%dPositionX=%.2f\n", i, enemies[i].Position.x);
            fprintf(file, "Enemy%dPositionY=%.2f\n", i, enemies[i].Position.y);
            fprintf(file, "Enemy%dHealth=%d\n", i, enemies[i].hitpoints);
            fprintf(file, "Enemy%dDead=%d\n", i, enemies[i].dead);
        }
        fprintf(file, "CameraTargetX=%.2f\n", camera->target.x);
        fprintf(file, "CameraTargetY=%.2f\n", camera->target.y);
        fprintf(file, "CameraZoom=%.2f\n", camera->zoom);

        fclose(file);
    }
}

void loadgamedata(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, int* enemycount, struct Playerinfo enemies[MAX_ENEMIES], Camera2D* camera) {
    FILE *file = fopen("savegame.txt", "r");
    char line[100];
    if (!file) {
        printf("Error: No save file found. Starting a new game.\n");
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "PlayerPositionX=")) {
            sscanf(line, "PlayerPositionX=%f", &Playerdata->Position.x);
        } else if (strstr(line, "PlayerPositionY=")) {
            sscanf(line, "PlayerPositionY=%f", &Playerdata->Position.y);
        } else if (strstr(line, "PlayerHealth=")) {
            sscanf(line, "PlayerHealth=%d", &Playerdata->currenthp);
        } else if (strstr(line, "PlayerMaxHealth=")) {
            sscanf(line, "PlayerMaxHealth=%d", &Playerdata->hitpoints);
        } else if (strstr(line, "PlayerCurrency=")) {
            sscanf(line, "PlayerCurrency=%d", &Playerdata->currency);
        } else if (strstr(line, "MusicVolume=")) {
            sscanf(line, "MusicVolume=%f", musicVolume);
        } else if (strstr(line, "EnemyCount=")) {
            sscanf(line, "EnemyCount=%d", enemycount);
        } else if (strstr(line, "CameraTargetX=")) {
            sscanf(line, "CameraTargetX=%f", &camera->target.x);
        } else if (strstr(line, "CameraTargetY=")) {
            sscanf(line, "CameraTargetY=%f", &camera->target.y);
        } else if (strstr(line, "CameraZoom=")) {
            sscanf(line, "CameraZoom=%f", &camera->zoom);
        }
    }

    while (fgets(line, sizeof(line), file)){
        for (int i = 0; i < *enemycount; i++) {
            if (sscanf(line, "Enemy%dPositionX=%f", &i, &enemies[i].Position.x) == 2) {
                sscanf(line, "Enemy%dPositionY=%f", &i, &enemies[i].Position.y);
                sscanf(line, "Enemy%dHealth=%d", &i, &enemies[i].hitpoints);
                sscanf(line, "Enemy%dDead=%d", &i, &enemies[i].dead);
            }
        }
    }

    while (fgets(line, sizeof(line), file)) {
        if (fscanf(file, "CameraTargetX=%f\n", &camera->target.x) != 1 ||
            fscanf(file, "CameraTargetY=%f\n", &camera->target.y) != 1 ||
            fscanf(file, "CameraZoom=%f\n", &camera->zoom) != 1) 
        {
            camera->target = (Vector2){0, 0};
            camera->zoom = 1.0f;
        }
    }
    fclose(file);
    printf("Game data loaded successfully.\n");
}

int loadmap(const char* filename) {
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
                i++;
            } else if (line[col] == '2') {
                blocksarray[i].rect = (Rectangle){col * blockwidth, row * blockheight, blockwidth, blockheight};
                blocksarray[i].colour = GOLD; // Example: Set a unique color for chest blocks
                i++;
            } else if (line[col] == '3') {
                blocksarray[i].rect = (Rectangle){col * blockwidth, row * blockheight, blockwidth, blockheight};
                blocksarray[i].colour = RED; // Example: Set a unique color for enemy blocks
                i++;
            }
        }
        row++;
    }
    fclose(Fileread);
    return i; // Return the total number of blocks loaded
}

void drawobstacles(int* maxplatform, struct GameAssets* assets){
    Rectangle sourceRect ={240, 48, 63,30};
    Vector2 origin = {0, 0};
    for (int i=0; i<*maxplatform; i++){
        DrawTexturePro(assets->texture[12], sourceRect, blocksarray[i].rect, origin, 0, WHITE);
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

        if (!object->attack && !object->isJumping) {
            if (!object->isrunning && !object->onshield) {
                object->animationstate = 0; // Idle
            } else if (object->isrunning && !object->onshield) {
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
                if (abs(enemyonblock[j] - randomnum) < 2) { // Ensure at least 2 tiles away
                    duplicate = 1; // Mark as duplicate
                    break; // Break out of the for loop back to do-while loop again
                }
            }
        } while (duplicate); // Repeat if duplicate is found

        enemyonblock[i] = randomnum; // Save the number
    }
}



void enemyanimations(struct Playerinfo* enemy, struct GameAssets* assets){
    int framecount = 0;
    int frametimer = 0;
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

    if (enemy->animationstate == 2 && enemy->animationindex == 0) { //delay the attack animations, takes time to recharge
        enemy->animationtimer += dt;
        if (enemy->animationtimer < 3.0f) {
            enemy->animationindex = 0;
        } else {
            enemy->animationtimer = 0.0f;  // Reset timer after delay
            enemy->animationindex++;  
        }
    } else if (enemy->animationstate == 3 && enemy->animationindex == 5){
        enemy->animationindex = 5;
        enemy->deadtimer += dt; // Increment dead timer
        if (enemy->deadtimer >= 2.0f) {
            enemy->dead = true; // Mark the enemy as dead after 2 seconds
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
    //printf("Animation State: %d, Frame: %d\n", enemy->animationstate, enemy->animationindex);
    //printf("%.2f, %.2f\n", destRect.width, destRect.height);
}

void removeDeadEnemies(struct Playerinfo enemies[MAX_ENEMIES], int* enemyCount) {
    for (int i = 0; i < *enemyCount; i++) {
        if (enemies[i].dead && enemies[i].deadtimer >= 5.0f) {
            (*enemyCount)--;
            i--; // Check the current index again as it now contains the next enemy
        }
    }
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

void iteratearrowanimation(int facedirection, Texture2D texture, struct GameAssets* assets, Vector2* arrowPos,
                            Vector2 enemypos, Rectangle rec, Rectangle playerHitbox, int blockCount, struct Playerinfo* player, bool* arrowActive) {
    
    float dt = GetFrameTime();
    static bool arrowMovingDown = false; 
    static bool arrowonground = false;
    static float arrowongroundtimer = 0.0f;
    static float arrowtimer = 0.0f;
    static float speedX = 140.0f; 
    static float speedY = 30.0f; 
    static float arrowRotation = 0.0f;
    arrowtimer += dt;

    if (!*arrowActive) {
        arrowtimer = 0.0f; 
        return;
    }

    if (arrowtimer >= 0.3f) {
        arrowMovingDown = true; 
    }

    if (facedirection < 0){
        speedX = -fabs(speedX); 
    }
    arrowPos->x += speedX * dt;
    if (arrowMovingDown && !arrowonground){
        arrowPos->y += speedY * dt;
        arrowRotation += arrowtimer * 0.2;
    } 


    Rectangle src = {1, 0, 361, texture.height};
    Rectangle dst = {arrowPos->x, arrowPos->y, src.width / 3.5, src.height / 3.5};
    Vector2 origin = {dst.width / 2.0f, dst.height / 2.0f};
    Rectangle arrowtip = {arrowPos->x + dst.width / 2 - (arrowRotation * 0.45), arrowPos->y + (arrowRotation * 0.75), 4, 4}; //to offset the arrowtip hitbox

    if (facedirection < 0) {
        src.x += src.width;
        src.width = -fabs(src.width);
        dst.width = fabs(dst.width);
        arrowtip.x = arrowPos->x - dst.width / 2 + (arrowRotation * 0.45);
    }

    for (int i = 0; i < blockCount; i++) {
        if (CheckCollisionRecs(arrowtip, blocksarray[i].rect)) {
            printf("Arrow collided with block %d\n", i);
            *arrowActive = false; 
            arrowMovingDown = false;
            arrowtimer = 0.0f;
            return;
        }
    }

    // Check collision with the ground
    if (CheckCollisionRecs(arrowtip, rec)) {
        printf("Arrow collided with the ground.\n");
        speedX = 0.0f;
        speedY = 0.0f;
        arrowMovingDown = false;
        arrowonground = true;

        arrowongroundtimer += dt;
        if (arrowongroundtimer >= 3.0f){
            *arrowActive = false;
            arrowongroundtimer = 0.0f; 
            arrowtimer = 0.0f;
            speedX = 140.0f;
            speedY = 30.0f;
            arrowRotation = 0.0f;
            arrowonground = false;
            return;
        }
    }

    if (CheckCollisionRecs(arrowtip, playerHitbox)) {
        printf("Arrow collided with the player.\n");
        player->currenthp -= 10; 
        *arrowActive = false; 
        arrowtimer = 0.0f;
        speedX = 140.0f;
        speedY = 30.0f;
        arrowRotation = 0.0f;
        arrowonground = false;
        return;
    }

    DrawRectangleRec(arrowtip, RED);
    DrawTexturePro(texture, src, dst, origin, (arrowRotation) * facedirection, WHITE);
}

void checkPlayerAttackCollision(struct Playerinfo* player, struct Playerinfo enemies[MAX_ENEMIES], int facedirection) {
    Rectangle playerattackHitbox = {};
    
    if (!player->attack) {
        return; // No need to check for collisions if the player is not attacking
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
                enemies[i].hitpoints -= 1;
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
}

void playerenemyhpbar(struct Playerinfo* player, struct Playerinfo enemies[MAX_ENEMIES], struct GameAssets* assets, int enemyCount, Camera2D* camera) {
    static float playerlastframehp = 100.0f;   //player maxhealth is 100
    static int playerhpindex = 0;         
    float playerhpdiff = playerlastframehp - player->currenthp;

    if (playerhpdiff >= 12.5f) {
        playerhpindex++;
        if (playerhpindex > 7) {
            playerhpindex = 7; 
        }
    } /*else if (playerhpdiff < 0) { //if the player is healed
        playerhpindex--;
        if (playerhpindex < 0) {
            playerhpindex = 0; 
        }
    }*/
    playerlastframehp = player->currenthp;
    Rectangle playerhpsrc = {337 - (48 * playerhpindex), 1, 46, 14};
    // make the drawn texture follow the camera target so that it moves together
    Rectangle playerhpdest = {camera->target.x - 550, camera->target.y - 350, playerhpsrc.width * 3, playerhpsrc.height * 3};
    Vector2 origin = {0, 0};

    printf("\nplayerposition: %.2f", player->Position.y);
    //printf("\nhpdest: %.2f", playerhpdest.x);
    DrawTexturePro(assets->texture[21], playerhpsrc, playerhpdest, origin, 0, WHITE);
}


int main()
{
    InitWindow(windwidth, windheight, "Gravity game");
    InitAudioDevice();

    struct GameAssets assets = {0};
    {
    assets.images[assets.imagecount++] = LoadImage("Images/ori.png"); ///the icon
    assets.images[assets.imagecount++] = LoadImage("Images/tilecompleteset.png");  //probnot
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
    assets.images[assets.imagecount++] = LoadImage("Images/shopbg.png"); //12
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

    //assets.images[assets.imagecount++] = LoadImage("Landing_KG_2.gif");

    assets.music[assets.musiccount++] = LoadMusicStream("Music/13 Always With Me_ Spirited Away (Pi.mp3");
    assets.music[assets.musiccount++] = LoadMusicStream("Music/homemusic.mp3");
    assets.music[assets.musiccount++] = LoadMusicStream("Music/shop_music.mp3"); //2

    assets.sound[assets.soundcount++] = LoadSound("Music/Menu_Hover.mp3");
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
    }
    
    struct Playerinfo Playerdata;
    Camera2D camera = Camerasettings(&Playerdata);
    Gamestate currentGameState = MENU;
    int currentmusic, enemyonblock[MAX_ENEMIES]; 
    int blockcount = loadmap("map.txt");
    int playerlastframedirection = 1;
    int playercurrentframe = 0;
    int playeranimationindex = 0;
    int enemycount = MAX_ENEMIES;
    float musicVolume;
    randomenemypos(&blockcount, enemyonblock);
    initializeGameState(&assets, &Playerdata, &currentGameState, &currentmusic, &musicVolume);


    while (!WindowShouldClose())
    {
        UpdateMusicStream(assets.music[currentmusic]);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        handleGameState(&currentGameState, &camera, &assets, &Playerdata, &blockcount, 
                        &playerlastframedirection, &playercurrentframe, &playeranimationindex, 
                        enemyonblock, enemies, &enemycount, &currentmusic);
    
        EndDrawing();
        //printf("Player Position: (%.2f, %.2f)\n", Playerdata.Position.x, Playerdata.Position.y + Playerdata.height);
    }

    FILE* file = fopen("settings.txt", "r");
    if (file){
        fscanf(file, "MusicVolume=%f\n", &musicVolume);
        fclose(file);}
    savegamedata(&Playerdata, &currentGameState, &currentmusic, musicVolume, enemycount, enemies, &camera);
    Unloadresources(&assets); 
    CloseAudioDevice();
    CloseWindow();
    return 0;
}