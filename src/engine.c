#include "engine.h"

void Engine_Init(Engine *eng, EngineConfig cfg)
{
    eng->config = cfg;
    InitWindow(cfg.screenWidth, cfg.screenHeight, cfg.title);
    SetTargetFPS(cfg.targetFPS);
}

void Engine_Run(Engine *eng)
{
    eng->running = true;
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        Engine_OnUpdate(eng, dt);

        BeginDrawing();
        ClearBackground(CLITERAL(Color) { 24, 24, 24, 255 });

        Engine_OnRender(eng);

        EndDrawing();
    }
    CloseWindow();
    eng->running = false;
}

__attribute__((weak)) void Engine_OnUpdate(Engine *eng, float delta) {}
__attribute__((weak)) void Engine_OnRender(Engine *eng) {}

