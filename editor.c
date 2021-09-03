#include "editor.h"
#include "log.h"
#include "text.h"
#include "bounding_box.h"
#include "text.h"
#include "loader.h"
#include "window.h"
#include "math.h"
#include "image_loader.h"
#include "shaders.h"
#include "sprite.h"

#define MAX_COMMAND_LENGTH 256
#define CONFIG_PATH "Resources/Config"
#define COMMAND_LOC_X 10
#define COMMAND_LOC_Y 10
#define MAX_DROPDOWN_ENTRIES 64

typedef struct {
	char *names[MAX_DROPDOWN_ENTRIES];
	void *datas[MAX_DROPDOWN_ENTRIES];
	int num;
} DropDown;

static Sprite cursorSprite;
static Image cursorImage;
static Image fontImage;
static Config config;
static Object selected;
static EditorSpriteSet *selectedSet;
static char command[MAX_COMMAND_LENGTH];
static int enteringCommand;
static DropDown setsDropdown;

static void DrawDropDown(DropDown dropDown, float x, float y){

	int fontSize = fontImage.w / 16;

	int k;
	for(k = 0; k < dropDown.num; k++){

		if(!dropDown.names[k]) break;
	
		int len = strlen(dropDown.names[k]) * fontSize;

		Shaders_DrawRect((IRect){x, y + (fontSize*k), -1, len, fontSize}, (Vec2){0,0}, 0, 0);
		
		Text_DrawText(fontImage, (Vec4){1,1,1,1}, (Vec2){x, y + (fontSize * k)}, dropDown.names[k]);
	}
}

static void DrawCommandBar(){

	int fontSize = fontImage.w / 16;

	int commandLen = strlen(command) * fontSize;

	Shaders_UseProgram(TEXTURELESS_SHADER);
	Shaders_SetUniformColor((Vec4){0.1,0.1,0.1,0.5});

	Shaders_DrawRect((IRect){COMMAND_LOC_X, COMMAND_LOC_Y, -1, commandLen, fontSize}, (Vec2){0,0}, 0, 0);
	Text_DrawText(fontImage, (Vec4){1,1,1,1}, (Vec2){COMMAND_LOC_X, COMMAND_LOC_Y}, command);

	DrawDropDown(setsDropdown, COMMAND_LOC_X, COMMAND_LOC_Y + fontSize);
}

static void DrawSelectedObj(){

	ObjectData *objData = (ObjectData *)Memory_GetHandleMemory(selected);

	if(!objData) return;

	Vec2 cursorPos = Window_GetMousePos();

	cursorPos.x /= Window_GetWindowWidth();
	cursorPos.y /= Window_GetWindowHeight();
	cursorPos.x *= GAME_RESOLUTION_W;
	cursorPos.y *= GAME_RESOLUTION_H;

	objData->SetPosition(objData, cursorPos);

	BoundingBox_Draw(objData->bb);

	objData->Draw(objData);
}

void Editor_Draw(){

	Vec2 cursorPos = Window_GetMousePos();

	cursorSprite.screenRect.x = cursorPos.x * -cursorSprite.screenRect.z;
	cursorSprite.screenRect.y = cursorPos.y * -cursorSprite.screenRect.z;

	DrawSelectedObj();


	float proj[16], ortho[16];
	Shaders_GetProjectionMatrix(proj);
	Math_Ortho(ortho, 0, Window_GetWindowWidth(), 0, Window_GetWindowHeight(), 0, 10);

	Shaders_SetProjectionMatrix(ortho);

	Sprite_DrawNow(&cursorSprite);

	if(enteringCommand)
		DrawCommandBar();

	Shaders_SetProjectionMatrix(proj);
}

static void ParseCommand(int enterPressed){

	setsDropdown.num = 0;

	char *prefix = &command[1]; // skip the added '/'
	char *suffix = NULL;

	int split = 0;

	int len = strlen(prefix);

	for(split = 0; split < len; split++)
		if(command[split] == ' ') { --split; break; }

	int k;
	for(k = split+1; k < len; k++)
		if(command[k] != ' ') break;

	suffix = &command[k];

	int suffixLen = len - (k-1);

	if(strncmp(prefix, "use", 3) == 0){

		int k;
		for(k = 0; k < config.nEditorSpriteSets; k++){

			if(suffixLen == 0 || strncmp(config.editorSpriteSets[k].name, suffix, suffixLen) == 0){

				if(setsDropdown.num+1 > MAX_DROPDOWN_ENTRIES)
					break;

				setsDropdown.names[setsDropdown.num++] = config.editorSpriteSets[k].name;
				setsDropdown.datas[setsDropdown.num-1] = (void *)&config.editorSpriteSets[k];
			}
		}
	}

	if(enterPressed){

		if(setsDropdown.num > 0)
			selectedSet = (EditorSpriteSet *)setsDropdown.datas[0];

		command[0] = 0;
	    enteringCommand = 0;
	    setsDropdown.num = 0;
	    SDL_StopTextInput();
	}
}

static void CommandEnteringEvent(SDL_Event ev){

	if(ev.type == SDL_KEYDOWN){

		if(ev.key.keysym.sym == SDLK_BACKSPACE){
			
			int pos = strlen(command);
			if(pos > 1) command[pos-1] = 0;
		
		} else if(ev.key.keysym.sym == SDLK_RETURN){

			ParseCommand(1);
			return;
		}

	} else if(ev.type == SDL_TEXTINPUT){

    	int pos = strlen(command);

    	if(pos < MAX_COMMAND_LENGTH-1){
			command[pos] = ev.text.text[0];
			command[pos+1] = 0;
    	}
    }
	
	ParseCommand(0);
}

static void SelectSprite(int index){

	if(!selectedSet || index > selectedSet->nObjects) return;

	selected = selectedSet->objects[index];
}

void Editor_Event(SDL_Event ev){

	if(enteringCommand){
		CommandEnteringEvent(ev);
		return;
	}

	if(ev.type == SDL_KEYDOWN){
		
		if(ev.key.keysym.sym >= SDLK_0 && ev.key.keysym.sym <= SDLK_9){

			SelectSprite(ev.key.keysym.sym - SDLK_0);

		} else if(ev.key.keysym.sym == SDLK_SLASH){

			command[0] = '/';
			command[1] = 0;

			enteringCommand = 1;

		    SDL_StartTextInput();
		}

	} else if(ev.type == SDL_KEYUP){

	} else if(ev.type == SDL_MOUSEBUTTONDOWN){

	} else if(ev.type == SDL_MOUSEBUTTONUP){

	}
}

void Editor_Init(){

	config = Loader_Load(CONFIG_PATH);

    cursorSprite = Sprite_Create();

    cursorImage = ImageLoader_CreateImage("Resources/cursor.png", 0);
    fontImage = ImageLoader_CreateImage("Resources/font.png", 0);

    Sprite_SetToImage(&cursorSprite, cursorImage);

    cursorSprite.screenRect = (IRect){0,0,-1,cursorImage.w,cursorImage.h};
	
    cursorSprite.screenRect.w *= -cursorSprite.screenRect.z;
    cursorSprite.screenRect.h *= -cursorSprite.screenRect.z;

    SDL_StopTextInput();

    enteringCommand = 0;

    command[0] = 0;
	
	memset(&setsDropdown, 0, sizeof(DropDown));
    
    Text_Init();
}

void Editor_Close(){

	Text_Close();

	ImageLoader_DeleteImage(&cursorImage);
	ImageLoader_DeleteImage(&fontImage);
	Loader_Free(&config);
}