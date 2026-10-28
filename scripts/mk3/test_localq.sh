#!/usr/bin/env bash
set -euo pipefail

mk3_repo=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
mk3_test_build=${MK3_TEST_BUILD:-/tmp/mk3-osal-localq-host}
cmake -S "$mk3_repo/fsw/osal" -B "$mk3_test_build" \
    -DOSAL_EXT_SOURCE_DIR="$mk3_repo/cfg/mk3_osal" \
    -DOSAL_SYSTEM_BSPTYPE=generic-linux \
    -DOSAL_CONFIG_DEBUG_PERMISSIVE_MODE=ON \
    -DENABLE_UNIT_TESTS=OFF
cmake --build "$mk3_test_build" --target osal osal_bsp --parallel 4
cc -I "$mk3_repo/fsw/osal/src/os/inc" -I "$mk3_test_build/inc" \
    "$mk3_repo/scripts/mk3/test_localq.c" \
    "$mk3_test_build/libosal.a" "$mk3_test_build/libosal_bsp.a" \
    -pthread -ldl -lrt -o "$mk3_test_build/test_localq"
timeout 10s "$mk3_test_build/test_localq"
