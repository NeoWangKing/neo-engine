#ifndef ENGINE_H_
#define ENGINE_H_

#include "raylib.h"
#include <stdbool.h>

typedef struct {
    int screenWidth;
    int screenHeight;
    const char *title;
    int targetFPS;
} EngineConfig;

typedef struct {
    EngineConfig config;
    bool running;
} Engine;

void Engine_Init(Engine *eng, EngineConfig cfg);
void Engine_Run(Engine *eng);

void Engine_OnUpdate(Engine *eng, float dt);
void Engine_OnRender(Engine *eng);

#endif // ENGINE_H_
