#include <ctype.h>
#include <time.h>
#include <string.h>
#include "raylib.h"
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"
#include "game.h"

#define OLIVEC_IMPLEMENTATION
#include "olive.c"

static AudioStream audio_stream;

static Texture2D screen_texture;

static bool prev_key[65536] = {0};

static void update_input(Game *game)
{
    Controls *ctrl = game->controls;

    ctrl->mouse_dx = GetMouseDelta().x;
    ctrl->mouse_dy = GetMouseDelta().y;

    for (int key = 32; key <= 348; key++) {
        if (IsKeyDown(key)) {
            game_key_down(key);
        } else {
            game_key_up(key);
        }
    }
}

int main(void)
{
    Game game = game_init();

    InitWindow(game.display_width, game.display_height, game.title);
    SetTargetFPS(game.target_fps);

    // DisableCursor();

    Image screen_image = {
        .data    = game.display,
        .width   = game.display_width,
        .height  = game.display_height,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    screen_texture = LoadTextureFromImage(screen_image);

    audio_stream = LoadAudioStream(game.audio_sample_rate, 16, game.audio_channels);
    PlayAudioStream(audio_stream);

    while (!WindowShouldClose())
    {
        update_input(&game);

        game_update();

        UpdateTexture(screen_texture, game.display);
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTexture(screen_texture, 0, 0, WHITE);
        EndDrawing();

        int frames_per_buffer = game.audio_sample_rate / game.target_fps;
        UpdateAudioStream(audio_stream, game.audio, frames_per_buffer);
    }

    UnloadTexture(screen_texture);
    UnloadAudioStream(audio_stream);
    CloseWindow();
    return 0;
}
