#!/bin/bash
#
# Run an MK3 script (build.sh or package.sh) in the cross-compilation container, with Debian's
# armhf compiler (MK3_TOOLCHAIN=mk3-armhf). Used by `make fsw-mk3` and `make fsw-mk3-package`;
# MK3_PROFILE picks the profile (base, star or link; default link).
#
HERE=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source "$HERE/../env.sh"

case "$1" in
    build.sh|package.sh) ;;
    *) echo "usage: container.sh build.sh|package.sh" >&2; exit 2 ;;
esac

# Docker's layer cache makes this quick after the first time
$DCALL build -t $DBOX_MK3 "$HERE" || exit 1
$DFLAGS_CPUS -v "$BASE_DIR:$BASE_DIR" -w "$BASE_DIR" \
    -e MK3_TOOLCHAIN=mk3-armhf -e MK3_PROFILE="${MK3_PROFILE:-link}" -e MK3_JOBS="$NUM_CPUS" \
    $DBOX_MK3 bash "scripts/mk3/$1"
