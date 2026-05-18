#include "engine.h"
#include "raylib.h"

void Engine_OnUpdate(Engine *eng, float delta)
{
}

void Engine_OnRender(Engine *eng)
{
    DrawText("My 3D Engine", 10, 10, 20, WHITE);
}

int main(void) {
    Engine eng;
    Engine_Init(&eng, (EngineConfig){
        .screenWidth = 800,
        .screenHeight = 600,
        .title = "My 3D Engine",
        .targetFPS = 60
    });
    Engine_Run(&eng);
    return 0;
}
