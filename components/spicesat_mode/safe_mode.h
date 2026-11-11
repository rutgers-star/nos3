
#ifndef SAFE_MODE_H
#define SAFE_MODE_H

typedef enum {
    MODE_NORMAL,
    MODE_SAFE,
    MODE_EXPERIMENT
} SpacecraftMode;

typedef enum {
    SOURCE_AUTOMATIC,
    SOURCE_OPERATOR
} CommandSource;

void SafeMode_Init(void);
int SafeMode_SetMode(SpacecraftMode new_mode, CommandSource source);
SpacecraftMode SafeMode_GetMode(void);

#endif
