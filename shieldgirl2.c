#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <raymath.h>
#include <string.h>
#include <time.h>

const int windwidth = 1200;
const int windheight = 800;
const int mapwidth = windwidth * 3;
const int mapheight = -windheight * 5;

struct GameAssets 
{ //store game assets
    Image images[30];
    Music music[10];
    AudioStream audio[10];
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
    bool isfalling;
    bool isidling;
    bool isrunning;
    bool onplatform;
    bool onground;
    bool attack;
    int animationstate;
};

struct Playerinfo *blocksarray = NULL;

// Initialise all the background assets


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

    printf("%d audios to be unloaded....", (assets->audiocount));
    for (int i=0; i<assets->audiocount; i++){
        UnloadAudioStream(assets->audio[i]);
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


Camera2D Camerasettings(struct Playerinfo* player){
    Camera2D camera;
    //camera.offset = (Vector2){windwidth/2, windheight/2};
   // camera.target = (Vector2){player->Position.x + player->width / 2, player->Position.y + player->height / 2};
    camera.rotation = 0.0f;
    camera.zoom = 1;
    return camera;
}

//camera pos = target - offset (-100)
//if want spaces then just offset (350) and the else if + 100, and offset for second +50
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
        camera->offset = (Vector2){windwidth - 50, windheight-150};
        camera->target = (Vector2){5*windwidth/2 + 100, player->Position.y + player->height / 2};
        printf("camera target3: %.2f, %.2f\n", camera->target.x, camera->target.y);
    }
    

    ///////p.s. there is an issue when the character jumps from 2850 area to the fixed camera area and the y axis stuck
}

void drawbackground (struct GameAssets* assets){
    //draw background


    for (int i=0; i<3; i++){ //mountain
        Rectangle mountainsrc = {0, 0, assets->texture[12].width, assets->texture[12].height};
        Rectangle mountaindest = {(-windwidth)/2 + (i * windwidth), windheight-mountainsrc.height-1200, windwidth, windheight*1.8};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[12], mountainsrc, mountaindest, origin, 0, WHITE);
    }

    for (int i=0; i<3; i++){ //mountain
        Rectangle mountainsrc = {0, 0, assets->texture[13].width, assets->texture[13].height};
        Rectangle mountaindest = {(-windwidth)/2 + (i * windwidth), windheight-mountainsrc.height-1200, windwidth, windheight*1.8};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[13], mountainsrc, mountaindest, origin, 0, WHITE);
    }

    for (int i=0; i<3; i++){ //mountain
        Rectangle mountainsrc = {0, 0, assets->texture[14].width, assets->texture[14].height};
        Rectangle mountaindest = {(-windwidth)/2 + (i * windwidth), windheight-mountainsrc.height-1200, windwidth, windheight*1.8};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[14], mountainsrc, mountaindest, origin, 0, WHITE);
    }

    /*for (int i=0; i<3; i++){ //nightsky
        Rectangle mountainsrc = {112, 32, 64, 32};
        Rectangle mountaindest = {500, 300, 800, 200};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[16], mountainsrc, mountaindest, origin, 0, WHITE);
    }*/

    ///can flip the image upside down to draw leaves on top
    
    /*for (int i=0; i<3; i++){ //forest
        Rectangle mountainsrc = {0, 0, assets->texture[11].width, assets->texture[11].height};
        Rectangle mountaindest = {(-windwidth)/2 + (i * windwidth), windheight-mountainsrc.height + 400, windwidth, windheight};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[11], mountainsrc, mountaindest, origin, 0, WHITE);
    }*/


    ///trees, mountain, sky
    drawtrees(assets, 2, 0, windheight/3, 2); //
    drawtrees(assets, 3, 0, 400, 2);
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


int calculatemovement(struct Playerinfo *player, int* maxplatform, struct GameAssets *assets) {
    static int facedirection = 1;
    float dt = GetFrameTime();
    float speed = 300.0f;
    float jumpForce = -500.0f;
    float gravity = 800.0f;

    player->direction.x = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A));

    if (player->direction.x > 0) {
         facedirection = 1;
    } else if (player->direction.x < 0) {
        facedirection = -1;
    }

    player->Position.x += dt * speed * player->direction.x;
    //collisionplayerblocks('x', player, maxplatform, &facedirection);

    player->direction.y = (IsKeyDown(KEY_S) - IsKeyDown(KEY_W));

    player->Position.y += speed * dt * player->direction.y;
    //collisionplayerblocks('y', player, maxplatform, &facedirection);

    if (player->direction.x > 0) {
         facedirection = 1;
    } else if (player->direction.x < 0) {
        facedirection = -1;
    }

    return facedirection;
}



void LoadAnimationData(char direction, struct GameAssets* assets) {
    FILE *file = fopen("shieldgirlpos.txt", "r");
    if (file == NULL) {
        printf("Error: Cannot open file for reading!\n");
    }

    char line[100];  // Buffer for reading lines

    while (fgets(line, sizeof(line), file)) {

            if (direction == 'i') //idle
            {
                if (strstr(line, "idle animations:")) 
                {
                    fscanf(file, "srcx = %f, %f, %f, %f\n", &assets->src_idlex[0], &assets->src_idlex[1], &assets->src_idlex[2], &assets->src_idlex[3]);
                    fscanf(file, "srcy = %f, %f, %f, %f\n", &assets->src_idley[0], &assets->src_idley[1], &assets->src_idley[2], &assets->src_idley[3]);
                }
            }
            else if (direction == 'w') //walking
            {
                if (strstr(line, "walking animations: 7")) 
                {
                    fscanf(file, "srcx = %f, %f, %f, %f, %f, %f, %f\n", &assets->src_runningx[0], &assets->src_runningx[1], 
                        &assets->src_runningx[2], &assets->src_runningx[3], &assets->src_runningx[4], &assets->src_runningx[5], &assets->src_runningx[6]);
                    fscanf(file, "width = %f, %f, %f, %f, %f, %f, %f\n", &assets->src_runningwidth[0], &assets->src_runningwidth[1], 
                        &assets->src_runningwidth[2], &assets->src_runningwidth[3], &assets->src_runningwidth[4], &assets->src_runningwidth[5], &assets->src_runningwidth[6]);
                    fscanf(file, "srcy = %f, %f, %f, %f, %f, %f, %f\n", &assets->src_runningy[0], &assets->src_runningy[1], 
                        &assets->src_runningy[2], &assets->src_runningy[3], &assets->src_runningy[4], &assets->src_runningy[5], &assets->src_runningy[6]);
                } 
            }

            else if (direction == 'f') //falling
            {
                if (strstr(line, "falling animations:")) 
                {
                    fscanf(file, "srcx = %f, %f, %f\n", &assets->src_fallingx[0], &assets->src_fallingx[1], &assets->src_fallingx[2]);
                    fscanf(file, "srcy = %f, %f, %f\n", &assets->src_fallingy[0], &assets->src_fallingy[1], &assets->src_fallingy[2]);
                }
            }

        else if (direction == 'j') //jumping
        {
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
        }

        else if (direction == 'a') //jumping
        {
            if (strstr(line, "attacking animations:")) 
            {
                fscanf(file, "srcx = %f, %f, %f, %f, %f, %f\n", &assets->src_attackingx[0], &assets->src_attackingx[1], 
                       &assets->src_attackingx[2], &assets->src_attackingx[3], &assets->src_attackingx[4], &assets->src_attackingx[5]);
                fscanf(file, "srcy = %f, %f, %f, %f, %f, %f\n", &assets->src_attackingy[0], &assets->src_attackingy[1], 
                       &assets->src_attackingy[2], &assets->src_attackingy[3], &assets->src_attackingy[4], &assets->src_attackingy[5]);
                fscanf(file, "width = %f, %f, %f, %f, %f, %f\n", &assets->src_attackingwidth[0], &assets->src_attackingwidth[1], 
                       &assets->src_attackingwidth[2], &assets->src_attackingwidth[3], &assets->src_attackingwidth[4], &assets->src_attackingwidth[5]);
            }
        }
    }
    fclose(file);
}

void iterateanimationsprites(struct GameAssets* assets, struct Playerinfo* player, int* currentframecount, int* facedirection, int* i) {
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

    else if (player->animationstate == 4){
        framescount = 6;
        frametimer = 8;
    }

    (*currentframecount)++;

    if (*currentframecount % (frametimer) == 1) {
        (*i)++;
        *currentframecount = 2;
    }

    if (*i >= framescount) {
        *i = 0;

        if (player->animationstate == 2 && player->isJumping) //let the character jumping frame to iterate till it finishes jumping
        {
            player->animationstate = 2;
            *i = 3;
        }

    }

    //printf("AnimationState: %d, isFalling: %d, isRunning: %d, onPlatform: %d\n", 
        //player->animationstate, player->isfalling, player->isrunning, player->onplatform);

    Rectangle sourceRect, destRect;

    if (player->animationstate == 0) {  // Idle Animation
        texture = assets->texture[1];
        sourceRect = (Rectangle){assets->src_idlex[*i], assets->src_idley[*i], 50, 61};
        player->width = 100;
        player->height = 100;
    } 
    
    else if (player->animationstate == 1) // Running Animation
    {  
        texture = assets->texture[0];
        sourceRect = (Rectangle){assets->src_runningx[*i], assets->src_runningy[*i], assets->src_runningwidth[*i],
                                 assets->texture[0].height - assets->src_runningy[*i]};
        player->width = 100;
        player->height = 100;
    } 

    else if (player->animationstate == 2) // Jumping Animation
    {  
        texture = assets->texture[3];
        sourceRect = (Rectangle){assets->src_jumpingx[*i], assets->src_jumpingy[*i], assets->src_jumpingwidth[*i], assets->src_jumpingheight[*i]};
        player->width = 100;
        player->height = 100;
    }

    else if (player->animationstate == 3) // falling Animation
    {
        texture = assets->texture[2];
        sourceRect = (Rectangle){assets->src_fallingx[*i], assets->src_fallingy[*i], 50, 57};
        player->width = 100;
        player->height = 100;
    }

    else if (player->animationstate == 4){
        player->attack = true;
        texture = assets->texture[4];
        sourceRect = (Rectangle){assets->src_attackingx[*i], assets->src_attackingy[*i], 
                      assets->src_attackingwidth[*i], assets->texture[4].height - assets->src_attackingy[*i]};
        player->width = 100;
        player->height = 100;
    
        printf("Attack Animation Frame: %d, Attack Flag: %d\n", *i, player->attack); // Debugging print
        if (*i == 5){
            *i = 0;
            player->attack = false; //think abt how to make the fifth frame iterate finish 
            //printf("Attack Animation Frame: %d, Attack Flag: %d\n", *i, player->attack); // Debugging print
        }

    }
    

    destRect = (Rectangle){player->Position.x, player->Position.y, player->width, player->height};

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
                            "0100110000000100",
                            "0001000000100000",
                            "0000000000000000",                       
                            "0010000000001100",
                            "2010000001000000",
                            "0000000000000000",
                            "0111111110000100",
                            "0000000000000000",
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
    int blockheight = 45;
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
    Rectangle sourceRect ={240, 48, 63, 30};
    Vector2 origin = {0, 0};
    for (int i = 0; i < *maxplatform; i++) {
        DrawTexturePro(assets->texture[5], sourceRect, blocksarray[i].rect, origin, 0, WHITE);
    
        Vector2 textPosition = { 
            (blocksarray[i].rect.x + blocksarray[i].width / 2), 
            (blocksarray[i].rect.y + blocksarray[i].height / 2) 
        };
    
        DrawText(TextFormat("Position: %.0f, %.0f", blocksarray[i].rect.x, blocksarray[i].rect.y), 
                 textPosition.x, textPosition.y, 20, BLACK); // Increased font size
    }

    for (int i = 0; i<9; i++){
        Rectangle groundrect = {0, 160, 126, 62};
        Rectangle grounddest = {400 * i, 720, 400, 100};
        DrawTexturePro(assets->texture[8], groundrect, grounddest, origin, 0, WHITE);}
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
    if ((*object).Position.y > (700.22)){
        (*object).Position.y = 700.22;
        (*object).velocityY = 0;
        (*object).isJumping = false;
        object->isfalling = false;
        object->onground = true;
        object->onplatform = true;

        if (!object->isrunning){
            object->animationstate = 0;
        }

        else if (object->isrunning){
            object->animationstate = 1;
        }

        if (object->onground) //this block of statements is for the animation change when the player is on the ground level
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
    assets.images[assets.imagecount++] = LoadImage("Images/Tile1.png"); ///maybe not in use
    assets.images[assets.imagecount++] = LoadImage("Images/sky.png");
    assets.images[assets.imagecount++] = LoadImage("Images/mountain.png"); //9
    assets.images[assets.imagecount++] = LoadImage("Images/background.png");
    assets.images[assets.imagecount++] = LoadImage("Images/tree2.png");
    assets.images[assets.imagecount++] = LoadImage("Images/forestbackground.png"); ///might not use
    assets.images[assets.imagecount++] = LoadImage("background_layer_1.png");//13
    assets.images[assets.imagecount++] = LoadImage("background_layer_2.png");
    assets.images[assets.imagecount++] = LoadImage("background_layer_3.png");
    assets.images[assets.imagecount++] = LoadImage("tilecompleteset.png"); //16
    assets.images[assets.imagecount++] = LoadImage("tileset.png");
    //assets.images[assets.imagecount++] = LoadImage("Landing_KG_2.gif");

    assets.music[assets.musiccount++] = LoadMusicStream("Music/13 Always With Me_ Spirited Away (Pi.mp3");

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
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[15]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[16]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[17]); //16

    Texture2D mytexture = assets.texture[0]; //walking
    if (mytexture.id == 0) {
        printf("Error: Texture failed to load!\n");
    }

    Texture2D mytexture2 = assets.texture[1]; //idle
    if (mytexture2.id == 0) {
        printf("Error: Texture failed to load!\n");
    }

    Texture2D mytexture3 = assets.texture[2]; //falling
    if (mytexture2.id == 0) {
        printf("Error: Texture failed to load!\n");
    }

    Texture2D mytexture4 = assets.texture[3]; //jumping
    if (mytexture2.id == 0) {
        printf("Error: Texture failed to load!\n");
    }

    Texture2D mytexture5 = assets.texture[4]; //attacking
    if (mytexture2.id == 0) {
        printf("Error: Texture failed to load!\n");
    }

    Texture2D background1 = assets.texture[5];
    if (mytexture2.id == 0) {
        printf("Error: Texture failed to load!\n");
    }

    struct Playerinfo Playerdata ={.Position = {0, windheight},
                                   .isJumping = false,
                                   .attack = false
                                  };

    int blockcount = loadmap("map.txt");
    int playerlastframedirection = 1;
    int currentFrame = 0;
    float animationTimer = 0;
    int animationindex = 0;
    LoadAnimationData('f', &assets);
    LoadAnimationData('i', &assets);
    LoadAnimationData('w', &assets);
    LoadAnimationData('j', &assets);
    LoadAnimationData('a', &assets);

    Camera2D camera = Camerasettings(&Playerdata);

    while (!WindowShouldClose())
    {
        UpdateMusicStream(assets.music[0]);
        BeginDrawing();
        BeginMode2D(camera);
        ClearBackground(RAYWHITE);
        drawbackground(&assets);
        drawobstacles(&blockcount, &assets);
        playerlastframedirection = calculatemovement(&Playerdata, &blockcount, &assets);
        updatecamera(&camera, &Playerdata);
        printf("Player: %.2f, %.2f, %.2f, %.2f\n", Playerdata.Position.x, Playerdata. Position.y, Playerdata.width, Playerdata.height);

        /*for (int i=0; i<6; i++){
            Rectangle sourceRect ={assets.src_attackingx[i], assets.src_attackingy[i], assets.src_attackingwidth[i], 
                                    mytexture5.height - assets.src_attackingy[i]};
            Rectangle destRect = {(170 * i), 150, (sourceRect.width) * 2, sourceRect.height*2 };
            Vector2 origin = {0, 0};

            printf("%.2f, %.2f\n", sourceRect.x, destRect.x);
            DrawTexturePro(mytexture5, sourceRect, destRect, origin, 0, WHITE);
        }*/
        /*Rectangle sourceRect ={240, 48, 63,30};
        Rectangle destRect = {(300), 300, (sourceRect.width) * 2, sourceRect.height*2 };
        Vector2 origin = {0, 0};*/
            
        Texture2D texture = assets.texture[1];
        Rectangle sourcerect = {assets.src_idlex[0], assets.src_idley[0], 50, 61};
        Rectangle destrec = {Playerdata.Position.x, Playerdata.Position.y, 100, 100};
        Vector2 origin = {0, 0};

        DrawTexturePro(texture, sourcerect, destrec, origin, 0, WHITE);
        keepobjectwithinscreen(&Playerdata);
        iterateanimationsprites(&assets, &Playerdata, &currentFrame, &playerlastframedirection, &animationindex);
        EndMode2D();
        EndDrawing();
    }

    Unloadresources(&assets); 
    CloseAudioDevice();
    CloseWindow();
    return 0;
}