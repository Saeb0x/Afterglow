#if !defined(AFTERGLOW_H)

struct GameOffScreenBitmapBuffer
{
    void* Data;

    int32 Width;
    int32  Height;

    uint8 BytesPerPixel;
    int32 Pitch;
};
static void GameUpdateAndRender(GameOffScreenBitmapBuffer* buffer, int32 xOffset);

#define AFTERGLOW_H
#endif
