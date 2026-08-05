#if defined(AG_DEBUG)
#if !defined(AFTERGLOW_CONSOLE_H)
#define AFTERGLOW_CONSOLE_H

#include "Engine/Types.h"
#include "Input.h"
#include "Text.h"
#include "Colors.h"

#include <string.h>

#define CONSOLE_MAX_LINES 256
#define CONSOLE_LINE_LENGTH 128
#define CONSOLE_INPUT_LENGTH 128
#define CONSOLE_MAX_COMMANDS 64

struct Console;
typedef void (*ConsoleCommandFunc)(Console* console, const char* args, void* data);

struct ConsoleCommand
{
    const char* Name;
    ConsoleCommandFunc Handler;
};

struct Console
{
    bool32 Opened;

    char Lines[CONSOLE_MAX_LINES][CONSOLE_LINE_LENGTH];
    uint32 LineCount;

    char Input[CONSOLE_INPUT_LENGTH];
    uint32 InputLength;

    ConsoleCommand Commands[CONSOLE_MAX_COMMANDS];
    uint32 CommandCount;

    void* Data;
};

static void ConsoleRegisterCommand(Console* console, const char* name, ConsoleCommandFunc handler)
{
    if(console->CommandCount < CONSOLE_MAX_COMMANDS)
    {
        ConsoleCommand* command = &console->Commands[console->CommandCount++];
        command->Name = name;
        command->Handler = handler;
    }
}

static void CommandClear(Console* console, const char* args, void* data)
{
    memset(console->Lines, 0, sizeof(console->Lines));
    console->LineCount = 0;
}

static void ConsoleInit(Console* console, void* data)
{
    console->Opened = false;
    console->LineCount = 0;
    console->InputLength = 0;
    console->Input[0] = 0;
    console->CommandCount = 0;
    console->Data = data;

    ConsoleRegisterCommand(console, "clear", CommandClear);
}

static void ConsolePrint(Console* console, const char* text)
{
    char* line = console->Lines[console->LineCount % CONSOLE_MAX_LINES];

    uint32 i = 0;
    while(text[i] && i < CONSOLE_LINE_LENGTH - 1)
    {
        line[i] = text[i];
        ++i;
    }
    line[i] = 0;

    ++console->LineCount;
}

static void ConsoleExecute(Console* console)
{
    if(console->InputLength == 0)
    {
        return;
    }

    char* name = console->Input;
    const char* args = "";
    for(uint32 i = 0; i < console->InputLength; ++i)
    {
        if(console->Input[i] == ' ')
        {
            console->Input[i] = 0;
            args = &console->Input[i + 1];

            break;
        }
    }

    bool32 found = false;
    for(uint32 i = 0; i < console->CommandCount; ++i)
    {
        if(strcmp(console->Commands[i].Name, name) == 0)
        {
            console->Commands[i].Handler(console, args, console->Data);
            found = true;

            break;
        }
    }

    if(!found)
    {
        // ConsolePrint(console, "Where did you come up with that :)");
    }

    console->InputLength = 0;
    console->Input[0] = 0;
}

static void ConsoleUpdateAndRender(Console* console, GameInput* input, RenderCommands* render, Font* font, int32 screenWidth, int32 screenHeight)
{
    if(Pressed(input->Keys[KEY_TILDE]))
    {
        console->Opened = !console->Opened;
        return;
    }

    if(!console->Opened)
    {
        return;
    }

    for(uint32 i = 0; i < input->TypedCharacterCount; ++i)
    {
        if(console->InputLength < CONSOLE_INPUT_LENGTH - 1)
        {
            console->Input[console->InputLength++] = input->TypedCharacters[i];
            console->Input[console->InputLength] = 0;
        }
    }

    if(Pressed(input->Keys[KEY_BACKSPACE]) && console->InputLength > 0)
    {
        console->Input[--console->InputLength] = 0;
    }

    if(Pressed(input->Keys[KEY_ENTER]))
    {
        ConsoleExecute(console);
    }

    if(Pressed(input->Keys[KEY_ESCAPE]))
    {
        console->Opened = false;
    }

    render->CurrentLayer = 1; // Console sits above the HUD

    real32 padding = 8.0f;
    real32 textScale = 0.6f;
    real32 lineHeight = (real32)font->LineHeight * textScale;
    real32 panelHeight = screenHeight * 0.3f;

    uint32 textColor = WHITE;
    uint32 lineColor = WHITE;

    PushQuad(render, 0.0f, 0.0f, (real32)screenWidth, panelHeight, SKY_BLUE);

    real32 inputY = panelHeight - lineHeight - padding;
    real32 promptWidth = TextWidth(font, "> ", textScale);
    PushText(render, font, padding, inputY, "> ", textColor, textScale);
    PushText(render, font, padding + promptWidth, inputY, console->Input, textColor, textScale);

    real32 caretX = padding + promptWidth + TextWidth(font, console->Input, textScale);
    PushQuad(render, caretX, inputY, 2.0f, lineHeight, textColor);

    real32 y = inputY - lineHeight - padding;
    uint32 shown = 0;
    while(shown < console->LineCount && shown < CONSOLE_MAX_LINES && y > -lineHeight)
    {
        uint32 lineIndex = (console->LineCount - 1 - shown) % CONSOLE_MAX_LINES;
        PushText(render, font, padding, y, console->Lines[lineIndex], lineColor, textScale);
        y -= lineHeight;
        ++shown;
    }
}

#endif
#endif
