#!/usr/bin/env bash
set -euo pipefail

mk3_repo=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
mk3_profile=${MK3_PROFILE:-base}
export MK3_PROFILE="$mk3_profile"
export OSAL_EXT_SOURCE_DIR="$mk3_repo/cfg/mk3_osal"
case "$mk3_profile" in
    base) mk3_build="$mk3_repo/fsw/build-mk3-localq" ;;
    star)
        mk3_build="$mk3_repo/fsw/build-mk3-star-localq"
        export CFS_APP_PATH="$mk3_repo/components${CFS_APP_PATH:+:$CFS_APP_PATH}"
        ;;
    *) echo "MK3_PROFILE must be base or star" >&2; exit 2 ;;
esac
mk3_sdk_env=${MK3_SDK_ENV:-}

if [[ -n "$mk3_sdk_env" ]]; then
    [[ -f "$mk3_sdk_env" ]] || { echo "SDK environment not found: $mk3_sdk_env" >&2; exit 2; }
    # The vendor-generated setup file is sourced only inside this process.
    set +u
    source "$mk3_sdk_env"
    set -u
    read -r -a mk3_cc_words <<< "${CC:-}"
    read -r -a mk3_cxx_words <<< "${CXX:-}"
    [[ ${#mk3_cc_words[@]} -gt 0 && ${#mk3_cxx_words[@]} -gt 0 ]] || {
        echo "SDK did not define CC and CXX" >&2; exit 2;
    }
    export MK3_CC="$(command -v "${mk3_cc_words[0]}")"
    export MK3_CXX="$(command -v "${mk3_cxx_words[0]}")"
    export MK3_SYSROOT="${SDKTARGETSYSROOT:?SDK did not define SDKTARGETSYSROOT}"
    export MK3_BINUTILS_DIR="$(dirname "$MK3_CC")"
    export MK3_SDK_CFLAGS="${CFLAGS:-}"
    export MK3_SDK_CXXFLAGS="${CXXFLAGS:-}"
elif [[ -n "${MK3_YOCTO_ROOT:-}" ]]; then
    mk3_yocto="$(cd "$MK3_YOCTO_ROOT" && pwd)"
    mk3_tools="$mk3_yocto/build/tmp/sysroots-components/x86_64"
    mk3_cross="$mk3_tools/gcc-cross-arm/usr/bin/arm-gomspace-linux-gnueabi"
    mk3_binutils="$mk3_tools/binutils-cross-arm/usr/bin/arm-gomspace-linux-gnueabi"
    export MK3_CC="$mk3_cross/arm-gomspace-linux-gnueabi-gcc"
    export MK3_CXX="$mk3_cross/arm-gomspace-linux-gnueabi-g++"
    export MK3_SYSROOT="$mk3_yocto/build/tmp/work/nanomind_z7030_nv3-gomspace-linux-gnueabi/nanocom-sdr-image/1.0-r0/recipe-sysroot"
    mkdir -p "$mk3_build/binutils"
    ln -sfn "$mk3_binutils/arm-gomspace-linux-gnueabi-as" "$mk3_build/binutils/as"
    ln -sfn "$mk3_binutils/arm-gomspace-linux-gnueabi-ld" "$mk3_build/binutils/ld"
    export MK3_BINUTILS_DIR="$mk3_build/binutils"
    export MK3_SDK_CFLAGS=""
    export MK3_SDK_CXXFLAGS=""
    if [[ "$mk3_profile" == star ]]; then
        export MK3_SOCKETCAN_ROOT="$mk3_yocto/build/tmp/sysroots-components/cortexa9t2hf-neon/libsocketcan/usr"
    fi
else
    echo "Set MK3_SDK_ENV to the matching GomSpace 4.1.4 SDK setup file, or MK3_YOCTO_ROOT to its local Yocto build." >&2
    exit 2
fi

for mk3_path in "$MK3_CC" "$MK3_CXX" "$MK3_SYSROOT" "$MK3_BINUTILS_DIR"; do
    [[ -e "$mk3_path" ]] || { echo "Missing MK3 prerequisite: $mk3_path" >&2; exit 2; }
done
if [[ "$mk3_profile" == star ]]; then
    if [[ -z "${MK3_SOCKETCAN_ROOT:-}" ]]; then
        if [[ -f "$MK3_SYSROOT/usr/include/libsocketcan.h" ]]; then
            export MK3_SOCKETCAN_ROOT="$MK3_SYSROOT/usr"
        else
            echo "STAR build requires libsocketcan development headers and library; set MK3_SOCKETCAN_ROOT." >&2
            exit 2
        fi
    fi
    [[ -f "$MK3_SOCKETCAN_ROOT/include/libsocketcan.h" && -e "$MK3_SOCKETCAN_ROOT/lib/libsocketcan.so" ]] || {
        echo "Incomplete libsocketcan development files: $MK3_SOCKETCAN_ROOT" >&2
        exit 2
    }
fi

echo "MK3 compiler: $MK3_CC"
echo "MK3 sysroot: $MK3_SYSROOT"
echo "MK3 profile: $mk3_profile"
"$MK3_CC" -dumpmachine

cmake -S "$mk3_repo/fsw/cfe" -B "$mk3_build" \
    -DMISSION_SOURCE_DIR="$mk3_repo/fsw" \
    -DMISSION_DEFS="$mk3_repo/cfg/mk3_defs" \
    -DMISSIONCONFIG=mk3 \
    -DOSAL_EXT_SOURCE_DIR="$OSAL_EXT_SOURCE_DIR" \
    -DMK3_PROFILE="$mk3_profile" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$mk3_build/exe" \
    -DENABLE_UNIT_TESTS=OFF
cmake --build "$mk3_build" --target mission-install --parallel "${MK3_JOBS:-4}"
