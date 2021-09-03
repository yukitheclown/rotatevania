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

static void LoadLevel(FILE *fp, char *relativePath, Level *lvl){

	void *lowerStartPointer = Memory_GetStackPointer(0);

	char *currStr = NULL;
	char *key = NULL;

	int isString = 0, isKey = 0, isNumber = 0, numSign = 1;

	char *layers[MAX_LAYERS];
	memset(layers, 0, sizeof(layers));

	int onLayer = 0;

	int dataLen = 0;

	while(!feof(fp)){

		int character = fgetc(fp);

		if(character == '"'){

			if(currStr){
				*((char *)Memory_AllocStackPeice(1, 0)) = 0;
				Memory_FinishPeicesAlloc(0);
			}

			if(isString && currStr){

				isString = 0;

				if(key && strcmp(key, "image") == 0){

					sprintf(&relativePath[strlen(relativePath)], "/%s", currStr);
				
					lvl->img = ImageLoader_CreateImage(relativePath);
				}

				if(key)
					Memory_PopStack(1,0);

				Memory_PopStack(1,0);

				currStr = key = NULL;
				isString = isKey = isNumber = 0;
				numSign = 1;

			} else if(!isString){

				isString = 1;
			}

			continue;
		}

		if(!isString){

			if(((character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z'))){
				
				if(!isNumber && !isKey){
					
					if(key){
						Memory_PopStack(1,0);
						key = NULL;
					}

					isKey = 1;
				}

			} else {

				if(isKey){
					*((char *)Memory_AllocStackPeice(1, 0)) = 0;
					Memory_FinishPeicesAlloc(0);
				}

				isKey = 0;
			}
			
			if(character >= '0' && character <= '9' && !isKey)
				isNumber = 1;

			if(character == '-' && !isKey && !isNumber)
				numSign = -1;
	
			if(!((character >= '0' && character <= '9') || character == '.') && isNumber){

				*((char *)Memory_AllocStackPeice(1, 0)) = 0;

				Memory_FinishPeicesAlloc(0);

				float value = atof(currStr) * numSign;

				currStr = NULL;
				isString = isKey = isNumber = 0;
				numSign = 1;

				Memory_PopStack(1,0);

				if(strcmp(key, "data") == 0){

					if(layers[onLayer])
						Memory_ResumeTopAlloc(0);

					char *currChar = ((char *)Memory_AllocStackPeice(1, 0));

					Memory_FinishPeicesAlloc(0);

					*currChar = (char)value;

					if(layers[onLayer] == NULL)
						layers[onLayer] = currChar;
				
					++dataLen;

				} else if(strcmp(key, "width") == 0){
					
					lvl->width = value;

				} else if(strcmp(key, "height") == 0){
					
					lvl->height = value;

				} else if(strcmp(key, "tilewidth") == 0){
					
					lvl->tileWidth = value;

				} else if(strcmp(key, "tileheight") == 0){
					
					lvl->tileHeight = value;
				}
			}
		}

		if(!isString && character == '}'){

			if(layers[onLayer])
				++onLayer;
		}
		
		if(isString || (character >= 'A' && character <= 'Z') || 
			(character >= 'a' && character <= 'z') || (character >= '0' && character <= '9') || (character == '.' && isNumber)){

			char *currChar = ((char *)Memory_AllocStackPeice(1, 0));

			if(isKey && !key) key = currChar;
			if(!isKey && !currStr) currStr = currChar;

			*currChar = (char)character;
		}
	}

	fclose(fp);

	World_Init((Rect2D){0,0, lvl->width * lvl->tileWidth, lvl->height * lvl->tileHeight}, 512);

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

	FILE *fp = fopen(path, "rb");

	if(!fp){
		LOGF(LOG_RED, "Error loading level: %s\n", path);
		return ret;
	}

	ret.startingStackPointer = Memory_GetStackPointer(1);

	char relPath[PATH_MAX];

	int j;
	for(j = strlen(path)-1; j >= 0; j--)
		if(path[j] == '/') break;

	memcpy(relPath, path, j);
	relPath[j] = 0;

	LoadLevel(fp, relPath, &ret);

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