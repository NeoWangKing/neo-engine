#include <assert.h>
#include <math.h>
#include <X11/keysym.h>
#include <stddef.h>
#include <stdint.h>
#include "nob.h"
#undef temp_alloc
#include "vec.h"
#define OLIVEC_IMPLEMENTATION
#include "olive.c"
#define NVC_AA_RES 1
#include "neovin.c"
#include "game.h"
#include "stb_vorbis.c"
#include "../assets/model3d/utahTeapot.c"
// #include "../assets/model3d/Amiya1.c"

#define TARGET_FPS 60
#define DISPLAY_WIDTH  800
#define DISPLAY_HEIGHT 600
#define DISPLAY_ASPECT (float)DISPLAY_HEIGHT/DISPLAY_WIDTH
// #define DISPLAY_WIDTH  1920
// #define DISPLAY_HEIGHT 1080
#define DELTA_TIME (1.0f/TARGET_FPS)
#define AUDIO_SAMPLE_RATE 44100
static_assert(AUDIO_SAMPLE_RATE%TARGET_FPS == 0, "Sample rate must be divisible by FPS");
#define AUDIO_CHANNELS 2
#define AUDIO_CAPACITY (AUDIO_SAMPLE_RATE/TARGET_FPS*AUDIO_CHANNELS)
#define NEAR_CLIP 0.1f
#define FAR_CLIP 5.0f

#define KEY_CTRL_L XK_Control_L
#define KEY_CTRL_R XK_Control_R

static Controls controls = {0};

typedef struct {
    uint8_t b, g, r, a;
} Color;

static inline Color color_lerp(Color c1, Color c2, float t)
{
    return (Color) {
        .r = lerp(c1.r, c2.r, t),
        .g = lerp(c1.g, c2.g, t),
        .b = lerp(c1.b, c2.b, t),
        .a = lerp(c1.a, c2.a, t),
    };
}

static uint32_t BACKGROUND_COLOR = 0xFFFFFFAA;
static uint32_t FOREGROUND_COLOR = 0xFF9999FF;

typedef struct {
    Color *items;
    size_t count;
    size_t capacity;
} Colors;

static Color display[DISPLAY_WIDTH*DISPLAY_HEIGHT];
static float rzbuffer[DISPLAY_WIDTH*DISPLAY_HEIGHT] = {0};
static int16_t audio[AUDIO_CAPACITY];

static struct {
    int key;
    Vector3 vector;
    float angle_yaw;
    float angle_pit;
} cam_ctrl[] = {
    { .key = 'w',       .vector = { 0.0f,  0.0f,  1.0f}, .angle_yaw =  0.0f, .angle_pit =  0.0f},
    { .key = 's',       .vector = { 0.0f,  0.0f, -1.0f}, .angle_yaw =  0.0f, .angle_pit =  0.0f},
    { .key = 'a',       .vector = {-1.0f,  0.0f,  0.0f}, .angle_yaw =  0.0f, .angle_pit =  0.0f},
    { .key = 'd',       .vector = { 1.0f,  0.0f,  0.0f}, .angle_yaw =  0.0f, .angle_pit =  0.0f},
    { .key = ' ',       .vector = { 0.0f,  1.0f,  0.0f}, .angle_yaw =  0.0f, .angle_pit =  0.0f},
    { .key = KEY_CTRL_L,.vector = { 0.0f, -1.0f,  0.0f}, .angle_yaw =  0.0f, .angle_pit =  0.0f},
    { .key = 'q',       .vector = { 0.0f,  0.0f,  0.0f}, .angle_yaw =  1.0f, .angle_pit =  0.0f},
    { .key = 'e',       .vector = { 0.0f,  0.0f,  0.0f}, .angle_yaw = -1.0f, .angle_pit =  0.0f},
    { .key = 'r',       .vector = { 0.0f,  0.0f,  0.0f}, .angle_yaw =  0.0f, .angle_pit =  1.0f },
    { .key = 'f',       .vector = { 0.0f,  0.0f,  0.0f}, .angle_yaw =  0.0f, .angle_pit = -1.0f },
};

static struct {
    Vector3s vertices;
    Colors   colors;
    Vector3  cam_pos;
    float    cam_yaw;
    float    cam_pit;
} renderer = {
    .cam_pos = {0.0f, 0.0f, -2.0f},
    .cam_yaw = 0.0f,
    .cam_pit = 0.0f,
};

static inline void renderer_reset(void)
{
    renderer.vertices.count = 0;
    renderer.colors.count = 0;

    for (size_t i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; ++i) {
        rzbuffer[i] = 0;
        display[i] = *(Color*)&BACKGROUND_COLOR;
    }
}

static inline void renderer_push_vertex(Vector3 vertex, Color color)
{
    da_append(&renderer.vertices, vertex);
    da_append(&renderer.colors, color);
}

static inline Vector3 clip_line(Vector3 p1, Vector3 p2, float z_plane, float *t)
{
    Vector3 q = { .z = NEAR_CLIP };
    *t = (z_plane - p1.z)/(p2.z - p1.z);
    q.x = (p2.x - p1.x)*(*t) + p1.x;
    q.y = (p2.y - p1.y)*(*t) + p1.y;
    return q;
}

static inline void renderer_push_triangle(Vector3 _v1, Vector3 _v2, Vector3 _v3, Color _c1, Color _c2, Color _c3)
{
    Vector3 forward = make_vector3(cosf(renderer.cam_pit)*sinf(renderer.cam_yaw), sinf(renderer.cam_pit), cosf(renderer.cam_pit)*cosf(renderer.cam_yaw));
    Vector3 world_up = make_vector3(0.0f, 1.0f, 0.0f);
    Vector3 right = vector3_norm(vector3_cross(forward, world_up));
    Vector3 up = vector3_cross(right, forward);

    Vector3 v[3] = { _v1, _v2, _v3 };
    Color   c[3] = { _c1, _c2, _c3 };

    for (int i = 0; i < 3; ++i) {
        v[i] = vector3_sub(v[i], renderer.cam_pos);
        v[i] = make_vector3(vector3_dot(v[i], right), vector3_dot(v[i], up), vector3_dot(v[i], forward));
        v[i].x *= DISPLAY_ASPECT;
    }

    int clip[3] = {0};
    int clip_count = 0;
    int unclip[3] = {0};
    int unclip_count = 0;

    for (int i = 0; i < 3; ++i) {
        if (v[i].z < NEAR_CLIP) {
            clip[clip_count++] = i;
        } else {
            unclip[unclip_count++] = i;
        }
    }

    switch (clip_count) {
        case 0:
            {
                for (size_t i = 0; i < 3; ++i) {
                    renderer_push_vertex(v[i], c[i]);
                }
                break;
            }
        case 1:
            {
                float t1, t2;
                Vector3 q1 = clip_line(v[clip[0]], v[unclip[0]], NEAR_CLIP, &t1);
                Color cq1 = color_lerp(c[clip[0]], c[unclip[0]], t1);
                Vector3 q2 = clip_line(v[clip[0]], v[unclip[1]], NEAR_CLIP, &t2);
                Color cq2 = color_lerp(c[clip[0]], c[unclip[1]], t2);
                renderer_push_vertex(v[unclip[0]], c[unclip[0]]);
                renderer_push_vertex(q1          , cq1);
                renderer_push_vertex(q2          , cq2);

                renderer_push_vertex(v[unclip[0]], c[unclip[0]]);
                renderer_push_vertex(v[unclip[1]], c[unclip[1]]);
                renderer_push_vertex(q2          , cq2);
                break;
            }
        case 2:
            {
                float t1, t2;
                Vector3 q1 = clip_line(v[clip[0]], v[unclip[0]], NEAR_CLIP, &t1);
                Color cq1 = color_lerp(c[clip[0]], c[unclip[0]], t1);
                Vector3 q2 = clip_line(v[clip[1]], v[unclip[0]], NEAR_CLIP, &t2);
                Color cq2 = color_lerp(c[clip[1]], c[unclip[0]], t2);
                renderer_push_vertex(v[unclip[0]], c[unclip[0]]);
                renderer_push_vertex(q1          , cq1);
                renderer_push_vertex(q2          , cq2);
                break;
            }
        case 3:
        default:
            {
                break;
            }
    }
}

static void renderer_flush_triangle(Vector3 v1, Vector3 v2, Vector3 v3, Color c1, Color c2, Color c3)
{
    Olivec_Canvas oc = olivec_canvas((uint32_t *)display, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_WIDTH);

    Vector2 p1 = project_2d_scr(project_3d_2d(v1), DISPLAY_WIDTH, DISPLAY_HEIGHT);
    Vector2 p2 = project_2d_scr(project_3d_2d(v2), DISPLAY_WIDTH, DISPLAY_HEIGHT);
    Vector2 p3 = project_2d_scr(project_3d_2d(v3), DISPLAY_WIDTH, DISPLAY_HEIGHT);

    int x1 = p1.x;
    int x2 = p2.x;
    int x3 = p3.x;
    int y1 = p1.y;
    int y2 = p2.y;
    int y3 = p3.y;
    int lx, hx, ly, hy;
    if (olivec_normalize_triangle(oc.width, oc.height, x1, y1, x2, y2, x3, y3, &lx, &hx, &ly, &hy)) {
        for (int y = ly; y <= hy; ++y) {
            for (int x = lx; x <= hx; ++x) {
                int u1, u2, det;
                if (olivec_barycentric(x1, y1, x2, y2, x3, y3, x, y, &u1, &u2, &det)) {
                    int u3 = det - u1 - u2;
                    float rz = 1/v1.z*u1/det + 1/v2.z*u2/det + 1/v3.z*u3/det;
                    if (1.0f/FAR_CLIP < rz && rz < 1.0f/NEAR_CLIP && rz > rzbuffer[y*DISPLAY_WIDTH + x]) {
                        rzbuffer[y*DISPLAY_WIDTH + x] = rz;

                        Color c = {.g = 255, .a = 255};
                        c.r = (c1.r*u1/v1.z + c2.r*u2/v2.z + c3.r*u3/v3.z)/(rz*det);
                        c.g = (c1.g*u1/v1.z + c2.g*u2/v2.z + c3.g*u3/v3.z)/(rz*det);
                        c.b = (c1.b*u1/v1.z + c2.b*u2/v2.z + c3.b*u3/v3.z)/(rz*det);
                        c.a = 255;

                        OLIVEC_PIXEL(oc, x, y) = *(uint32_t*)&c;;

                        // The Fog
                        if (1) {
                            float z = 1.0f / rz;
                            float t = (z - NEAR_CLIP) / (FAR_CLIP - NEAR_CLIP);
                            if (t < 0.0f) t = 0.0f;
                            if (t > 1.0f) t = 1.0f;

                            uint32_t alpha = (uint32_t)(t * 255.0f);
                            if (alpha > 0) {
                                uint32_t fog_r = OLIVEC_RED(BACKGROUND_COLOR);
                                uint32_t fog_g = OLIVEC_GREEN(BACKGROUND_COLOR);
                                uint32_t fog_b = OLIVEC_BLUE(BACKGROUND_COLOR);
                                uint32_t fog_color = OLIVEC_RGBA(fog_r, fog_g, fog_b, alpha);
                                olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), fog_color);
                            }
                        }
                    }
                }
            }
        }
    }
}

static void renderer_flush(void)
{
    for (size_t i = 0; i + 3 <= renderer.vertices.count; i += 3) {
        Vector3 v1 = renderer.vertices.items[i + 0];
        Vector3 v2 = renderer.vertices.items[i + 1];
        Vector3 v3 = renderer.vertices.items[i + 2];
        // v1 = vector3_scale(v1, 0.1);
        // v2 = vector3_scale(v2, 0.1);
        // v3 = vector3_scale(v3, 0.1);
        Color c1 = renderer.colors.items[i + 0];
        Color c2 = renderer.colors.items[i + 1];
        Color c3 = renderer.colors.items[i + 2];

        renderer_flush_triangle(v1, v2, v3, c1, c2, c3);
    }
}

static void push_utah_teapot(Vector3 teapot_pos, float teapot_angle)
{
    for (size_t i = 0; i < faces_count; ++i) {
        int a = faces[i][FACE_V1];
        int b = faces[i][FACE_V2];
        int c = faces[i][FACE_V3];
        Vector3 v1 = rotate_y(make_vector3(vertices[a][0], vertices[a][1], vertices[a][2]), teapot_angle);
        Vector3 v2 = rotate_y(make_vector3(vertices[b][0], vertices[b][1], vertices[b][2]), teapot_angle);
        Vector3 v3 = rotate_y(make_vector3(vertices[c][0], vertices[c][1], vertices[c][2]), teapot_angle);
        v1 = vector3_add(v1, teapot_pos);
        v2 = vector3_add(v2, teapot_pos);
        v3 = vector3_add(v3, teapot_pos);
        Color c1 = *(Color*)&FOREGROUND_COLOR;
        Color c2 = *(Color*)&FOREGROUND_COLOR;
        Color c3 = *(Color*)&FOREGROUND_COLOR;
        // Color c1 = { .r =255, .a = 255 };
        // Color c2 = { .g =255, .a = 255 };
        // Color c3 = { .b =255, .a = 255 };

        renderer_push_triangle(v1, v2, v3, c1, c2, c3);
    }
}

static stb_vorbis *ogg = NULL;

Game game_init(void)
{
    ogg = stb_vorbis_open_filename("assets/sounds/blast.ogg", NULL, NULL);
    assert(ogg);
    assert(ogg->channels == AUDIO_CHANNELS);
    assert(ogg->sample_rate == AUDIO_SAMPLE_RATE);
    stb_vorbis_seek_start(ogg);

    return (Game) {
        .target_fps        = TARGET_FPS,
        .display           = (uint32_t*)display,
        .display_width     = DISPLAY_WIDTH,
        .display_height    = DISPLAY_HEIGHT,
        .audio             = audio,
        .audio_sample_rate = AUDIO_SAMPLE_RATE,
        .audio_channels    = AUDIO_CHANNELS,
        .title             = "The Game",
        .controls          = &controls,
    };
}

static float angle = 0;

void game_update(void)
{
    // Audio
    memset(audio, 0, sizeof(audio));
    stb_vorbis_get_samples_short_interleaved(ogg, AUDIO_CHANNELS, audio, AUDIO_CAPACITY);

    // Display
    Vector3 teapot_pos = {0, 0, 0};
    renderer_reset();
    {
        push_utah_teapot(teapot_pos, angle);

        size_t vet_count = 3;
        float mag = 1;
        float height = -1.0;
        float dangle = 2*M_PI/vet_count;
        Vector3 p1 = make_vector3(mag*cosf(dangle*0), height, mag*sinf(dangle*0));
        Vector3 p2 = make_vector3(mag*cosf(dangle*1), height, mag*sinf(dangle*1));
        Vector3 p3 = make_vector3(mag*cosf(dangle*2), height, mag*sinf(dangle*2));
        Color c1 = { .r =255, .a = 255 };
        Color c2 = { .g =255, .a = 255 };
        Color c3 = { .b =255, .a = 255 };
        renderer_push_triangle(p1, p2, p3, c1, c2, c3);
    }
    renderer_flush();

    Vector3 forward = make_vector3(sinf(renderer.cam_yaw), 0.0f, cosf(renderer.cam_yaw));
    Vector3 world_up = make_vector3(0.0f, 1.0f, 0.0f);
    Vector3 right = vector3_norm(vector3_cross(forward, world_up));
    Vector3 up = vector3_cross(right, forward);

    Vector3 cam_vel = {0, 0, 0};
    for (size_t i = 0; i < ARRAY_LEN(cam_ctrl); ++i) {
        if (controls.keyboard[cam_ctrl[i].key]) {
            cam_vel = vector3_add(cam_vel, cam_ctrl[i].vector);
            renderer.cam_yaw += cam_ctrl[i].angle_yaw * DELTA_TIME * 2;
            renderer.cam_pit += cam_ctrl[i].angle_pit * DELTA_TIME; 
        }
    }

#define MOUSE_SENSITIVITY 0.003f
    renderer.cam_yaw += controls.mouse_dx * MOUSE_SENSITIVITY;
    renderer.cam_pit += controls.mouse_dy * MOUSE_SENSITIVITY;

    const float max_pitch = 89.9*M_PI/180;
    if (renderer.cam_pit > max_pitch) renderer.cam_pit = max_pitch;
    if (renderer.cam_pit < -max_pitch) renderer.cam_pit = -max_pitch;

    controls.mouse_dx = 0;
    controls.mouse_dy = 0;


    Vector3 world_vel = vector3_add(
        vector3_add(
            vector3_scale(right, cam_vel.x),
            vector3_scale(up, cam_vel.y)
        ),
        vector3_scale(forward, cam_vel.z)
    );

    float speed = 3.0f;
    renderer.cam_pos = vector3_add(renderer.cam_pos,
        vector3_scale(world_vel, speed * DELTA_TIME));

    angle += 0.25*M_PI*DELTA_TIME;
}

void game_key_up(int key)
{
    if (key >= 0 && key < 65536) {
        controls.keyboard[key] = false;
    }
}

void game_key_down(int key)
{
    if (key >= 0 && key < 65536) {
        controls.keyboard[key] = true;
    }
}
