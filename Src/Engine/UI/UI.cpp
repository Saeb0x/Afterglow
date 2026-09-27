#include "UI.h"

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"
#include "Engine/Renderer/Renderer.h"

#define UI_TITLE_HEIGHT 24.0f
#define UI_PANEL_MIN_WIDTH 150.0f
#define UI_PANEL_MIN_HEIGHT 100.0f // Larger than the title bar plus the grip, so they never overlap
#define UI_GRIP_SIZE 16.0f
#define UI_PADDING 8.0f // Around the content and between rows, at scale 1
#define UI_ROW_HEIGHT 32.0f // At scale 1
#define UI_PANEL_MIN_SCALE 0.75f // A panel can't shrink below 75% of its base size, so text stays readable

// NOTE(saeb): FNV-1a starting from seed, so IDs can be derived from a parent's: the same text under two different seeds gives two different IDs.
static UIID UIHashSeeded(uint32 seed, StringView8 text)
{
    uint32 hash = seed;
    for(usize index = 0; index < text.Length; ++index)
    {
        hash ^= (uint8)text.Data[index];
        hash *= 16777619u;
    }

    return((hash == 0) ? 1 : hash);
}

// NOTE(saeb): Same label, same ID every frame; 0 is reserved for "none".
static UIID UIHash(StringView8 text)
{
    return(UIHashSeeded(2166136261u, text)); // FNV-1a's standard starting value
}

// NOTE(saeb): Inside a panel, IDs derive from the panel's, so "Reset" in two panels gives two different IDs.
static UIID UIWidgetID(UIContext* ui, StringView8 label)
{
    return(ui->Seed ? UIHashSeeded(ui->Seed, label) : UIHash(label));
}

// NOTE(saeb): Hands out the next row and moves the cursor below it.
static void UINextRow(UIContext* ui, real32 baseHeight, real32* x, real32* y, real32* width, real32* height)
{
    *x = ui->LayoutX;
    *y = ui->LayoutY;
    *width = ui->LayoutWidth;
    *height = baseHeight * ui->Scale;

    ui->LayoutY += *height + UI_PADDING * ui->Scale;
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
    ui->Seed = 0;
    ui->Scale = 1.0f; // Widgets outside any panel draw at their base size

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
    UIID gripId = UIHashSeeded(id, SV8(u8"#Resize"));
    UIID bodyId = UIHashSeeded(id, SV8(u8"#Body"));

    // NOTE(saeb): The body catches the mouse first; the title bar, grip and widgets come later and override it. Nothing behind the panel lights up, and UIWantsMouse sees it. The body never becomes Active, so pressing on it does nothing.
    bool overBody = (ui->MouseX >= panel->X && ui->MouseX < panel->X + panel->Width) && (ui->MouseY >= panel->Y && ui->MouseY < panel->Y + panel->Height);
    if(overBody && (ui->Active == 0 || ui->Active == bodyId))
    {
        ui->NextHot = bodyId;
    }

    // Hot: title bar and grip, both tested against last frame's rect.
    bool overTitle = (ui->MouseX >= panel->X && ui->MouseX < panel->X + panel->Width) && (ui->MouseY >= panel->Y && ui->MouseY < panel->Y + UI_TITLE_HEIGHT);
    if(overTitle && (ui->Active == 0 || ui->Active == id))
    {
        ui->NextHot = id;
    }

    // The grip is a small square in the bottom-right corner.
    real32 gripX = panel->X + panel->Width - UI_GRIP_SIZE;
    real32 gripY = panel->Y + panel->Height - UI_GRIP_SIZE;
    bool overGrip = (ui->MouseX >= gripX && ui->MouseX < gripX + UI_GRIP_SIZE) && (ui->MouseY >= gripY && ui->MouseY < gripY + UI_GRIP_SIZE);
    if(overGrip && (ui->Active == 0 || ui->Active == gripId))
    {
        ui->NextHot = gripId;
    }

    // Press: grab it, and remember where on the panel it was grabbed.
    if(ui->Hot == id && ui->MousePressed)
    {
        ui->Active = id;
        ui->DragOffsetX = ui->MouseX - panel->X;
        ui->DragOffsetY = ui->MouseY - panel->Y;
    }

    // Press on the grip. Remember where it was grabbed, relative to the bottom-right corner.
    if(ui->Hot == gripId && ui->MousePressed)
    {
        ui->Active = gripId;
        ui->DragOffsetX = ui->MouseX - (panel->X + panel->Width);
        ui->DragOffsetY = ui->MouseY - (panel->Y + panel->Height);
    }

    // Held: follow the mouse, keeping the grab point under it (UIEnd clears Active on release).
    if(ui->Active == id)
    {
        panel->X = ui->MouseX - ui->DragOffsetX;
        panel->Y = ui->MouseY - ui->DragOffsetY;
    }

    // Held grip. The corner follows the mouse, so the size changes and X, Y stay put. Capped at the window's edge here, so dragging past it doesn't push the panel left or up.
    if(ui->Active == gripId)
    {
        panel->Width = ui->MouseX - ui->DragOffsetX - panel->X;
        panel->Height = ui->MouseY - ui->DragOffsetY - panel->Y;

        if(panel->Width > ui->ScreenWidth - panel->X)
        {
            panel->Width = ui->ScreenWidth - panel->X;
        }

        if(panel->Height > ui->ScreenHeight - panel->Y)
        {
            panel->Height = ui->ScreenHeight - panel->Y;
        }
    }

    // NOTE(saeb): A base size of 0 means the game didn't set one; treat the starting size as the base, so the scale is 1 and never divides by zero.
    if(panel->BaseWidth <= 0.0f)
    {
        panel->BaseWidth = panel->Width;
    }

    if(panel->BaseHeight <= 0.0f)
    {
        panel->BaseHeight = panel->Height;
    }

    // The smallest a panel may get: UI_PANEL_MIN_SCALE of its base size, but never below the fixed minimum.
    real32 minWidth = panel->BaseWidth * UI_PANEL_MIN_SCALE;
    real32 minHeight = panel->BaseHeight * UI_PANEL_MIN_SCALE;
    if(minWidth < UI_PANEL_MIN_WIDTH)
    {
        minWidth = UI_PANEL_MIN_WIDTH;
    }

    if(minHeight < UI_PANEL_MIN_HEIGHT)
    {
        minHeight = UI_PANEL_MIN_HEIGHT;
    }

    // Size limits, every frame. Not larger than the window first, then not smaller than the minimum, so the minimum wins if the window is ever smaller.
    if(panel->Width > ui->ScreenWidth)
    {
        panel->Width = ui->ScreenWidth;
    }

    if(panel->Width < minWidth)
    {
        panel->Width = minWidth;
    }

    if(panel->Height > ui->ScreenHeight)
    {
        panel->Height = ui->ScreenHeight;
    }

    if(panel->Height < minHeight)
    {
        panel->Height = minHeight;
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

    // Draw the grip, after the body so it's visible.
    real32 gripShade = (ui->Active == gripId) ? 0.6f : (ui->Hot == gripId) ? 0.45f : 0.3f;
    RendererQuad grip = {};
    grip.X = panel->X + panel->Width - UI_GRIP_SIZE; grip.Y = panel->Y + panel->Height - UI_GRIP_SIZE;
    grip.Width = UI_GRIP_SIZE; grip.Height = UI_GRIP_SIZE;
    grip.R = gripShade; grip.G = gripShade; grip.B = gripShade; grip.A = 1.0f;
    RendererPushQuad(&grip);

    // NOTE(saeb): The same fit rule as the renderer's design size: contents designed for the base size always fit, whatever shape the panel is.
    real32 scaleX = panel->Width / panel->BaseWidth;
    real32 scaleY = panel->Height / panel->BaseHeight;
    ui->Scale = (scaleX < scaleY) ? scaleX : scaleY;

    // Layout: rows start below the title bar, inset by the padding on both sides.
    real32 padding = UI_PADDING * ui->Scale;
    ui->LayoutX = panel->X + padding;
    ui->LayoutY = panel->Y + UI_TITLE_HEIGHT + padding;
    ui->LayoutWidth = panel->Width - 2.0f * padding;

    ui->Seed = id;
}

void UIPanelEnd(UIContext* ui)
{
    ui->Seed = 0;
    ui->Scale = 1.0f;
}

bool UIWantsMouse(const UIContext* ui)
{
    return(ui->Hot != 0 || ui->Active != 0);
}

bool UIButton(UIContext* ui, StringView8 label)
{
    UIID id = UIWidgetID(ui, label);

    real32 x, y, width, height;
    UINextRow(ui, UI_ROW_HEIGHT, &x, &y, &width, &height);

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
