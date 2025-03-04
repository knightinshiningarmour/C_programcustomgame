#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <raymath.h>
#include <string.h>
#include <time.h>

const int windwidth = 1200;
const int windheight = 800;

struct GameAssets 
{ //store game assets
    Image images[20];
    Music music[10];
    AudioStream audio[10];
    Texture2D texture[20];
    float src_idlex[5];
    float src_idley[5];
    float src_runningx[7];
    float src_runningy[7];
    float src_jumpingx[10];
    float src_jumpingy[10];
    float src_jumpingwidth[10];
    float src_jumpingheight[10];

    int frameCount;                // Total frames in this animation
    float frameSpeed; 
    int texturecount;
    int imagecount;
    int musiccount;
    int audiocount;
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
    bool isidling;
    bool isrunning;
    bool onplatform;
    int animationstate;
};


struct Playerinfo *blocksarray = NULL;

void Unloadresources(struct GameAssets* assets){
    printf("%d images to be unloaded....", (assets->imagecount));
    for (int i=0; i<assets->imagecount; i++){
        UnloadImage(assets->images[i]);
    }

    printf("%d musics to be unloaded....", (assets->musiccount));
    for (int i=0; i<assets->musiccount; i++){
        UnloadMusicStream(assets->music[i]);
    }

    printf("%d audio to be unloaded....", (assets->audiocount));
    for (int i=0; i<assets->audiocount; i++){
        UnloadAudioStream(assets->audio[i]);
    }
    
    printf("%d textureright to be unloaded....", (assets->texturecount));
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


void collisionplayerblocks(char axis, struct Playerinfo* object, int* maxplatform, int* facedirection) {
    Rectangle player = {(*object).Position.x, (*object).Position.y, (*object).width, (*object).height};
    for (int i=0; i<*maxplatform; i++)
    {
        if (axis == 'y')
        {
            if (CheckCollisionRecs(player, blocksarray[i].rect))
            {
                object->animationstate = 1;
                //object->isJumping = false;
                if ((*object).velocityY < 0) //means moving upwards
                {
                    (*object).Position.y = blocksarray[i].rect.y + blocksarray[i].rect.height;
                    (*object).velocityY = 0;
                    object->onplatform = false;
                }
                else if ((*object).velocityY > 0){
                    (*object).Position.y = blocksarray[i].rect.y - (*object).height;
                    object->onplatform = true; //detected collision from above the platform
                    (*object).velocityY = 0;  
                }

                if (object->onplatform){
                    object->isJumping = false;
                }

                //printf("Player Y: %.2f, Platform Y: %.2f\n", object->Position.y, blocksarray[i].rect.y);
            }
        }

        if (axis == 'x')
        {
            if (CheckCollisionRecs(player, blocksarray[i].rect))
            {
                if ((*object).direction.x > 0) // moving right
                {
                    (*object).Position.x = blocksarray[i].rect.x - (*object).width;
                }
                else if ((*object).direction.x < 0) // moving left
                {
                    (*object).Position.x = blocksarray[i].rect.x + blocksarray[i].rect.width;
                }
            }
        }
    }
}

int calculatemovement(struct Playerinfo *player, int* maxplatform) {
    static int facedirection = 1;
    float dt = GetFrameTime();
    float speed = 150.0f;
    float jumpForce = -500.0f;
    float gravity = 800.0f;

    if (IsKeyPressed(KEY_SPACE) && !player->isJumping) {
        player->velocityY = jumpForce;
        player->isJumping = true;
        player->animationstate = 2;  // Jumping state
        player->isrunning = false;
    }

    player->direction.x = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
    if (player->direction.x > 0){
         facedirection = 1;
    }
    else if (player->direction.x < 0) {
        facedirection = -1;}

    player->Position.x += dt * speed * player->direction.x;
    collisionplayerblocks('x', player, maxplatform, &facedirection);
    player->velocityY += gravity * dt;
    player->Position.y += player->velocityY * dt;
    collisionplayerblocks('y', player, maxplatform, &facedirection);

    if (player->animationstate == 2) {
        // If currently in a jump animation, let it finish before changing state
        return facedirection;
    }

    player->animationstate = 0;
    
    if ((IsKeyDown(KEY_A) || IsKeyDown(KEY_D)) && !(IsKeyPressed(KEY_SPACE))) {
        player->animationstate = 1;
    }

    player->direction.x = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A));

    if (IsKeyDown(KEY_D) && IsKeyDown(KEY_A)){
        player->animationstate = 0; // both keys pressed at the same time, idling
    }

    if (player->direction.x > 0){
         facedirection = 1;
    }
    else if (player->direction.x < 0) {
        facedirection = -1;}

    if (player->isrunning) {
        player->animationstate = 1; // Running state
    }

    player->Position.x += dt * speed * player->direction.x;
    collisionplayerblocks('x', player, maxplatform, &facedirection);

    return facedirection;
}


void LoadAnimationData(char direction, struct GameAssets* assets) {
    FILE *file = fopen("walking animations.txt", "r");
    if (file == NULL) {
        printf("Error: Cannot open file for reading!\n");
    }

    char line[100];  // Buffer for reading lines

    while (fgets(line, sizeof(line), file)) {
        if (direction == 'r'){
            if (strstr(line, "running animations:")) 
            {
                fscanf(file, "srcx = %f, %f, %f, %f, %f, %f\n", &assets->src_runningx[0], &assets->src_runningx[1], 
                       &assets->src_runningx[2], &assets->src_runningx[3], &assets->src_runningx[4], &assets->src_runningx[5]);
                fscanf(file, "srcy = %f, %f, %f, %f, %f, %f\n", &assets->src_runningy[0], &assets->src_runningy[1], 
                       &assets->src_runningy[2], &assets->src_runningy[3], &assets->src_runningy[4], &assets->src_runningy[5]);
            } 
        }

        else if (direction == 'i'){
            if (strstr(line, "idling animations:")) 
            {
                fscanf(file, "srcx = %f, %f, %f, %f\n", &assets->src_idlex[0], &assets->src_idlex[1], &assets->src_idlex[2], &assets->src_idlex[3]);
                fscanf(file, "srcy = %f, %f, %f, %f\n", &assets->src_idley[0], &assets->src_idley[1], &assets->src_idley[2], &assets->src_idley[3]);
            }
        }

        else if (direction == 'j'){
            if (strstr(line, "jumping animations:")) 
            {
                fscanf(file, "srcx = %f, %f, %f, %f, %f, %f, %f, %f, %f\n", &assets->src_jumpingx[0], &assets->src_jumpingx[1], 
                       &assets->src_jumpingx[2], &assets->src_jumpingx[3], &assets->src_jumpingx[4], &assets->src_jumpingx[5], 
                       &assets->src_jumpingx[6], &assets->src_jumpingx[7], &assets->src_jumpingx[8]);
                fscanf(file, "width = %f, %f, %f, %f, %f, %f, %f, %f, %f\n", &assets->src_jumpingwidth[0], &assets->src_jumpingwidth[1], 
                       &assets->src_jumpingwidth[2], &assets->src_jumpingwidth[3], &assets->src_jumpingwidth[4], &assets->src_jumpingwidth[5], 
                       &assets->src_jumpingwidth[6], &assets->src_jumpingwidth[7], &assets->src_jumpingwidth[8]);
                fscanf(file, "srcy = %f, %f, %f, %f, %f, %f, %f, %f, %f\n", &assets->src_jumpingy[0], &assets->src_jumpingy[1], 
                       &assets->src_jumpingy[2], &assets->src_jumpingy[3], &assets->src_jumpingy[4], &assets->src_jumpingy[5], 
                       &assets->src_jumpingy[6], &assets->src_jumpingy[7], &assets->src_jumpingy[8]);
                fscanf(file, "height = %f, %f, %f, %f, %f, %f, %f, %f, %f\n", &assets->src_jumpingheight[0], &assets->src_jumpingheight[1], 
                       &assets->src_jumpingheight[2], &assets->src_jumpingheight[3], &assets->src_jumpingheight[4], &assets->src_jumpingheight[5], 
                       &assets->src_jumpingheight[6], &assets->src_jumpingheight[7], &assets->src_jumpingheight[8]);
            }
        }
    }
    fclose(file);
}

void iterateanimationsprites(Texture2D texture, struct GameAssets* assets, struct Playerinfo* player, int* currentframecount, int* facedirection, int* i) {
    int framescount = 0;
    int frametimer = 0;
    if (player->animationstate == 0) {
        framescount = 4; 
        frametimer = 9; 
    } 
    else if (player->animationstate == 1) {
        framescount = 6; 
        frametimer = 9;
    } 
    else if (player->animationstate == 2) {
        framescount = 9; 
        frametimer = 6;
    }


    (*currentframecount)++;

    if (*currentframecount % (frametimer) == 1) {
        (*i)++;
        *currentframecount = 2;
    }

    if (*i >= framescount) {
        *i = 0;
        if (player->animationstate == 2 && player->isJumping == false) {
            player->animationstate = 0;  
            player->isJumping = false;
        }

        else if (player->animationstate == 2 && player->isJumping){
            player->animationstate = 2;
            *i = 3;
        }
    }

    Rectangle sourceRect, destRect;

    if (player->animationstate == 1) {  // Running Animation
        sourceRect = (Rectangle){assets->src_runningx[*i], assets->src_runningy[*i], 500, 500};
        player->width = sourceRect.width * 0.2;
        player->height = sourceRect.height * 0.2;
    } 
    else if (player->animationstate == 0) {  // Idle Animation
        sourceRect = (Rectangle){assets->src_idlex[*i], assets->src_idley[*i], 300, 500};
        player->width = sourceRect.width * 0.2;
        player->height = sourceRect.height * 0.2;
    } 
    else if (player->animationstate == 2) {  // Jumping Animation
        sourceRect = (Rectangle){assets->src_jumpingx[*i], assets->src_jumpingy[*i], assets->src_jumpingwidth[*i], assets->src_jumpingheight[*i]};
        player->width = 300 * 0.2;
        player->height = 500 * 0.2;
    }

    destRect = (Rectangle){player->Position.x, player->Position.y, player->width, player->height};

    // Flip sprite if needed
    if (*facedirection < 0) {
        sourceRect.width = -fabs(sourceRect.width);
        destRect.width = fabs(destRect.width);
    }

    Vector2 origin = {0, 0};
    DrawTexturePro(texture, sourceRect, destRect, origin, 0.0f, WHITE);
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
                            "0000000000100000",
                            "0000000000000000",                       
                            "0010000000001100",
                            "0010000001000000",
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

    int blockwidth = 70;
    int blockheight = 50;
    int MAP_HEIGHT = sizeof(platforms)/sizeof(platforms[0]);
    int MAP_WIDTH = sizeof(platforms[0])/sizeof(platforms[0][0]);
    printf("%d, %d", MAP_HEIGHT, MAP_WIDTH);
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
                blocksarray[i].colour = GRAY;
                i++;
            }
        }
        row++;
    }

    return i;
    fclose(Fileread);
}

void drawobstacles(int* maxplatform){
    for (int i=0; i<*maxplatform; i++){
        DrawRectangleRec(blocksarray[i].rect, blocksarray[i].colour);
        //printf("%.2f, %.2f, %.2f\n", blocksarray[i].rect.x, blocksarray[i].rect.y, blocksarray[i].rect.width);
    }
}
    


void keepobjectwithinscreen(struct Playerinfo* object){
    if ((*object).Position.x < 0){
        (*object).Position.x = 0;
    }
    if ((*object).Position.x > (windwidth - object->width)){
        (*object).Position.x = 1200-(object->width);
    }
    if ((*object).Position.y < 0){
        (*object).Position.y = 0;
        object->velocityY = 0;
    }
    if ((*object).Position.y > (windheight - object->height)){
        (*object).Position.y = windheight - (object->height);
        (*object).velocityY = 0;
        (*object).isJumping = false;
    }
}



int main()
{

    InitWindow(windwidth, windheight, "Gravity game");
    InitAudioDevice();

    struct GameAssets assets = {0};
    assets.images[assets.imagecount++] = LoadImage("ori.png");
    assets.images[assets.imagecount++] = LoadImage("sos.png");
    assets.images[assets.imagecount++] = LoadImage("yellowhaircharass.png"); //2
    assets.music[assets.musiccount++] = LoadMusicStream("13 Always With Me_ Spirited Away (Pi.mp3");

    ImageFormat(&assets.images[0], PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    SetWindowIcon(assets.images[0]); 
    PlayMusicStream(assets.music[0]);
    SetTargetFPS(60);

    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[2]);
    Texture2D mytexture = assets.texture[0];
    if (mytexture.id == 0) {
        printf("Error: Texture failed to load!\n");
    }

    struct Playerinfo Playerdata ={.Position = {400, (200)},
                                   .isJumping = false,
                                   //.walking = false
                                  };

    int blockcount = loadmap("map.txt");
    int playerlastframedirection = 1;
    int currentFrame = 0;
    float animationTimer = 0;
    int animationindex = 0;
    LoadAnimationData('i', &assets);
    LoadAnimationData('j', &assets);
    LoadAnimationData('r', &assets);

    while (!WindowShouldClose())
    {
        UpdateMusicStream(assets.music[0]);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        drawobstacles(&blockcount);
        playerlastframedirection = calculatemovement(&Playerdata, &blockcount);
        //printf("%d", playerlastframedirection); //check if lastframe player direction is really correct
        iterateanimationsprites(mytexture, &assets, &Playerdata, &currentFrame, &playerlastframedirection, &animationindex);
        //printf("%d", Playerdata.animationstate);
        
        keepobjectwithinscreen(&Playerdata);
        EndDrawing();
    }

    Unloadresources(&assets); 
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

