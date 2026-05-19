#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include "nob.h"

#define NVC_AA_RES 1
#include "neovin.c"

#include "game.h"

#define TARGET_FPS 60
#define DISPLAY_WIDTH  800
#define DISPLAY_HEIGHT 600
#define DELTA_TIME (1.0f/TARGET_FPS)

typedef struct {
    // TODO: the format should be RGBA32 and it is responsibility of the platform/console to convert the pixels the game renders.
    uint8_t b, g, r, a;
} Color;

static Color display[DISPLAY_WIDTH*DISPLAY_HEIGHT];

#define AUDIO_SAMPLE_RATE 44100
static_assert(AUDIO_SAMPLE_RATE%TARGET_FPS == 0, "Sample rate must be divisible by FPS");
#define AUDIO_CHANNELS 2
#define AUDIO_CAPACITY (AUDIO_SAMPLE_RATE/TARGET_FPS*AUDIO_CHANNELS)

static int16_t audio[AUDIO_CAPACITY];

Game game_init(void)
{
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

float angle = 0;

void game_update(void)
{
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
    float cx = (float)DISPLAY_WIDTH/2;
    float cy = (float)DISPLAY_HEIGHT/2;
    float dangle = 2*M_PI/vert_count;
    float mag = (float)DISPLAY_WIDTH/4;

    Vec2D p1 = Vec2D(cx + cosf(dangle*0 + angle)*mag, cy + sinf(dangle*0 + angle)*mag);
    uint32_t c1 = 0xFF0000FF; // Red
    Vec2D p2 = Vec2D(cx + cosf(dangle*1 + angle)*mag, cy + sinf(dangle*1 + angle)*mag);
    uint32_t c2 = 0xFF00FF00; // Green
    Vec2D p3 = Vec2D(cx + cosf(dangle*2 + angle)*mag, cy + sinf(dangle*2 + angle)*mag);
    uint32_t c3 = 0xFFFF0000; // Blue

    NVC_Fill_Triangle_C3(oc, p1, p2, p3, c1, c2, c3);

    static float phase = 0.0f;
    // const float freq = 440.0f + 200*sinf(angle);
    const float freq = 440.0f;
    const float delta_phase = 2.0f * M_PI * freq / (float)AUDIO_SAMPLE_RATE;

    for (size_t i = 0; i < AUDIO_CAPACITY; i += 2) {
        int16_t sample = (int16_t)(sinf(phase) * 16000);
        audio[i]   = sample;
        audio[i+1] = sample;
        phase += delta_phase;
        if (phase >= 2.0f * M_PI) phase -= 2.0f * M_PI;
    }

    angle += 2*M_PI*DELTA_TIME;
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
