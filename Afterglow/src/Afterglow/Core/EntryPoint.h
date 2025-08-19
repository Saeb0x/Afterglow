#pragma once

#include "Log.h"

#ifndef AG_PLATFORM_WINDOWS
	#error Afterglow only supports Windows for now.
#endif

#define AG_IMPLEMENT_APP(ApplicationType)                          \
    int main(int argc, char** argv) {                               \
        Afterglow::Log::Init(#ApplicationType);                      \
                                                                      \
        ApplicationType* App = new ApplicationType();                  \
        App->Run();                                                     \
        delete App;                                                      \
                                                                          \
        return 0;                                                          \
    }