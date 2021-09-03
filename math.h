#ifndef MATH_DEF
#define MATH_DEF
#include <math.h>
#include <string.h>
#include <float.h>

#define SWAP(x, y, T) do { T SWAP = x; x = y; y = SWAP; } while(0)

#define EPSILON 0.000005
#define PI 3.14159265359

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    int x;
    int y;
} IVec2;

typedef struct {
    float x;
    float y;
    float z;
} Vec3;

typedef struct {
    int x;
    int y;
    int z;
} IVec3;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec4;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Quat;

typedef struct {
    Vec2 pos;
    Vec2 line;
} Ray;

typedef struct {
    float x;
    float y;
    float z;
    float w;
    float h;
} Rect;

typedef struct {
    float x;
    float y;
    float w;
    float h;
} Rect2D;

typedef struct {
    int x;
    int y;
    int z;
    int w;
    int h;
} IRect;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} IRect2D;

static inline char Math_Vec2EqualToVec2(Vec2 v1, Vec2 v2){ return (char)(v1.x == v2.x && v1.y == v2.y); }
static inline Vec2 Math_Vec2MultFloat(Vec2 v1, float s){ return (Vec2){v1.x * s, v1.y * s}; }
static inline Vec2 Math_Vec2MultVec2(Vec2 v1, Vec2 v2){ return (Vec2){v1.x * v2.x, v1.y * v2.y}; }
static inline Vec2 Math_Vec2DivideFloat(Vec2 v1, float s){ return (Vec2){v1.x / s, v1.y / s}; }
static inline Vec2 Math_Vec2DivideVec2(Vec2 v1, Vec2 v2){ return (Vec2){v1.x / v2.x, v1.y / v2.y}; }
static inline Vec2 Math_Vec2AddFloat(Vec2 v1, float s){ return (Vec2){v1.x + s, v1.y + s}; }
static inline Vec2 Math_Vec2AddVec2(Vec2 v1, Vec2 v2){ return (Vec2){v1.x + v2.x, v1.y + v2.y}; }
static inline Vec2 Math_Vec2SubFloat(Vec2 v1, float s){ return (Vec2){v1.x - s, v1.y - s}; }
static inline Vec2 Math_Vec2SubVec2(Vec2 v1, Vec2 v2){ return (Vec2){v1.x - v2.x, v1.y - v2.y}; }
static inline float Math_Vec2Magnitude(Vec2 v) { return sqrt(v.x*v.x + v.y*v.y); }
static inline float Math_Vec2Dot(Vec2 v1, Vec2 v2){ return (v1.x * v2.x) + (v1.y * v2.y); }
static inline Vec2 Math_RoundVec2(Vec2 v){ return (Vec2){roundf(v.x), roundf(v.y)}; }

static inline Vec2 Math_Vec2Reflect(Vec2 v, Vec2 l){
    return Math_Vec2SubVec2(Math_Vec2MultFloat(Math_Vec2MultFloat(l, 2), (Math_Vec2Dot(v, l) / Math_Vec2Dot(l, l))), v);
}

static inline Vec2 Math_Vec2Normalize(Vec2 v){
    float mag = Math_Vec2Magnitude(v);
    v.x /= mag;
    v.y /= mag;
    return v;
}

static inline char Math_IVec2EqualToIVec2(IVec2 v1, IVec2 v2){ return (char)(v1.x == v2.x && v1.y == v2.y); }
static inline Vec2 Math_IVec2ToVec2(IVec2 v1){ return (Vec2){v1.x, v1.y}; }
static inline IVec2 Math_IVec2MultInt(IVec2 v1, int s){ return (IVec2){v1.x * s, v1.y * s}; }
static inline IVec2 Math_IVec2MultIVec2(IVec2 v1, IVec2 v2){ return (IVec2){v1.x * v2.x, v1.y * v2.y}; }
static inline IVec2 Math_IVec2DivideInt(IVec2 v1, int s){ return (IVec2){v1.x / s, v1.y / s}; }
static inline IVec2 Math_IVec2DivideIVec2(IVec2 v1, IVec2 v2){ return (IVec2){v1.x / v2.x, v1.y / v2.y}; }
static inline IVec2 Math_IVec2AddInt(IVec2 v1, int s){ return (IVec2){v1.x + s, v1.y + s}; }
static inline IVec2 Math_IVec2AddIVec2(IVec2 v1, IVec2 v2){ return (IVec2){v1.x + v2.x, v1.y + v2.y}; }
static inline IVec2 Math_IVec2SubInt(IVec2 v1, int s){ return (IVec2){v1.x - s, v1.y - s}; }
static inline IVec2 Math_IVec2SubIVec2(IVec2 v1, IVec2 v2){ return (IVec2){v1.x - v2.x, v1.y - v2.y}; }
static inline int Math_IVec2Magnitude(IVec2 v) { return sqrt(v.x*v.x + v.y*v.y); }
static inline int Math_IVec2Dot(IVec2 v1, IVec2 v2){ return (v1.x * v2.x) + (v1.y * v2.y); }
static inline IVec2 Math_RoundIVec2(IVec2 v){ return (IVec2){roundf(v.x), roundf(v.y)}; }

static inline IVec2 Math_IVec2Reflect(IVec2 v, IVec2 l){
    return Math_IVec2SubIVec2(Math_IVec2MultInt(Math_IVec2MultInt(l, 2), (Math_IVec2Dot(v, l) / Math_IVec2Dot(l, l))), v);
}

static inline Vec2 Math_IVec2Normalize(IVec2 v){
    int mag = Math_IVec2Magnitude(v);
    Vec2 ret = Math_IVec2ToVec2(v);
    v.x /= mag;
    v.y /= mag;
    return ret;
}

static inline char Math_Vec3EqualToVec3(Vec3 v1, Vec3 v2){ return (char)(v1.x == v2.x && v1.y == v2.y && v1.z == v2.z); }
static inline Vec3 Math_Vec3MultFloat(Vec3 v1, float s){ return (Vec3){v1.x * s, v1.y * s, v1.z * s}; }
static inline Vec3 Math_Vec3MultVec3(Vec3 v1, Vec3 v2){ return (Vec3){v1.x * v2.x, v1.y * v2.y, v1.z * v2.z}; }
static inline Vec3 Math_Vec3DivideFloat(Vec3 v1, float s){ return (Vec3){v1.x / s, v1.y / s, v1.z / s}; }
static inline Vec3 Math_Vec3DivideVec3(Vec3 v1, Vec3 v2){ return (Vec3){v1.x / v2.x, v1.y / v2.y, v1.z / v2.z}; }
static inline Vec3 Math_Vec3AddFloat(Vec3 v1, float s){ return (Vec3){v1.x + s, v1.y + s, v1.z + s}; }
static inline Vec3 Math_Vec3AddVec3(Vec3 v1, Vec3 v2){ return (Vec3){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z}; }
static inline Vec3 Math_Vec3SubFloat(Vec3 v1, float s){ return (Vec3){v1.x - s, v1.y - s, v1.z - s}; }
static inline Vec3 Math_Vec3SubVec3(Vec3 v1, Vec3 v2){ return (Vec3){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}; }
static inline float Math_Vec3Magnitude(Vec3 v) { return sqrt(v.x*v.x + v.y*v.y + v.z* v.z); }
static inline float Math_Vec3Dot(Vec3 v1, Vec3 v2){ return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z); }
static inline Vec3 Math_Vec3Cross(Vec3 v1, Vec3 v) { return (Vec3){(v1.y * v.z) - (v1.z * v.y), (v1.z * v.x) - (v1.x * v.z), (v1.x * v.y) - (v1.y * v.x) }; }
static inline Vec2 Math_Vec3ToVec2(Vec3 v){ return (Vec2){v.x, v.y }; }
static inline IVec3 Math_Vec3ToIVec3(Vec3 v){ return (IVec3){v.x, v.y, v.z}; }
static inline Vec3 Math_Vec2ToVec3(Vec2 v){ return (Vec3){v.x, v.y, 0}; }
static inline Vec3 Math_RoundVec3(Vec3 v){ return (Vec3){roundf(v.x), roundf(v.y), roundf(v.z)}; }

static inline Vec3 Math_Vec3Reflect(Vec3 v, Vec3 l){
    return Math_Vec3SubVec3(Math_Vec3MultFloat(Math_Vec3MultFloat(l, 2), (Math_Vec3Dot(v, l) / Math_Vec3Dot(l, l))), v);
}

static inline Vec3 Math_Vec3Normalize(Vec3 v){
    float mag = Math_Vec3Magnitude(v);
    v.x /= mag;
    v.y /= mag;
    v.z /= mag;
    return v;
}

static inline char Math_IVec3EqualToIVec3(IVec3 v1, IVec3 v2){ return (char)(v1.x == v2.x && v1.y == v2.y && v1.z == v2.z); }
static inline IVec3 Math_IVec3MultInt(IVec3 v1, int s){ return (IVec3){v1.x * s, v1.y * s, v1.z * s}; }
static inline IVec3 Math_IVec3MultIVec3(IVec3 v1, IVec3 v2){ return (IVec3){v1.x * v2.x, v1.y * v2.y, v1.z * v2.z}; }
static inline IVec3 Math_IVec3DivideInt(IVec3 v1, int s){ return (IVec3){v1.x / s, v1.y / s, v1.z / s}; }
static inline IVec3 Math_IVec3DivideIVec3(IVec3 v1, IVec3 v2){ return (IVec3){v1.x / v2.x, v1.y / v2.y, v1.z / v2.z}; }
static inline IVec3 Math_IVec3AddInt(IVec3 v1, int s){ return (IVec3){v1.x + s, v1.y + s, v1.z + s}; }
static inline IVec3 Math_IVec3AddIVec3(IVec3 v1, IVec3 v2){ return (IVec3){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z}; }
static inline IVec3 Math_IVec3SubInt(IVec3 v1, int s){ return (IVec3){v1.x - s, v1.y - s, v1.z - s}; }
static inline IVec3 Math_IVec3SubIVec3(IVec3 v1, IVec3 v2){ return (IVec3){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}; }
static inline int Math_IVec3Magnitude(IVec3 v) { return sqrt(v.x*v.x + v.y*v.y + v.z* v.z); }
static inline float Math_IVec3Dot(IVec3 v1, IVec3 v2){ return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z); }
static inline IVec3 Math_IVec3Cross(IVec3 v1, IVec3 v) { return (IVec3){(v1.y * v.z) - (v1.z * v.y), (v1.z * v.x) - (v1.x * v.z), (v1.x * v.y) - (v1.y * v.x) }; }
static inline Vec2 Math_IVec3ToVec2(IVec3 v){ return (Vec2){v.x, v.y }; }
static inline IVec2 Math_IVec3ToIVec2(IVec3 v){ return (IVec2){v.x, v.y }; }
static inline Vec3 Math_IVec3ToVec3(IVec3 v){ return (Vec3){v.x, v.y, v.z }; }
static inline IVec3 Math_Vec2ToIVec3(Vec2 v){ return (IVec3){v.x, v.y, 0}; }
static inline IVec3 Math_RoundIVec3(IVec3 v){ return (IVec3){roundf(v.x), roundf(v.y), roundf(v.z)}; }

static inline IVec3 Math_IVec3Reflect(IVec3 v, IVec3 l){
    return Math_IVec3SubIVec3(Math_IVec3MultInt(Math_IVec3MultInt(l, 2), (Math_IVec3Dot(v, l) / Math_IVec3Dot(l, l))), v);
}

static inline Vec3 Math_IVec3Normalize(IVec3 v){
    
    float mag = Math_IVec3Magnitude(v);
    
    Vec3 res = {
        .x = v.x / mag,
        .y = v.y / mag,
        .z = v.z / mag,
    };

    return res;
}

static inline Vec3 Math_Vec4ToVec3(Vec4 v){ return (Vec3){v.x, v.y, v.z}; }
static inline Vec2 Math_Vec4ToVec2(Vec4 v){ return (Vec2){v.x, v.y }; }
static inline char Math_Vec3EqualToVec4(Vec4 v1, Vec4 v2){ return (char)(v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w); }
static inline Vec4 Math_Vec4MultFloat(Vec4 v1, float s){ return (Vec4){v1.x * s, v1.y * s, v1.z * s, v1.w * s}; }
static inline Vec4 Math_Vec4MultVec4(Vec4 v1, Vec4 v2){ return (Vec4){v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w}; }
static inline Vec4 Math_Vec4DivideFloat(Vec4 v1, float s){ return (Vec4){v1.x / s, v1.y / s, v1.z / s, v1.w / s}; }
static inline Vec4 Math_Vec4DivideVec4(Vec4 v1, Vec4 v2){ return (Vec4){v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w}; }
static inline Vec4 Math_Vec4AddFloat(Vec4 v1, float s){ return (Vec4){v1.x + s, v1.y + s, v1.z + s, v1.w + s}; }
static inline Vec4 Math_Vec4AddVec4(Vec4 v1, Vec4 v2){ return (Vec4){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w}; }
static inline Vec4 Math_Vec4SubFloat(Vec4 v1, float s){ return (Vec4){v1.x - s, v1.y - s, v1.z - s, v1.w - s}; }
static inline Vec4 Math_Vec4SubVec4(Vec4 v1, Vec4 v2){ return (Vec4){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w}; }
static inline float Math_Vec4Magnitude(Vec4 v) { return sqrt(v.x*v.x + v.y*v.y + v.z* v.z + v.w*v.w); }
static inline Vec4 Math_RoundVec4(Vec4 v){ return (Vec4){roundf(v.x), roundf(v.y), roundf(v.z), roundf(v.w)}; }

static inline Vec4 Math_Vec4Normalize(Vec4 v){
    float mag = Math_Vec4Magnitude(v);
    v.x /= mag;
    v.y /= mag;
    v.z /= mag;
    v.w /= mag;
    return v;
}

static inline Rect Math_Rect(Vec3 pos, float width, float height){ return (Rect){pos.x, pos.y, pos.z, width, height}; }
static inline Vec3 Math_RectXYZ(Rect r){ return (Vec3){r.x, r.y, r.z};  }
static inline Rect2D Math_RoundRect2D(Rect2D r){ return (Rect2D){roundf(r.x), roundf(r.y), roundf(r.w), roundf(r.h) }; }
static inline Rect Math_RoundRect(Rect r){ return (Rect){roundf(r.x), roundf(r.y), roundf(r.z), roundf(r.w), roundf(r.h) }; }

inline char Math_CheckCollisionRect(Rect r1, Rect r){
    if(r1.x <= r.x + r.w  && r1.w  + r1.x >= r.x &&
       r1.y <= r.y + r.h && r1.h + r1.y >= r.y) return 1;

    return 0;
}

inline char Math_CheckCollisionRect2D(Rect2D r1, Rect2D r){
    if(r1.x <= r.x + r.w  && r1.w  + r1.x >= r.x &&
       r1.y <= r.y + r.h && r1.h + r1.y >= r.y) return 1;

    return 0;
}

inline Rect Math_Rect2DToRect(Rect2D r, float d){
    return (Rect){r.x, r.y, d, r.w, r.h};
}

inline IRect Math_Rect2DToIRect(Rect2D r, float d){
    return (IRect){r.x, r.y, d, r.w, r.h};
}

inline char Math_RectIsCompletelyInside(Rect r1, Rect r) {
    if(r1.x >= r.x && r1.x + r1.w <= r.x + r.w && 
       r1.y >= r.y && r1.y + r1.h <= r.y + r.h) return 1;

    return 0;
}

inline char Math_Rect2DIsCompletelyInside(Rect2D r1, Rect2D r) {
    if(r1.x >= r.x && r1.x + r1.w <= r.x + r.w && 
       r1.y >= r.y && r1.y + r1.h <= r.y + r.h) return 1;

    return 0;
}

static inline IRect Math_IRect(IVec3 pos, int width, int height){ return (IRect){pos.x, pos.y, pos.z, width, height}; }
static inline IVec3 Math_IRectXYZ(IRect r){ return (IVec3){r.x, r.y, r.z};  }

static inline void Math_SetIRectXYZ(IRect *r, IVec3 pos){
    r->x = pos.x; 
    r->y = pos.y;
    r->z = pos.z;
}

static inline IRect2D Math_RoundIRect2D(IRect2D r){ return (IRect2D){r.x, r.y, r.w, r.h }; }
static inline IRect Math_RoundIRect(IRect r){ return (IRect){r.x, r.y, r.z, r.w, r.h }; }

inline char Math_CheckCollisionIRect(IRect r1, IRect r){
    if(r1.x <= r.x + r.w  && r1.w  + r1.x >= r.x &&
       r1.y <= r.y + r.h && r1.h + r1.y >= r.y) return 1;

    return 0;
}

inline char Math_CheckCollisionIRect2D(Rect2D r1, Rect2D r){
    if(r1.x <= r.x + r.w  && r1.w  + r1.x >= r.x &&
       r1.y <= r.y + r.h && r1.h + r1.y >= r.y) return 1;

    return 0;
}

inline IRect Math_IRect2DToIRect(Rect2D r, int d){
    return (IRect){r.x, r.y, d, r.w, r.h};
}

inline char Math_IRectIsCompletelyInside(IRect r1, IRect r) {
    if(r1.x >= r.x && r1.x + r1.w <= r.x + r.w && 
       r1.y >= r.y && r1.y + r1.h <= r.y + r.h) return 1;

    return 0;
}

int Math_IRectCheckCollisionRay(IRect r, Ray ray);
float Math_GetDistanceFloat(float min1, float max1, float min2, float max2);
Vec2 Math_GetDistanceRect2D(Rect2D r1, Rect2D r2);
Vec2 Math_GetDistanceRect(Rect r1, Rect r2);
int Math_GetDistanceInt(int min1, int max1, int min2, int max2);
IVec2 Math_GetDistanceIRect(IRect r1, IRect r2);
float Math_RectCheckCollisionRay(Rect r, Ray ray);
Vec2 Math_GetDistanceRect2D(Rect2D r1, Rect2D r2);

Vec3 Math_LerpVec3(Vec3 a1, Vec3 a2, float t);
float Math_Lerp(float a1, float a2, float t);
float Math_DistanceToLine(Vec2 n, Vec2 o, Vec2 p);
char Math_QuatEqualToQuat(Quat q1, Quat q2);
Quat Math_QuatMult(Quat q1, Quat q2);
Vec3 Math_QuatRotate(Quat q, Vec3 v);
Quat Math_Quat(Vec3 v, float a);
Quat Math_QuatConj(Quat q);
float Math_QuatMag(Quat q);
Quat Math_QuatInv(Quat q);
Quat Math_QuatNormalize(Quat q);
Quat Math_QuatLookAt(Vec3 forward, Vec3 up);
Vec3 Math_RotateMatrixToEuler(float *m);
float Math_Clamp(float m, float min, float max);
void Math_InverseMatrixNxN(float *mat, int n);
float Math_Determinant(float *mat, int n);
void Math_Mat4ToMat3(float *mat4, float *mat3);
void Math_InverseMatrixMat3(float *mat3);
void Math_TransposeMatrix(float *matrix, int n);
Quat Math_MatrixToQuat(float *matrix);
void Math_OuterProduct(Vec3 vec, Vec3 trans, float *matrix);
void Math_RotateMatrix(float *matrix, Vec3 angles);
void Math_Perspective( float *matrix, float fov, float a, float n, float f);
void Math_Perspective2(float *matrix, float l, float r, float t, float b, float n, float f);
void Math_Ortho(float *matrix, float l, float r, float t, float b, float n, float f);
void Math_MatrixMatrixMult(float *res, float *a, float *b);
void Math_LookAt(float *ret, Vec3 eye, Vec3 center, Vec3 up );
void Math_RotateAroundAxis(Vec3 p, float a, float *);
void Math_MatrixFromQuat(Quat q, float*);
Quat Math_Slerp(Quat q, Quat q2, float);
Vec3 Math_MatrixMult(Vec3,float*);
Vec4 Math_MatrixMult4(Vec4,float*);
void Math_CopyMatrix(float *m1, float *m2);
void Math_InverseMatrix(float *m);
void Math_ScaleMatrix(float *matrix, int n, float amount);
void Math_ScalingMatrixXYZ(float *matrix, const Vec3 amount);
Vec3 Math_Rotate(Vec3 pos, Vec3 angles);
void Math_TranslateMatrix(float *matrix, Vec3 vector);
void Math_ScalingMatrix(float *matrix, float amount);
void Math_MatrixMatrixAdd(float *matrix, float *m0, float *m1);
void Math_MatrixMatrixSub(float *matrix, float *m0, float *m1);
Vec3 Math_QuatToAxisAngle(Quat quat, float *angle);
Vec3 Math_AxisAngleToEuler(Vec3 axis, float angle) ;
Vec3 Math_QuatToEuler(Quat quat);
Quat Math_EulerToQuat(const Vec3 euler);
void Math_Identity(float *matrix);
// Vec3 Math_RotateMatrixToEuler(float *m);
Vec2 Math_LerpVec2(Vec2 a1, Vec2 a2, float t);

#endif