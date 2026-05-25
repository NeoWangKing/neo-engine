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
#include "models/cube.h"
#include "models/utahTeapot.h"
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

#define GAME_SCREEN_MENU  0
#define GAME_SCREEN_MAIN  1
#define GAME_SCREEN_PAUSE 2

#define FOG_FADE_SPEED 2.0f
#define FOG_DISTANCE_MAX FAR_CLIP

static int game_screen = GAME_SCREEN_MENU;
static Controls controls = {0};
static stb_vorbis *ogg = NULL;
static float stand_h = 1.6f;
static float squat_h = 1.0f;
static float cam_height = 2;
static int mouse_x = DISPLAY_WIDTH/2;
static int mouse_y = DISPLAY_HEIGHT/2;
static float fog_distance = 0;
static float fog_fader = 0;
static Vector3 sun = {-1, -2, 0};
static bool toggle_fog = true;
static bool toggle_back_cult = true;
static bool toggle_sun = true;

// key pressed
static bool key_1_was_pressed      = false;
static bool key_2_was_pressed      = false;
static bool key_3_was_pressed      = false;
static bool key_space_was_pressed  = false;
static bool key_escape_was_pressed = false;
static bool key_return_was_pressed = false;

static float angle = 0;

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
    Vector3s vertices;
    Colors   colors;
    Vector3  cam_pos;
    Vector3  cam_vel;
    float    cam_yaw;
    float    cam_pit;
} renderer = {
    .cam_pos = {0.0f, 2.0f, -2.0f},
    .cam_vel = {0.0f, 0.0f, 0.0f},
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
    Vector3 center = make_vector3( (v1.x + v2.x + v3.x) / 3.0f, (v1.y + v2.y + v3.y) / 3.0f, (v1.z + v2.z + v3.z) / 3.0f);
    Vector3 center_to_camera = vector3_norm(vector3_sub(renderer.cam_pos, center));
    if (toggle_back_cult && vector3_dot(face_normal, center_to_camera) < 0) return;

    float bright = (1-vector3_dot(face_normal, vector3_norm(sun)))/2;

    Vector3 world_up = make_vector3(0.0f, 1.0f, 0.0f);
    Vector3 forward = make_vector3(cosf(renderer.cam_pit)*sinf(renderer.cam_yaw), sinf(renderer.cam_pit), cosf(renderer.cam_pit)*cosf(renderer.cam_yaw));
    Vector3 right = vector3_norm(vector3_cross(forward, world_up));
    Vector3 up = vector3_cross(right, forward);

    int clip[3] = {0};
    int clip_n = 0;
    int unclip[3] = {0};
    int unclip_n = 0;

    for (int i = 0; i < 3; ++i) {
        if (toggle_sun) {
            c[i].r *= bright; c[i].g *= bright; c[i].b *= bright;
        }
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
                            if (toggle_fog) {
                                float z = 1.0f / rz;
                                float t = 1.0f;
                                if (fog_distance > NEAR_CLIP) t= (z - NEAR_CLIP) / (fog_distance - NEAR_CLIP);
                                if (t < 0.0f) t = 0.0f;
                                if (t > 1.0f) t = 1.0f;
                                // t *= fog_fader;
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

static void renderer_push_cube(Vector3 cube_pos, float width, float depth, float height, float cube_rot_xz, Color cube_color)
{
    for (size_t i = 0; i < cube_faces_count; ++i) {
        int a = cube_faces[i][FACE_V1];
        int b = cube_faces[i][FACE_V2];
        int c = cube_faces[i][FACE_V3];
        Vector3 v1 = make_vector3(cube_vertices[a][0], cube_vertices[a][1], cube_vertices[a][2]);
        Vector3 v2 = make_vector3(cube_vertices[b][0], cube_vertices[b][1], cube_vertices[b][2]);
        Vector3 v3 = make_vector3(cube_vertices[c][0], cube_vertices[c][1], cube_vertices[c][2]);
        v1.x *= width; v2.x *= width; v3.x *= width;
        v1.y *= height; v2.y *= height; v3.y *= height;
        v1.z *= depth; v2.z *= depth; v3.z *= depth;
        v1 = rotate_y(v1, cube_rot_xz);
        v2 = rotate_y(v2, cube_rot_xz);
        v3 = rotate_y(v3, cube_rot_xz);
        v1 = vector3_add(v1, cube_pos);
        v2 = vector3_add(v2, cube_pos);
        v3 = vector3_add(v3, cube_pos);

        renderer_push_triangle(v1, v2, v3, cube_color, cube_color, cube_color);
    }
}

static void renderer_push_utahteapot(Vector3 teapot_pos, float teapot_rot_xz, float teapot_scale)
{
    for (size_t i = 0; i < utahteapot_faces_count; ++i) {
        int a = utahteapot_faces[i][FACE_V1];
        int b = utahteapot_faces[i][FACE_V2];
        int c = utahteapot_faces[i][FACE_V3];
        Vector3 v1 = rotate_y(make_vector3(utahteapot_vertices[a][0], utahteapot_vertices[a][1], utahteapot_vertices[a][2]), teapot_rot_xz);
        Vector3 v2 = rotate_y(make_vector3(utahteapot_vertices[b][0], utahteapot_vertices[b][1], utahteapot_vertices[b][2]), teapot_rot_xz);
        Vector3 v3 = rotate_y(make_vector3(utahteapot_vertices[c][0], utahteapot_vertices[c][1], utahteapot_vertices[c][2]), teapot_rot_xz);
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
                renderer_push_cube(make_vector3((i+0.5)*grid_size, floor_height + height/2, (j+0.5)*grid_size), 1, 1, height, 0, c);
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

void game_jetbrainsmono_text(const char *message, float pen_x, float pen_y, float font_scale, Color color)
{
    font_scale = font_scale * 0.01;
    int n = strlen(message);
    for (int i = 0; i < n; ++i) {
        int index = message[i] - JetBrainsMonoNerdFont_Regular_first_char;
        stbtt_bakedchar cdata = JetBrainsMonoNerdFont_Regular_cdata[index];

        int scaled_x0 = (int)(cdata.x0 * font_scale);
        int scaled_y0 = (int)(cdata.y0 * font_scale);
        int scaled_x1 = (int)(cdata.x1 * font_scale);
        int scaled_y1 = (int)(cdata.y1 * font_scale);
        int scaled_xoff = (int)(cdata.xoff * font_scale);
        int scaled_yoff = (int)(cdata.yoff * font_scale);
        float scaled_xadvance = cdata.xadvance * font_scale;

        int orig_width = cdata.x1 - cdata.x0;
        int orig_height = cdata.y1 - cdata.y0;

        for (int dy = scaled_y0; dy < scaled_y1; ++dy) {
            int src_y = cdata.y0 + (int)((float)(dy - scaled_y0) / font_scale);
            if (src_y < cdata.y0) src_y = cdata.y0;
            if (src_y >= cdata.y1) src_y = cdata.y1 - 1;

            for (int dx = scaled_x0; dx < scaled_x1; ++dx) {
                int src_x = cdata.x0 + (int)((float)(dx - scaled_x0) / font_scale);
                if (src_x < cdata.x0) src_x = cdata.x0;
                if (src_x >= cdata.x1) src_x = cdata.x1 - 1;

                unsigned char intensity = JetBrainsMonoNerdFont_Regular_pixels[src_y * JetBrainsMonoNerdFont_Regular_width + src_x];
                int x = pen_x + dx - scaled_x0 + scaled_xoff;
                int y = pen_y + dy - scaled_y0 + scaled_yoff;

                if (0 <= x && x < DISPLAY_WIDTH && 0 <= y && y < DISPLAY_HEIGHT) {
                    int j = y * DISPLAY_WIDTH + x;
                    Color color_b = display[j];
                    Color color_f = color;
                    color_f.a = intensity;

                    Color blended = {0};
                    blended.a = color_f.a + color_b.a * (255 - color_f.a) / 255;
                    if (blended.a == 0) {
                        display[j] = (Color){0};
                        continue;
                    }
                    uint32_t sum;
                    sum = (uint32_t)color_f.r * color_f.a * 255 + (uint32_t)color_b.r * color_b.a * (255 - color_f.a);
                    blended.r = (uint8_t)(sum / (255 * blended.a));
                    sum = (uint32_t)color_f.g * color_f.a * 255 + (uint32_t)color_b.g * color_b.a * (255 - color_f.a);
                    blended.g = (uint8_t)(sum / (255 * blended.a));
                    sum = (uint32_t)color_f.b * color_f.a * 255 + (uint32_t)color_b.b * color_b.a * (255 - color_f.a);
                    blended.b = (uint8_t)(sum / (255 * blended.a));

                    display[j] = blended;
                }
            }
        }
        pen_x += scaled_xadvance;
    }
}

void game_jetbrainsmono_text_center(const char *message, float center_x, float pen_y, float font_scale, Color color)
{
    float total_width = 0;
    for (const char *p = message; *p; ++p) {
        int idx = *p - JetBrainsMonoNerdFont_Regular_first_char;
        if (idx >= 0 && idx < 95) {
            stbtt_bakedchar cdata = JetBrainsMonoNerdFont_Regular_cdata[idx];
            total_width += cdata.xadvance * font_scale * 0.01;
        }
    }
    float start_x = center_x - total_width * 0.5f;
    game_jetbrainsmono_text(message, start_x, pen_y, font_scale, color);
}

void renderer_main_scene(void)
{
    renderer_begin();
    {
        // utahTeapot
        Vector3 teapot_pos = {0, 1, 0};
        float teapot_angle_xz = angle;
        float teapot_scale = 0.5f;
        renderer_push_utahteapot(teapot_pos, teapot_angle_xz, teapot_scale);

        // Floor
        float floor_height = 0.0f;
        renderer_push_floor(floor_height, 1);
    }
    renderer_end();
}

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

void game_main(void)
{
    // Logic
    float cam_h_acc = 25.0f;
    float cam_ground_f = 10.0f;
    float cam_ground_vf = 0.5f;
    float cam_air_vf = 0.0f;
    float gravity = -9.8f;
    float jump_vel = 5.0f;

    Vector3 cam_forward = { sinf(renderer.cam_yaw), 0, cosf(renderer.cam_yaw) };
    Vector3 cam_right   = {-cosf(renderer.cam_yaw), 0, sinf(renderer.cam_yaw) };
    
    bool on_ground = (renderer.cam_pos.y <= cam_height);
    if (controls.keyboard['c']) {
        cam_height = squat_h;
    } else {
        cam_height = stand_h;
    }

    if (on_ground) {
        renderer.cam_pos.y = cam_height;
    }

    Vector3 move_h_acc = {0,0,0};
    if (controls.keyboard['w']) move_h_acc = vector3_add(move_h_acc, cam_forward);
    if (controls.keyboard['s']) move_h_acc = vector3_sub(move_h_acc, cam_forward);
    if (controls.keyboard['a']) move_h_acc = vector3_sub(move_h_acc, cam_right);
    if (controls.keyboard['d']) move_h_acc = vector3_add(move_h_acc, cam_right);
    if (vector3_len(move_h_acc) > 0) move_h_acc = vector3_norm(move_h_acc);
    move_h_acc = on_ground ? vector3_scale(move_h_acc, cam_h_acc) : vector3_scale(move_h_acc, cam_h_acc*0.1);
    float vel_xz = sqrtf(renderer.cam_vel.x*renderer.cam_vel.x + renderer.cam_vel.z*renderer.cam_vel.z);
    float vf = on_ground ? cam_ground_vf : cam_air_vf;
    move_h_acc.x -= vf * renderer.cam_vel.x * vel_xz;
    move_h_acc.z -= vf * renderer.cam_vel.z * vel_xz;
    float f = on_ground ? cam_ground_f : 0;
    if (vel_xz > 1e-6f) {
        move_h_acc.x -= f * renderer.cam_vel.x / vel_xz;
        move_h_acc.z -= f * renderer.cam_vel.z / vel_xz;
    }

    renderer.cam_vel.x += move_h_acc.x * DELTA_TIME;
    renderer.cam_vel.z += move_h_acc.z * DELTA_TIME;
    renderer.cam_vel.y += gravity * DELTA_TIME;

    if (controls.keyboard[' '] && on_ground && !key_space_was_pressed) {
        renderer.cam_vel.y = jump_vel;
        key_space_was_pressed = true;
    } else if (!controls.keyboard[' ']) {
        key_space_was_pressed = false;
    }

    renderer.cam_pos.x += renderer.cam_vel.x * DELTA_TIME;
    renderer.cam_pos.y += renderer.cam_vel.y * DELTA_TIME;
    renderer.cam_pos.z += renderer.cam_vel.z * DELTA_TIME;

    if (vel_xz < 0.1 &&
            !controls.keyboard['w'] &&
            !controls.keyboard['s'] &&
            !controls.keyboard['a'] &&
            !controls.keyboard['d']) {
        renderer.cam_vel.x = 0;
        renderer.cam_vel.z = 0;
        vector3_scale(move_h_acc, 0);
    }

    if (renderer.cam_pos.y < cam_height) {
        renderer.cam_pos.y = cam_height;
        if (renderer.cam_vel.y < 0) renderer.cam_vel.y = 0;
    }

    if (controls.keyboard['q']) renderer.cam_yaw += 1.0f * DELTA_TIME;
    if (controls.keyboard['e']) renderer.cam_yaw -= 1.0f * DELTA_TIME;
    if (controls.keyboard['r']) renderer.cam_pit += 1.0f * DELTA_TIME;
    if (controls.keyboard['f']) renderer.cam_pit -= 1.0f * DELTA_TIME;

    const float MAX_PITCH = 89.9f * M_PI / 180.0f;
    if (renderer.cam_pit >  MAX_PITCH) renderer.cam_pit =  MAX_PITCH;
    if (renderer.cam_pit < -MAX_PITCH) renderer.cam_pit = -MAX_PITCH;

    // renderer.cam_yaw   += controls.mouse_dx * 0.003f;
    // renderer.cam_pit   += controls.mouse_dy * 0.003f;
    controls.mouse_dx = 0;
    controls.mouse_dy = 0;

    // Toggle Fog
    if (controls.keyboard['1'] && !key_1_was_pressed) {
        toggle_fog = !toggle_fog;
        key_1_was_pressed = true;
    } else if (!controls.keyboard['1']) {
        key_1_was_pressed = false;
    }
    
    // Toggle Back Culting
    if (controls.keyboard['2'] && !key_2_was_pressed) {
        toggle_back_cult = !toggle_back_cult;
        key_2_was_pressed = true;
    } else if (!controls.keyboard['2']) {
        key_2_was_pressed = false;
    }

    // Toggle Sun
    if (controls.keyboard['3'] && !key_3_was_pressed) {
        toggle_sun = !toggle_sun;
        key_3_was_pressed = true;
    } else if (!controls.keyboard['3']) {
        key_3_was_pressed = false;
    }

    if (controls.keyboard[XK_Escape] && !key_escape_was_pressed) {
        key_escape_was_pressed = true;
        game_screen = GAME_SCREEN_PAUSE;
    } else if (!controls.keyboard[XK_Escape]) {
        key_escape_was_pressed = false;
    }

    if (fog_fader < 1.0f) {
        fog_fader += FOG_FADE_SPEED*DELTA_TIME;
        if (fog_fader > 1.0f) fog_fader = 1.0f;
        fog_distance = fog_fader*fog_fader*FOG_DISTANCE_MAX;
    }
    renderer_main_scene();

    game_jetbrainsmono_text(temp_sprintf("Pos: %5.2f,%5.2f,%5.2f", renderer.cam_pos.x, renderer.cam_pos.y-cam_height, renderer.cam_pos.z), 10, 50, 24, COLOR_FOREGROUND);
    game_jetbrainsmono_text(temp_sprintf("Vel: %5.2f,%5.2f,%5.2f", renderer.cam_vel.x, renderer.cam_vel.y, renderer.cam_vel.z), 10, 75, 24, COLOR_FOREGROUND);
    game_jetbrainsmono_text(temp_sprintf("Vxz: %5.2f", vel_xz), 10, 100, 24, COLOR_FOREGROUND);
}

void game_menu(void)
{
    for (int i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; ++i) {
        display[i] = COLOR_BACKGROUND;
    }

    if (fog_fader > 0) {
        fog_fader -= FOG_FADE_SPEED*DELTA_TIME;
        if (fog_fader < 0.0f) fog_fader = 0.0f;
        fog_distance = fog_fader*fog_fader*FOG_DISTANCE_MAX;
        renderer_main_scene();
    }

    game_jetbrainsmono_text_center("MENU", (float)DISPLAY_WIDTH/2, (float)DISPLAY_HEIGHT/4, 100, COLOR_FOREGROUND);
    game_jetbrainsmono_text_center("Press <ENTER> to start", (float)DISPLAY_WIDTH/2, (float)DISPLAY_HEIGHT/2, 48, COLOR_FOREGROUND);
    game_jetbrainsmono_text_center("Press <ESC> to exit", (float)DISPLAY_WIDTH/2, (float)DISPLAY_HEIGHT/2 + 100, 48, COLOR_FOREGROUND);

    if (controls.keyboard[XK_Return] && !key_return_was_pressed) {
        game_screen = GAME_SCREEN_MAIN;
        key_return_was_pressed = true;
    } else if (!controls.keyboard[XK_Return]) {
        key_return_was_pressed = false;
    }

    if (controls.keyboard[XK_Escape] && !key_escape_was_pressed) {
        exit(0);
        key_escape_was_pressed = true;
    } else if (!controls.keyboard[XK_Escape]) {
        key_escape_was_pressed = false;
    }
}

void game_pause(void)
{
    for (int i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; ++i) {
        display[i] = COLOR_BACKGROUND;
    }

    if (fog_fader > 0) {
        fog_fader -= FOG_FADE_SPEED*DELTA_TIME;
        if (fog_fader < 0.0f) fog_fader = 0.0f;
        fog_distance = fog_fader*fog_fader*FOG_DISTANCE_MAX;
        renderer_main_scene();
    }

    game_jetbrainsmono_text_center("PAUSED", (float)DISPLAY_WIDTH/2, (float)DISPLAY_HEIGHT/4, 100, COLOR_FOREGROUND);
    game_jetbrainsmono_text_center("Press <ESC> to resume", (float)DISPLAY_WIDTH/2, (float)DISPLAY_HEIGHT/2, 48, COLOR_FOREGROUND);
    game_jetbrainsmono_text_center("Press <Q> to menu", (float)DISPLAY_WIDTH/2, (float)DISPLAY_HEIGHT/2 + 100, 48, COLOR_FOREGROUND);

    if (controls.keyboard[XK_Escape] && !key_escape_was_pressed) {
        game_screen = GAME_SCREEN_MAIN;
        key_escape_was_pressed = true;
    } else if (!controls.keyboard[XK_Escape]) {
        key_escape_was_pressed = false;
    }

    if (controls.keyboard['q']) {
        game_screen = GAME_SCREEN_MENU;
    };
}

void game_update(void)
{
    uint64_t perf_begin = nanos_since_unspecified_epoch();

    switch (game_screen) {
        case GAME_SCREEN_MAIN:
            {
                game_main();  break;
            }
        case GAME_SCREEN_MENU:
            {
                game_menu();  break;
            }
        case GAME_SCREEN_PAUSE:
            {
                game_pause(); break;
            }
        default: UNREACHABLE("game_screen");
    }

    // Audio
    memset(audio, 0, sizeof(audio));
    stb_vorbis_get_samples_short_interleaved(ogg, AUDIO_CHANNELS, audio, AUDIO_CAPACITY);

    // FPS computing
    uint64_t perf_end = nanos_since_unspecified_epoch();
    uint64_t delta = perf_end - perf_begin;
    int fps = 1.0/((double)delta/NANOS_PER_SEC);
    game_jetbrainsmono_text(temp_sprintf("FPS: %3d", fps), 10, 25, 24, COLOR_FOREGROUND);

    game_jetbrainsmono_text(temp_sprintf("SCENE: %1d", game_screen), 10, DISPLAY_HEIGHT, 24, COLOR_RED);

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
