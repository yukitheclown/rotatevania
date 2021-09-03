#include "quadtree.h"
#include "shaders.h"
#include "object.h"
#include "log.h"
#include "memory.h"
#include <assert.h>

static void InitQuad(QuadtreeLeaf *q, QuadtreeLeaf *parent, int index, int divisions){

	if(parent != NULL){
		q->rect.w = parent->rect.w / 2;
		q->rect.h = parent->rect.h / 2;
		q->rect.x = parent->rect.x + ((index % 2) * q->rect.w);
		q->rect.y = parent->rect.y + (round((index % 4) / 2) * q->rect.h);
		q->level = parent->level+1;
	}

	int k;
	if(q->level <= divisions){
		for(k = 0; k < 4; k++){

			q->children[k] = (QuadtreeLeaf *)Memory_AllocStack(sizeof(QuadtreeLeaf), LEVEL_STACK);
			assert(q->children[k]);

			memset(q->children[k], 0, sizeof(QuadtreeLeaf));

			InitQuad(q->children[k], q, k, divisions);
		}
	}
}

QuadtreeLeaf *Quadtree_Create(Rect2D rect, int divisions){

	QuadtreeLeaf *quad = (QuadtreeLeaf *)Memory_AllocStack(sizeof(QuadtreeLeaf), LEVEL_STACK);
	assert(quad);

	memset(quad, 0, sizeof(QuadtreeLeaf));

	quad->rect = rect;
	
	InitQuad(quad, NULL, 0, divisions);
	
	return quad;
}

static int GetIndex(QuadtreeLeaf *q, Rect2D visRect){

	int ySplit = q->rect.y + (q->rect.h/2);
	int xSplit = q->rect.x + (q->rect.w/2);

	if(visRect.x + visRect.w < xSplit){
		if(visRect.y + visRect.h < ySplit) return 0;
		if(visRect.y > ySplit) return 2;
	}

	if(visRect.x > xSplit){
		if(visRect.y + visRect.h < ySplit) return 1;
		if(visRect.y > ySplit) return 3;
	}

	return -1;
}

void QuadtreeLeaf_Insert(QuadtreeLeaf *q, Object *obj, Rect2D visRect){

	if(q->children[0]){

		int index = GetIndex(q, visRect);

		if(index != -1){
			QuadtreeLeaf_Insert(q->children[index], obj, visRect);
			return;
		}
	}

	Object **into = NULL;

	int k;
	for(k = 0; k < q->numObjects; k++)
		if(q->objects[k] == NULL)
			into = &q->objects[k];

	if(!into && q->numObjects < MAX_QUADTREE_OBJECTS)
		into = &q->objects[q->numObjects++];

	if(!into){
		LOG(LOG_RED, "MAX QUADTREE OBJECTS REACHED!\n");
		return;
	}

	*into = obj;

	obj->inQuads[IN_QUAD_TMP] = q;

	obj->AddUser(obj);
}

void QuadtreeLeaf_Remove(QuadtreeLeaf *q, Object *obj){

	int k;
	for(k = 0; k < q->numObjects; k++){

        if(q->objects[k] == obj){

        	if(q->iter >= k)
	        	--q->iter;

		    obj->RemoveUser(obj);

	    	q->objects[k] = NULL;

		    if(k < MAX_QUADTREE_OBJECTS-1)
		    	memmove(&q->objects[k], &q->objects[k+1], sizeof(Object *) * (q->numObjects - (k+1)));

		    --q->numObjects;

            return;
        }
	}
}

void QuadtreeLeaf_ResolveCollisions(QuadtreeLeaf *q, Object *obj, BoundingBox *box, Rect2D minRect){

	if(!Math_CheckCollisionRect2D(q->rect, minRect))
		return;

	for(q->iter = 0; q->iter < q->numObjects; q->iter++){

		BoundingBox *bb = q->objects[q->iter]->bb;

		if(!Math_CheckCollisionRect2D(bb->rect, minRect) || bb == box || obj == q->objects[q->iter]){

			continue;
		}

		Object_ResolveCollision(obj, box, q->objects[q->iter], q->objects[q->iter]->bb);
	}

	int k;
	if(q->children[0]){
		for(k = 0; k < 4; k++){
			QuadtreeLeaf_ResolveCollisions(q->children[k], obj, box, minRect);
		}
	}
}

void QuadtreeLeaf_Draw(QuadtreeLeaf *q, Rect2D viewRect){

	if(!Math_CheckCollisionRect2D(q->rect, viewRect))
		return;

	Shaders_UseProgram(TEXTURELESS_SHADER);
	Shaders_SetUniformColor((Vec4){0,0,1,1});

	Vec3 points[5];

	points[0] = (Vec3){q->rect.x, q->rect.y, -1};
	points[1] = (Vec3){q->rect.x+q->rect.w, q->rect.y, -1};
	points[2] = (Vec3){q->rect.x+q->rect.w, q->rect.y+q->rect.h, -1};
	points[3] = (Vec3){q->rect.x, q->rect.y+q->rect.h, -1};
	points[4] = (Vec3){q->rect.x, q->rect.y, -1};

	Shaders_DrawLines(points, 5);

	int k;
	if(q->children[0]){
		for(k = 0; k < 4; k++){
			QuadtreeLeaf_Draw(q->children[k], viewRect);
		}
	}
}

void QuadtreeLeaf_UpdateCamera(QuadtreeLeaf *q, Rect2D *rect, Rect2D *xRect, Rect2D *yRect){

	if(!Math_CheckCollisionRect2D(q->rect, *rect))
		return;

	for(q->iter = 0; q->iter < q->numObjects; q->iter++){

		BoundingBox *bb = q->objects[q->iter]->bb;

		if(!Math_CheckCollisionRect2D(bb->rect, *rect))
			continue;

		if(Math_CheckCollisionRect2D(bb->rect, *yRect)){
			Vec2 distance = Math_GetDistanceRect2D(*yRect, bb->rect);
			yRect->y -= distance.y;
			rect->y -= distance.y;
			xRect->y -= distance.y;
		}

		if(Math_CheckCollisionRect2D(bb->rect, *xRect)){
			Vec2 distance = Math_GetDistanceRect2D(*xRect, bb->rect);
			xRect->x -= distance.x;
			rect->x -= distance.x;
			yRect->x -= distance.x;
		}
	}

	int k;
	if(q->children[0]){
		for(k = 0; k < 4; k++){
			QuadtreeLeaf_UpdateCamera(q->children[k], rect, xRect, yRect);
		}
	}
}

void QuadtreeLeaf_VisibleObjectsAction(QuadtreeLeaf *q, Rect2D rect, int type){

	if(!Math_CheckCollisionRect2D(q->rect, rect))
		return;

	for(q->iter = 0; q->iter < q->numObjects; q->iter++){

		BoundingBox *bb = q->objects[q->iter]->bb;

		if(!Math_CheckCollisionRect2D(bb->rect, rect))
			continue;

		if(type == QUADTREE_UPDATE_OBJECTS){

			if(q->objects[q->iter]->Update)
				q->objects[q->iter]->Update(q->objects[q->iter]);

		} else if(type == QUADTREE_DRAW_OBJECTS){
		
			if(q->objects[q->iter]->Draw)
				q->objects[q->iter]->Draw(q->objects[q->iter]);

		} else if(type == QUADTREE_DRAW_BOUNDING_BOXES){
		
			BoundingBox_Draw(q->objects[q->iter]->bb);

		}
	}

	int k;
	if(q->children[0]){
		for(k = 0; k < 4; k++){
			QuadtreeLeaf_VisibleObjectsAction(q->children[k], rect, type);
		}
	}
}

void QuadtreeLeaf_Clear(QuadtreeLeaf *q){

	int k;
	for(k = 0; k < q->numObjects; k++){

		if(q->objects[k]->inQuads[IN_QUAD_COLLISION] == q)
			q->objects[k]->inQuads[IN_QUAD_COLLISION] = 0;
		else if(q->objects[k]->inQuads[IN_QUAD_UPDATE] == q)
			q->objects[k]->inQuads[IN_QUAD_UPDATE] = 0;
		else if(q->objects[k]->inQuads[IN_QUAD_CAMERA] == q)
			q->objects[k]->inQuads[IN_QUAD_CAMERA] = 0;

		q->objects[k]->RemoveUser(q->objects[k]);
	}

	--q->numObjects;

	if(q->children[0]){
		for(k = 0; k < 4; k++)
			QuadtreeLeaf_Clear(q->children[k]);
	}
}