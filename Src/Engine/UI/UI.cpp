#include "UI.h"

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"
#include "Engine/Renderer/Renderer.h"

#define UI_TITLE_HEIGHT 30.0f
#define UI_PANEL_MIN_WIDTH 150.0f
#define UI_PANEL_MIN_HEIGHT 100.0f // Larger than the title bar plus the grip, so they never overlap
#define UI_GRIP_SIZE 16.0f
#define UI_PADDING 8.0f // Around the content and between rows, at scale 1
#define UI_ROW_HEIGHT 32.0f // At scale 1
#define UI_PANEL_MIN_SCALE 0.75f // A panel can't shrink below 75% of its base size, so text stays readable
#define UI_TEXT_SIZE 0.6f // Text height as a fraction of its row's height

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

static bool UIIsMouseOver(UIContext* ui, real32 x, real32 y, real32 width, real32 height)
{
    return((ui->MouseX >= x && ui->MouseX < x + width) && (ui->MouseY >= y && ui->MouseY < y + height));
}

// NOTE(saeb): Drawn with the UI's font, not as plain quads, so rects and text share one texture and pipeline and the whole UI batches into a single draw call.
static void UIDrawRect(UIContext* ui, real32 x, real32 y, real32 width, real32 height, real32 r, real32 g, real32 b, real32 a)
{
    TextDrawRect(ui->Font, x, y, width, height, r, g, b, a);
}

// NOTE(saeb): Text sized to the row and centred vertically in it; centred horizontally too, or left-aligned at x. TextDraw's y is the top of the text and TextMeasure's height is ascent + descent, so this centres exactly.
static void UIDrawText(UIContext* ui, StringView8 text, real32 x, real32 y, real32 width, real32 height, bool centre)
{
    real32 textSize = height * UI_TEXT_SIZE;
    real32 textWidth, textHeight;
    TextMeasure(ui->Font, textSize, text, &textWidth, &textHeight);

    real32 textX = centre ? x + (width - textWidth) * 0.5f : x;
    TextDraw(ui->Font, textX, y + (height - textHeight) * 0.5f, textSize, 1.0f, 1.0f, 1.0f, 1.0f, text);
}

// NOTE(saeb): Hot and active handling shared by everything that clicks (buttons, checkboxes). Returns true on the frame it's clicked.
static bool UIClickBehavior(UIContext* ui, UIID id, bool over)
{
    // NOTE(saeb): While another widget is held, nothing else becomes hot; dragging a slider across a button doesn't light it up.
    if(over && (ui->Active == 0 || ui->Active == id))
    {
        ui->NextHot = id;
    }

    if(ui->Hot == id && ui->MousePressed)
    {
        ui->Active = id;
    }

    // NOTE(saeb): After the press check, so a press and release within one frame still clicks. Releasing away from the widget cancels, like any OS button.
    bool clicked = false;
    if(ui->Active == id && ui->MouseReleased)
    {
        clicked = over;
        ui->Active = 0;
    }

    return(clicked);
}

// NOTE(saeb): Look for anything that can be pressed: pressed, hovered, idle.
static real32 UIShade(UIContext* ui, UIID id)
{
    return((ui->Active == id) ? 0.35f : (ui->Hot == id) ? 0.25f : 0.15f);
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

    int32 windowClientAreaWidth, windowClientAreaHeight;
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
    UIDrawRect(ui, panel->X, panel->Y, panel->Width, panel->Height, 0.08f, 0.08f, 0.08f, 0.9f);

    real32 shade = (ui->Active == id) ? 0.35f : (ui->Hot == id) ? 0.25f : 0.18f;
    UIDrawRect(ui, panel->X, panel->Y, panel->Width, UI_TITLE_HEIGHT, shade, shade, shade, 1.0f);

    real32 textSize = UI_TITLE_HEIGHT * UI_TEXT_SIZE;
    real32 textWidth, textHeight;
    TextMeasure(ui->Font, textSize, title, &textWidth, &textHeight);
    TextDraw(ui->Font, panel->X + 8.0f, panel->Y + (UI_TITLE_HEIGHT - textHeight) * 0.5f, textSize, 1.0f, 1.0f, 1.0f, 1.0f, title);

    // Draw the grip, after the body so it's visible.
    real32 gripShade = (ui->Active == gripId) ? 0.6f : (ui->Hot == gripId) ? 0.45f : 0.3f;
    UIDrawRect(ui, panel->X + panel->Width - UI_GRIP_SIZE, panel->Y + panel->Height - UI_GRIP_SIZE, UI_GRIP_SIZE, UI_GRIP_SIZE, gripShade, gripShade, gripShade, 1.0f);

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

    bool clicked = UIClickBehavior(ui, id, UIIsMouseOver(ui, x, y, width, height));

    real32 shade = UIShade(ui, id);
    UIDrawRect(ui, x, y, width, height, shade, shade, shade, 1.0f);
    UIDrawText(ui, label, x, y, width, height, true);

    return(clicked);
}

void UILabel(UIContext* ui, StringView8 text)
{
    real32 x, y, width, height;
    UINextRow(ui, UI_ROW_HEIGHT, &x, &y, &width, &height);

    UIDrawText(ui, text, x, y, width, height, false);
}

bool UICheckbox(UIContext* ui, StringView8 label, bool* value)
{
    UIID id = UIWidgetID(ui, label);

    real32 x, y, width, height;
    UINextRow(ui, UI_ROW_HEIGHT, &x, &y, &width, &height);

    // NOTE(saeb): The whole row is clickable, not just the box; a bigger target is easier to hit.
    bool toggled = UIClickBehavior(ui, id, UIIsMouseOver(ui, x, y, width, height));
    if(toggled)
    {
        *value = !*value;
    }

    // Box on the left, the label after it.
    real32 boxSize = height * UI_TEXT_SIZE;
    real32 boxX = x;
    real32 boxY = y + (height - boxSize) * 0.5f;
    real32 shade = UIShade(ui, id);
    UIDrawRect(ui, boxX, boxY, boxSize, boxSize, shade, shade, shade, 1.0f);

    if(*value)
    {
        real32 inset = boxSize * 0.25f;
        UIDrawRect(ui, boxX + inset, boxY + inset, boxSize - 2.0f * inset, boxSize - 2.0f * inset, 1.0f, 1.0f, 1.0f, 1.0f);
    }

    real32 labelX = boxX + boxSize + UI_PADDING * ui->Scale;
    UIDrawText(ui, label, labelX, y, width - (labelX - x), height, false);

    return(toggled);
}

bool UISlider(UIContext* ui, StringView8 label, real32* value, real32 minimum, real32 maximum)
{
    UIID id = UIWidgetID(ui, label);

    real32 x, y, width, height;
    UINextRow(ui, UI_ROW_HEIGHT, &x, &y, &width, &height);

    bool over = UIIsMouseOver(ui, x, y, width, height);
    if(over && (ui->Active == 0 || ui->Active == id))
    {
        ui->NextHot = id;
    }

    if(ui->Hot == id && ui->MousePressed)
    {
        ui->Active = id;
    }

    // NOTE(saeb): While held, the value follows the mouse's position along the track, even outside it (clamped to the ends). No grab offset: pressing anywhere on the track jumps the value there. UIEnd clears Active on release.
    real32 oldValue = *value;
    if(ui->Active == id && maximum > minimum && width > 0.0f)
    {
        real32 t = (ui->MouseX - x) / width;
        if(t < 0.0f)
        {
            t = 0.0f;
        }

        if(t > 1.0f)
        {
            t = 1.0f;
        }

        *value = minimum + t * (maximum - minimum);
    }

    // Track, then the filled part up to the value.
    real32 shade = UIShade(ui, id);
    UIDrawRect(ui, x, y, width, height, shade, shade, shade, 1.0f);

    real32 fill = (maximum > minimum) ? (*value - minimum) / (maximum - minimum) : 0.0f;
    if(fill < 0.0f)
    {
        fill = 0.0f;
    }

    if(fill > 1.0f)
    {
        fill = 1.0f;
    }

    UIDrawRect(ui, x, y, width * fill, height, 0.25f, 0.45f, 0.7f, 1.0f);

    // "Label: 0.50" centred on the track, built in a local buffer.
    char8 buffer[128];
    String8 text = { buffer, 0, sizeof(buffer) };
    String8Append(&text, label);
    String8Append(&text, SV8(u8": "));
    String8AppendReal(&text, *value, 2);

    UIDrawText(ui, StringView8{ text.Data, text.Length }, x, y, width, height, true);

    return(*value != oldValue);
}
