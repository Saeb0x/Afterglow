#include "Afterglow.h"

void GameUpdateAndRender(GameMemory* gameMemory, RenderCommands* renderCommands)
{
    PushQuad(renderCommands, 100.0f, 100.0f, 200.0f, 150.0f, PackColor(0, 0, 255, 255));
}
