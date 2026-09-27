#include "UI.h"

#include "Engine/Platform/Input.h"
#include "Engine/Renderer/Renderer.h"

// NOTE(saeb): FNV-1a. Same label, same ID every frame; 0 is reserved for "none".
static UIID UIHash(StringView8 text)
{
    uint32 hash = 2166136261u;
    for(usize index = 0; index < text.Length; ++index)
    {
        hash ^= (uint8)text.Data[index];
        hash *= 16777619u;
    }

    return((hash == 0) ? 1 : hash);
}

void UIBegin(UIContext* ui, const Font* font)
{
    int32 mouseX, mouseY;
    InputGetMouseXY(&mouseX, &mouseY);
    ui->MouseX = (real32)mouseX;
    ui->MouseY = (real32)mouseY;
    ui->MouseDown = InputMouseButtonDown(InputMouseButton::Left);
    ui->MousePressed = InputMouseButtonPressed(InputMouseButton::Left);
    ui->MouseReleased = InputMouseButtonReleased(InputMouseButton::Left);

    ui->NextHot = 0;
    ui->Font = font;

    RendererSetSpace(RendererSpace::Window); // UI lives in window pixels, like the mouse
}

void UIEnd(UIContext* ui)
{
    ui->Hot = ui->NextHot;

    // NOTE(saeb): If the active widget wasn't drawn this frame (hidden, or its panel closed), nothing else would ever clear it.
    if(!ui->MouseDown)
    {
        ui->Active = 0;
    }

    RendererSetSpace(RendererSpace::Design);
}

bool UIButton(UIContext* ui, StringView8 label, real32 x, real32 y, real32 width, real32 height)
{
    UIID id = UIHash(label);

    bool over = (ui->MouseX >= x && ui->MouseX < x + width) && (ui->MouseY >= y && ui->MouseY < y + height);

    // NOTE(saeb): While another widget is held, nothing else becomes hot; dragging a slider across a button doesn't light it up.
    if(over && (ui->Active == 0 || ui->Active == id))
    {
        ui->NextHot = id;
    }

    bool clicked = false;

    if(ui->Hot == id && ui->MousePressed)
    {
        ui->Active = id;
    }

    // NOTE(saeb): After the press check, so a press and release within one frame still clicks. Releasing away from the button cancels, like any OS button.
    if(ui->Active == id && ui->MouseReleased)
    {
        clicked = over;
        ui->Active = 0;
    }

    // Look: pressed, hovered, idle.
    real32 shade = (ui->Active == id) ? 0.35f : (ui->Hot == id) ? 0.25f : 0.15f;

    RendererQuad quad = {};
    quad.X = x; quad.Y = y; quad.Width = width; quad.Height = height;
    quad.R = shade; quad.G = shade; quad.B = shade; quad.A = 1.0f;
    RendererPushQuad(&quad);

    // Label centred; TextDraw's y is the top of the text and TextMeasure's height is ascent + descent, so this centres exactly.
    real32 textSize = height * 0.6f;
    real32 textWidth, textHeight;
    TextMeasure(ui->Font, textSize, label, &textWidth, &textHeight);
    TextDraw(ui->Font, x + (width - textWidth) * 0.5f, y + (height - textHeight) * 0.5f, textSize, 1.0f, 1.0f, 1.0f, 1.0f, label);

    return(clicked);
}
