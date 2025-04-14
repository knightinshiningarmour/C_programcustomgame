#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <raymath.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define MAX_ENEMIES 3

const int windwidth = 1200;
const int windheight = 800;
const int mapwidth = windwidth * 3;
const int mapheight = -windheight * 5;

struct GameAssets 
{ //store game assets
    Image images[30];
    Music music[10];
    Sound sound[20];
    Texture2D texture[30];
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
    float deadtimer;
};

typedef enum 
{
    MENU,
    LOADSAVES,
    OPTIONS,
    QUIT,
    PLAYING,
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
void keepobjectwithinscreen(struct Playerinfo* object);
void LoadAnimationDataplayer(struct GameAssets* assets);
void iterateanimationplayer(struct GameAssets* assets, struct Playerinfo* player, int* currentframecount, int* facedirection, int* i);
void enemymovement(struct Playerinfo* enemy, struct Playerinfo* player, int enemyonblock[MAX_ENEMIES], int enemyIndex);
void enemyanimations(struct Playerinfo* enemy, struct GameAssets* assets);
void checkPlayerAttackCollision(struct Playerinfo* player, struct Playerinfo enemies[MAX_ENEMIES], int facedirection);
void removeDeadEnemies(struct Playerinfo enemies[MAX_ENEMIES], int* enemyCount);
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

        case PAUSE: 
        {
            Rectangle pausebgsrc = {0, 0, 120, 140};
            Rectangle pausebgdest = {600, 400, 800, windheight + 40};
            Vector2 pausebgorigin = {pausebgdest.width/2, pausebgdest.height/2};
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

            DrawTexturePro(assets->texture[25], pausebgsrc, pausebgdest, pausebgorigin, 0, WHITE);
            DrawTexturePro(assets->texture[5], charactersrc, characterdest, characterorigin, 0, WHITE);
            aligntextcentre(200 + pausebgdest.width/2, 150, 50, "PAUSED", BLACK);   
            DrawTexturePro(assets->texture[19], pauseenemyskelsrc, pauseenemyskeldestright, pauseenemyskelorigin, 0, WHITE);
            
            for (int i = 0; i < 3; i++){
                Rectangle emptybuttonsrc = {0, 0, assets->texture[23].width, assets->texture[23].height};
                Rectangle emptybuttondest = {450, 250 + i * 150, 300, 100};
                DrawTexturePro(assets->texture[23], emptybuttonsrc, emptybuttondest, (Vector2){0, 0}, 0.0f, WHITE);
                aligntextcentre(emptybuttondest.x + emptybuttondest.width / 2, emptybuttondest.y + emptybuttondest.height / 2, 30, text[i], BLACK);
            }
        
            Rectangle resumeButton = {450, 250, 300, 100};
            Rectangle saveButton = {450, 400, 300, 100};
            Rectangle backButton = {450, 550, 300, 100};

            if (CheckCollisionPointRec(mousePos, saveButton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                confirmedsave = true;
            }
            if (CheckCollisionPointRec(mousePos, resumeButton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                *currentGameState = PLAYING; // Resume the game
                savetimer = 0.0f;
            }
            if (CheckCollisionPointRec(mousePos, backButton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                *currentGameState = MENU; // Return to the main menu
                gamedataloaded = false;
            }

            if (confirmedsave) {
                DrawTexturePro(assets->texture[23], (Rectangle){0, 0, assets->texture[23].width, assets->texture[23].height},(Rectangle){280, saveButton.y, 640, saveButton.height}, (Vector2){0, 0}, 0.0f, WHITE);
                aligntextcentre(605, saveButton.y + saveButton.height / 2, 20, "Confirm Save? The previous data will be overwritten.", BLACK);
                
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
                        savegamedata(Playerdata, currentGameState, currentmusic, musicVolume, *enemycount, enemies, camera);
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
            break;
        }

        case GAMEOVER:
        {
            DrawText("Game Over. Press ENTER to Restart", windwidth / 2 - 150, windheight / 2, 20, BLACK);
            if (IsKeyPressed(KEY_ENTER))
            {
                *currentGameState = MENU;
            }
            break;
        }
    }
}

void initializeGameState(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume){
    // Initialize player data
    Playerdata->Position = (Vector2){windwidth / 2 - 50, windheight - 100};
    Playerdata->width = 50;
    Playerdata->height = 50;
    Playerdata->currenthp = 100;
    Playerdata->hitpoints = 100;
    Playerdata->isJumping = false;
    Playerdata->isfalling = false;
    Playerdata->attack = false;
    Playerdata->onshield = false;
    Playerdata->animationstate = 0;
    *currentGameState = MENU;
    *currentmusic = -1; 
    *musicVolume = 0.5f;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i] = (struct Playerinfo){
            .Position = {200 + i * 100, windheight - 240},
            .width = 50,
            .height = 50,
            .currenthp = 75,
            .hitpoints = 75,
            .dead = false,
            .animationstate = 0,
            .facedirection = (i % 2 == 0) ? 1 : -1
        };
    }
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
    drawbackground(assets, camera, 1, 0.7f);
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
}

void handleLoadSaves(struct GameAssets* assets, struct Playerinfo* Playerdata, Gamestate* currentGameState, int* currentmusic, float* musicVolume, Camera2D* camera, bool *gamedataloaded){
    
    Rectangle savesbgsrc = {0, 0, 120, 140};
    Rectangle savesbgdest = {600, 400, 800, windheight + 40};
    Vector2 savesbgorigin = {savesbgdest.width/2, savesbgdest.height/2};
    Rectangle loadButton = {450, 250, 300, 100};
    Rectangle newGameButton = {450, 400, 300, 100};
    Rectangle backButton = {450, 550, 300, 100};
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

    for (int i = 0; i < 3; i++){
        Rectangle emptybuttonsrc = {0, 0, assets->texture[23].width, assets->texture[23].height};
        Rectangle emptybuttondest = {450, 250 + i * 150, 300, 100};
        DrawTexturePro(assets->texture[23], emptybuttonsrc, emptybuttondest, (Vector2){0, 0}, 0.0f, WHITE);
        aligntextcentre(emptybuttondest.x + emptybuttondest.width / 2, emptybuttondest.y + emptybuttondest.height / 2, 30, textload[i], BLACK);
    }

    if (CheckCollisionPointRec(mousePos, newGameButton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Playerdata->Position.x = windwidth / 2 - Playerdata->width / 2;
        Playerdata->Position.y = windheight - Playerdata->height;
        *currentGameState = PLAYING; 
        *gamedataloaded = true;
    }
    if (CheckCollisionPointRec(mousePos, backButton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        *currentGameState = MENU;
    }
    if (CheckCollisionPointRec(mousePos, loadButton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        *currentGameState = PLAYING;
    }
}

void handleOptionsState(struct GameAssets* assets, Gamestate* currentGameState, int* currentmusic, float* musicVolume){
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

    const char* controls[] = {"Space", "A/D", "Left Mouse", "Shift", "P"};
    const char* functions[] = {"Jump", "Left/Right", "Attack", "Parry", "Pause"};
    int FontSize = 20; 
    int emptybuttonWidth = 150; 
    int emptybuttonHeight = 50; 
    int controlsStartY = volumebuttondest.y + volumebuttondest.height + 50; // Starting Y position for controls
    int buttonSpacing = 20; // Spacing between rows
    int buttonGap = 20; // Gap between key and function buttons
    static float settingssavedtimer = 0.0f;
    
    // Draw "Controls" title
    DrawText("Controls:", 300, controlsStartY, FontSize + 5, BLACK);

    int numControls = 5;
    for (int i = 0; i < numControls; i++) {
        Rectangle keyButtonRect = {300, 
                                    controlsStartY + (i + 1) * (emptybuttonHeight + buttonSpacing) - 30, emptybuttonWidth,emptybuttonHeight};
        Rectangle functionButtonRect = {keyButtonRect.x + keyButtonRect.width + buttonGap, controlsStartY + (i + 1) * (emptybuttonHeight + buttonSpacing) - 30, 
                                        emptybuttonWidth + 50, emptybuttonHeight};
        DrawTexturePro(assets->texture[23], (Rectangle){0, 0, assets->texture[23].width, assets->texture[23].height},
                        keyButtonRect, (Vector2){0, 0}, 0.0f, WHITE );
        DrawTexturePro(assets->texture[23],(Rectangle){0, 0, assets->texture[23].width, assets->texture[23].height},
                        functionButtonRect, (Vector2){0, 0}, 0.0f, WHITE);
        aligntextcentre(keyButtonRect.x + keyButtonRect.width / 2, keyButtonRect.y + keyButtonRect.height / 2, FontSize, controls[i], BLACK);
        aligntextcentre(functionButtonRect.x + functionButtonRect.width / 2, functionButtonRect.y + functionButtonRect.height / 2, FontSize, functions[i], BLACK);
    }

    Rectangle saveButton = {windwidth / 2 + 150, controlsStartY + 200, 200, 50};
    Rectangle backButton = {saveButton.x, saveButton.y + 80, 200, 50};
    DrawRectangleRec(saveButton, LIGHTGRAY);
    aligntextcentre(saveButton.x + saveButton.width / 2, saveButton.y + saveButton.height / 2, 20, "Save Settings", BLACK);
    DrawRectangleRec(backButton, LIGHTGRAY);
    aligntextcentre(backButton.x + backButton.width / 2, backButton.y + backButton.height / 2, 20, "Back", BLACK);
    DrawRectangleLines(saveButton.x, saveButton.y, saveButton.width, saveButton.height, BLACK);
    DrawRectangleLines(backButton.x, backButton.y, backButton.width, backButton.height, BLACK);
    
    if (CheckCollisionPointRec(mousePos, saveButton)){
        DrawRectangleRec(saveButton, YELLOW);
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
        DrawRectangleRec(backButton, YELLOW);
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
    if (*currentmusic != 0) { 
        StopMusicStream(assets->music[*currentmusic]);
        PlayMusicStream(assets->music[0]);
        *currentmusic = 0;
    }
    BeginMode2D(*camera);
    if (!*gamedataloaded) {
        *gamedataloaded = true;
        printf("Load Game button clicked. Calling loadgamedata...\n");
        loadgamedata(assets, Playerdata, currentGameState, currentmusic, musicVolume, enemycount, enemies, camera);
        printf("Load Game\n");
    }
    for (int j = 1; j < 5; j++){
        if (j == 1){
            drawbackground(assets, camera, j, 0.7);
        }
        drawbackground(assets, camera, j, 1.0);
    }
    drawobstacles(blockcount, assets);
    *playerlastframedirection = calculatemovementplayer(Playerdata, blockcount, assets);
    updatecamera(camera, Playerdata);
    keepobjectwithinscreen(Playerdata);
    iterateanimationplayer(assets, Playerdata, playercurrentframe, playerlastframedirection, playeranimationindex);

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemymovement(&enemies[i], Playerdata, enemyonblock, i);
        enemyanimations(&enemies[i], assets);
    }
    checkPlayerAttackCollision(Playerdata, enemies, *playerlastframedirection);
    removeDeadEnemies(enemies, enemycount);
    EndMode2D();

    if (IsKeyPressed(KEY_P)){
        *currentGameState = PAUSE;
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
}

Camera2D Camerasettings(struct Playerinfo* player){
    Camera2D camera;
    //camera.offset = (Vector2){windwidth/2, windheight/2};
   // camera.target = (Vector2){player->Position.x + player->width / 2, player->Position.y + player->height / 2};
    camera.rotation = 0.0f;
    camera.zoom = 1;
    return camera;
}

void updatecamera(Camera2D* camera, struct Playerinfo* player){
    if (player->Position.x + player->width/2  < windwidth/2 - 100){
        camera->offset =  (Vector2){250, windheight - 150};
        //points to the centre point of the first half frame
        camera->target = (Vector2){(windwidth/2-player->width)/2, player->Position.y + player->height / 2};
        //printf("camera target1: %.2f\n", camera->target.y);
    }

    else if (player->Position.x + player->width/2 >= windwidth/2 - 100 && player->Position.x + player->width/2 <= 5*windwidth/2 - 100){
        camera->offset = (Vector2){(windwidth-player->width)/2 - 50, windheight-150};
        camera->target = (Vector2){player->Position.x + player->width / 2, player->Position.y + player->height / 2};
        //printf("camera target2: %.2f\n", camera->target.x);
    }

        else if (player->Position.x + player->width/2 >= 5*windwidth/2 - 100) {
        camera->offset = (Vector2){(windwidth-player->width)/2 + 100, windheight-150};
        camera->target = (Vector2){5*windwidth/2 + 100 - player->width/2, player->Position.y + player->height / 2};
        //printf("camera target3: %.2f, %.2f\n", camera->target.x, camera->target.y);
    }

    ///////p.s. there is an issue when the character jumps from 2850 area to the fixed camera area and the y axis stuck
}

void drawbackground (struct GameAssets* assets, Camera2D* camera, int x, float scalefactor) {
    int numTiles = (mapwidth / windwidth) + 1; 
    float skySpeed = 0.1f;      
    float mountainSpeed1 = 0.3f; 
    float mountainSpeed2 = 0.5f; 

    float skyOffset = -camera->target.x * skySpeed;
    float mountainOffset1 = -camera->target.x * mountainSpeed1;
    float mountainOffset2 = -camera->target.x * mountainSpeed2;

    switch (x){
        case 1:
            for (int i = 0; i<(mapwidth/128*scalefactor); i++){
                Rectangle groundtilessrc = {0, 0, 128, 128};
                Rectangle groundtilesdest = {i * (groundtilessrc.width*scalefactor), windheight - (groundtilessrc.height*scalefactor), (float)(groundtilessrc.width * scalefactor), (float)(groundtilessrc.height * scalefactor)};
                Vector2 origin = {0, 0};
                DrawTexturePro(assets->texture[13], groundtilessrc, groundtilesdest, origin, 0, WHITE);
            }
            break;
        case 2:
            for (int i = 0; i < numTiles; i++) {
                Rectangle skysrc = {0, 0, assets->texture[6].width, assets->texture[6].height};
                Rectangle skydest = {i * (skysrc.width*scalefactor), -100, (float)skysrc.width * scalefactor, (float)skysrc.height * scalefactor};  
                Vector2 origin = {0, 0};
                DrawTexturePro(assets->texture[6], skysrc, skydest, origin, 0, WHITE);
            }
            break;
        case 3:
            for (int i = 0; i < numTiles; i++) {
                Rectangle mountainsrc = {0, 0, assets->texture[7].width, assets->texture[7].height};
                Rectangle mountaindest = {i * mountainsrc.width, windheight - 500, windwidth, 400}; // Adjust position and size
                Vector2 origin = {0, 0};
                DrawTexturePro(assets->texture[7], mountainsrc, mountaindest, origin, 0, WHITE);
            }
            break;
        case 4:
            for (int i = 0; i < numTiles; i++) {
                Rectangle mountainsrc2 = {0, 0, assets->texture[8].width, assets->texture[8].height};
                Rectangle mountaindest2 = {i * mountainsrc2.width, windheight - 400, windwidth, 300}; // Adjust position and size
                Vector2 origin = {0, 0};
                DrawTexturePro(assets->texture[8], mountainsrc2, mountaindest2, origin, 0, WHITE);
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

    if (!player->onplatform && (!player->attack) &!player->onshield) {  
        player->isrunning = false;
        player->velocityY += gravity * dt;

        if (player->velocityY >= 0) {
            player->isfalling = true;
            player->animationstate = 3; //falling animation
        } else {
            player->isfalling = false;
        }
    }

    player->Position.y += player->velocityY * dt;
    collisionplayerblocks('y', player, maxplatform, &facedirection);

    if (player->animationstate == 2) { // If currently in a jump animation, let it finish before changing state
        return facedirection;
    }

    if (player->onplatform && !player->attack && !player->onshield) {
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_D)) 
        {
            if (IsKeyDown(KEY_A) && IsKeyDown(KEY_D)) //make sure that when both keys pressed at the same time character doesn't move
            {
                player->animationstate = 0;
                player->isrunning = false;
            }
            else
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
        if (player->animationstate == 5) //if shielding, maintain the last frame
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

int loadmap(const char* filename){
    FILE* File = fopen(filename, "w");
    char platforms[][20] = {
                            "0000000000000000", 
                            "0000000000000000",
                            "0000000000000000",
                            "0000000000000000",
                            "0000000000000000",
                            "0000000000000000",
                            "0000000000000000",
                            "0100110000000100",
                            "0001000000100000",
                            "0000000000000000",                       
                            "0010000000001100",
                            "2010000001000000",
                            "0000000000000000",
                            "0111111110000100",
                                };
    for (int i=0; i<(sizeof(platforms)/sizeof(platforms[0])); i++) 
    {
        fprintf(File, "%s\n", platforms[i]);
    }
    fclose(File);

    //read from the txt file
    FILE* Fileread = fopen(filename, "r");
    if (!Fileread){
        printf("Failed to open map file! ");
        return 0;
    }

    int blockwidth = 100;
    int blockheight = 50;
    int MAP_HEIGHT = sizeof(platforms)/sizeof(platforms[0]); 
    int MAP_WIDTH = sizeof(platforms[0])/sizeof(platforms[0][0]);
    char line[sizeof(platforms[0]) + 2]; // +2 for '\n' and '\0' when reading the txt file
    int row = 0;
    int i = 0;

    blocksarray = malloc(sizeof(struct Playerinfo) * MAP_HEIGHT * MAP_WIDTH);
    if (blocksarray == NULL) {
        printf("Memory allocation for blocksarray failed!\n");
        fclose(Fileread);
        return 0;
    }

    while (fgets(line, sizeof(line), File) && row < MAP_HEIGHT) 
    {
        for (int col = 0; col < MAP_WIDTH; col++) 
        {
            if (line[col] == '1') 
            {
                blocksarray[i].rect = (Rectangle){col * blockwidth, row * blockheight, blockwidth, blockheight};
                i++;
            }
        }
        for (int col = 0; col < MAP_WIDTH; col++) 
        {
            if (line[col] == '2')
            {
                blocksarray[i].rect = (Rectangle){col * blockwidth, row * blockheight * 3, blockwidth, blockheight * 3};
                i++;
            }
        }
        row++;
    }

    return i;
    fclose(Fileread);
}

void drawobstacles(int* maxplatform, struct GameAssets* assets){
    Rectangle sourceRect ={240, 48, 63,30};
    Vector2 origin = {0, 0};
    for (int i=0; i<*maxplatform; i++){
        DrawTexturePro(assets->texture[12], sourceRect, blocksarray[i].rect, origin, 0, WHITE);
    }
}

void keepobjectwithinscreen(struct Playerinfo* object){
    if ((*object).Position.x < 0){
        (*object).Position.x = 0;
    }
    if ((*object).Position.x > (mapwidth - object->width)){
        (*object).Position.x = mapwidth-(object->width);
    }
    if ((*object).Position.y < mapheight){
        (*object).Position.y = mapheight;
        object->velocityY = 0;
    }
    if ((*object).Position.y > (windheight - object->height)){
        (*object).Position.y = windheight - (object->height);
        (*object).velocityY = 0;
        (*object).isJumping = false;
        object->isfalling = false;
        object->onground = true;
        object->onplatform = true;

        if (!object->isrunning && !object->onshield){
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
        }
    }
}

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

void initializeEnemy(struct Playerinfo* enemy, struct Playerinfo* block, int index, Texture2D texture) {
    *enemy = (struct Playerinfo){
        .Position = {block->rect.x + 10 + index * 20, block->rect.y - ((texture.height / 2) - 45) * 0.15},
        .width = 100,
        .height = 100,
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
        printf("Enemy dead animation index: %d, deadtimer: %.2f\n", enemy->animationindex, enemy->deadtimer); // Debug print
        if (enemy->deadtimer >= 2.0f) {
            enemy->dead = true; // Mark the enemy as dead after 2 seconds
            printf("Enemy marked as dead\n"); // Debug print
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
void enemymovement(struct Playerinfo* enemy, struct Playerinfo* player, int enemyonblock[MAX_ENEMIES], int enemyIndex){
    float dt = GetFrameTime();
    float speed = 50.0f;

    if (fabs(enemy->Position.x - player->Position.x) <= 150 && !enemy->dead){
        if (enemy->Position.y - player->Position.y <= 100 && enemy->Position.y - player->Position.y >= -100){
            enemy->animationstate = 2; // Attack animation
            enemy->attack = true;
            speed = 0;

            if (enemy->Position.x < player->Position.x){ //if enemy is on the left of player, turn right
                enemy->facedirection = 1;
            } else {
                enemy->facedirection = -1;
            }

        } else {
            enemy->animationstate = 1; // Walking animation
            enemy->attack = false;
        }

    } else {
        enemy->animationstate = 1; // Walking animation
        enemy->attack = false;
    }

    if (!enemy->attack && !enemy->dead){
        if (enemy->Position.x + enemy->width/2 <= blocksarray[enemyonblock[enemyIndex]].rect.x + 10){
            enemy->facedirection = 1;
        }
        else if (enemy->Position.x + enemy->width/2 >= blocksarray[enemyonblock[enemyIndex]].rect.x + blocksarray[enemyonblock[enemyIndex]].rect.width){
            enemy->facedirection = -1;
        }
    }

    if (enemy->dead){
        if (enemy->animationindex > 5){
            enemy->animationindex = 0;
        }
        enemy->animationstate = 3;
        speed = 0;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemy != &enemies[i] && !enemies[i].dead) {
            if (CheckCollisionRecs((Rectangle){enemy->Position.x+5, enemy->Position.y, enemy->width - 10, enemy->height},
                                   (Rectangle){enemies[i].Position.x, enemies[i].Position.y, enemies[i].width, enemies[i].height})) {
                speed = 0;
            }
        }
    }

    //printf("%d", enemy->facedirection);
    enemy->Position.x += dt * speed * (enemy->facedirection);
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
        }
    }
}

int main()
{
    InitWindow(windwidth, windheight, "Gravity game");
    InitAudioDevice();

    struct GameAssets assets = {0};
    assets.images[assets.imagecount++] = LoadImage("Images/ori.png");
    assets.images[assets.imagecount++] = LoadImage("Images/tilecompleteset.png");
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
    assets.images[assets.imagecount++] = LoadImage("Images/forestbackground.png"); ///might not use
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
    //assets.images[assets.imagecount++] = LoadImage("Landing_KG_2.gif");

    assets.music[assets.musiccount++] = LoadMusicStream("Music/13 Always With Me_ Spirited Away (Pi.mp3");
    assets.music[assets.musiccount++] = LoadMusicStream("Music/homemusic.mp3");
    assets.music[assets.musiccount++] = LoadMusicStream("Music/shopmusic.mp3"); //2

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
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[12]);
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
    for (int i = 0; i < MAX_ENEMIES; i++) {
        initializeEnemy(&enemies[i], &blocksarray[enemyonblock[i]], i, assets.texture[16]);
    }

    while (!WindowShouldClose())
    {
        UpdateMusicStream(assets.music[currentmusic]);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        handleGameState(&currentGameState, &camera, &assets, &Playerdata, &blockcount, 
                        &playerlastframedirection, &playercurrentframe, &playeranimationindex, 
                        enemyonblock, enemies, &enemycount, &currentmusic);
    
        EndDrawing();
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