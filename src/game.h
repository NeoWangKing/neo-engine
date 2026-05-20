#ifndef GAME_H_
#define GAME_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool keyboard[65536];
    int mouse_dx;
    int mouse_dy;
} Controls;

typedef struct {
    size_t target_fps;

    uint32_t *display;
    size_t display_width;
    size_t display_height;

    int16_t *audio;
    size_t audio_sample_rate;
    size_t audio_channels;

    char *title;
    Controls *controls;
} Game;

Game game_init(void);
void game_update(void);
void game_key_up(int key);
void game_key_down(int key);

#endif // GAMG_H_
