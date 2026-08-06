#if !defined(AFTERGLOW_WIN32UTILS_H)
#define AFTERGLOW_WIN32UTILS_H

#include "Engine/Types.h"

#if defined(AG_DEBUG)
    #define EMB(error) \
        do { \
            MessageBox(0, error, "Afterglow Error", MB_OK | MB_ICONERROR); \
            SSTL_ASSERT(!error); \
        } while(0)
#else
    #define EMB(error) ((void)0)
#endif

#endif
