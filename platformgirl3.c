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
    float src_shieldx[7];
    float src_shieldy[7];
    float src_shieldwidth[7];

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
    bool onshield;
    bool attack;
    bool dead;
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
        printf("camera target1: %.2f\n", camera->target.y);
    }

    else if (player->Position.x + player->width/2 >= windwidth/2 - 100 && player->Position.x + player->width/2 <= 5*windwidth/2 - 100){
        camera->offset = (Vector2){(windwidth-player->width)/2 - 50, windheight-150};
        camera->target = (Vector2){player->Position.x + player->width / 2, player->Position.y + player->height / 2};
        printf("camera target2: %.2f\n", camera->target.x);
    }

        else if (player->Position.x + player->width/2 >= 5*windwidth/2 - 100) {
        camera->offset = (Vector2){(windwidth-player->width)/2 + 100, windheight-150};
        camera->target = (Vector2){5*windwidth/2 + 100 - player->width/2, player->Position.y + player->height / 2};
        printf("camera target3: %.2f, %.2f\n", camera->target.x, camera->target.y);
    }

    ///////p.s. there is an issue when the character jumps from 2850 area to the fixed camera area and the y axis stuck
}

void drawbackground (struct GameAssets* assets){
    //draw background
    for (int i=0; i<4; i++){ //sky
        Rectangle skysrc = {0, 0, assets->texture[6].width, assets->texture[6].height};
        Rectangle skydest = {(-windwidth)/2 + (i * windwidth), -100, windwidth, windheight + 200};  
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[6], skysrc, skydest, origin, 0, WHITE);
    }

    for (int i=0; i<3; i++){ //mountain
        Rectangle mountainsrc = {0, 0, assets->texture[7].width, assets->texture[7].height};
        Rectangle mountaindest = {(-windwidth)/2 + (i * windwidth), windheight-mountainsrc.height-740, windwidth, windheight*1.3};
        Vector2 origin = {0, 0};
        DrawTexturePro(assets->texture[7], mountainsrc, mountaindest, origin, 0, WHITE);
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
        player->width = 100;
        player->height = 100;
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

void loadenemyanimations(struct Playerinfo* enemy){

}

void randomenemypos(int* maxplatform, int enemyonblock[5]){
    for (int i = 0; i < 5; i++) 
    {
        int randomnum;
        int duplicate;
        do 
        {
            duplicate = 0;
            randomnum = GetRandomValue(0, *maxplatform - 1);
            for (int j = 0; j < i; j++) 
            {
                if (enemyonblock[j] == randomnum) 
                {
                    duplicate = 1; // Mark as duplicate
                    break; //break out of the for loop back to do-while loop again
                }
            }
        }while (duplicate); // Repeat if duplicate is found
    
        enemyonblock[i] = randomnum; //save the number
    }
}

void enemyanimations(struct Playerinfo* enemy, struct GameAssets* assets, int* enemycurrentframe, int* facedirection, int* j, float* animationtimer){
    int framecount = 0;
    int frametimer = 0;
    float dt = GetFrameTime();
    Texture2D texture;

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
    (*enemycurrentframe)++;

    if (enemy->animationstate == 2 && *j == 0) { //delay the attack animations, takes time to recharge
        *animationtimer += dt;
        if (*animationtimer < 3.0f) {
            *j = 0;
        } else {
            *animationtimer = 0.0f;  // Reset timer after delay
            (*j)++;  
        }
    } else if (*enemycurrentframe % frametimer == 1){
        (*j)++;
        (*enemycurrentframe) = 2;}

    if ((*j) > framecount){
        (*j) = 0;}

    Rectangle sourceRect, destRect;
    if (enemy->animationstate == 1)
    {
        texture = assets->texture[16];
        int frameIndex = (*j) % 5;  // 0-4 for first row, 0-4 again for second row
        int rowIndex = ((*j) < 5) ? 0 : 1; // First 5 use row 0, next 5 use row 1
        float frameWidth = texture.width / 5;

        sourceRect = (Rectangle){(frameWidth * (frameIndex))+10, rowIndex * (assets->texture[16].height / 2), 
                                frameWidth, assets->texture[16].height / 2 - 45};

        destRect = (Rectangle){enemy->Position.x, enemy->Position.y + 10, 105, 140}; 
    } else if (enemy->animationstate == 2)
    {
        texture = assets->texture[15];
        int frameIndex = (*j) % 5;  // 0-4 for first row, 0-4 again for second row
        int rowIndex = ((*j) < 5) ? 0 : 1; // First 5 use row 0, next 5 use row 1
        float frameWidth = texture.width / 5;

        sourceRect = (Rectangle){(frameWidth * (frameIndex))+10, rowIndex * (assets->texture[15].height / 2), 
                                frameWidth, assets->texture[15].height / 2 - 45};

        destRect = (Rectangle){enemy->Position.x, enemy->Position.y + 10, 105, 140};
    } else if (enemy->animationstate == 3){
        texture = assets->texture[17];
        int frameIndex = (*j) % 5;
        float frameWidth = texture.width/5;

        sourceRect = (Rectangle){(frameWidth * (frameIndex)), 0, frameWidth, 1010};
        destRect = (Rectangle){enemy->Position.x, enemy->Position.y + 10, 105, 140};
    }

    if (*facedirection > 0) {
        sourceRect.x += sourceRect.width;
        sourceRect.width = -fabs(sourceRect.width);
        destRect.width = fabs(destRect.width);
    }

    Vector2 origin = {0, 0};
    DrawTexturePro(texture, sourceRect, destRect, origin, 0, WHITE);
    printf("Animation State: %d, Frame: %d\n", enemy->animationstate, *j);
    printf("%.2f, %.2f\n", destRect.width, destRect.height);
}


//have to randomise their positions on blovksarray
void enemymovement(struct Playerinfo* enemy, struct Playerinfo* player, int enemyonblock[5], int* facedirection){
    float dt = GetFrameTime();
    float speed = 50.0f;

    if (fabs(enemy->Position.x - player->Position.x) <= 250 && !enemy->dead){
        enemy->animationstate = 2; // Attack animation
        enemy->attack = true;
        speed = 0;
    } else {
        enemy->animationstate = 1; // Walking animation
        enemy->attack = false;
    }

    if (!enemy->attack && !enemy->dead){
        if (enemy->Position.x + enemy->width/2 <= blocksarray[enemyonblock[0]].rect.x - 20){
            *facedirection = 1;
        }
        else if (enemy->Position.x + enemy->width/2 >= blocksarray[enemyonblock[0]].rect.x + blocksarray[enemyonblock[0]].rect.width - 60){
            *facedirection = -1;
        }
    }

    if (enemy->dead){
        enemy->animationstate = 3;
        speed = 0;
    }
    printf("%d", *facedirection);
    enemy->Position.x += dt * speed * (*facedirection);
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
    assets.images[assets.imagecount++] = LoadImage("Images/sky.png");
    assets.images[assets.imagecount++] = LoadImage("Images/mountain.png"); //9
    assets.images[assets.imagecount++] = LoadImage("Images/background.png");
    assets.images[assets.imagecount++] = LoadImage("Images/tree2.png");
    assets.images[assets.imagecount++] = LoadImage("Images/forestbackground.png"); ///might not use
    assets.images[assets.imagecount++] = LoadImage("Images/Tile1.png"); //13
    assets.images[assets.imagecount++] = LoadImage("tileset.png");
    assets.images[assets.imagecount++] = LoadImage("enemies/arrow.png");
    assets.images[assets.imagecount++] = LoadImage("enemies/arrowskelattack.png");
    assets.images[assets.imagecount++] = LoadImage("enemies/arrowskelwalk.png"); //17
    assets.images[assets.imagecount++] = LoadImage("enemies/arrowskeldead.png");
    assets.images[assets.imagecount++] = LoadImage("enemies/arrow.png"); //19
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
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[15]); //14 arrow.png
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[16]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[17]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[18]); //17
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[19]);

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

    Texture2D skelwalk = assets.texture[16];
    if (skelwalk.id == 0) {
        printf("Error: Texture failed to load!\n");
    }

    struct Playerinfo Playerdata ={.Position = {0, windheight},
                                   .isJumping = false,
                                   .attack = false
                                  };
                                  
    int blockcount = loadmap("map.txt");

    //init player
    int playerlastframedirection = 1;
    int playercurrentframe = 0;
    int playeranimationindex = 0;

    //init enemy
    int enemyonblock[5];
    int enemylastframedirection = -1;
    int enemycurrentframe = 0;
    int enemyanimationindex = 0;
    float enemyanimationtimer = 0.0f;

    LoadAnimationDataplayer(&assets);
    randomenemypos(&blockcount, enemyonblock);

    struct Playerinfo enemydata = { .Position = {blocksarray[enemyonblock[0]].rect.x + 10, 
                                                 blocksarray[enemyonblock[0]].rect.y - ((assets.texture[16].height/2)-45)*0.15},
                                    .attack = false,
                                    .dead = false,
                                    .isrunning = true,
                                  };

    Camera2D camera = Camerasettings(&Playerdata);
    while (!WindowShouldClose())
    {
        UpdateMusicStream(assets.music[0]);
        BeginDrawing();
        BeginMode2D(camera);
        ClearBackground(RAYWHITE);
        drawbackground(&assets);
        drawobstacles(&blockcount, &assets);
        playerlastframedirection = calculatemovementplayer(&Playerdata, &blockcount, &assets);
        updatecamera(&camera, &Playerdata);
        printf("Player: %.2f, %.2f, %.2f, %.2f\n", Playerdata.Position.x, Playerdata. Position.y, Playerdata.width, Playerdata.height);

        /*for (int i = 0; i < 5; i++) // Loop for 10 frames
        {
            int frameIndex = (i) % 5;
            float frameWidth = assets.texture[17].width/5;
    
            Rectangle sourceRect = {(frameWidth * (frameIndex)), 0, -frameWidth, assets.texture[17].height / 2};
            Rectangle destRect = {100 + 150*i, 300, 100, 150};
            Vector2 origin = {0, 0};
            DrawTexturePro(assets.texture[17], sourceRect, destRect, origin, 0, WHITE);
        }*/
        /*Rectangle sourceRect ={240, 48, 63,30};
        Rectangle destRect = {(300), 300, (sourceRect.width) * 2, sourceRect.height*2 };
        Vector2 origin = {0, 0};*/

        keepobjectwithinscreen(&Playerdata);
        iterateanimationplayer(&assets, &Playerdata, &playercurrentframe, &playerlastframedirection, &playeranimationindex);
        enemymovement(&enemydata, &Playerdata, enemyonblock, &enemylastframedirection);
        enemyanimations(&enemydata, &assets, &enemycurrentframe, &enemylastframedirection, &enemyanimationindex, &enemyanimationtimer);
        EndMode2D();
        EndDrawing();
    }

    Unloadresources(&assets); 
    CloseAudioDevice();
    CloseWindow();
    return 0;
}