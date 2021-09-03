#ifndef OBJECT_DEF
#define OBJECT_DEF

#include <stdio.h>
#include "bounding_box.h"
#include "window.h"
#include "memory.h"

typedef struct Object Object;

typedef struct QuadtreeLeaf QuadtreeLeaf;

enum {
	IN_QUAD_TMP = 0,
	IN_QUAD_COLLISION,
	IN_QUAD_UPDATE,
	IN_QUAD_CAMERA,
	NUM_QUADTREES,
};

typedef struct {
	Object *obj1;
	Object *obj2;
	BoundingBox *bb1;
	BoundingBox *bb2;
} Collision;

struct Object {

	void (*Free)(Object *this);
	void (*AddUser)(Object *this);
	void (*RemoveUser)(Object *this);
	void (*OnCollision)(Object *this, Object *obj2, BoundingBox *bb1, BoundingBox *bb2);
	void (*OnOtherCollision)(Object *this, Object *obj2, BoundingBox *bb1, BoundingBox *bb2);
	void (*Update)(Object *this);
	void (*Draw)(Object *this);
	void (*AddDrawsToQueue)(Object *this);
	void (*OnLevelSwitch)(Object *this);
	void (*SetPosition)(Object *this, Vec2 pos);
	void (*Rotate)(Object *this, float rot);
	void (*Save)(Object *this, FILE *fp);
	void (*Event)(Object *this, SDL_Event ev);
	// GUIwin *(*CreateAttribsTable)(Object *this, Text *font);

	int inUpdateQueue;

	int eventsEnabled;

	BoundingBox *bb;

	int offScreenUpdated;

	QuadtreeLeaf *inQuads[NUM_QUADTREES];

	void *data;

	void *userData;

	int numUsers;
};

Object *Object_Create();
void Object_Free(Object *obj);
Object *Object_Set(Object *obj);
void Object_AddUser(Object *obj);
void Object_RemoveUser(Object *obj);
void Object_ResolveCollision(Object *obj1, BoundingBox *bb1, Object *obj2, BoundingBox *bb2);

#endif

