#include "UI.h"

#include "Engine/Platform/Window.h"
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

    uint32 windowClientAreaWidth, windowClientAreaHeight;
    WindowGetClientAreaDimensions(&windowClientAreaWidth, &windowClientAreaHeight);
    ui->ScreenWidth = (real32)windowClientAreaWidth;
    ui->ScreenHeight = (real32)windowClientAreaHeight;

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

void UIPanelBegin(UIContext* ui, UIPanel* panel, StringView8 title)
{
    UIID id = UIHash(title);

    // Hot: is the mouse over the title bar? Same rule as the button.
    bool overTitle = (ui->MouseX >= panel->X && ui->MouseX < panel->X + panel->Width) && (ui->MouseY >= panel->Y && ui->MouseY < panel->Y + UI_TITLE_HEIGHT);
    if(overTitle && (ui->Active == 0 || ui->Active == id))
    {
        ui->NextHot = id;
    }

    // Press: grab it, and remember where on the panel it was grabbed.
    if(ui->Hot == id && ui->MousePressed)
    {
        ui->Active = id;
        ui->DragOffsetX = ui->MouseX - panel->X;
        ui->DragOffsetY = ui->MouseY - panel->Y;
    }

    // Held: follow the mouse, keeping the grab point under it. (UIEnd clears Active on release.)
    if(ui->Active == id)
    {
        panel->X = ui->MouseX - ui->DragOffsetX;
        panel->Y = ui->MouseY - ui->DragOffsetY;
    }

    // Keep it inside the window. Every frame, so shrinking the window pushes it back in too. The "< 0" check comes last, so a panel wider than the window pins to the left edge.
    if(panel->X > ui->ScreenWidth - panel->Width)
    {
        panel->X = ui->ScreenWidth - panel->Width;
    }
    if(panel->X < 0.0f)
    {
        panel->X = 0.0f;
    }
    if(panel->Y > ui->ScreenHeight - panel->Height)
    {
        panel->Y = ui->ScreenHeight - panel->Height;
    }
    if(panel->Y < 0.0f)
    {
        panel->Y = 0.0f;
    }

    // Draw: body, then title bar on top, then the title text.
    RendererQuad body = {};
    body.X = panel->X; body.Y = panel->Y; body.Width = panel->Width; body.Height = panel->Height;
    body.R = 0.08f; body.G = 0.08f; body.B = 0.08f; body.A = 0.9f;
    RendererPushQuad(&body);

    real32 shade = (ui->Active == id) ? 0.35f : (ui->Hot == id) ? 0.25f : 0.18f;
    RendererQuad titleBar = {};
    titleBar.X = panel->X; titleBar.Y = panel->Y; titleBar.Width = panel->Width; titleBar.Height = UI_TITLE_HEIGHT;
    titleBar.R = shade; titleBar.G = shade; titleBar.B = shade; titleBar.A = 1.0f;
    RendererPushQuad(&titleBar);

    real32 textSize = UI_TITLE_HEIGHT * 0.6f;
    real32 textWidth, textHeight;
    TextMeasure(ui->Font, textSize, title, &textWidth, &textHeight);
    TextDraw(ui->Font, panel->X + 8.0f, panel->Y + (UI_TITLE_HEIGHT - textHeight) * 0.5f, textSize, 1.0f, 1.0f, 1.0f, 1.0f, title);
}

void UIPanelEnd(UIContext* ui)
{
    // NOTE(saeb): Nothing yet; the layout ends here.
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
