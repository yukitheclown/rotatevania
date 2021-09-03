#include "player.h"
#include "image_loader.h"
#include "world.h"
#include "shaders.h"
#include "spell_shields.h"
#include "spell_bursts.h"
#include "sprite.h"
#include "bounding_box.h"
#include "sprite.h"
#include <assert.h>

static Image walkAnimsImg;
static Image idleAnimImg;
static Image jumpAnimsImg;
static Image crouchAnimsImg;
static Image spellsAnimsImg;
static Animation cast1Anim;
static Animation cast2Anim;
static Animation cast3Anim;
static Animation cast4Anim;
static Animation cast5Anim;
static Animation cast6Anim;
static Animation cast7Anim;
static Animation cast8Anim;
static Animation cast9Anim;
static Animation cast10Anim;
static Animation cast11Anim;
static Animation cast12Anim;
static Animation crouchAnim;
static Animation idleToWalkAnim;
static Animation walkToIdleAnim;
static Animation walkAnim;
static Animation idleAnim;
static Animation jumpStartAnim;
static Animation jumpAscendingAnim;
static Animation transitionToFallAnim;
static Animation fallAnim;
static Animation fallLandAnim;

static void GetViewCheckRects(Player *player, Rect2D *xRect, Rect2D *yRect){

	Rect2D bbRect = player->character.bb->rect;

	Vec2 pos = (Vec2){bbRect.x, bbRect.y};

	int width = bbRect.w;
	int height = bbRect.h;

	Vec2 camPos = Shaders_GetViewPos();

	if(player->character.vel.x > 0){

		pos.x += width;

		int toEdgeW = (camPos.x + GAME_RESOLUTION_W) - pos.x;
	
		*xRect = (Rect2D){pos.x, pos.y, toEdgeW, height};
	
	} else {

		int toEdgeW = pos.x - camPos.x;
	
		*xRect = (Rect2D){camPos.x, pos.y, toEdgeW, height};
	}
	
	if(player->character.vel.y >= 0){

		pos.y += height;

		int toEdgeH = (camPos.y + GAME_RESOLUTION_H) - pos.y;
	
		*yRect = (Rect2D){pos.x, pos.y, width, toEdgeH};

	} else {

		int toEdgeH = pos.y - camPos.y;
	
		*yRect = (Rect2D){pos.x, camPos.y, width, toEdgeH};
	}
}


static void ObjPlayerUpdate(Object *obj){

	Player *player = (Player *)obj->data;
	assert(player);

	Character_Update(&player->character, Window_GetDeltaTime());

	Rect2D xRect, yRect;
	GetViewCheckRects(player, &xRect, &yRect);

	Rect2D bbRect = player->character.bb->rect;

	World_UpdateCamera(bbRect, player->character.vel);	
}

static void ObjPlayerDraw(Object *obj){

	Player *player = (Player *)obj->data;
	assert(player);

	Character_Draw(&player->character);


	Rect2D xRect, yRect;
	GetViewCheckRects(player, &xRect, &yRect);

	Shaders_UseProgram(TEXTURELESS_SHADER);
	Shaders_SetUniformColor((Vec4){1,0,0,0.2});

	Shaders_DrawRect((IRect2D){xRect.x, xRect.y, xRect.w, xRect.h}, (Vec2){0,0}, 0, 0);
	Shaders_DrawRect((IRect2D){yRect.x, yRect.y, yRect.w, yRect.h}, (Vec2){0,0}, 0, 0);
}

static void ObjPlayerOnCollision(Object *this, Object *obj2, BoundingBox *bb1, BoundingBox *bb2){

	(void)obj2;

	Player *player = (Player *)this->data;
	assert(player);

	Character_OnCollision(&player->character, obj2, bb1, bb2);
}

void Player_Event(Object *obj, SDL_Event ev){

	Player *player = (Player *)obj->data;

	if(ev.type == SDL_KEYDOWN){

		if(ev.key.keysym.sym == player->keybindings[PLAYER_KEYBINDING_MOVE_LEFT]){
		
			player->moveLeftDown = 1;
			Character_StartWalking(&player->character, CHARACTER_MOVE_LEFT);
		
		} else if(ev.key.keysym.sym == player->keybindings[PLAYER_KEYBINDING_MOVE_RIGHT]){
		
			player->moveRightDown = 1;
			Character_StartWalking(&player->character, CHARACTER_MOVE_RIGHT);
		
		} else if (ev.key.keysym.sym == player->keybindings[PLAYER_KEYBINDING_ABILITY]){

			Vec2 pos = player->character.pos;
			pos.y = player->character.bb->rect.y + player->character.bb->rect.h;

			player->earthSspellShield.Use(&player->earthSspellShield, pos);
		}

	} else if(ev.type == SDL_KEYUP){
			
		if(ev.key.keysym.sym == player->keybindings[PLAYER_KEYBINDING_MOVE_LEFT]){

			if(player->moveRightDown)
				Character_StartWalking(&player->character, CHARACTER_MOVE_RIGHT);
			else
				Character_StopWalking(&player->character);
		
			player->moveLeftDown = 0;

		} else if(ev.key.keysym.sym == player->keybindings[PLAYER_KEYBINDING_MOVE_RIGHT]){

			if(player->moveLeftDown)
				Character_StartWalking(&player->character, CHARACTER_MOVE_LEFT);
			else
				Character_StopWalking(&player->character);

			player->moveRightDown = 0;
		}
	}
}

void Player_Init(){

	walkAnimsImg = ImageLoader_CreateImage("Resources/Metroidvania_WIP/Sprites/Player/sister_walk_transitions.png");
	idleAnimImg = ImageLoader_CreateImage("Resources/Metroidvania_WIP/Sprites/Player/sister_idle.png");
	jumpAnimsImg = ImageLoader_CreateImage("Resources/Metroidvania_WIP/Sprites/Player/sister_jump.png");
	crouchAnimsImg = ImageLoader_CreateImage("Resources/Metroidvania_WIP/Sprites/Player/sister_crouch.png");
	spellsAnimsImg = ImageLoader_CreateImage("Resources/Metroidvania_WIP/Sprites/Player/sister_quickcast_spells.png");

	cast1Anim = (Animation){spellsAnimsImg, 6, 12, 83, 0, 6};
	cast2Anim = (Animation){spellsAnimsImg, 6, 12, 83, 6, 12};
	cast3Anim = (Animation){spellsAnimsImg, 6, 12, 83, 12, 18};
	cast4Anim = (Animation){spellsAnimsImg, 6, 12, 83, 18, 24};
	cast5Anim = (Animation){spellsAnimsImg, 6, 12, 83, 24, 30};
	cast6Anim = (Animation){spellsAnimsImg, 6, 12, 83, 30, 36};
	cast7Anim = (Animation){spellsAnimsImg, 6, 12, 83, 36, 42};
	cast8Anim = (Animation){spellsAnimsImg, 6, 12, 83, 42, 48};
	cast9Anim = (Animation){spellsAnimsImg, 6, 12, 83, 48, 54};
	cast10Anim = (Animation){spellsAnimsImg, 6, 12, 83, 54, 60};
	cast11Anim = (Animation){spellsAnimsImg, 6, 12, 83, 60, 66};
	cast12Anim = (Animation){spellsAnimsImg, 6, 12, 83, 60, 72};

	crouchAnim = (Animation){crouchAnimsImg, 3, 1, 83, 0, 3};
	idleToWalkAnim = (Animation){walkAnimsImg, 5, 4, 83, 0, 2};
	walkToIdleAnim = (Animation){walkAnimsImg, 5, 4, 83, 5, 8};
	walkAnim = (Animation){walkAnimsImg, 5, 4, 83, 10, 20};
	idleAnim = (Animation){idleAnimImg, 4, 2, 166, 0, 8};
	jumpStartAnim = (Animation){jumpAnimsImg, 6, 3, 83, 0, 2};
	jumpAscendingAnim = (Animation){jumpAnimsImg, 6, 3, 83, 2, 4};
	transitionToFallAnim = (Animation){jumpAnimsImg, 6, 3, 83, 4, 9};
	fallAnim = (Animation){jumpAnimsImg, 6, 3, 83, 9, 11};
	fallLandAnim = (Animation){jumpAnimsImg, 6, 3, 83, 11, 17};
}

Object *Player_Create(IVec2 spawnPos){

	Object *ret = Object_Create();

	ret->bb = BoundingBox_Create((Rect2D){-10,-20,20,46});

	BoundingBox_MoveBy(ret->bb, (Vec2){spawnPos.x, spawnPos.y});

	ret->OnCollision = ObjPlayerOnCollision;
	ret->Update = ObjPlayerUpdate;
	ret->Draw = ObjPlayerDraw;
	ret->data = Memory_Alloc(sizeof(Player));

	Player *player = (Player *)ret->data;

	memset(player, 0, sizeof(Player));

	player->character = Character_Create(ret, ret->bb, spawnPos);

	Character_AttachAnimation(&player->character, CHARACTER_ANIMATION_WALK, walkAnim);
	Character_AttachAnimation(&player->character, CHARACTER_ANIMATION_IDLE, idleAnim);
	Character_AttachAnimation(&player->character, CHARACTER_ANIMATION_IDLE_TO_WALK, idleToWalkAnim);
	Character_AttachAnimation(&player->character, CHARACTER_ANIMATION_WALK_TO_IDLE, walkToIdleAnim);
	Character_AttachAnimation(&player->character, CHARACTER_ANIMATION_CROUCH, crouchAnim);
	Character_AttachAnimation(&player->character, CHARACTER_ANIMATION_JUMP_START, jumpStartAnim);
	Character_AttachAnimation(&player->character, CHARACTER_ANIMATION_JUMP_ASCENDING, jumpAscendingAnim);
	Character_AttachAnimation(&player->character, CHARACTER_ANIMATION_FALL_TRANSITION, transitionToFallAnim);
	Character_AttachAnimation(&player->character, CHARACTER_ANIMATION_FALL, fallAnim);
	Character_AttachAnimation(&player->character, CHARACTER_ANIMATION_LAND, fallLandAnim);

	Character_StopWalking(&player->character);

	memset(&player->earthSspellShield, 0, sizeof(Ability));
	player->earthSspellShield.type = SPELLBURST_LIGHT;
	player->earthSspellShield.Use = SpellBurst_Cast;

	player->keybindings[PLAYER_KEYBINDING_ABILITY] = SDLK_j;
	player->keybindings[PLAYER_KEYBINDING_MOVE_LEFT] = SDLK_a;
	player->keybindings[PLAYER_KEYBINDING_MOVE_RIGHT] = SDLK_d;

	return ret;
}

void Player_Close(){

	ImageLoader_DeleteImage(&walkAnimsImg);
	ImageLoader_DeleteImage(&idleAnimImg);
	ImageLoader_DeleteImage(&jumpAnimsImg);
	ImageLoader_DeleteImage(&crouchAnimsImg);
	ImageLoader_DeleteImage(&spellsAnimsImg);
}
