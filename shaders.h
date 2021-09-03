#ifndef SHADERS_DEF
#define SHADERS_DEF

#include "math.h"
#include "framebuffers.h"
#include "sprite.h"

enum {
	SHADERS_POSITION_ATTRIB = 0,
	SHADERS_COORD_ATTRIB,
	SHADERS_CENTER_ATTRIB,
	SHADERS_REFLECTION_COORD_ATTRIB,
};


enum {
    PARTICLE_GPU_SHADER = 0,
    PARTICLE_CPU_SHADER,
    TEXTURELESS_SHADER,
    TEXTURED_SHADER,
    WATER_SHADER,
	POST_PROCESSING_SHADER,
	TEXT_2D_SHADER,
	PARTICLE_COMPUTE_SHADER,
	PASSTHROUGH_SHADER,
	BLUR_SHADER,
	SIMPLE_TEXTURED_SHADER,
	NUM_SHADERS,
};


typedef struct {
	Vec3 pos;
	float dist;
	float glowAmount;
	Vec4 color;
} PostProcessingLight;

void Shaders_ClearLights();
void Shaders_AddLight(PostProcessingLight light);
void Shaders_AddLights(PostProcessingLight *lights, int num);
void Shaders_UpdateLights();

ColorBuffer *Shaders_GetTempColorFbo();
GBuffer *Shaders_GetPostProcessingFbo();

void Shaders_OnResize();
void Shaders_SetBumpiness(float a);
void Shaders_DrawTexturelessVerts(Vec3 *verts, short *elements, int num);
void PostProcessingShader_Render();
unsigned int Shaders_GetVao();
int Shaders_GetCoordVbo();
int Shaders_GetNormVbo();
int Shaders_GetTangentVbo();
int Shaders_GetColorVbo();
int Shaders_GetPosVbo();
int Shaders_GetEbo();
void Shaders_SetTime(float a);
int Shaders_GetReflectionCoordVbo();
void Shaders_SetUseNormalMap(char c);
void Shaders_SetReflectionVector(Vec2 vec);
void Shaders_Init();
unsigned int Shaders_GetProgram(int program);
void Shaders_Close();
void Shaders_SetViewMatrix(float *matrix);
void Shaders_UpdateViewMatrix();
void Shaders_GetViewMatrix(float *matrix);
void Shaders_SetProjectionMatrix(float *matrix);
void Shaders_UpdateProjectionMatrix();
void Shaders_GetProjectionMatrix(float *matrix);
void Shaders_SetModelMatrix(float *matrix);
void Shaders_UpdateModelMatrix();
void Shaders_GetModelMatrix(float *matrix);
void Shaders_UseProgram(int program);
void Shaders_GetInvViewMatrix(float *matrix);
void Shaders_SetUniformColor(Vec4 color);
void Shaders_SetCameraClipMatrix(float *matrix);
void Shaders_UpdateCameraClipMatrix();
void Shaders_GetCameraClipMatrix(float *matrix);
void Shaders_SetBlurAmount(float a);
void Shaders_SetReflectionAmount(float a);
void Shaders_SetGlowAmount(float a);
void Shaders_DrawRect(IRect2D rect, Vec2 origin, int originSet, float rotation);
void Shaders_DrawTexturedRect(Rect2D imgRect, IRect2D screenRect, Material mat, int flipped, float rotation, Vec2 origin, int originSet);
void Shaders_SetFontSize(float size);
void Shaders_DrawLines(Vec3 *lines, int nLines);
void Shaders_DrawPoints(Vec3 *points, int nPoints);
Vec2 Shaders_GetViewPos();
void Shaders_SetViewPos(Vec2 pos);

#endif