#ifndef GAME_H_
#define GAME_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool keyboard[65536];
    bool key_just_pressed[65536];
    bool key_just_released[65536];
    int mouse_dx_prev;
    int mouse_dx;
    int mouse_dy_prev;
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

typedef struct {
    int selected;           // 当前选中项索引
    int count;              // 菜单项计数（用于定位）
    float start_y;          // 第一个菜单项的 Y 坐标（基线）
    float item_spacing;     // 项之间的垂直间距
} Menu;

Game game_init(void);
void game_update(void);
void game_key_up(int key);
void game_key_down(int key);
bool game_key_pressed(int key);

#endif // GAMG_H_
