
#include "safe_mode.h"

static SpacecraftMode current_mode = MODE_SAFE;

void SafeMode_Init(void)
{
    current_mode = MODE_SAFE;
}

int SafeMode_SetMode(SpacecraftMode new_mode, CommandSource source)
{
    // Reject unknown modes.
    if (new_mode != MODE_NORMAL &&
        new_mode != MODE_SAFE &&
        new_mode != MODE_EXPERIMENT) {
        return 0;
    }

    // FSW-2: Only an operator command may exit Safe Mode.
    if (current_mode == MODE_SAFE &&
        new_mode != MODE_SAFE &&
        source != SOURCE_OPERATOR) {
        return 0;
    }

    current_mode = new_mode;
    return 1;
}

SpacecraftMode SafeMode_GetMode(void)
{
    return current_mode;
}
