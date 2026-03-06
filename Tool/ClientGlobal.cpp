#include "ClientPch.h"
#include "ClientGlobal.h"
#include "EngineGlobal.h"

bool GRunning = true;

void InitGame()
{
    GameEngine::InitEngine();
}

void ReleaseGame()
{
    GameEngine::ReleaseEngine();
}
