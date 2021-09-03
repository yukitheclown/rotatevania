#ifndef WORLD_DEF
#define WORLD_DEF

#include "object.h"
#include "math.h"

void World_UpdateObjectInWorld(Object *obj, int worldUpdates);
void World_ResolveCollisions(Object *obj, BoundingBox *bb);
void World_RemoveObjectFromWorld(Object *obj);
void World_Free();
void World_Init(Rect2D rect, int quadWidth);
void World_UpdateVisibleObjects();
void World_DrawVisibleObjects();
void World_DrawVisibleObjectsBoundingBoxes();
void World_UpdateCamera(Rect2D characterRect, Vec2 vel);
void World_DrawQuadtree();
void World_AddCameraBoundary(Object *obj);
void World_DrawVisibleCameraBoundaries();

#endif