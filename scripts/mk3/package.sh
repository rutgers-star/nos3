#!/usr/bin/env bash
set -euo pipefail

mk3_repo=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
mk3_profile=${MK3_PROFILE:-base}
mk3_toolchain=${MK3_TOOLCHAIN:-mk3-arm}
source "$mk3_repo/scripts/mk3/build-dir.sh"
source "$mk3_repo/scripts/mk3/source-state.sh"
export LC_ALL=C
mk3_cpu="$mk3_build/exe/cpu1"
mk3_sbtool="$mk3_build/exe/bin/sbtool"
case "$mk3_profile" in
    base) mk3_expected_modules=(ci_lab to_lab) ;;
    star) mk3_expected_modules=(ci_lab to_lab hwlib generic_eps tmp100 thermal_control payload_if raspberry_pi mgr) ;;
    link) mk3_expected_modules=(ci_lab to_lab hwlib mgr payload_if) ;;
esac
# Debian's cross toolchain keeps the target C library outside a sysroot, and its other target
# libraries (libsocketcan) under the multiarch directory
mk3_extra_libdir=/nonexistent
mk3_strip=${MK3_STRIP:-}
if [[ "$mk3_toolchain" == mk3-armhf ]]; then
    MK3_SYSROOT=${MK3_SYSROOT:-/usr/arm-linux-gnueabihf}
    mk3_extra_libdir=/usr/lib/arm-linux-gnueabihf
    mk3_strip=${mk3_strip:-arm-linux-gnueabihf-strip}
elif [[ -n "${MK3_YOCTO_ROOT:-}" ]]; then
    source "$mk3_repo/scripts/mk3/yocto-paths.sh"
    MK3_SYSROOT=${MK3_SYSROOT:-$mk3_yocto_sysroot}
    MK3_SOCKETCAN_ROOT=${MK3_SOCKETCAN_ROOT:-$mk3_yocto_socketcan}
fi
mk3_sysroot=${MK3_SYSROOT:-}
[[ -d "$mk3_cpu/cf" && -x "$mk3_cpu/core-cpu1" && -x "$mk3_sbtool" && -f "$mk3_build/source-built.txt" ]] || {
    echo "MK3 install tree missing; run scripts/mk3/build.sh first" >&2; exit 2;
}
[[ ! -e "$mk3_cpu/cf/mgr.bin" && ! -L "$mk3_cpu/cf/mgr.bin" ]] || {
    echo "Move the MGR runtime state out of the install tree before packaging: $mk3_cpu/cf/mgr.bin" >&2; exit 2;
}
[[ -d "$mk3_sysroot" ]] || {
    echo "Set MK3_YOCTO_ROOT or MK3_SYSROOT to locate the matching GomSpace 4.1.4 target sysroot" >&2; exit 2;
}
mk3_current_source=$(mk3_source_state)
[[ "$mk3_current_source" == "$(< "$mk3_build/source-built.txt")" ]] || {
    echo "Source revision or tracked changes differ from the MK3 build; rerun build.sh" >&2; exit 2;
}

mk3_interpreter=/lib/ld-linux-armhf.so.3
[[ -e "$mk3_sysroot$mk3_interpreter" ]] || {
    echo "Interpreter absent from sysroot: $mk3_interpreter" >&2; exit 2;
}

mapfile -d '' -t mk3_modules < <(find "$mk3_cpu" -type f -name '*.so' -print0)
for mk3_elf in "$mk3_cpu/core-cpu1" "$mk3_sbtool" "${mk3_modules[@]}"; do
    file "$mk3_elf" | grep -q 'ELF 32-bit.*ARM' || {
        echo "Not an ARM32 ELF: $mk3_elf" >&2; exit 2;
    }
    readelf -h "$mk3_elf" | grep -q 'hard-float ABI' || {
        echo "Not a hard-float ELF: $mk3_elf" >&2; exit 2;
    }
    if [[ "$mk3_elf" != *.so ]]; then
        mk3_loader=$(readelf -l "$mk3_elf" | sed -n 's/.*Requesting program interpreter: \([^]]*\).*/\1/p')
        [[ "$mk3_loader" == "$mk3_interpreter" ]] || {
            echo "Unexpected ELF interpreter for $mk3_elf: $mk3_loader" >&2; exit 2;
        }
    fi
    while IFS= read -r mk3_library; do
        [[ -e "$mk3_sysroot/lib/$mk3_library" || -e "$mk3_sysroot/usr/lib/$mk3_library" ||
           -e "$mk3_extra_libdir/$mk3_library" ||
           ( "$mk3_library" == libsocketcan.so* && -e "${MK3_SOCKETCAN_ROOT:-/nonexistent}/lib/$mk3_library" ) ]] || {
            echo "Target library absent from sysroot for $mk3_elf: $mk3_library" >&2; exit 2;
        }
    done < <(readelf -d "$mk3_elf" | awk '/Shared library:/ {gsub(/[][]/, "", $5); print $5}')
done

[[ -f "$mk3_cpu/cf/cfe_es_startup.scr" ]] || { echo "Startup script absent" >&2; exit 2; }
if [[ "$mk3_profile" == link ]] && ! awk -F '[, ]+' '/^CFE_APP/ && $2 == "mgr" && $3 == "MGR_AppMain" { found=1 } END { exit !found }' "$mk3_cpu/cf/cfe_es_startup.scr"; then
    echo "MGR startup entry absent from link profile" >&2
    exit 2
fi
while IFS= read -r mk3_app; do
    [[ -f "$mk3_cpu/cf/$mk3_app.so" ]] || {
        echo "Startup module absent from install tree: $mk3_app" >&2; exit 2;
    }
done < <(awk -F '[, ]+' '/^CFE_(APP|LIB)/ {print $2}' "$mk3_cpu/cf/cfe_es_startup.scr")
[[ -f "$mk3_cpu/cf/to_lab_sub.tbl" ]] || { echo "TO_LAB table absent" >&2; exit 2; }
for mk3_app in "${mk3_expected_modules[@]}"; do
    [[ -f "$mk3_cpu/cf/$mk3_app.so" ]] || {
        echo "Expected $mk3_profile module absent: $mk3_app.so" >&2; exit 2;
    }
done

mk3_stamp=$(date -u +%Y%m%dT%H%M%SZ)
mk3_short=$(git -C "$mk3_repo" rev-parse --short HEAD)
mk3_name="spicesat-mk3-${mk3_profile}-localq-${mk3_short}-${mk3_stamp}"
mk3_dist="$mk3_repo/dist/mk3"
mk3_stage="$mk3_dist/$mk3_name"
[[ ! -e "$mk3_stage" && ! -e "$mk3_stage.tar.gz" ]] || {
    echo "Package path already exists: $mk3_name" >&2; exit 2;
}
mkdir -p "$mk3_stage"
cp -a "$mk3_cpu" "$mk3_stage/cpu1"
mkdir "$mk3_stage/bin"
cp -a "$mk3_sbtool" "$mk3_stage/bin/sbtool"
# Unstripped, the package is too big to copy over the board's debug console (about 2.5 KB/s);
# the build tree keeps the symbols
if [[ -n "$mk3_strip" ]]; then
    find "$mk3_stage" -type f \( -name 'core-cpu1' -o -name '*.so' -o -name sbtool \) -exec "$mk3_strip" {} +
fi
mk3_startup=$(awk -F '[, ]+' '/^CFE_APP/ {printf "%s ", $2}' "$mk3_stage/cpu1/cf/cfe_es_startup.scr")

{
    echo "package=$mk3_name"
    echo "profile=$mk3_profile"
    echo "created_utc=$mk3_stamp"
    cat "$mk3_build/source-built.txt"
    echo "target=nanomind-z7030-nv3"
    echo "image=GomSpace Linux 4.1.4 (langdale); board kernel 4.14.0-adi"
    if [[ "$mk3_toolchain" == mk3-armhf ]]; then
        echo "toolchain=mk3-armhf: Debian arm-linux-gnueabihf (scripts/mk3/Dockerfile); Cortex-A9 hard-float NEON"
    else
        echo "toolchain=mk3-arm: arm-gomspace-linux-gnueabi; Cortex-A9 hard-float NEON"
    fi
    echo "stripped=${mk3_strip:-no}"
    echo "sysroot=$mk3_sysroot"
    echo "dynamic_loader=$mk3_interpreter"
    echo "mission_defs=cfg/mk3_defs"
    echo "queue_backend=cfg/mk3_osal/posix process-local pthread queue"
    echo "working_directory=cpu1"
    echo "compiled_modules=${mk3_expected_modules[*]}"
    echo "automatic_startup=${mk3_startup% }"
    echo "repository_status_begin"
    git -C "$mk3_repo" status --porcelain
    echo "repository_status_end"
    echo "mk3_source_sha256_begin"
    (
        cd "$mk3_repo"
        sha256sum .gitignore
        find cfg/mk3_defs cfg/mk3_osal scripts/mk3 -type f -print0 | sort -z | xargs -0 sha256sum
    )
    echo "mk3_source_sha256_end"
} > "$mk3_stage/MANIFEST.txt"
{
    echo "SPICEsat MK3 $mk3_profile ARM package"
    echo "Start only from cpu1, after verifying the archive and SHA256SUMS."
    echo "This build uses the MK3 process-local queue backend, not kernel POSIX mqueues."
    echo "What has been verified, in QEMU and on the board: docs/MK3_BRINGUP_RESULTS.md."
    echo "Do not flash a QEMU-only kernel."
    if [[ "$mk3_profile" == star ]]; then
        echo "STAR modules are compiled and included but NOT auto-started."
        echo "EPS, TMP100, thermal control, and payload_if need real hardware validation."
        echo "raspberry_pi.so is legacy reference code, not the separate PayOBC service."
    fi
    if [[ "$mk3_profile" == link ]]; then
        echo "payload_if starts on /dev/ttyS3 (J14). Start cFS only once pl-rs422.service is active."
        echo "MGR starts and stores its state in cpu1/cf/mgr.bin. Start from a writable release."
        echo "bin/sbtool and scripts/mk3/bench drive it from the board's shell or from OpenC3."
    fi
} > "$mk3_stage/README.txt"
(
    cd "$mk3_stage"
    find cpu1 bin -type f -print0 | sort -z | xargs -0 sha256sum
    sha256sum MANIFEST.txt
    sha256sum README.txt
) > "$mk3_stage/SHA256SUMS"
(
    cd "$mk3_stage"
    sha256sum -c SHA256SUMS
)
tar -czf "$mk3_dist/$mk3_name.tar.gz" -C "$mk3_stage" .
(
    cd "$mk3_dist"
    sha256sum "$mk3_name.tar.gz"
) > "$mk3_dist/$mk3_name.tar.gz.sha256"
echo "Package: $mk3_dist/$mk3_name.tar.gz"
echo "Archive checksum: $mk3_dist/$mk3_name.tar.gz.sha256"
