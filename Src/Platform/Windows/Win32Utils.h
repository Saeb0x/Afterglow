#if !defined(WIN32UTILS_H)

#define EMB(error) \
    do { \
        MessageBox(0, error, "Afterglow Error", MB_OK | MB_ICONERROR); \
        abort(); \
    } while(0)

#define WIN32UTILS_H
#endif
