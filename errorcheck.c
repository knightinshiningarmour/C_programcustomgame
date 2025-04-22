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
    Texture2D texturewalkleft[20];
    Texture2D texturewalkright[20];
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
    int texturewalkrightcount;
    int texturewalkleftcount;  
    int texturecount;
    int imagecount;
    int musiccount;
    int audiocount;
};

struct Playerinfo 
{
    Rectangle rect;
    Color colour;
    Vector2 Position;
    Vector2 direction;
    float width;
    float height;
    float velocityY;
    bool isJumping;
    //bool walking;
};

struct Playerinfo *blocksarray = NULL;

void resizeimage(char direction, struct GameAssets* asset, int first, int last, float cropthreshold, int texturewidth, int textureheight){
    for (int i = first; i <= last; i++) {
        ImageAlphaCrop(&asset->images[i], cropthreshold); 
        ImageResize(&asset->images[i], texturewidth, textureheight);  

        if (direction == 'r') {
            asset->texturewalkright[asset->texturewalkrightcount++] = LoadTextureFromImage(asset->images[i]);
        } 
        else if (direction == 'l') {
            ImageFlipHorizontal(&asset->images[i]);
            asset->texturewalkleft[asset->texturewalkleftcount++] = LoadTextureFromImage(asset->images[i]);
        }
    }
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

    printf("%d audio to be unloaded....", (assets->audiocount));
    for (int i=0; i<assets->audiocount; i++){
        UnloadAudioStream(assets->audio[i]);
    }
    
    printf("%d textureright to be unloaded....", (assets->texturewalkrightcount));
    for (int i=0; i<assets->texturewalkrightcount; i++){
        if (assets->texturewalkright[i].id > 0){ 
            UnloadTexture(assets->texturewalkright[i]);
        }
    }

    printf("%d textureleft to be unloaded....", (assets->texturewalkleftcount));
    for (int i=0; i<assets->texturewalkleftcount; i++){
        if (assets->texturewalkleft[i].id > 0){ 
            UnloadTexture(assets->texturewalkleft[i]);
        }
    }
    
    if (blocksarray != NULL) {
        free(blocksarray);
        blocksarray = NULL;
    }
}

void collisionplayerblocks(char axis, struct Playerinfo* object, int* maxplatform){
    Rectangle player = {(*object).Position.x, (*object).Position.y, (*object).width, (*object).height};
    for (int i=0; i<*maxplatform; i++)
    {
        if (axis == 'y')
        {
            if (CheckCollisionRecs(player, blocksarray[i].rect))
            {
                if ((*object).velocityY < 0) //means moving upwards
                {
                    (*object).Position.y = blocksarray[i].rect.y + blocksarray[i].rect.height;
                    (*object).velocityY = 0;
                }
                else if ((*object).velocityY > 0){
                    (*object).Position.y = blocksarray[i].rect.y - (*object).height;
                    (*object).velocityY = 0;  
                    (*object).isJumping = false; 
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

int calculatemovement(struct Playerinfo *player, int* maxplatform){

    //printf("Playerdata x : %f, y: %f\n", player->Position.x, player->Position.y);

    static int facedirection = 1;
    float dt = GetFrameTime();
    float speed = 100.0f;
    float jumpForce = -500.0f; // neg value to move upwards
    float gravity = 500.0f;

    player->direction.x = (IsKeyDown(KEY_D)-IsKeyDown(KEY_A));

    if (player->direction.x > 0) {
        facedirection = 1; 
    }
    else if (player->direction.x < 0) {
        facedirection = -1;
    }

    player->Position.x += dt * speed * player->direction.x;
    collisionplayerblocks('x', player, maxplatform);

    if (IsKeyPressed(KEY_SPACE) && !player->isJumping){
        player->velocityY = jumpForce;
        player->isJumping = true;
    }

    player->velocityY += gravity * dt;                // The velocityY eventually gets smaller meaning it will stop at the highest point, up in window is smaller value
    player->Position.y += player->velocityY * dt; //can think of it as (ut + 0.5at^2 ) times with the deltatime
    collisionplayerblocks('y', player, maxplatform);

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



bool checkplatformsoverlap(Rectangle rec, int* maxplatform){
    float spacebtwblock = 50.0f;

    for (int i=0; i<*maxplatform; i++){
        Rectangle expandedrec = {
                                 blocksarray[i].rect.x, 
                                 blocksarray[i].rect.y, 
                                 blocksarray[i].rect.width + (2 * spacebtwblock), 
                                 blocksarray[i].rect.height + (2 * spacebtwblock)
                                };

        if ((CheckCollisionRecs(rec, expandedrec))){
            return true;
        }
    }
    return false;
}

int loadmap(const char* filename){
    // create a txt file with platforms initialised
    FILE* File = fopen(filename, "w");
    char platforms[][20] = {
                            "0000000000000000", 
                            "0000000000000000",
                            "0000000000000000",
                            "0000000000000000",
                            "0000000000000000",
                            "0000000000000000",
                            "0000000000000000",
                            "0100110001000100",
                            "0000000000000000",
                            "0000000000000000",                       
                            "0010001000000100",
                            "0000000000000000",
                            "0000000000000000",
                            "0111000000000100",
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
    }
}
    


void keepobjectwithinscreen(struct Playerinfo* object, int width, int height){
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

void iterateanimationsprites(char animation, Texture2D texture, struct GameAssets* assets, struct Playerinfo* player, int* currentframecount, int* facedirection, int framescount, int* i) {
    (*currentframecount)++; 

    if (*currentframecount % (framescount*3) == 1) { 
        (*i)++; // Move to next frame
        *currentframecount = 2;
    }

    if (*i >= framescount) {  
        *i = 0; 
    }

    if (animation == 'r')
    {
        Rectangle sourceRect ={assets->src_runningx[*i], assets->src_runningy[*i], 500, 500};
        player->width = sourceRect.width * 0.2;
        player->height = sourceRect.height * 0.2;

        Rectangle destRect = {player->Position.x, player->Position.y, player->width, player->height};

        if (*facedirection < 0) 
        {
            sourceRect.width = -sourceRect.width;
            destRect.width = fabs(destRect.width);
        }

        Vector2 origin = {0, 0};
        DrawTexturePro(texture, sourceRect, destRect, origin, 0.0f, WHITE);
    }

    else if (animation == 'i') //%3 framecount for idle
    {
        Rectangle sourceRect ={assets->src_idlex[*i], assets->src_idley[*i], 300, 500};
        player->width = sourceRect.width * 0.3;
        player->height = sourceRect.height * 0.3;

        Rectangle destRect = {player->Position.x, player->Position.y, player->width, player->height};
        if (*facedirection < 0) 
        {
            sourceRect.width = -sourceRect.width;
            destRect.width = fabs(destRect.width);
        }

        Vector2 origin = {0, 0};
        DrawTexturePro(texture, sourceRect, destRect, origin, 0.0f, WHITE);
    }
    
    else if (animation == 'j')
    {
        Rectangle sourceRect ={assets->src_jumpingx[*i], assets->src_jumpingy[*i], assets->src_jumpingwidth[*i], assets->src_jumpingheight[*i]};
        Rectangle destRect = {player->Position.x, player->Position.y, sourceRect.width*0.4, sourceRect.height*0.4};
        if (*facedirection < 0) 
        {
            sourceRect.width = -sourceRect.width;
            destRect.width = fabs(destRect.width);
        }

        Vector2 origin = {0, 0};
        DrawTexturePro(texture, sourceRect, destRect, origin, 0.0f, WHITE);
    }

}

void iteratearrowanimation(int facedirection, Texture2D texture, struct GameAssets* assets, Vector2* arrowPos, float* arrowRotation, float speedX, float speedY, Vector2 enemypos, Rectangle rec) {
    float dt = GetFrameTime();
    static bool movedown = false;

    Rectangle src = {1, 0, 361, texture.height};
    Rectangle dst = {arrowPos->x, arrowPos->y, src.width / 3, src.height / 3 };
    Vector2 origin = {dst.width / 2.0f, dst.height / 2.0f};
    Rectangle arrowtip = {arrowPos->x + dst.width/2 - (*arrowRotation*0.45), arrowPos->y + (*arrowRotation*0.95), 4, 4}; // Adjust the size of the arrow tip 
    
    printf("Arrow Position: %.2f, %.2f, %.2f, %.2f\n", dst.x, dst.y, dst.width/2, dst.height/2);
    printf("Arrow hitbox: %.2f, %.2f\n", arrowtip.x, arrowtip.y);

    if (facedirection < 0) {
        src.x += src.width;
        src.width = -fabs(src.width);
        dst.width = fabs(dst.width);
        arrowtip.x = arrowPos->x - dst.width/2 + (*arrowRotation*0.45);
        arrowtip.width = -fabs(arrowtip.width);
        speedX = -fabs(speedX); 
    }

    if (facedirection >= 1 && arrowPos->x + dst.width/2 >= enemypos.x ||
        facedirection < 0 && arrowPos->x - dst.width/2 <= enemypos.x) {
        movedown = true;
    }

    if (CheckCollisionRecs(arrowtip, rec)) {
        speedX = 0.0f;
        speedY = 0.0f;
        movedown = false;
    }
    arrowPos->x += speedX * dt;
    if (movedown) {
        *arrowRotation += 20*dt;
        arrowPos->y += speedY * dt;
    }

    DrawRectangleRec(arrowtip, RED);
    DrawTexturePro(texture, src, dst, origin, (*arrowRotation)*facedirection, WHITE);
}

void drawshop (struct GameAssets* assets){
    Rectangle shopbgsrc = {0, 0, 1024, 1024};
    Rectangle shopbgdest = {0, 0, windwidth, windheight};
    Rectangle shopInventorysrc = {36, 62, 652, 290};
    Rectangle shopInventorydest = {100, 150, 1000, 600};
    Rectangle potionspos[4];
    int potionbought[4];
    int potionindexbought[4][1];

    float potiontexturewidth = assets->texture[6].width / 4;
    DrawTexturePro(assets->texture[4], shopbgsrc, shopbgdest, (Vector2){0, 0}, 0.0f, WHITE);
    DrawTexturePro(assets->texture[5], shopInventorysrc, shopInventorydest, (Vector2){0, 0}, 0.0f, WHITE);   

    for (int i=0; i<4; i++){
        Rectangle potionsrc = {0 + (i * potiontexturewidth), 0, potiontexturewidth, assets->texture[6].height/2};
        Rectangle potionsdest = {448 + (i * potionsrc.width/6), 220, potionsrc.width/8 - 3, potionsrc.height/8};
        potionspos[i] = (Rectangle){potionsdest.x, potionsdest.y, potionsdest.width, potionsdest.height};
        DrawTexturePro(assets->texture[6], potionsrc, potionsdest, (Vector2){0, 0}, 0.0f, WHITE);
    }

    Vector2 mousePos = GetMousePosition();
    for (int i=0; i<4; i++){
        if (CheckCollisionPointRec(mousePos, potionspos[i])){
            DrawRectangleRec(potionspos[i], (Color){255, 255, 255, 100});
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                printf("Potion %d clicked!\n", i+1);
                potionindexbought[i][potionbought[0]++];
            }
        }
    }


}

void drawplayerinven(struct GameAssets* assets, float timelapse){
    Vector2 mousePos = GetMousePosition();
    Rectangle playerinvensrc = {0, 0, assets->texture[7].width, assets->texture[7].height};
    Rectangle invenbg = {3740, 3695, 582, 180};
    Rectangle invenbgdest = {100, 50, 1000, 300};
    Rectangle playeridlesrc = {21, 0, 50, assets->texture[9].height};
    Rectangle playeridledest = {50, 400, 250, 300};
    Rectangle homebuttonsrc = {53, 30, 110, 103};
    Rectangle homebuttondest = {1000, 40, 100, 90};
    if (CheckCollisionPointRec(mousePos, homebuttondest)){
        homebuttonsrc.x = 173;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            printf("Home button clicked!\n");
        }
    }

    DrawTexturePro(assets->texture[10], homebuttonsrc, homebuttondest, (Vector2){0, 0}, 0.0f, WHITE);
    DrawTexturePro(assets->texture[8], invenbg, invenbgdest, (Vector2){0, 0}, 0.0f, WHITE);
    DrawTexturePro(assets->texture[9], playeridlesrc, playeridledest, (Vector2){0, 0}, 0.0f, WHITE);
    DrawText("Inventory", 435, invenbgdest.y + 20, 70, BLUE);
       

    int y = 0;
    for (int i=0; i<12; i++){
        int x = i % 4;
        if (i%4 == 0 && i != 0){
            y++;
        }
        Rectangle playerinvendest = {400 + (x * 100), 200 + (y * 100), 98, 98};
        DrawTexturePro(assets->texture[7], playerinvensrc, playerinvendest, (Vector2){0, 0}, 0.0f, WHITE);
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
    assets.images[assets.imagecount++] = LoadImage("Enemies/arrow.png");
    assets.images[assets.imagecount++] = LoadImage("Images/shopbg.png"); //4
    assets.images[assets.imagecount++] = LoadImage("Images/shopinventory.png"); //5
    assets.images[assets.imagecount++] = LoadImage("Images/potions.png"); //6
    assets.images[assets.imagecount++] = LoadImage("Images/playerinventory.png"); //7
    assets.images[assets.imagecount++] = LoadImage("Images/inventorybg.png"); //8
    assets.images[assets.imagecount++] = LoadImage("Images/Idle_KG_2.png"); //9
    assets.images[assets.imagecount++] = LoadImage("Images/buttons.png"); //10


    assets.music[assets.musiccount++] = LoadMusicStream("13 Always With Me_ Spirited Away (Pi.mp3");

    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[0]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[1]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[2]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[3]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[4]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[5]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[6]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[7]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[8]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[9]);
    assets.texture[assets.texturecount++] = LoadTextureFromImage(assets.images[10]);

    
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        float timelapse;
        timelapse += dt;

        BeginDrawing();
        ClearBackground(RAYWHITE);
        //drawshop(&assets);
        drawplayerinven(&assets, timelapse);
        EndDrawing();
    }

    Unloadresources(&assets); 
    CloseAudioDevice();
    CloseWindow();
    return 0;
}