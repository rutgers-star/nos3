#!/bin/bash
#
# Builds and runs test_spp_forwarding.c against the patched CryptoLib standalone
# tool. Run `make build-cryptolib` first (inside the NOS3 build image), or use
# `make test-cryptolib`.
#
set -eu
HERE=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
BASE_DIR=$(cd "$HERE/../../../../.." && pwd)
GSW_BUILD=${GSWBUILDDIR:-$BASE_DIR/gsw/build}
SRC=${CRYPTOLIB_SRC:-$GSW_BUILD/cryptolib-src}

gcc -w -DSTANDALONE_C="\"$SRC/support/standalone/standalone.c\"" \
    -I "$SRC/include" -I "$SRC/support/standalone" \
    "$HERE/test_spp_forwarding.c" -o "$GSW_BUILD/test_spp_forwarding" \
    -L "$GSW_BUILD/src" -Wl,-rpath,"$GSW_BUILD/src" -lcryptolib -lpthread
"$GSW_BUILD/test_spp_forwarding"
