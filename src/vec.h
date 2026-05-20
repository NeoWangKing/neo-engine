#ifndef VEC_H_
#define VEC_H_

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    float x, y;
} Vector2;

typedef struct {
    Vector2 *items;
    size_t count;
    size_t capacity;
} Vector2s;

static inline Vector2 make_vector2(float x, float y)
{
    Vector2 v2;
    v2.x = x;
    v2.y = y;
    return v2;
}

typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    Vector3 *items;
    size_t count;
    size_t capacity;
} Vector3s;

static inline Vector3 make_vector3(float x, float y, float z)
{
    Vector3 v3;
    v3.x = x;
    v3.y = y;
    v3.z = z;
    return v3;
}

#define EPSILON 1e-6

static inline Vector2 project_3d_2d(Vector3 v3)
{
    if (v3.z < 0) v3.z = -v3.z;
    if (v3.z < EPSILON) v3.z += EPSILON;
    return make_vector2(v3.x/v3.z, v3.y/v3.z);
}

static inline Vector2 project_2d_scr(Vector2 v2, int width, int height)
{
    return make_vector2((v2.x + 1)/2*width, (1 - (v2.y + 1)/2)*height);
}

static inline Vector3 rotate_y(Vector3 p, float delta_angle)
{
    float angle = atan2f(p.z, p.x) + delta_angle;
    float mag = sqrtf(p.x*p.x + p.z*p.z);
    return make_vector3(cosf(angle)*mag, p.y, sinf(angle)*mag);
}

typedef enum {
    FACE_V1,
    FACE_V2,
    FACE_V3,
    FACE_VT1,
    FACE_VT2,
    FACE_VT3,
    FACE_VN1,
    FACE_VN2,
    FACE_VN3,
} Face_Index;

static inline float vector3_dot(Vector3 a, Vector3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

static inline Vector3 vector3_add(Vector3 a, Vector3 b)
{
    return make_vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3 vector3_sub(Vector3 a, Vector3 b)
{
    return make_vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vector3 vector3_scale(Vector3 a, float s)
{
    return make_vector3(a.x * s, a.y * s, a.z * s);
}

static inline Vector3 vector3_cross(Vector3 a, Vector3 b)
{
    return make_vector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
            );
}

static inline Vector3 vector3_norm(Vector3 v)
{
    float len = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    if (len > 1e-6f) return vector3_scale(v, 1.0f/len);
    return make_vector3(0, 0, 1);
}

static inline float vector2_dist(Vector2 a, Vector2 b)
{
    return sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}

static inline float vector3_dist(Vector3 a, Vector3 b)
{
    return sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y) + (a.z-b.z)*(a.z-b.z));
}

static inline float lerp(float a, float b, float t)
{
    return (b - a)*t + a;
}

static inline Vector2 vector2_lerp(Vector2 a, Vector2 b, float t)
{
    return make_vector2(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}

static inline Vector3 vector3_lerp(Vector3 a, Vector3 b, float t)
{
    return make_vector3(lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t));
}

static inline float ilerp(float a, float b, float c)
{
    if (a == b) return 0.0f;
    return (c - a)/(b - a);
}

static inline float vector2_ilerp(Vector2 a, Vector2 b, Vector2 c)
{
    if (vector2_dist(a, b) == 0.0f) return 0.0f;
    return vector2_dist(a,c) / vector2_dist(a,b);
}

static inline float vector3_ilerp(Vector3 a, Vector3 b, Vector3 c)
{
    return vector3_dist(a,c) / vector3_dist(a,b);
}

#endif // VEC_H_
