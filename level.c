#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include <GL/glew.h>
#include <stdio.h>
#include <assert.h>
#include <linux/limits.h>
#include "image_loader.h"
#include "world.h"
#include "object.h"
#include "world.h"
#include "sprite.h"
#include "window.h"
#include "shaders.h"
#include "log.h"
#include "level.h"

#define LEVEL_STACK_USED 1

enum {
	NONE_BLOCK = 0,
	STANDARD_BLOCK,
};

enum {
	STONE_STAIR1_BOTTOM = 63,
	STONE_FLOOR1 = 35,
	STONE_FLOOR2 = 37,
};

static int GetCollisionType(int block){

	if(block == STONE_FLOOR1 || block == STONE_STAIR1_BOTTOM || block == STONE_FLOOR2)
		return BB_TYPE_NORMAL;

	return BB_TYPE_NONE;
}

static BoundingBox *CreateBoundingBox(int tile, Rect2D rect){
	
	BoundingBox *bb;

	if(tile == STONE_STAIR1_BOTTOM){

		rect.y -= rect.h;
		rect.w = rect.w * 2;
		rect.h = rect.h * 2;

		bb = BoundingBox_CreateStack(rect);

		BoundingBox_AddPoint(bb, (Vec2){30, 112});
		BoundingBox_AddPoint(bb, (Vec2){124, 16});

	} else if(tile == STONE_FLOOR2){

		rect.h -= 16;
		rect.y += 16;

		bb = BoundingBox_CreateStack(rect);

	} else {

		bb = BoundingBox_CreateStack(rect);
	}

	BoundingBox_AddType(bb, GetCollisionType(tile));

	return bb;
}

static void LoadChunk(char *data, Level *lvl, int chunkIndex, int layer){
	
	LevelChunk *chunk = &lvl->chunks[layer][chunkIndex];

	int width = lvl->width / SQRT_NUM_LEVEL_CHUNKS;
	int height = lvl->height / SQRT_NUM_LEVEL_CHUNKS;

	int startX = (chunkIndex % SQRT_NUM_LEVEL_CHUNKS) * width;
	int startY = (chunkIndex / SQRT_NUM_LEVEL_CHUNKS) * height;

	int num = 0;

	int x, y;
	for(y = startY; y < startY+height; y++){

		for(x = startX; x < startX+width; x++){

			int index = (y * lvl->width) + x;

			if(data[index] <= 0) continue;

			++num;
		}
	}

	if(!num) return;

	chunk->size = num;

    glGenVertexArrays(1, &chunk->vao);
    glBindVertexArray(chunk->vao);

    glGenBuffers(1, &chunk->posVbo);
    glGenBuffers(1, &chunk->ebo);
    glGenBuffers(1, &chunk->uvVbo);

    glEnableVertexAttribArray(SHADERS_POSITION_ATTRIB);
    glEnableVertexAttribArray(SHADERS_COORD_ATTRIB);

    glBindBuffer(GL_ARRAY_BUFFER,chunk->uvVbo);
    glVertexAttribPointer(SHADERS_COORD_ATTRIB, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER,chunk->posVbo);
    glVertexAttribPointer(SHADERS_POSITION_ATTRIB, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER,chunk->uvVbo);
    glBufferData(GL_ARRAY_BUFFER,num*4*sizeof(Vec2), NULL, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,chunk->posVbo);
    glBufferData(GL_ARRAY_BUFFER, num*4*sizeof(Vec3), NULL, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num*6*sizeof(short), NULL, GL_STATIC_DRAW);

	Vec2 coords[4];
	Vec3 verts[4];
	short elements[] = {0, 2, 1, 0, 3, 2};

	int currLevelSize = 0;

	for(y = startY; y < startY + height; y++){

		for(x = startX; x < startX + width; x++){

			int index = (y * lvl->width) + x;

			if(data[index] <= 0) continue;

			Rect2D imgRect = (Rect2D){ 0, 0, lvl->tileWidth, lvl->tileHeight };

			int tile = data[index]-1;

			int tilesX = lvl->img.w / lvl->tileWidth;

			imgRect.x = (tile % tilesX) * lvl->tileWidth;
			imgRect.y = floor(tile / tilesX) * lvl->tileHeight;

			IRect2D sRect = (IRect2D){x * lvl->tileWidth,y * lvl->tileHeight,lvl->tileWidth,lvl->tileHeight};

			Sprite_GetCoordsFromRect(imgRect, coords, 0, &lvl->img);
			Sprite_GetVertsFromRect(&sRect, verts, 0, (Vec2){0,0}, 0);

		    glBindBuffer(GL_ARRAY_BUFFER,chunk->uvVbo);
		    glBufferSubData(GL_ARRAY_BUFFER, currLevelSize*4*sizeof(Vec2), 4*sizeof(Vec2), coords);
 
		    glBindBuffer(GL_ARRAY_BUFFER,chunk->posVbo);
		    glBufferSubData(GL_ARRAY_BUFFER, currLevelSize*4*sizeof(Vec3), 4*sizeof(Vec3), verts);

		    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ebo);
		    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, currLevelSize*6*sizeof(short), 6*sizeof(short), elements);

		    int k;
		    for(k = 0; k < 6; k++)
			    elements[k] += 4;

		    ++currLevelSize;
		}
	}

    glBindVertexArray(0);
}

static void CreateLevel(char *data, Level *lvl, int layer){

	int k;
	for(k = 0; k < NUM_LEVEL_CHUNKS; k++)
		LoadChunk(data, lvl, k, layer);

	int x, y;
	for(y = 0; y < lvl->height; y++){

		for(x = 0; x < lvl->width; x++){

			int index = (y * lvl->width) + x;

			if(data[index] <= 0) continue;

			int block = data[index];
			int collisionType = GetCollisionType(block);

			if(collisionType == BB_TYPE_NONE)
				continue;

			Rect2D bbRect = (Rect2D){0,0,0,0};

			bbRect.x = x * lvl->tileWidth;
			bbRect.y = y * lvl->tileHeight;

			Object *obj = Object_Create();

			int rectHeight = 0;
			int rectWidth = 0;
			
			int k, j;
			for(k = y; k < lvl->height; k++){

				int index = (k * lvl->width) + x;
			
				if(GetCollisionType((int)data[index]) == collisionType)
					++rectHeight;
				else
					break;
			}

			for(k = x; k < lvl->width; k++){

				int tile = block;

				for(j = y; j < y + rectHeight; j++){

					int index = (j * lvl->width) + k;
				
					tile = data[index];

					if(GetCollisionType(tile) != collisionType) break;
				}

				if(GetCollisionType(tile) == collisionType)
					++rectWidth;
				else
					break;
			}

			for(k = y; k < y + rectHeight; k++)
				for(j = x; j < x + rectWidth; j++)
					data[(k * lvl->width) + j] = 1;

			bbRect.w = lvl->tileWidth * rectWidth;
			bbRect.h = lvl->tileHeight * rectHeight;

			obj->bb = CreateBoundingBox(block, bbRect);
			
			obj->AddUser(obj);
			World_UpdateObjectInWorld(obj, 0);
			obj->RemoveUser(obj);
		}
	}
}

static Rect2D LoadObject(lua_State *L){

	Rect2D rect;

	lua_pushnil(L);
	while(lua_next(L, -2) != 0){

		const char *type = lua_tostring(L, -2);

		if(strcmp(type, "width") == 0)
			rect.w = lua_tonumber(L, -1);
		else if(strcmp(type, "height") == 0)
			rect.h = lua_tonumber(L, -1);
		else if(strcmp(type, "x") == 0)
			rect.x = lua_tonumber(L, -1);
		else if(strcmp(type, "y") == 0)
			rect.y = lua_tonumber(L, -1);

		lua_pop(L, 1);
	}

	return rect;
}

static void LoadTilesets(lua_State *L, char *relativePath, Level *lvl){

	int len = lua_rawlen(L, -1);

	int k;
	for(k = 1; k <= len; k++){

		lua_rawgeti(L, -1, k);

		lua_pushnil(L);
		while(lua_next(L, -2) != 0){

			const char *type = lua_tostring(L, -2);

			if(strcmp(type, "image") == 0){

				const char *path = lua_tostring(L, -1);

				int lenPath = strlen(relativePath);

				relativePath[lenPath] = '/';

				strcpy(&relativePath[lenPath+1], path);

				lvl->img = ImageLoader_CreateImage(relativePath);

				relativePath[lenPath] = 0;
			}

			lua_pop(L, 1);
		}

		lua_pop(L, 1);
	}
}

static void LoadLayersData(lua_State *L, char **layers, int *onLayer, int tableIndex){

	int len = lua_rawlen(L, tableIndex);

	layers[*onLayer] = Memory_AllocStack(len, LEVEL_STACK_USED);

	int k;
	for(k = 1; k <= len; k++){

		lua_rawgeti(L, tableIndex, k);

		layers[*onLayer][k-1] = (char)lua_tonumber(L, -1);

		lua_pop(L, 1);
	}

	++(*onLayer);

}

static void LoadStandardLayer(lua_State *L, char **layers, int *onLayer, int tableIndex){

	lua_getfield(L, tableIndex, "data");
	LoadLayersData(L, layers, onLayer, -1);
	lua_pop(L, 1);
}



static void LoadObjectLayer(lua_State *L, int tableIndex){

	lua_getfield(L, tableIndex, "objects");

	int len = lua_rawlen(L, -1);

	int j;
	for(j = 1; j <= len; j++){

		lua_rawgeti(L, -1, j);

		Rect2D rect = LoadObject(L);

		Object *obj = Object_Create();

		obj->bb = BoundingBox_CreateStack(rect);
		
		obj->AddUser(obj);
		World_AddCameraBoundary(obj);
		obj->RemoveUser(obj);

		lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

static void LoadCameraBoundaries(lua_State *L){

	int len = lua_rawlen(L, -1);

	int k;
	for(k = 1; k <= len; k++){

		lua_rawgeti(L, -1, k);

		if(!lua_istable(L, -1)){
			lua_pop(L, 1);
			continue;
		}

		lua_getfield(L, -1, "type");

		if(lua_isstring(L, -1) && strcmp(lua_tostring(L, -1), "objectgroup") == 0)
			LoadObjectLayer(L, -2);

		lua_pop(L, 2);
	}
}

static void LoadLayers(lua_State *L, char **layers, int *onLayer){

	int len = lua_rawlen(L, -1);

	int k;
	for(k = 1; k <= len; k++){

		lua_rawgeti(L, -1, k);

		if(!lua_istable(L, -1)){
			lua_pop(L, 1);
			continue;
		}

		lua_getfield(L, -1, "type");

		if(lua_isstring(L, -1) && strcmp(lua_tostring(L, -1), "tilelayer") == 0)
			LoadStandardLayer(L, layers, onLayer, -2);

		lua_pop(L, 2);
	}
}

static void LoadTable(lua_State *L, char *relativePath, Level *lvl, char **layers, int *onLayer){

	// why the fuck am i not just using getfield()?????

	lua_getfield(L, -1, "width");
	lvl->width = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "height");
	lvl->height = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "tilewidth");
	lvl->tileWidth = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "tileheight");
	lvl->tileHeight = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "tilesets");
	LoadTilesets(L, relativePath, lvl);
	lua_pop(L, 1);

	lua_getfield(L, -1, "layers");
	LoadLayers(L, layers, onLayer);
	lua_pop(L, 1);
}

static void LoadLevel(lua_State *L, char *relativePath, Level *lvl){

	void *lowerStartPointer = Memory_GetStackPointer(0);

	char *layers[MAX_LAYERS];
	memset(layers, 0, sizeof(layers));

	int onLayer = 0;

	LoadTable(L, relativePath, lvl, layers, &onLayer);

	World_Init((Rect2D){0,0, lvl->width * lvl->tileWidth, lvl->height * lvl->tileHeight}, 512);

	lua_getfield(L, -1, "layers");
	LoadCameraBoundaries(L);
	lua_pop(L, 1);

	Shaders_UseProgram(TEXTURED_SHADER);

	lvl->nLayers = onLayer;

	int k;
	for(k = 0; k < onLayer; k++)
	    CreateLevel(layers[k], lvl, k);

	Memory_SetStackPointer(lowerStartPointer, 0);
}

Level Level_Load(char *path){
	
	Level ret;
	memset(&ret, 0, sizeof(Level));

	lua_State *L = luaL_newstate();

	if(luaL_dofile(L, path) != 0){
		LOGF(LOG_RED, "Error: %s: %s\n", path, lua_tostring(L, -1));
		lua_pop(L, 1);
		lua_close(L);
		return ret;
	}

	ret.startingStackPointer = Memory_GetStackPointer(1);

	char relPath[PATH_MAX];

	int j;
	for(j = strlen(path)-1; j >= 0; j--)
		if(path[j] == '/') break;

	memcpy(relPath, path, j);
	relPath[j] = 0;

	LoadLevel(L, relPath, &ret);

	lua_pop(L, 1);
	lua_close(L);

    return ret;
}

static void DrawLayers(Level *lvl, int chunk){

	int k;
	for(k = 0; k < lvl->nLayers; k++){
		glBindVertexArray(lvl->chunks[k][chunk].vao);
		glDrawElements(GL_TRIANGLES, lvl->chunks[k][chunk].size*6, GL_UNSIGNED_SHORT, 0);
	}
}

void Level_Draw(Level *lvl){

	Shaders_UseProgram(TEXTURED_SHADER);

	Shaders_UpdateProjectionMatrix();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, lvl->img.glTexture);

	IVec2 pos = Shaders_GetViewPos();

	Shaders_UpdateViewMatrix();

	int width = (lvl->width / SQRT_NUM_LEVEL_CHUNKS) * lvl->tileWidth;
	int height = (lvl->height / SQRT_NUM_LEVEL_CHUNKS) * lvl->tileHeight;

	int chunksRight = pos.x > 0 ? floor(pos.x / width) : 0;
	int chunksDown = pos.y > 0 ? floor(pos.y / height) : 0;

	int inChunk = chunksRight + (chunksDown * SQRT_NUM_LEVEL_CHUNKS);

	if(inChunk >= NUM_LEVEL_CHUNKS){
		inChunk = NUM_LEVEL_CHUNKS-1;
		DrawLayers(lvl, inChunk);
		return;
	}

	int drawRight = 0;
	int drawDown = 0;

	if(chunksRight < SQRT_NUM_LEVEL_CHUNKS-1){

		int rightChunkX = ((inChunk % SQRT_NUM_LEVEL_CHUNKS) * width) + width;

		if(pos.x+GAME_RESOLUTION_W > rightChunkX) drawRight = 1;
	}	

	if(inChunk + SQRT_NUM_LEVEL_CHUNKS < NUM_LEVEL_CHUNKS){

		int downChunkY = ((inChunk / SQRT_NUM_LEVEL_CHUNKS) * height) + height;

		if(pos.y+GAME_RESOLUTION_H > downChunkY) drawDown = 1;
	}	

	if(drawRight)
		DrawLayers(lvl, inChunk+1);

	if(drawDown){

		DrawLayers(lvl, inChunk+SQRT_NUM_LEVEL_CHUNKS);

		if(drawRight && inChunk+SQRT_NUM_LEVEL_CHUNKS+1 < NUM_LEVEL_CHUNKS)
			DrawLayers(lvl, inChunk+SQRT_NUM_LEVEL_CHUNKS+1);
	}

	DrawLayers(lvl, inChunk);
}

void Level_Close(Level *lvl){

	int k, j;
	for(j = 0; j < lvl->nLayers; j++){
		for(k = 0; k < NUM_LEVEL_CHUNKS; k++){
		    glDeleteBuffers(1, &lvl->chunks[j][k].posVbo);
		    glDeleteBuffers(1, &lvl->chunks[j][k].uvVbo);
		    glDeleteBuffers(1, &lvl->chunks[j][k].ebo);
		    glDeleteVertexArrays(1, &lvl->chunks[j][k].vao);
		}
	}

	Memory_SetStackPointer(lvl->startingStackPointer,1);

	World_Free();
}