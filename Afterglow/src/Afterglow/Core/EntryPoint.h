#pragma once

#include "Log.h"

#ifndef AG_PLATFORM_WINDOWS
    #error Afterglow only supports Windows for now.
#endif

#ifdef AG_SHIPPING
#define AG_IMPLEMENT_APP(ApplicationType)                                                                   \
        int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)     \
        {                                                                                                   \
            Afterglow::Log::Init(#ApplicationType);                                                         \
            ApplicationType* App = new ApplicationType();                                                   \
            App->Run();                                                                                     \
            delete App;                                                                                     \
                                                                                                            \
            return 0;                                                                                       \
        }
#else
#define AG_IMPLEMENT_APP(ApplicationType)                                                                   \
        int main(int argc, char** argv)                                                                     \
        {                                                                                                   \
            Afterglow::Log::Init(#ApplicationType);                                                         \
            ApplicationType* App = new ApplicationType();                                                   \
            App->Run();                                                                                     \
            delete App;                                                                                     \
                                                                                                            \
            return 0;                                                                                       \
        }
#endif