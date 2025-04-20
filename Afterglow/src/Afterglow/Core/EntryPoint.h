#pragma once

#ifndef AG_PLATFORM_WINDOWS
	#error Afterglow only supports Windows for now.
#endif

#define IMPLEMENT_AFTERGLOW_APP(ApplicationType) \
    int main(int argc, char** argv) { \
        Afterglow::Log::Init(); \
                                \
        ApplicationType* App = new ApplicationType(); \
        App->Init(); \
        App->Run(); \
        delete App; \
        return 0; \
    }