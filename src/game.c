#include <assert.h>
#include <math.h>
#include "nob.h"
#undef temp_alloc
#define NVC_AA_RES 1
#include "neovin.c"
#include "game.h"
#include "stb_vorbis.c"

typedef struct {
    uint8_t b, g, r, a;
} Color;

#define TARGET_FPS 60
#define DISPLAY_WIDTH  800
#define DISPLAY_HEIGHT 600
#define DELTA_TIME (1.0f/TARGET_FPS)
static Color display[DISPLAY_WIDTH*DISPLAY_HEIGHT];

#define AUDIO_SAMPLE_RATE 44100
static_assert(AUDIO_SAMPLE_RATE%TARGET_FPS == 0, "Sample rate must be divisible by FPS");
#define AUDIO_CHANNELS 2
#define AUDIO_CAPACITY (AUDIO_SAMPLE_RATE/TARGET_FPS*AUDIO_CHANNELS)
static int16_t audio[AUDIO_CAPACITY];

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
    };
}

static float angle = 0;

static float cx = (float)DISPLAY_WIDTH/2;
static float cy = (float)DISPLAY_HEIGHT/2;
static float vx = 1000;
static float vy = 1000;
static float ax = 0;
static float ay = 0;

void game_update(void)
{
    memset(audio, 0, sizeof(audio));

    // Display
    for (size_t i = 0; i < ARRAY_LEN(display); ++i) {
        display[i] = (Color) {
            .r = 24,
            .g = 24,
            .b = 24,
            .a = 255,
        };
    }

    NVC_Canvas oc = {
        .pixels = (uint32_t*)display,
        .width  = DISPLAY_WIDTH,
        .height = DISPLAY_HEIGHT,
        .stride = DISPLAY_WIDTH,
    };

    size_t vert_count = 3;
    float x = (float)DISPLAY_WIDTH/2;
    float y = (float)DISPLAY_HEIGHT/2;
    float dangle = 2*M_PI/vert_count;
    float mag = (float)DISPLAY_WIDTH/4;

    Vec2D p1 = Vec2D(x + cosf(dangle*0 + angle)*mag, y + sinf(dangle*0 + angle)*mag);
    uint32_t c1 = 0xFF0000FF; // Red
    Vec2D p2 = Vec2D(x + cosf(dangle*1 + angle)*mag, y + sinf(dangle*1 + angle)*mag);
    uint32_t c2 = 0xFF00FF00; // Green
    Vec2D p3 = Vec2D(x + cosf(dangle*2 + angle)*mag, y + sinf(dangle*2 + angle)*mag);
    uint32_t c3 = 0xFFFF0000; // Blue

    NVC_Fill_Triangle_C3(oc, p1, p2, p3, c1, c2, c3);

    float radius = 20;

    Vec2D p = Vec2D(cx, cy);

    angle += 2*M_PI*DELTA_TIME;
    cx += vx*DELTA_TIME;
    cy += vy*DELTA_TIME;
    vx += ax*DELTA_TIME;
    vy += ay*DELTA_TIME;
    int hit = 0;
    if (cx < radius) {
        hit = 1;
        vx -= ax*DELTA_TIME;
        vx = -vx;
        cx = radius;
    }
    if (cy < radius) {
        hit = 1;
        vy -= ay*DELTA_TIME;
        vy = -vy;
        cy = radius;
    }
    if (cx > oc.width - radius) {
        hit = 1;
        vx = -vx;
        vx -= ax*DELTA_TIME;
        cx = oc.width - radius - 1;
    }
    if (cy > oc.height - radius) {
        hit = 1;
        vy -= ay*DELTA_TIME;
        vy = -vy;
        cy = oc.height - radius - 1;
    }

    if (hit) stb_vorbis_seek_start(ogg);

    // Audio
    stb_vorbis_get_samples_short_interleaved(ogg, AUDIO_CHANNELS, audio, AUDIO_CAPACITY);

    NVC_Fill_Circle(oc, p, radius, 0xFF5555FF);
}

void game_key_up(int key)
{
    UNUSED(key);
    TODO("game_key_up");
}

void game_key_down(int key)
{
    UNUSED(key);
    TODO("game_key_down");
}
