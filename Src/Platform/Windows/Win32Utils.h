#if !defined(WIN32UTILS_H)

#include "Core/Types.h"

#if defined(AG_DEBUG)
    #define EMB(error) \
        do { \
            MessageBox(0, error, "Afterglow Error", MB_OK | MB_ICONERROR); \
            Assert(!error); \
        } while(0)
#else
    #define EMB(error) ((void)0)
#endif

#define WIN32UTILS_H
#endif
