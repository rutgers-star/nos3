
#include <stdio.h>
#include <assert.h>
#include "safe_mode.h"

int main(void)
{
    SafeMode_Init();

    // An automatic experiment attempts to start.
    int result = SafeMode_SetMode(MODE_EXPERIMENT, SOURCE_AUTOMATIC);
    assert(result == 0);
    assert(SafeMode_GetMode() == MODE_SAFE);
    printf("Automatic experiment blocked while in Safe Mode.\n");


    // Automatic command to Normal Mode is also blocked.
    result = SafeMode_SetMode(MODE_NORMAL, SOURCE_AUTOMATIC);
    assert(result == 0);
    assert(SafeMode_GetMode() == MODE_SAFE);
    printf("Automatic mode_change blocked while in Safe Mode.\n");


    // Random modes are rejected.
    result = SafeMode_SetMode(999, SOURCE_OPERATOR);
    assert(result == 0);
    assert(SafeMode_GetMode() == MODE_SAFE);
    printf("Invalid mode rejected.\n");


    // Operator explicitly commands exit to Normal Mode.
    result = SafeMode_SetMode(MODE_NORMAL, SOURCE_OPERATOR);
    assert(result == 1);
    assert(SafeMode_GetMode() == MODE_NORMAL);
    printf("Operator successfully exited Safe Mode.\n");


    // Now an automatic experiment mode transition is allowed.
    result = SafeMode_SetMode(MODE_EXPERIMENT, SOURCE_AUTOMATIC);
    assert(result == 1);
    assert(SafeMode_GetMode() == MODE_EXPERIMENT);
    printf("Experiment Mode entered.\n");

    return 0;
}
