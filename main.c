#include <GL/glew.h>
#include "window.h"
#include "abilities.h"
#include "player.h"
#include "shaders.h"
#include "world.h"
#include "level.h"
#include "memory.h"

static Level level;
static Object *player;

static void Update(){

    World_UpdateVisibleObjects();
}

static void Draw(){

	glClearColor(1,0,0,1);

    glClear(GL_COLOR_BUFFER_BIT);

	Level_Draw(&level);
    
    World_DrawVisibleCameraBoundaries();

    World_DrawVisibleObjects();
    
    // World_DrawQuadtree();
}

static void OnResize(){

}

static void Event(SDL_Event ev){

    Player_Event(player, ev);
}

static void Focus(){

}

int main(){

    // parallax

    // lvl_load then if tiles bb type is the same and it's a square then combine them
    // currently it just only combines for certian tiles.

    // need to do something about combining backgrounds into 1 tile instead of many.
    // must change all vec3 to vec2. !!@!
    // bounding_box.c is very broken

    Memory_Init((0x01 << 20) * 32);

    Window_Open("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GAME_RESOLUTION_W*2, GAME_RESOLUTION_H*2, 0);
    
    glDisable(GL_DITHER);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glClearDepthf(0);
    glDepthFunc(GL_ALWAYS);
    glLineWidth(1);

    Shaders_Init();

    glClearColor(0,0,0,1);

    float proj[16];
    float view[16];
    Math_Perspective2(proj, 0, GAME_RESOLUTION_W, 0, GAME_RESOLUTION_H, 1, 100);
    Math_Identity(view);

    Shaders_SetProjectionMatrix(proj);
    Shaders_SetViewMatrix(view);
    Shaders_SetModelMatrix(view);

    level = Level_Load("Resources/level.lua");

    Player_Init();
    Abilities_Init();
    player = Player_Create((IVec2){100,100});
    World_UpdateObjectInWorld(player, 1);
    Window_MainLoop(Update, Event, Draw, Focus, OnResize, 1);
    Abilities_Close();
    Player_Close();

    Level_Close(&level);
    Memory_Dump();
    Memory_Close();
    Shaders_Close();
    Window_Close();

    return 0;
}