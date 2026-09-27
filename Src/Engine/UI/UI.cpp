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

static void UIDrawRect(real32 x, real32 y, real32 width, real32 height, real32 r, real32 g, real32 b, real32 a)
{
    RendererQuad quad = {};
    quad.X = x; quad.Y = y; quad.Width = width; quad.Height = height;
    quad.R = r; quad.G = g; quad.B = b; quad.A = a;
    RendererPushQuad(&quad);
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

// NOTE(saeb): Fixed-point text for a number, such as "-12.50", without the C runtime's printf. Returns the length written; the buffer is always null-terminated. Values too large to show exactly print as "big".
static usize UIFormatReal(real32 value, uint32 decimals, char8* buffer, usize capacity)
{
    usize length = 0;
    if(capacity < 24)
    {
        if(capacity > 0)
        {
            buffer[0] = '\0';
        }

        return(0);
    }

    if(value != value)
    {
        buffer[0] = 'n'; buffer[1] = 'a'; buffer[2] = 'n'; buffer[3] = '\0';
        return(3);
    }

    // NOTE(saeb): More than 6 decimals is beyond a real32's precision anyway, and keeps the result well inside the buffer.
    if(decimals > 6)
    {
        decimals = 6;
    }

    uint64 power = 1;
    for(uint32 index = 0; index < decimals; ++index)
    {
        power *= 10;
    }

    bool negative = value < 0.0f;
    real64 magnitude = negative ? -(real64)value : (real64)value;
    real64 scaled = magnitude * (real64)power + 0.5;
    if(scaled >= 1.0e15)
    {
        buffer[0] = 'b'; buffer[1] = 'i'; buffer[2] = 'g'; buffer[3] = '\0';
        return(3);
    }

    uint64 fixed = (uint64)scaled;
    if(negative && fixed > 0) // No "-0.00"
    {
        buffer[length++] = '-';
    }

    // Whole part, written backwards into a scratch buffer, then copied forwards.
    uint64 whole = fixed / power;
    char8 digits[20];
    usize digitCount = 0;
    do
    {
        digits[digitCount++] = (char8)('0' + (whole % 10));
        whole /= 10;
    } while(whole > 0);

    while(digitCount > 0)
    {
        buffer[length++] = digits[--digitCount];
    }

    // Fraction, with its leading zeros: 0.05 is "05", not "5".
    if(decimals > 0)
    {
        buffer[length++] = '.';
        uint64 fraction = fixed % power;
        for(uint64 place = power / 10; place > 0; place /= 10)
        {
            buffer[length++] = (char8)('0' + (fraction / place) % 10);
        }
    }

    buffer[length] = '\0';

    return(length);
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

    real32 textSize = UI_TITLE_HEIGHT * UI_TEXT_SIZE;
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

    bool clicked = UIClickBehavior(ui, id, UIIsMouseOver(ui, x, y, width, height));

    real32 shade = UIShade(ui, id);
    UIDrawRect(x, y, width, height, shade, shade, shade, 1.0f);
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
    UIDrawRect(boxX, boxY, boxSize, boxSize, shade, shade, shade, 1.0f);

    if(*value)
    {
        real32 inset = boxSize * 0.25f;
        UIDrawRect(boxX + inset, boxY + inset, boxSize - 2.0f * inset, boxSize - 2.0f * inset, 1.0f, 1.0f, 1.0f, 1.0f);
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
    UIDrawRect(x, y, width, height, shade, shade, shade, 1.0f);

    real32 fill = (maximum > minimum) ? (*value - minimum) / (maximum - minimum) : 0.0f;
    if(fill < 0.0f)
    {
        fill = 0.0f;
    }

    if(fill > 1.0f)
    {
        fill = 1.0f;
    }

    UIDrawRect(x, y, width * fill, height, 0.25f, 0.45f, 0.7f, 1.0f);

    // "Label: 0.50" centred on the track.
    char8 text[128];
    usize length = 0;
    for(usize index = 0; index < label.Length && length < 96; ++index)
    {
        text[length++] = label.Data[index];
    }

    text[length++] = ':';
    text[length++] = ' ';
    length += UIFormatReal(*value, 2, text + length, sizeof(text) - length);

    UIDrawText(ui, StringView8{ text, length }, x, y, width, height, true);

    return(*value != oldValue);
}
