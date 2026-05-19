#include <stdio.h>
#include <stdlib.h>
#define RGFW_IMPLEMENTATION
#include "RGFW.h"
#include <SDL2/SDL.h>
// #include <pulse/sample.h>
// #include <pulse/simple.h>
// #include <pulse/error.h> 

#include "game.h"

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

static void stretch_nearest(uint32_t* dst, int dst_w, int dst_h, uint32_t* src, int src_w, int src_h)
{
    float x_ratio = (float)src_w / dst_w;
    float y_ratio = (float)src_h / dst_h;
    for (int y = 0; y < dst_h; y++) {
        int src_y = (int)(y * y_ratio);
        uint32_t* dst_row = dst + y * dst_w;
        uint32_t* src_row = src + src_y * src_w;
        for (int x = 0; x < dst_w; x++) {
            int src_x = (int)(x * x_ratio);
            dst_row[x] = src_row[src_x];
        }
    }
}

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    Game *game = (Game*)userdata;
    int bytes_per_frame = (int)(game->audio_sample_rate / game->target_fps) * game->audio_channels * sizeof(int16_t);
    int offset = 0;
    while (len > 0) {
        int copy = len < bytes_per_frame ? len : bytes_per_frame;
        memcpy(stream + offset, game->audio, copy);
        len -= copy;
        offset += copy;
    }
}

int main(void)
{
    Game game = game_init();

    printf("game.target_fps        = %zu\n", game.target_fps);
    printf("game.display_width     = %zu\n", game.display_width);
    printf("game.display_height    = %zu\n", game.display_height);
    printf("game.audio_sample_rate = %zu\n", game.audio_sample_rate);
    printf("game.audio_channels    = %zu\n", game.audio_channels);

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = (int)game.audio_sample_rate;
    want.format = AUDIO_S16LSB;
    want.channels = (uint8_t)game.audio_channels;
    want.samples = (uint16_t)(game.audio_sample_rate/game.target_fps);
    // want.samples = 2048;
    want.callback = audio_callback;
    want.userdata = &game;

    if (SDL_OpenAudio(&want, &have) < 0) {
        fprintf(stderr, "SDL_OpenAudio failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_PauseAudio(0);

    RGFW_window* window = RGFW_createWindow(
            "The Game",
            0, 0,
            (int)game.display_width, (int)game.display_height,
            RGFW_windowCenter | RGFW_windowNoResize);

    if (!window) {
        fprintf(stderr, "ERROR: failed to create window\n");
        return 1;
    }

    int phys_w, phys_h;
    if (!RGFW_window_getSizeInPixels(window, &phys_w, &phys_h)) {
        phys_w = (int)game.display_width;
        phys_h = (int)game.display_height;
    }
    printf("Physical window size: %dx%d\n", phys_w, phys_h);

    uint32_t* stretched = (uint32_t*)malloc(phys_w * phys_h * sizeof(uint32_t));
    if (!stretched) {
        fprintf(stderr, "ERROR: out of memory for stretch buffer\n");
        RGFW_window_close(window);
        return 1;
    }

    RGFW_surface surface;
    if (!RGFW_createSurfacePtr((uint8_t*)stretched, phys_w, phys_h,
                RGFW_formatARGB8, &surface)) {
        fprintf(stderr, "ERROR: failed to create surface\n");
        free(stretched);
        RGFW_window_close(window);
        return 1;
    }

    RGFW_window_setExitKey(window, RGFW_keyEscape);
    uint64_t delta_time = NANOS_PER_SEC / game.target_fps;
    int quit = 0;

    while (!quit && !RGFW_window_shouldClose(window)) {
        static int frame_count = 0;
        static uint64_t last_frame = 0;

        frame_count++;
        uint64_t frame_start = nanos_since_unspecified_epoch();
        if (frame_start - last_frame >= NANOS_PER_SEC) {
            printf("FPS: %3d\n", frame_count);
            fflush(stdout);
            frame_count = 0;
            last_frame = frame_start;
        }


        RGFW_event event;
        while (RGFW_window_checkEvent(window, &event)) {
            switch (event.type) {
                case RGFW_keyPressed:
                    if (event.key.value == RGFW_keyEscape) {
                        quit = 1;
                    }
                    break;
                case RGFW_mousePosChanged:
                    break;
                case RGFW_windowClose:
                    quit = 1;
                    break;
                default:
                    break;
            }
        }
        game_update();

        stretch_nearest(stretched, phys_w, phys_h,
                (uint32_t*)game.display,
                (int)game.display_width, (int)game.display_height);

        RGFW_window_blitSurface(window, &surface);

        uint64_t frame_end = nanos_since_unspecified_epoch();
        uint64_t delta = frame_end - frame_start;
        if (delta < delta_time) usleep((delta_time - delta) / 1000);

        static uint64_t total_delta = 0;
        static int frame_counter = 0;
        // 在每帧末尾（usleep 之后）：
        total_delta += delta;
        frame_counter++;
        if (frame_counter % 600 == 0) {
            float avg_ms = ((float)total_delta / 600) / 1e6;
            printf("Avg frame time: %.3f ms (target 16.667 ms)\n", avg_ms);
            total_delta = 0;
            frame_counter = 0;
        }
    }

    RGFW_surface_freePtr(&surface);
    free(stretched);
    RGFW_window_close(window);
    return 0;
}
