#include <assert.h>
#include <math.h>
#include <X11/keysym.h>
#include <stddef.h>
#include <stdint.h>
// #include "X11/Xlib.h"
#include "nob.h"
#undef temp_alloc
#include "vec.h"
#define OLIVEC_IMPLEMENTATION
#include "olive.c"
#include "game.h"
#include "stb_vorbis.c"
#include "stb_truetype.h"
#include "models/utahTeapot.h"
// #include "models/amiya1.h"
#include "fonts/JetBrainsMonoNerdFont_Regular.h"

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
#define FAR_CLIP 10.0f

#define COLOR_RED        ((Color) { .r = 0xFF, .g = 0xAA, .b = 0xAA, .a = 0xFF })
#define COLOR_GREEN      ((Color) { .r = 0xAA, .g = 0xFF, .b = 0xAA, .a = 0xFF })
#define COLOR_BLUE       ((Color) { .r = 0xAA, .g = 0xAA, .b = 0xFF, .a = 0xFF })
#define COLOR_PURPLE     ((Color) { .r = 0xFF, .g = 0xAA, .b = 0xFF, .a = 0xFF })
#define COLOR_CYAN       ((Color) { .r = 0xFF, .g = 0xFF, .b = 0xAA, .a = 0xFF })
#define COLOR_BACKGROUND ((Color) { .r = 0xFF, .g = 0xFF, .b = 0xAA, .a = 0xFF })
#define COLOR_FOREGROUND ((Color) { .r = 0xAA, .g = 0xAA, .b = 0xFF, .a = 0xFF })

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
    .cam_pos = {0.0f, 1.0f, -2.0f},
    .cam_yaw = 0.0f,
    .cam_pit = 0.0f,
};

static inline void renderer_begin(void)
{
    renderer.vertices.count = 0;
    renderer.colors.count = 0;

    for (size_t i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; ++i) {
        rzbuffer[i] = 0;
        display[i] = COLOR_BACKGROUND;
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

static inline void renderer_push_triangle(Vector3 v1, Vector3 v2, Vector3 v3, Color c1, Color c2, Color c3)
{
    Vector3 v[3] = { v1, v2, v3 };
    Color   c[3] = { c1, c2, c3 };

    Vector3 face_normal = vector3_norm(vector3_cross(vector3_sub(v2, v1), vector3_sub(v3, v1)));

    Vector3 center = make_vector3(
            (v1.x + v2.x + v3.x) / 3.0f,
            (v1.y + v2.y + v3.y) / 3.0f,
            (v1.z + v2.z + v3.z) / 3.0f
            );

    Vector3 v1_to_camera = vector3_norm(vector3_sub(renderer.cam_pos, v1));
    Vector3 v2_to_camera = vector3_norm(vector3_sub(renderer.cam_pos, v2));
    Vector3 v3_to_camera = vector3_norm(vector3_sub(renderer.cam_pos, v3));

    if (vector3_dot(face_normal, v1_to_camera) < 0 &&
            vector3_dot(face_normal, v2_to_camera) < 0 &&
            vector3_dot(face_normal, v3_to_camera) < 0 ) return;

    Vector3 world_up = make_vector3(0.0f, 1.0f, 0.0f);
    Vector3 forward = make_vector3(cosf(renderer.cam_pit)*sinf(renderer.cam_yaw), sinf(renderer.cam_pit), cosf(renderer.cam_pit)*cosf(renderer.cam_yaw));
    Vector3 right = vector3_norm(vector3_cross(forward, world_up));
    Vector3 up = vector3_cross(right, forward);

    int clip[3] = {0};
    int clip_n = 0;
    int unclip[3] = {0};
    int unclip_n = 0;

    for (int i = 0; i < 3; ++i) {
        v[i] = vector3_sub(v[i], renderer.cam_pos);
        v[i] = make_vector3(vector3_dot(v[i], right), vector3_dot(v[i], up), vector3_dot(v[i], forward));
        v[i].x *= DISPLAY_ASPECT;

        if (v[i].z < NEAR_CLIP) {
            clip[clip_n++] = i;
        } else {
            unclip[unclip_n++] = i;
        }
    }

    switch (clip_n) {
        case 0:
            {
                for (size_t i = 0; i < 3; ++i) {
                    renderer_push_vertex(v[i], c[i]);
                }
                break;
            }
        case 1:
            {
                float t;
                Vector3 q1 = clip_line(v[clip[0]], v[unclip[0]], NEAR_CLIP, &t);
                Color cq1 = color_lerp(c[clip[0]], c[unclip[0]], t);
                Vector3 q2 = clip_line(v[clip[0]], v[unclip[1]], NEAR_CLIP, &t);
                Color cq2 = color_lerp(c[clip[0]], c[unclip[1]], t);
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
                float t;
                Vector3 q1 = clip_line(v[clip[0]], v[unclip[0]], NEAR_CLIP, &t);
                Color cq1 = color_lerp(c[clip[0]], c[unclip[0]], t);
                Vector3 q2 = clip_line(v[clip[1]], v[unclip[0]], NEAR_CLIP, &t);
                Color cq2 = color_lerp(c[clip[1]], c[unclip[0]], t);
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

static void renderer_push_cube(Vector3 cube_pos, float width, float depth, float height, Color cube_color)
{
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float hd = depth * 0.5f;

    Vector3 vs[8] = {
        {-hw, -hh, -hd}, // 0
        { hw, -hh, -hd}, // 1
        { hw, -hh,  hd}, // 2
        {-hw, -hh,  hd}, // 3
        {-hw,  hh, -hd}, // 4
        { hw,  hh, -hd}, // 5
        { hw,  hh,  hd}, // 6
        {-hw,  hh,  hd}  // 7
    };

    for (int i = 0; i < 8; i++) {
        vs[i] = vector3_add(vs[i], cube_pos);
    }

    int fs[6][4] = {
        {0,1,2,3}, // D (y = -hh)
        {4,7,6,5}, // U (y =  hh)
        {3,2,6,7}, // F (z =  hd)
        {0,4,5,1}, // B (z = -hd)
        {0,3,7,4}, // L (x = -hw)
        {1,5,6,2}  // R (x =  hw)
    };

    for (int f = 0; f < 6; f++) {
        int i0     = fs[f][0];
        int i1     = fs[f][1];
        int i2     = fs[f][2];
        int i3     = fs[f][3];
        Vector3 v0 = vs[i0];
        Vector3 v1 = vs[i1];
        Vector3 v2 = vs[i2];
        Vector3 v3 = vs[i3];

        renderer_push_triangle(v0, v1, v2, cube_color, cube_color, cube_color);
        renderer_push_triangle(v0, v2, v3, cube_color, cube_color, cube_color);
    }
}

static void renderer_end(void)
{
    for (size_t i = 0; i + 3 <= renderer.vertices.count; i += 3) {
        Vector3 v1 = renderer.vertices.items[i + 0];
        Vector3 v2 = renderer.vertices.items[i + 1];
        Vector3 v3 = renderer.vertices.items[i + 2];
        Color c1 = renderer.colors.items[i + 0];
        Color c2 = renderer.colors.items[i + 1];
        Color c3 = renderer.colors.items[i + 2];

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
                                    uint32_t fog_r = OLIVEC_RED(*(uint32_t*)&COLOR_BACKGROUND);
                                    uint32_t fog_g = OLIVEC_GREEN(*(uint32_t*)&COLOR_BACKGROUND);
                                    uint32_t fog_b = OLIVEC_BLUE(*(uint32_t*)&COLOR_BACKGROUND);
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
}

static void renderer_push_utahteapot(Vector3 teapot_pos, float teapot_rot, float teapot_scale)
{
    for (size_t i = 0; i < utahteapot_faces_count; ++i) {
        int a = utahteapot_faces[i][FACE_V1];
        int b = utahteapot_faces[i][FACE_V2];
        int c = utahteapot_faces[i][FACE_V3];
        Vector3 v1 = rotate_y(make_vector3(utahteapot_vertices[a][0], utahteapot_vertices[a][1], utahteapot_vertices[a][2]), teapot_rot);
        Vector3 v2 = rotate_y(make_vector3(utahteapot_vertices[b][0], utahteapot_vertices[b][1], utahteapot_vertices[b][2]), teapot_rot);
        Vector3 v3 = rotate_y(make_vector3(utahteapot_vertices[c][0], utahteapot_vertices[c][1], utahteapot_vertices[c][2]), teapot_rot);
        v1 = vector3_scale(v1, teapot_scale);
        v2 = vector3_scale(v2, teapot_scale);
        v3 = vector3_scale(v3, teapot_scale);
        v1 = vector3_add(v1, teapot_pos);
        v2 = vector3_add(v2, teapot_pos);
        v3 = vector3_add(v3, teapot_pos);
        // Color c1 = { .r = 100, .g = 100, .b = 100, .a = 255 };
        // Color c2 = { .r = 100, .g = 100, .b = 100, .a = 255 };
        // Color c3 = { .r = 100, .g = 100, .b = 100, .a = 255 };
        Color c1 = { .r = 176, .g = 176, .b = 255, .a = 255 };
        Color c2 = { .r = 176, .g = 176, .b = 255, .a = 255 };
        Color c3 = { .r = 176, .g = 176, .b = 255, .a = 255 };

        renderer_push_triangle(v1, v2, v3, c1, c2, c3);
    }
}

// static void renderer_push_amiya(Vector3 amiya_pos, float amiya_rot, float amiya_scale)
// {
//     for (size_t i = 0; i < amiya1_faces_count; ++i) {
//         int a = amiya1_faces[i][FACE_V1];
//         int b = amiya1_faces[i][FACE_V2];
//         int c = amiya1_faces[i][FACE_V3];
//         Vector3 v1 = rotate_y(make_vector3(amiya1_vertices[a][0], amiya1_vertices[a][1], amiya1_vertices[a][2]), amiya_rot);
//         Vector3 v2 = rotate_y(make_vector3(amiya1_vertices[b][0], amiya1_vertices[b][1], amiya1_vertices[b][2]), amiya_rot);
//         Vector3 v3 = rotate_y(make_vector3(amiya1_vertices[c][0], amiya1_vertices[c][1], amiya1_vertices[c][2]), amiya_rot);
//         v1 = vector3_scale(v1, amiya_scale);
//         v2 = vector3_scale(v2, amiya_scale);
//         v3 = vector3_scale(v3, amiya_scale);
//         v1 = vector3_add(v1, amiya_pos);
//         v2 = vector3_add(v2, amiya_pos);
//         v3 = vector3_add(v3, amiya_pos);
//         // Color c1 = { .r = 100, .g = 100, .b = 100, .a = 255 };
//         // Color c2 = { .r = 100, .g = 100, .b = 100, .a = 255 };
//         // Color c3 = { .r = 100, .g = 100, .b = 100, .a = 255 };
//         Color c1 = { .r = 176, .g = 176, .b = 255, .a = 255 };
//         Color c2 = { .r = 176, .g = 176, .b = 255, .a = 255 };
//         Color c3 = { .r = 176, .g = 176, .b = 255, .a = 255 };
//
//         renderer_push_triangle(v1, v2, v3, c1, c2, c3);
//     }
// }

static void renderer_push_floor(float floor_height, float grid_size)
{
    int grid_count = 10;
    for (int i = -grid_count; i < grid_count; ++i) {
        for (int j = -grid_count; j < grid_count; ++j) {
            Color c;
            if ((i+j)%2){
                c.r = 0x18; c.g = 0x18; c.b = 0x18; c.a = 0xFF;
            } else {
                c.r = 0xE7; c.g = 0xE7; c.b = 0xE7; c.a = 0xFF;
            }
            if ((i-j+5)%10 == 0) {
                float height = 10;
                renderer_push_cube(make_vector3((i+0.5)*grid_size, floor_height + height/2, (j+0.5)*grid_size), 1, 1, height, c);
            } else {
                Vector3 p1 = make_vector3(    i * grid_size, floor_height,     j * grid_size);
                Vector3 p2 = make_vector3(    i * grid_size, floor_height, (j+1) * grid_size);
                Vector3 p3 = make_vector3((i+1) * grid_size, floor_height, (j+1) * grid_size);
                Vector3 p4 = make_vector3((i+1) * grid_size, floor_height,     j * grid_size);
                renderer_push_triangle(p1, p2, p3, c, c, c);
                renderer_push_triangle(p1, p3, p4, c, c, c);
            }
        }
    }
}

void game_jetbrainsmono_text(const char *message, float pen_x, float pen_y, Color color)
{
    int n = strlen(message);
    for (int i = 0; i < n; ++i) {
        int index = message[i] - JetBrainsMonoNerdFont_Regular_first_char;
        stbtt_bakedchar cdata = JetBrainsMonoNerdFont_Regular_cdata[index];
        for (int dy = cdata.y0; dy < cdata.y1; ++dy) {
            for (int dx = cdata.x0; dx < cdata.x1; ++dx) {
                unsigned char intensity = JetBrainsMonoNerdFont_Regular_pixels[dy*JetBrainsMonoNerdFont_Regular_width + dx];
                int x = pen_x + dx - cdata.x0 + cdata.xoff;
                int y = pen_y + dy - cdata.y0 + cdata.yoff;
                if (0 <= x && x < DISPLAY_WIDTH && 0 <= y && y < DISPLAY_HEIGHT) {
                    int j = y*DISPLAY_WIDTH + x;
                    Color color_b = display[j];
                    color.a = intensity;

                    Color color_f = {0};

                    color_f.a = color.a + color_b.a*(255 - color.a)/255;
                    if (color_f.a == 0) {
                        display[j] = (Color) {0};
                        continue;
                    }

                    uint32_t sum;
                    sum = (uint32_t)color.r*color.a*255 + (uint32_t)color_b.r*color_b.a*(255 - color.a);
                    color_f.r = (uint8_t)(sum/ (255*color_f.a));
                    sum = (uint32_t)color.g*color.a*255 + (uint32_t)color_b.g*color_b.a*(255 - color.a);
                    color_f.g = (uint8_t)(sum/ (255*color_f.a));
                    sum = (uint32_t)color.b*color.a*255 + (uint32_t)color_b.b*color_b.a*(255 - color.a);
                    color_f.b = (uint8_t)(sum/ (255*color_f.a));

                    display[j] = color_f;
                }
            }
        }
        pen_x += cdata.xadvance;
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
    uint64_t perf_begin = nanos_since_unspecified_epoch();

    // Logic
    Vector3 world_forward = make_vector3(sinf(renderer.cam_yaw), 0, cosf(renderer.cam_yaw));
    Vector3 world_up = make_vector3(0.0f, 1.0f, 0.0f);
    Vector3 forward = make_vector3(cosf(renderer.cam_pit)*sinf(renderer.cam_yaw), sinf(renderer.cam_pit), cosf(renderer.cam_pit)*cosf(renderer.cam_yaw));
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

    Vector3 world_vel = vector3_add(
            vector3_add(
                vector3_scale(right, cam_vel.x),
                vector3_scale(world_up, cam_vel.y)),
            vector3_scale(world_forward, cam_vel.z));

    float speed = 5.0f;
    renderer.cam_pos = vector3_add(renderer.cam_pos, vector3_scale(world_vel, speed * DELTA_TIME));

    const float max_pitch = 89.9*M_PI/180;
    if (renderer.cam_pit > max_pitch) renderer.cam_pit = max_pitch;
    if (renderer.cam_pit < -max_pitch) renderer.cam_pit = -max_pitch;

    // #define MOUSE_SENSITIVITY 0.003f
    //     renderer.cam_yaw += controls.mouse_dx * MOUSE_SENSITIVITY;
    //     renderer.cam_pit += controls.mouse_dy * MOUSE_SENSITIVITY;
    //
    //
    //     controls.mouse_dx = 0;
    //     controls.mouse_dy = 0;

    // Audio
    memset(audio, 0, sizeof(audio));
    stb_vorbis_get_samples_short_interleaved(ogg, AUDIO_CHANNELS, audio, AUDIO_CAPACITY);

    // Display
    renderer_begin();
    {
        // Model
        // utahTeapot
        Vector3 teapot_pos = {0, 1, 0};
        float teapot_angle = angle;
        float teapot_scale = 0.5f;
        renderer_push_utahteapot(teapot_pos, teapot_angle, teapot_scale);

        // amiya1
        // Vector3 amiya_pos = {0, 1, 0};
        // float amiya_angle = angle;
        // float amiya_scale = 0.1f;
        // renderer_push_amiya(amiya_pos, amiya_angle, amiya_scale);

        // Floor
        float floor_height = 0.0f;
        renderer_push_floor(floor_height, 1);
    }
    renderer_end();

    angle += 0.25*M_PI*DELTA_TIME;

    // FPS computing
    uint64_t perf_end = nanos_since_unspecified_epoch();
    uint64_t delta = perf_end - perf_begin;
    int fps = 1.0/((double)delta/NANOS_PER_SEC);
    game_jetbrainsmono_text(temp_sprintf("FPS: %3d, RES: %d x %d", fps, DISPLAY_WIDTH, DISPLAY_HEIGHT), 10, 50, COLOR_FOREGROUND);
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
