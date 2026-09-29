#include "Engine/Game.h"

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Text.h"
#include "Engine/UI/UI.h"
#include "Engine/Asset/Asset.h"

#include <box2d/box2d.h>

static b2WorldId PhysicsWorld;
static b2BodyId FallingBox;

static Camera WorldCamera = { { 0.0f, 0.0f }, { 20.0f, 20.0f }, 1.0f, 0.0f };

void GameConfigure()
{
    WindowSetFlags(WindowFlags_None);
    WindowSetTitle(SV8(u8"Afterglow Sandbox"));
    WindowSetClientAreaDimensions(1280, 720);
    WindowSetMinClientAreaDimensions(360, 360);
    InputSetFlags(InputFlags_Mouse | InputFlags_Keyboard);
    RendererSetFlags(RendererFlags_VSync);
}

bool GameInit(StackAllocator* allocator)
{
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = { 0.0f, -10.0f }; // Metres per second squared, y up: the same space as the camera
    PhysicsWorld = b2CreateWorld(&worldDef);

    b2BodyDef groundDef = b2DefaultBodyDef(); // Static by default
    groundDef.position = { 0.0f, -1.0f };
    b2BodyId ground = b2CreateBody(PhysicsWorld, &groundDef);
    b2Polygon groundShape = b2MakeBox(10.0f, 1.0f); // Half extents: 20 x 2 m
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(ground, &groundShapeDef, &groundShape);

    b2BodyDef boxDef = b2DefaultBodyDef();
    boxDef.type = b2_dynamicBody;
    boxDef.position = { 0.0f, 6.0f };
    boxDef.rotation = b2MakeRot(0.6f); // Tilted, so it lands on a corner and tumbles
    FallingBox = b2CreateBody(PhysicsWorld, &boxDef);
    b2Polygon boxShape = b2MakeBox(0.5f, 0.5f);
    b2ShapeDef boxShapeDef = b2DefaultShapeDef();
    boxShapeDef.density = 1.0f;
    b2CreatePolygonShape(FallingBox, &boxShapeDef, &boxShape);

    return(true);
}

void GameUpdate(StackAllocator* allocator, real64 deltaTime)
{
    RendererSetCamera(&WorldCamera);

    b2World_Step(PhysicsWorld, 1.0f / 60.0f, 4); // Fixed 1/60 s; fine at VSync 60, the toy will use a proper fixed timestep

    b2Vec2 position = b2Body_GetPosition(FallingBox);
    RendererQuad box = {};
    box.X = position.x - 0.5f; box.Y = position.y - 0.5f; box.Width = 1.0f; box.Height = 1.0f;
    box.R = 0.9f; box.G = 0.6f; box.B = 0.2f; box.A = 1.0f;
    box.Rotation = b2Rot_GetAngle(b2Body_GetRotation(FallingBox));
    RendererPushQuad(&box);

    RendererQuad groundQuad = {};
    groundQuad.X = -10.0f; groundQuad.Y = -2.0f; groundQuad.Width = 20.0f; groundQuad.Height = 2.0f;
    groundQuad.R = 0.3f; groundQuad.G = 0.3f; groundQuad.B = 0.35f; groundQuad.A = 1.0f;
    RendererPushQuad(&groundQuad);
}

void GameShutdown(StackAllocator* allocator)
{
    b2DestroyWorld(PhysicsWorld);
}
