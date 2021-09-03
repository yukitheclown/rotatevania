#include "world.h"
#include "quadtree.h"
#include "shaders.h"
#include <assert.h>

static QuadtreeLeaf *updateQuadtree;
static QuadtreeLeaf *collisionQuadtree;
static QuadtreeLeaf *cameraQuadtree;
static int quadtreeDivisions, standardQuadWidth;

void World_Init(Rect2D rect, int quadWidth){

	standardQuadWidth = quadWidth;

	int divisions = roundf(log2((rect.w / quadWidth)));

	collisionQuadtree = Quadtree_Create(rect, divisions);
	updateQuadtree = Quadtree_Create(rect, divisions);
	cameraQuadtree = Quadtree_Create(rect, divisions);
}

void World_Free(){
	QuadtreeLeaf_Clear(collisionQuadtree);
	QuadtreeLeaf_Clear(updateQuadtree);
	QuadtreeLeaf_Clear(cameraQuadtree);
}

void World_RemoveObjectFromWorld(Object *obj){

	obj->AddUser(obj);

	if(obj->inQuads[IN_QUAD_COLLISION]){
		QuadtreeLeaf_Remove(obj->inQuads[IN_QUAD_COLLISION], obj);
		obj->inQuads[IN_QUAD_COLLISION] = NULL;
	}

	if(obj->inQuads[IN_QUAD_UPDATE]){
		QuadtreeLeaf_Remove(obj->inQuads[IN_QUAD_UPDATE], obj);
		obj->inQuads[IN_QUAD_UPDATE] = NULL;
	}

	if(obj->inQuads[IN_QUAD_CAMERA]){
		QuadtreeLeaf_Remove(obj->inQuads[IN_QUAD_CAMERA], obj);
		obj->inQuads[IN_QUAD_CAMERA] = NULL;
	}

	obj->RemoveUser(obj);
}

static void UpdateObjectInQuad(Object *obj, QuadtreeLeaf **quad, QuadtreeLeaf *quadtree){

	obj->AddUser(obj);

	if(*quad){
		
		if((*quad)->level == quadtreeDivisions &&
			Math_Rect2DIsCompletelyInside(obj->bb->rect, (*quad)->rect)){

			obj->RemoveUser(obj);
			return;
		}

		QuadtreeLeaf_Remove(*quad, obj);
	 	*quad = NULL;
	}

	QuadtreeLeaf_Insert(quadtree, obj, obj->bb->rect);

 	*quad = obj->inQuads[IN_QUAD_TMP];

	obj->RemoveUser(obj);
}

void World_UpdateObjectInWorld(Object *obj, int worldUpdates){

	UpdateObjectInQuad(obj, &obj->inQuads[IN_QUAD_COLLISION], collisionQuadtree);

	if(worldUpdates)
		UpdateObjectInQuad(obj, &obj->inQuads[IN_QUAD_UPDATE], updateQuadtree);
}

void World_ResolveCollisions(Object *obj, BoundingBox *bb){

	QuadtreeLeaf_ResolveCollisions(collisionQuadtree, obj, bb, bb->rect);
}

static inline Rect2D GetViewRect(){
	Vec2 viewPos = Shaders_GetViewPos();
	return (Rect2D){viewPos.x, viewPos.y, GAME_RESOLUTION_W, GAME_RESOLUTION_H};
}

void World_UpdateCamera(Rect2D characterRect, Vec2 vel){

	Vec2 viewPos = Shaders_GetViewPos();

	Rect2D xRect, yRect;

	xRect.x = 0;
	xRect.w = GAME_RESOLUTION_W;
	xRect.y = characterRect.y;
	xRect.h = characterRect.h;

	yRect.y = 0;
	yRect.h = GAME_RESOLUTION_H;
	yRect.x = characterRect.x;
	yRect.w = characterRect.w;

	Vec2 center = {characterRect.x + (characterRect.w/2), characterRect.y + (characterRect.h/2)};

	center.x -= GAME_RESOLUTION_W/2;	
	// center.y -= GAME_RESOLUTION_H/1.3;
	center.y -= GAME_RESOLUTION_H/2;

	if(center.y < 0) center.y = 0;
	if(center.x < 0) center.x = 0;

	xRect.x += center.x;
	xRect.y += center.y;
	yRect.x += center.x;
	yRect.y += center.y;

	Rect2D rect;
	
	rect.x = center.x;
	rect.y = center.y;
	rect.w = GAME_RESOLUTION_W;
	rect.h = GAME_RESOLUTION_H;

	QuadtreeLeaf_UpdateCamera(cameraQuadtree, &rect, &xRect, &yRect);

	float speed = 8 * Window_GetDeltaTime();

	Vec2 pos = Math_LerpVec2(viewPos, (Vec2){rect.x, rect.y}, speed);

	printf("%f %f\n", rect.x, rect.y);

	Shaders_SetViewPos(pos);
}

void World_UpdateVisibleObjects(){

	QuadtreeLeaf_VisibleObjectsAction(updateQuadtree, GetViewRect(), QUADTREE_UPDATE_OBJECTS);
}

void World_DrawVisibleObjects(){

	QuadtreeLeaf_VisibleObjectsAction(updateQuadtree, GetViewRect(), QUADTREE_DRAW_OBJECTS);
}

void World_DrawVisibleObjectsBoundingBoxes(){

	QuadtreeLeaf_VisibleObjectsAction(collisionQuadtree, GetViewRect(), QUADTREE_DRAW_BOUNDING_BOXES);
}

void World_DrawQuadtree(){

	QuadtreeLeaf_Draw(collisionQuadtree, GetViewRect());
}

void World_AddCameraBoundary(Object *obj){

	UpdateObjectInQuad(obj, &obj->inQuads[IN_QUAD_CAMERA], cameraQuadtree);
}

void World_DrawVisibleCameraBoundaries(){

	QuadtreeLeaf_VisibleObjectsAction(cameraQuadtree, GetViewRect(), QUADTREE_DRAW_BOUNDING_BOXES);
}