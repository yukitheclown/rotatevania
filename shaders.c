#include <GL/glew.h>
#include "shaders.h"
#include "log.h"

#define MAX_LIGHTS 100
#define STRINGIFY(x) #x

static ColorBuffer tmpColorFbo;
static GBuffer postProcessingFbo;
static float cameraClipMatrix[16];
static float viewMatrix[16];
static float projMatrix[16];
static float modelMatrix[16];
// static float invViewMatrix[16];
// static float invTransMatrix[9];
static GLint activeProgram = -1;
static PostProcessingLight lights[MAX_LIGHTS];
static int numLights = 0;
static Vec2 viewPos;

static struct Shader {
    GLint program;
    GLint fShader;
    GLint vShader;
    GLint modelLoc;
    GLint projLoc;
    GLint viewLoc;
    GLint mInvTransLoc;
    GLint depthMvpLoc;
    GLint clipMatrixLoc;
    GLint camRightLoc;
    GLint camUpLoc;
    GLint uniColorLoc;
    GLint blurAmountLoc;
    GLint reflectionVecLoc;
    GLint numLightsLoc;
    GLint lightsColorsLoc;
    GLint lightPositionsLoc;
    GLint lightsSizesLoc;
    GLint lightsStrengthsLoc;
    GLint reflectionAmountLoc;
    GLint useNormalMapLoc;
    GLint timeLoc;
    GLint lightColorsLoc;
    GLint fontSizeLoc;
    GLint lightGlowsLoc;
    GLint lightGlowDistancesLoc;
    GLint bumpinessLoc;
    GLuint vao;
    GLuint posVbo;
    GLuint coordVbo;
} shaders[NUM_SHADERS];

static void CreateShader(struct Shader *shader, const char *vSource, const char *fSource){
    
    memset(shader, 0, sizeof(struct Shader));

    shader->program         = glCreateProgram();
    shader->fShader         = glCreateShader(GL_FRAGMENT_SHADER);
    shader->vShader         = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* glSrc     = fSource;
    GLint status;
    char buffer[512];

    glShaderSource(shader->fShader,1,&glSrc,NULL);
    glCompileShader(shader->fShader);
    glGetShaderiv(shader->fShader, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE){
        glGetShaderInfoLog(shader->fShader,512,NULL,buffer);
        LOGF(LOG_YELLOW,"FSHADER: %s\n", buffer);
        return;
    }

    glAttachShader(shader->program, shader->fShader);

    glSrc  = vSource;
    glShaderSource(shader->vShader,1,&glSrc,NULL);
    glCompileShader(shader->vShader);
    glGetShaderiv(shader->vShader, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE){
        glGetShaderInfoLog(shader->vShader,512,NULL,buffer);
        LOGF(LOG_YELLOW,"VSHADER: %s\n", buffer);
        return;
    }

    glAttachShader(shader->program, shader->vShader);

    glLinkProgram(shader->program);

    glUseProgram(shader->program);

    shader->modelLoc = glGetUniformLocation(shader->program, "model");
    shader->projLoc = glGetUniformLocation(shader->program, "projection");
    shader->fontSizeLoc = glGetUniformLocation(shader->program, "fontSize");
    shader->viewLoc = glGetUniformLocation(shader->program, "view");
    shader->mInvTransLoc = glGetUniformLocation(shader->program, "invTrans");
    shader->camRightLoc = glGetUniformLocation(shader->program, "cameraRight");
    shader->camUpLoc = glGetUniformLocation(shader->program, "cameraUp");
    shader->uniColorLoc = glGetUniformLocation(shader->program, "uniColor");
    shader->clipMatrixLoc = glGetUniformLocation(shader->program, "clipMatrix");
    shader->blurAmountLoc = glGetUniformLocation(shader->program, "blurAmount");
    shader->reflectionVecLoc = glGetUniformLocation(shader->program, "reflectionVector");
    shader->reflectionAmountLoc = glGetUniformLocation(shader->program, "reflectionAmount");
    shader->useNormalMapLoc = glGetUniformLocation(shader->program, "useNormalMap");
    shader->timeLoc = glGetUniformLocation(shader->program, "time");
    shader->lightPositionsLoc = glGetUniformLocation(shader->program, "lightPositions");
    shader->lightColorsLoc = glGetUniformLocation(shader->program, "lightColors");
    shader->lightGlowsLoc = glGetUniformLocation(shader->program, "lightGlows");
    shader->lightGlowDistancesLoc = glGetUniformLocation(shader->program, "lightGlowDistances");
    shader->numLightsLoc = glGetUniformLocation(shader->program, "numLights");
    shader->bumpinessLoc = glGetUniformLocation(shader->program, "bumpiness");

    glBindAttribLocation(shader->program,SHADERS_POSITION_ATTRIB, "pos");
    glBindAttribLocation(shader->program, SHADERS_COORD_ATTRIB, "coord");
    glBindAttribLocation(shader->program, SHADERS_CENTER_ATTRIB, "center");
    glBindAttribLocation(shader->program, SHADERS_REFLECTION_COORD_ATTRIB, "reflectionCoord");

    glUniform1i(glGetUniformLocation(shader->program, "colorMap"), 2);
    glUniform1i(glGetUniformLocation(shader->program, "depthMap"), 3);
    glUniform1i(glGetUniformLocation(shader->program, "positionMap"), 4);
    glUniform1i(glGetUniformLocation(shader->program, "reflectionVectorMap"), 5);
    glUniform1i(glGetUniformLocation(shader->program, "fragAttribMap"), 6);
    glUniform1i(glGetUniformLocation(shader->program, "positionVelocityXTexture"), 7);
    glUniform1i(glGetUniformLocation(shader->program, "VelocityYAttributeTexture"), 8);
    glUniform1i(glGetUniformLocation(shader->program, "normalTexture"), 9);
    glUniform1i(glGetUniformLocation(shader->program, "normalMap"), 10);
}

static const char *texturelessVSource = "#version 120\n"
STRINGIFY(
attribute vec3 pos;
attribute vec3 norm;
varying vec4 Position;
varying vec3 Position_CamSpace;
uniform mat4 model = mat4(1);
uniform mat4 view;
uniform mat4 projection;

void main(){
    Position = model * vec4(pos,1);
    Position_CamSpace = (view * Position).xyz;
    gl_Position = projection * vec4(Position_CamSpace, 1);
}
);

static const char *texturelessFSource = "#version 120\n"
STRINGIFY(
// "varying vec4 outColor;
// "varying vec4 cameraSpacePos;
// "varying vec4 fragAttribTexture;
// "varying vec4 reflectionVectorTexture;
varying vec4 Position;
varying vec3 Position_CamSpace;
uniform vec4 uniColor = vec4(1,1,1,1);
uniform float blurAmount = 0.0;
uniform vec2 reflectionVector = vec2(0,0);

void main(){

    gl_FragData[0] = uniColor;
    gl_FragData[1] = vec4(blurAmount, 0, 0, uniColor.a);
    gl_FragData[2] = vec4(Position_CamSpace,uniColor.a);
    gl_FragData[3] = vec4(reflectionVector, 0, uniColor.a);
    gl_FragData[4] = vec4(0,0,1,0);
}
);

static const char *texturedVSource = "#version 120\n"
STRINGIFY(
attribute vec3 pos;
attribute vec2 coord;
varying vec2 TexCoord;
uniform mat4 view;
uniform mat4 projection;

void main(){
    TexCoord = coord;
    gl_Position = projection * view * vec4(pos, 1);
}
);

static const char *texturedFSource = "#version 120\n"
STRINGIFY(
varying vec2 TexCoord;
varying vec4 Position;
varying vec3 Position_CamSpace;
uniform sampler2D tex;
uniform sampler2D normalTexture;
uniform vec4 uniColor = vec4(1,1,1,1);
uniform vec2 reflectionVector = vec2(0,0);
uniform float blurAmount = 0.0;
uniform float reflectionAmount = 0.0;
uniform int numLights = 0;
uniform bool useNormalMap = false;

void main(){

    vec4 color = uniColor * texture2D(tex, TexCoord);
    if(color.a < 0.1) discard;

    gl_FragData[0] = color;
    // gl_FragData[1] = vec4(blurAmount, reflectionAmount, 0, color.a);
    // gl_FragData[2] = vec4(Position_CamSpace,color.a);
    // gl_FragData[3] = vec4(reflectionVector, 0, color.a);
    // if(useNormalMap) gl_FragData[4] = texture2D(normalTexture, TexCoord);
    // else gl_FragData[4] = vec4(0,0,1,0);
    // gl_FragData[4].a = color.a;
}
);

static const char *simpleTexturedVSource = "#version 120\n"
STRINGIFY(
attribute vec3 pos;
attribute vec2 coord;
varying vec2 TexCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    TexCoord = coord;
    gl_Position = projection * view * model * vec4(pos,1);
});

static const char *simpleTexturedFSource = "#version 120\n"
STRINGIFY(
varying vec2 TexCoord;
uniform sampler2D tex;

void main(){
    gl_FragColor = texture2D(tex, TexCoord);
}
);

static const char *shadowPassThroughVert = "#version 120\n"
STRINGIFY(
attribute vec3 pos;
attribute vec2 coord;
varying vec2 Uv;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
void main(){
    gl_Position = projection * view * model * vec4(pos, 1);
    Uv = coord;
}
);

static const char *shadowPassThroughFrag = "#version 120\n"
STRINGIFY(
uniform sampler2D tex;
varying vec2 Uv;
void main(){
    vec4 color = texture2D(tex, Uv);
    if(color.a < 0.3) discard;
}
);

static const char *postProcessingVSource = "#version 120\n"
STRINGIFY(
attribute vec2 pos;
varying vec2 TexCoord;
uniform mat4 projection;
void main(){
    TexCoord = (pos/2) + 0.5;
    gl_Position = vec4(pos, 1, 1);
});

static const char *postProcessingFSource = "#version 120\n"
STRINGIFY(
varying vec2 TexCoord;
uniform sampler2D colorMap;
uniform sampler2D positionMap;
uniform sampler2D depthMap;
uniform sampler2D fragAttribMap;
uniform sampler2D reflectionVectorMap;
uniform sampler2D normalMap;
uniform mat4 clipMatrix;
uniform mat4 view;
uniform vec4 uniColor = vec4(1,1,1,1);
uniform vec3 lightPositions[100];
uniform vec4 lightColors[100];
uniform float lightGlows[100];
uniform float lightGlowDistances[100];
uniform int numLights;

vec3 cameraSpaceToScreenSpace(in vec3 camSpace){
    vec4 clipSpace = clipMatrix * vec4(camSpace, 1);
    return 0.5 * (clipSpace.xyz / clipSpace.w) + 0.5;
}

vec4 GetReflectionColor(in vec2 coord);

vec4 GetFinalColor(in vec2 coord){

    vec4 color = texture2D(colorMap, coord.xy);

    float reflectionAmount = texture2D(fragAttribMap, coord.xy).g;
    if(reflectionAmount != 0) color = color*(1-reflectionAmount) + (GetReflectionColor(coord)*reflectionAmount);

    for(int j = 0; j < numLights; j++){

        vec3 startingPos = texture2D(positionMap, coord.xy).xyz;

        vec3 lightPos = (view*vec4(lightPositions[j], 1)).xyz;

        vec3 vec = lightPos - startingPos;

        float dist = length(vec);

        if(dist < lightGlowDistances[j]){

            bool isLit = true;
        
            vec3 screenSpaceLightPos = cameraSpaceToScreenSpace(lightPos);

            vec = normalize(vec);

            for(int k = 0; k < dist; k+=2){
                
                vec3 currPos = cameraSpaceToScreenSpace(startingPos + (k * vec));
                
                if(currPos.x > 1 || currPos.x < 0 || currPos.y > 1 || currPos.y < 0 || currPos.z > 1 || currPos.z < 0) break;

                float currZPos = texture2D(depthMap, currPos.xy).r;

                if(currZPos > screenSpaceLightPos.z){
                    isLit = false;
                    break;
                }
            }

            if(isLit){
                // vec4 t = texture2D(normalMap, coord.xy);
                // vec3 norm = normalize(vec3((t.x*2)-1, (t.y*2)-1, (t.z*2)-1));
                // float theta = dot(norm, normalize(vec3(vec.x, vec.y, 0)));
                // theta = clamp(theta, 0, 1);
                float attenuation = 1.0 - (dist / lightGlowDistances[j]);
                // color += theta * attenuation * (vec4(color.xyz * lightGlows[j], color.a) + lightColors[j]);
                color += attenuation * (vec4(color.xyz * lightGlows[j], color.a) + vec4(lightColors[j].rgb, 0));
            }
        }
    }

    return color;
}

vec4 GetReflectionColor(in vec2 coord){

    vec2 reflectVector = texture2D(reflectionVectorMap, coord.xy).xy;

    vec3 startingPos = texture2D(positionMap, coord.xy).xyz;
    vec2 reflectCoord = cameraSpaceToScreenSpace(startingPos + vec3(reflectVector,0)).xy;
    float reflectRayPos = texture2D(depthMap, reflectCoord).r;
    float ourZPos = texture2D(depthMap, cameraSpaceToScreenSpace(startingPos).xy).r;

    if(ourZPos < reflectRayPos)
        return texture2D(colorMap,reflectCoord);

    return texture2D(colorMap, coord);
}

void main(){
    vec4 color = GetFinalColor(TexCoord);
    gl_FragColor = uniColor * color;
}
);


static const char *blurVSource = "#version 120\n"
STRINGIFY(
attribute vec2 pos;
varying vec2 TexCoord;
uniform mat4 projection;
void main(){
    TexCoord = (pos/2) + 0.5;
    gl_Position = vec4(pos, 1, 1);
}
);

static const char *blurFSource = "#version 120\n"
STRINGIFY(
varying vec2 TexCoord;
uniform sampler2D colorMap;
uniform bool vertical = false;

void main(){
    
    vec4 color = vec4(0);
    vec2 addVec = vec2(0.001, 0);
    if(vertical) addVec = vec2(0, 0.001);

    for(int k = -10; k <= 10; k++){
        color += texture2D(colorMap, TexCoord + (k*addVec))*0.05;
    }

    gl_FragData[3] = vec4(0,0,0,0);
    gl_FragData[2] = vec4(0,0,0,0);
    gl_FragData[1] = vec4(0,0,0,0);
    gl_FragData[0] = color;
}
);

static const char *particleCPUVSource = "#version 120\n"
STRINGIFY(
attribute vec2 pos;
attribute float id;
attribute vec2 billboardSize;
attribute vec3 billboardCenter;
attribute vec4 color;
attribute float onFrame;
varying float OnFrame;
varying vec2 TexCoord;
varying vec2 Coord;
varying vec4 colorFromVShader;
uniform vec3 rotateAxis = vec3(1,1,1);
uniform mat4 model = mat4(1);
uniform mat4 view;
uniform mat4 projection;
uniform int texWidth;

void main(){

    vec3 newpos = billboardCenter + vec3(pos.x * billboardSize.x, pos.y * billboardSize.y, 0);

    TexCoord = pos + 0.5;
    gl_Position = projection * view * model * vec4(newpos.xyz,1);
    colorFromVShader = color;
    OnFrame = onFrame;
}

);

static const char *particleCPUFSource = "#version 120\n"
STRINGIFY(
varying vec2 TexCoord;
varying vec2 Coord;
varying float OnFrame;
varying vec4 colorFromVShader;
uniform vec2 numFrames= vec2(1,1);
uniform sampler2D tex;
uniform vec4 uniColor = vec4(1,1,1,1);
uniform sampler2D VelocityYAttributeTexture;

void main(){

    vec2 texPos = vec2(1,1) / numFrames;
    int frameX = int(mod(int(OnFrame), int(numFrames.x)));
    int frameY = int(OnFrame) / int(numFrames.x);
    texPos.x = (TexCoord.x/numFrames.x) + (float(frameX)*texPos.x);
    texPos.y = (TexCoord.y/numFrames.y) + (float(frameY)*texPos.y);

    vec4 t = texture2D(tex, vec2(1,1) - texPos);
    if(t.a == 0) discard;

    gl_FragData[0] = colorFromVShader * uniColor * t;
});

static const char *particleGPUVSource = "#version 120\n"
STRINGIFY(
attribute vec2 pos;
attribute float id;
varying float OnFrame;
varying vec2 TexCoord;
varying vec2 Coord;
varying vec4 colorFromVShader;
uniform vec3 rotateAxis = vec3(1,1,1);
uniform mat4 model = mat4(1);
uniform mat4 view;
uniform mat4 projection;
uniform sampler2D positionVelocityXTexture;
uniform sampler2D VelocityYAttributeTexture;
uniform int texWidth;
uniform float billboardSize = 4;

void main(){

    Coord = vec2(mod(int(id), texWidth), id / texWidth);
    Coord /= texWidth;

    vec3 billboardCenter = vec3(texture2D(positionVelocityXTexture, Coord).rg, 1);
    vec3 newpos = billboardCenter + vec3(pos.x * billboardSize, pos.y * billboardSize, 0);

    TexCoord = pos + 0.5;
    gl_Position = projection * view * model * vec4(newpos.xyz,1);
    // colorFromVShader = color;
    // OnFrame = onFrame;
    colorFromVShader = vec4(1,1,1,1);
    OnFrame = 0;

}
);

static const char *particleGPUFSource = "#version 120\n"
STRINGIFY(
varying vec2 TexCoord;
varying vec2 Coord;
varying float OnFrame;
varying vec4 colorFromVShader;
uniform vec2 numFrames= vec2(1,1);
uniform sampler2D tex;
uniform vec4 uniColor = vec4(1,1,1,1);
uniform sampler2D VelocityYAttributeTexture;
uniform int currTime;

void main(){

    vec2 texPos = vec2(1,1) / numFrames;
    int frameX = int(mod(int(OnFrame), int(numFrames.x)));
    int frameY = int(OnFrame) / int(numFrames.x);
    texPos.x = (TexCoord.x/numFrames.x) + (float(frameX)*texPos.x);
    texPos.y = (TexCoord.y/numFrames.y) + (float(frameY)*texPos.y);

    vec4 t = texture2D(tex, vec2(1,1) - texPos);
    if(t.a == 0) discard;

    float transparency = 1 - ((currTime - texture2D(VelocityYAttributeTexture, Coord).g) / texture2D(VelocityYAttributeTexture, Coord).b);
    if(transparency <= 0) discard;

    gl_FragData[0] = colorFromVShader * uniColor * t * transparency;
}
);

static const char *waterVSource = "#version 120\n"
STRINGIFY(
attribute vec3 pos;
attribute vec2 coord;
attribute vec2 reflectionCoord;
// attribute vec3 norm;
// attribute vec3 tangent;
attribute vec4 color;
varying vec2 TexCoord;
varying vec2 ReflectionCoord;
// varying vec3 Normal_Worldspace;
varying vec3 Position_CamSpace;
varying vec4 Position;
varying vec4 Color;
// varying mat3 LocalSurface2World;
// uniform mat3 invTrans = mat3(1);
uniform mat4 model = mat4(1);
uniform mat4 view;
uniform mat4 projection;

void main(){
    // Normal_Worldspace = (invTrans * norm).xyz;
    Position = model * vec4(pos,1);
    Position_CamSpace = (view * Position).xyz;
    TexCoord = coord;
    gl_Position = projection * vec4(Position_CamSpace, 1);
    Color = color;
    ReflectionCoord = reflectionCoord;
    // LocalSurface2World[0] = normalize(vec3(invTrans * tangent));
    // LocalSurface2World[1] = normalize(Normal_Worldspace);
    // LocalSurface2World[2] = normalize(cross(LocalSurface2World[0], LocalSurface2World[1]));
}
);

static const char *waterFSource = "#version 120\n"
STRINGIFY(
varying vec2 TexCoord;
varying vec4 Position;
varying vec4 Color;
// varying vec3 Normal_Worldspace;
// varying mat3 LocalSurface2World;
varying vec3 Position_CamSpace;
varying vec2 ReflectionCoord;
uniform sampler2D colorMap;
uniform sampler2D depthMap;
uniform float time;
uniform sampler2D tex;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 uniColor = vec4(0.1,0.6,0.7,0.1);
uniform float bumpiness = 0.01;
uniform float reflectionAmount = 0.7;

vec3 cameraSpaceToScreenSpace(in vec3 camSpace){
    vec4 clipSpace = projection * vec4(camSpace, 1);
    return 0.5 * (clipSpace.xyz / clipSpace.w) + 0.5;
}

void main(){

    vec3 screenSpace = cameraSpaceToScreenSpace(Position_CamSpace);

    if(texture2D(depthMap, screenSpace.xy).r >= screenSpace.z) discard;

    vec4 finalColor = uniColor;

    vec4 t;
    if(1-TexCoord.y < time)
        t = texture2D(tex, vec2(1-((1-TexCoord.x) + (1-time)), TexCoord.y));
    else
        t = texture2D(tex, vec2(TexCoord.x+time, TexCoord.y));

    // vec3 normal = normalize(normalize(vec3((t.x*2)-1, (t.y*2)-1, (t.z*2)-1)) * LocalSurface2World);
    vec3 normal = normalize(vec3((t.x*2)-1, (t.y*2)-1, (t.z*2)-1));
    // vec3 normal = normalize(Normal_Worldspace);

    vec2 reflectionVector = normal.xy*bumpiness;

    vec2 reflectionPos = ReflectionCoord + reflectionVector;
    
    finalColor += texture2D(colorMap,reflectionPos) * reflectionAmount;

    gl_FragData[0] = finalColor;

    gl_FragData[3] = vec4(0,0,0,1);
    gl_FragData[4] = vec4(0,0,1,1);
    gl_FragData[2] = vec4(0,0,0,1);
    gl_FragData[1] = vec4(0,0,0,1);
}
);

static const char *text2DFSource = "#version 120\n"
STRINGIFY(

varying vec2 TexCoord;
uniform sampler2D tex;
uniform vec4 uniColor;

void main(){

    gl_FragColor = texture2D(tex, vec2(TexCoord.x, 1-TexCoord.y));
}
);

static const char *text2DVSource = "#version 120\n"
STRINGIFY(

attribute vec2 pos;
attribute vec2 coord;
attribute vec2 center;

varying vec2 TexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform float fontSize;

void main(){

    TexCoord = vec2(coord.x + (pos.x * (1.0/16)), coord.y + (pos.y * (1.0/16)));

    gl_Position = projection * view * vec4((pos * fontSize) + center, -1, 1);
}
);

static const char *particleComputeFSource = "#version 120\n"
STRINGIFY(
varying vec2 Coord;
uniform sampler2D positionVelocityXTex;
uniform sampler2D VelocityYAttribsTex;
uniform int deltaTime = 1;
uniform vec2 gravity = vec2(0,0.000025);

void main(){
    
    vec2 vel;
    
    vec3 t2 = texture2D(VelocityYAttribsTex,Coord).rgb;
    vec3 t1 = texture2D(positionVelocityXTex,Coord).rgb;
        
    vel.x = t1.b;
    vel.y = t2.r;
    vel += (gravity * deltaTime);

    vec2 pos = t1.rg;

    pos += vel * deltaTime;

    gl_FragData[0] = vec4(pos.x, pos.y, vel.x, 1);
    gl_FragData[1] = vec4(vel.y, t2.g, t2.b, 1);
}
);

static const char *particleComputeVSource = "#version 120\n"
STRINGIFY(
attribute vec3 pos;
attribute vec2 coord;

varying vec2 Coord;
void main(){
    gl_Position = vec4(pos, 1);
    Coord = coord;
}
);

static struct {
    const char **vSource;
    const char **fSource;
} sources[NUM_SHADERS] = {
    { &particleGPUVSource, &particleGPUFSource },
    { &particleCPUVSource, &particleCPUFSource },
    { &texturelessVSource, &texturelessFSource },
    { &texturedVSource, &texturedFSource },
    { &waterVSource, &waterFSource },
    { &postProcessingVSource, &postProcessingFSource},
    { &text2DVSource, &text2DFSource},
    { &particleComputeVSource, &particleComputeFSource},
    { &shadowPassThroughVert, &shadowPassThroughFrag},
    { &blurVSource, &blurFSource},
    { &simpleTexturedVSource, &simpleTexturedFSource},
};


static void InitShaderBuffers(int shader, int uv){

    Shaders_UseProgram(shader);
    
    glGenVertexArrays(1, &shaders[shader].vao);
    glBindVertexArray(shaders[shader].vao);

    glGenBuffers(1, &shaders[shader].posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, shaders[shader].posVbo);
    glEnableVertexAttribArray(SHADERS_POSITION_ATTRIB);
    glVertexAttribPointer(SHADERS_POSITION_ATTRIB, 3, GL_FLOAT, GL_FALSE, 0, 0);

    if(uv){
        glGenBuffers(1, &shaders[shader].coordVbo);
        glBindBuffer(GL_ARRAY_BUFFER, shaders[shader].coordVbo);
        glEnableVertexAttribArray(SHADERS_COORD_ATTRIB);
        glVertexAttribPointer(SHADERS_COORD_ATTRIB, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }    

    glBindVertexArray(0);
}

static void PostProcessingShader_Create(){

    Shaders_UseProgram(POST_PROCESSING_SHADER);
    
    glGenVertexArrays(1, &shaders[POST_PROCESSING_SHADER].vao);
    glBindVertexArray(shaders[POST_PROCESSING_SHADER].vao);

    glGenBuffers(1, &shaders[POST_PROCESSING_SHADER].posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, shaders[POST_PROCESSING_SHADER].posVbo);

    glEnableVertexAttribArray(SHADERS_POSITION_ATTRIB);
    glVertexAttribPointer(SHADERS_POSITION_ATTRIB, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    Vec2 positions[] = {{-1, -1 },{ 1, -1 },{ 1,  1 },{ 1,  1 },{-1,  1 }, {-1, -1 }};
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2)*6, &positions[0].x, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Shaders_Init(int w, int h){

    int k;
    for(k = 0; k < NUM_SHADERS; k++)
        CreateShader(&shaders[k], *sources[k].vSource, *sources[k].fSource);

    // GPUParticleSystems_Init();
    // CPUParticleSystems_Init();

    int formats[] = {GL_RGB,GL_RGB,GL_RGBA16F,GL_RGBA16F,GL_RGB};
    int types[] = {GL_UNSIGNED_BYTE,GL_UNSIGNED_BYTE,GL_UNSIGNED_BYTE,GL_UNSIGNED_BYTE,GL_UNSIGNED_BYTE};
    
    int cformats[] = {GL_RGB};
    int ctypes[] = {GL_UNSIGNED_BYTE};

    tmpColorFbo = FrameBuffers_CreateColorBuffer(w, h, 1, cformats, ctypes);
    postProcessingFbo = FrameBuffers_CreateGBuffer(w, h, 5, formats, types);

    PostProcessingShader_Create();

    InitShaderBuffers(TEXTURELESS_SHADER, 0);
    InitShaderBuffers(TEXTURED_SHADER, 1);
}

void Shaders_OnResize(int w, int h){

    int formats[] = {GL_RGB,GL_RGB,GL_RGBA16F,GL_RGBA16F,GL_RGB};
    int types[] = {GL_UNSIGNED_BYTE,GL_UNSIGNED_BYTE,GL_UNSIGNED_BYTE,GL_UNSIGNED_BYTE,GL_UNSIGNED_BYTE};
    
    int cformats[] = {GL_RGB};
    int ctypes[] = {GL_UNSIGNED_BYTE};

    FrameBuffers_ResizeColorBuffer(Shaders_GetTempColorFbo(), w, h, cformats, ctypes);
    FrameBuffers_ResizeGBuffer(Shaders_GetPostProcessingFbo(), w, h, formats, types);

    glViewport(0,0,w,h);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Shaders_GetPostProcessingFbo()->fb);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Shaders_GetTempColorFbo()->fb);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

unsigned int Shaders_GetProgram(int program){
    return shaders[program].program;
}

void Shaders_Close(){

    int k;
    for(k = 0; k < NUM_SHADERS; k++){

        glDeleteProgram(shaders[k].program);
        glDeleteShader(shaders[k].fShader);
        glDeleteShader(shaders[k].vShader);
    }

    // GPUParticleSystems_CleanUp();
    // CPUParticleSystems_CleanUp();

    FrameBuffers_DestroyColorBuffer(&tmpColorFbo);
    FrameBuffers_DestroyGBuffer(&postProcessingFbo);
}

ColorBuffer *Shaders_GetTempColorFbo(){
    return &tmpColorFbo; 
}

GBuffer *Shaders_GetPostProcessingFbo(){
    return &postProcessingFbo;
}

Vec2 Shaders_GetViewPos(){

    return viewPos;
}

void Shaders_SetViewPos(Vec2 pos){

    Math_TranslateMatrix(viewMatrix, (Vec3){-pos.x, -pos.y, 0});
    viewPos = pos;
}

void Shaders_SetViewMatrix(float *matrix){
    memcpy(&viewMatrix[0], matrix, 16*sizeof(float));
    // memcpy(&invViewMatrix[0], matrix, 16*sizeof(float));
    // Math_InverseMatrix(invViewMatrix);
}

void Shaders_UpdateViewMatrix(){
    if(activeProgram == (GLint)-1) return;
    glUniformMatrix4fv(shaders[activeProgram].viewLoc, 1, GL_TRUE, viewMatrix);

    // if(shaders[activeProgram].invViewLoc != (GLint)-1)
    //     glUniformMatrix4fv(shaders[activeProgram].invViewLoc, 1, GL_TRUE, invViewMatrix);

    // if(shaders[activeProgram].camRightLoc != (GLint)-1 && shaders[activeProgram].camUpLoc != (GLint)-1){
    //     Vector3 cameraRight = (Vector3){viewMatrix[0], viewMatrix[4], viewMatrix[8]};
    //     Vector3 cameraUp    = (Vector3){viewMatrix[1], viewMatrix[5], viewMatrix[9]};
    //     glUniform3fv(shaders[activeProgram].camRightLoc, 1, &cameraRight.x);
    //     glUniform3fv(shaders[activeProgram].camUpLoc, 1, &cameraUp.x);
    // }
}

void Shaders_GetViewMatrix(float *matrix){
    memcpy(matrix, &viewMatrix[0], 16*sizeof(float));
}

void Shaders_SetProjectionMatrix(float *matrix){
    memcpy(&projMatrix[0], matrix, 16*sizeof(float));
}

void Shaders_UpdateProjectionMatrix(){
    if(activeProgram == (GLint)-1) return;
    glUniformMatrix4fv(shaders[activeProgram].projLoc, 1, GL_TRUE, projMatrix);
}

void Shaders_GetProjectionMatrix(float *matrix){
    memcpy(matrix, &projMatrix[0], 16*sizeof(float));
}

void Shaders_SetModelMatrix(float *matrix){
    memcpy(&modelMatrix[0], matrix, 16*sizeof(float));
    // Math_Mat4ToMat3(modelMatrix, invTransMatrix);
    // Math_InverseMatrixMat3(invTransMatrix);
    // Math_TransposeMatrix(invTransMatrix, 3);
}

void Shaders_UpdateModelMatrix(){
    if(activeProgram == (GLint)-1) return;
    glUniformMatrix4fv(shaders[activeProgram].modelLoc, 1, GL_TRUE, modelMatrix);

    // if(shaders[activeProgram].mInvTransLoc != (GLint)-1)
    //     glUniformMatrix3fv(shaders[activeProgram].mInvTransLoc, 1, GL_TRUE, invTransMatrix);
}

void Shaders_GetModelMatrix(float *matrix){
    memcpy(matrix, &modelMatrix[0], 16*sizeof(float));
}

void Shaders_SetUseNormalMap(char c){
    if(activeProgram == (GLint)-1 || shaders[activeProgram].useNormalMapLoc == (GLint)-1) return;
    glUniform1i(shaders[activeProgram].useNormalMapLoc, c);
}

void Shaders_UseProgram(int program){
    if((int)activeProgram == program) return;
    activeProgram = program;
    glUseProgram(Shaders_GetProgram(program));
}

void Shaders_SetFontSize(float size){
    glUniform1f(shaders[activeProgram].fontSizeLoc, size);
}

void Shaders_SetUniformColor(Vec4 color){
    glUniform4f(shaders[activeProgram].uniColorLoc, color.x, color.y, color.z, color.w);
}

void Shaders_SetCameraClipMatrix(float *matrix){
    memcpy(&cameraClipMatrix[0], matrix, 16*sizeof(float));
}

void Shaders_UpdateCameraClipMatrix(){
    if(activeProgram == (GLint)-1 || shaders[activeProgram].clipMatrixLoc == (GLint)-1) return;
    glUniformMatrix4fv(shaders[activeProgram].clipMatrixLoc, 1, GL_TRUE, cameraClipMatrix);
}

void Shaders_GetCameraClipMatrix(float *matrix){
    memcpy(matrix, &cameraClipMatrix[0], 16*sizeof(float));
}

void Shaders_SetTime(float a){
    if(activeProgram == (GLint)-1 || shaders[activeProgram].timeLoc == (GLint)-1) return;
    glUniform1f(shaders[activeProgram].timeLoc, a);
}

void Shaders_SetBlurAmount(float a){
    if(activeProgram == (GLint)-1 || shaders[activeProgram].blurAmountLoc == (GLint)-1) return;
    glUniform1f(shaders[activeProgram].blurAmountLoc, a);
}

void Shaders_SetReflectionVector(Vec2 vec){
    if(activeProgram == (GLint)-1 || shaders[activeProgram].reflectionVecLoc == (GLint)-1) return;
    glUniform2f(shaders[activeProgram].reflectionVecLoc, vec.x, vec.y);
}

void Shaders_SetReflectionAmount(float a){
    if(activeProgram == (GLint)-1 || shaders[activeProgram].reflectionAmountLoc == (GLint)-1) return;
    glUniform1f(shaders[activeProgram].reflectionAmountLoc, a);
}

void Shaders_SetBumpiness(float a){
    if(activeProgram == (GLint)-1 || shaders[activeProgram].bumpinessLoc == (GLint)-1) return;
    glUniform1f(shaders[activeProgram].bumpinessLoc, a);
}

void PostProcessingShader_Render(){
    Shaders_UseProgram(POST_PROCESSING_SHADER);
    glBindVertexArray(shaders[POST_PROCESSING_SHADER].vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Shaders_UpdateLights(){

    if(!numLights) return;

    Shaders_UseProgram(POST_PROCESSING_SHADER);

    Vec4 lightColors[numLights];
    Vec3 lightPositions[numLights];
    float lightGlowDistances[numLights];
    float lightGlows[numLights];

    int k;
    for(k = 0; k < numLights; k++){
        lightColors[k] = lights[k].color;
        lightPositions[k] = lights[k].pos;
        lightGlowDistances[k] = lights[k].dist;
        lightGlows[k] = lights[k].glowAmount;
    }

    glUniform1i(shaders[activeProgram].numLightsLoc, numLights);
    glUniform3fv(shaders[activeProgram].lightPositionsLoc, numLights, (GLfloat *)&lightPositions[0]);
    glUniform4fv(shaders[activeProgram].lightColorsLoc, numLights, (GLfloat *)&lightColors[0]);
    glUniform1fv(shaders[activeProgram].lightGlowDistancesLoc, numLights, (GLfloat *)&lightGlowDistances[0]);
    glUniform1fv(shaders[activeProgram].lightGlowsLoc, numLights, (GLfloat *)&lightGlows[0]);
}

void Shaders_ClearLights(){
    numLights = 0;
}

void Shaders_AddLight(PostProcessingLight light){
    if(numLights+1 > MAX_LIGHTS) return;
    ++numLights;
    lights[numLights-1] = light;
}

void Shaders_AddLights(PostProcessingLight *newLights, int num){
    if(numLights + num >= MAX_LIGHTS) return;
    memcpy(&lights[numLights], newLights, num * sizeof(PostProcessingLight));
    numLights += num;
}

void Shaders_DrawTexturelessVerts(Vec3 *verts, short *elements, int num){

    Shaders_UseProgram(TEXTURELESS_SHADER);

    glBindVertexArray(shaders[TEXTURELESS_SHADER].vao);

    glBindBuffer(GL_ARRAY_BUFFER, shaders[TEXTURELESS_SHADER].posVbo);
    glBufferData(GL_ARRAY_BUFFER, num*sizeof(Vec3), (float *)&verts[0].x, GL_STATIC_DRAW);

    Shaders_UpdateViewMatrix();
    Shaders_UpdateProjectionMatrix();

    glDrawElements(GL_TRIANGLES, (num/4) * 6, GL_UNSIGNED_SHORT, elements);
}

void Shaders_DrawRect(IRect2D rect, Vec2 origin, int originSet, float rotation){

    Vec3 verts[4];

    Sprite_GetVertsFromRect(&rect, verts, rotation, origin, originSet);

    short elements[] = {0,2,1,2,0,3};

    Shaders_DrawTexturelessVerts(verts, elements, 4);
}

void Shaders_DrawTexturedRect(Rect2D imgRect, IRect2D screenRect, Material mat, int flipped, float rotation, Vec2 origin, int originSet){

    Vec2 coords[4];
    Vec3 verts[4];
    Sprite_GetCoordsFromRect(imgRect, coords, flipped, &mat.img);
    Sprite_GetVertsFromRect(&screenRect, verts, rotation, origin, originSet);

    short elements[] = {0,2,1,2,0,3};

    Shaders_UseProgram(TEXTURED_SHADER);

    Shaders_UpdateViewMatrix();
    Shaders_UpdateProjectionMatrix();
    glBindVertexArray(shaders[TEXTURED_SHADER].vao);

    glBindBuffer(GL_ARRAY_BUFFER, shaders[TEXTURED_SHADER].posVbo);
    glBufferData(GL_ARRAY_BUFFER, 4*sizeof(Vec3), (float *)&verts[0].x, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, shaders[TEXTURED_SHADER].coordVbo);
    glBufferData(GL_ARRAY_BUFFER, 4*sizeof(Vec2), (float *)&coords[0].x, GL_STATIC_DRAW);

    Shaders_SetUniformColor(mat.color);
    Shaders_SetReflectionAmount(mat.reflectionAmount);
    Shaders_SetReflectionVector(mat.reflectionVec);

    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mat.img.glTexture);

    if(mat.normalImg.glTexture != 0){
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, mat.normalImg.glTexture);
        Shaders_SetUseNormalMap(1);
    } else {
        Shaders_SetUseNormalMap(0);
    }

    if(!mat.castShadow) glDepthMask(GL_FALSE);

    if(mat.additive)
        glBlendFunc(GL_ONE,GL_ONE);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, elements);

    if(mat.additive)
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    if(!mat.castShadow) glDepthMask(GL_TRUE);
}

void Shaders_DrawPoints(Vec3 *points, int nPoints){

    Shaders_UseProgram(TEXTURELESS_SHADER);

    Shaders_UpdateProjectionMatrix();
    Shaders_UpdateViewMatrix();

    glBindVertexArray(shaders[TEXTURELESS_SHADER].vao);

    glBindBuffer(GL_ARRAY_BUFFER,shaders[TEXTURELESS_SHADER].posVbo);
    glBufferData(GL_ARRAY_BUFFER, nPoints*sizeof(Vec3), points, GL_STATIC_DRAW);

    glDrawArrays(GL_POINTS, 0, nPoints);

    glBindVertexArray(0);
}

void Shaders_DrawLines(Vec3 *lines, int nLines){

    Shaders_UseProgram(TEXTURELESS_SHADER);
    Shaders_UpdateProjectionMatrix();
    Shaders_UpdateViewMatrix();

    glBindVertexArray(shaders[TEXTURELESS_SHADER].vao);

    glBindBuffer(GL_ARRAY_BUFFER,shaders[TEXTURELESS_SHADER].posVbo);
    glBufferData(GL_ARRAY_BUFFER, nLines*sizeof(Vec3), lines, GL_STATIC_DRAW);

    glDrawArrays(GL_LINE_STRIP, 0, nLines);

    glBindVertexArray(0);
}