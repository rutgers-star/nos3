#!/usr/bin/env bash
set -euo pipefail

mk3_repo=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
mk3_profile=${MK3_PROFILE:-base}
case "$mk3_profile" in
    base)
        mk3_cpu="$mk3_repo/fsw/build-mk3-localq/exe/cpu1"
        mk3_expected_modules=(ci_lab to_lab)
        ;;
    star)
        mk3_cpu="$mk3_repo/fsw/build-mk3-star-localq/exe/cpu1"
        mk3_expected_modules=(ci_lab to_lab hwlib generic_eps tmp100 thermal_control payload_if raspberry_pi mgr)
        ;;
    *) echo "MK3_PROFILE must be base or star" >&2; exit 2 ;;
esac
mk3_sysroot=${MK3_SYSROOT:-}
[[ -d "$mk3_cpu/cf" && -f "$mk3_cpu/core-cpu1" ]] || {
    echo "MK3 install tree missing; run scripts/mk3/build.sh first" >&2; exit 2;
}
[[ -d "$mk3_sysroot" ]] || {
    echo "Set MK3_SYSROOT to the matching GomSpace 4.1.4 target sysroot" >&2; exit 2;
}

mk3_interpreter=$(readelf -l "$mk3_cpu/core-cpu1" | sed -n 's/.*Requesting program interpreter: \([^]]*\).*/\1/p')
[[ "$mk3_interpreter" == /lib/ld-linux-armhf.so.3 ]] || {
    echo "Unexpected ELF interpreter: $mk3_interpreter" >&2; exit 2;
}
[[ -e "$mk3_sysroot$mk3_interpreter" ]] || {
    echo "Interpreter absent from sysroot: $mk3_interpreter" >&2; exit 2;
}

while IFS= read -r -d '' mk3_elf; do
    file "$mk3_elf" | grep -q 'ELF 32-bit.*ARM' || {
        echo "Not an ARM32 ELF: $mk3_elf" >&2; exit 2;
    }
    readelf -h "$mk3_elf" | grep -q 'hard-float ABI' || {
        echo "Not a hard-float ELF: $mk3_elf" >&2; exit 2;
    }
    while IFS= read -r mk3_library; do
        [[ -e "$mk3_sysroot/lib/$mk3_library" || -e "$mk3_sysroot/usr/lib/$mk3_library" ||
           ( "$mk3_profile" == star && -e "${MK3_SOCKETCAN_ROOT:-/nonexistent}/lib/$mk3_library" ) ]] || {
            echo "Target library absent from sysroot: $mk3_library" >&2; exit 2;
        }
    done < <(readelf -d "$mk3_elf" | awk '/Shared library:/ {gsub(/[][]/, "", $5); print $5}')
done < <(find "$mk3_cpu" -type f \( -name 'core-cpu1' -o -name '*.so' \) -print0)

while IFS= read -r mk3_app; do
    [[ -f "$mk3_cpu/cf/$mk3_app.so" ]] || {
        echo "Startup app absent from install tree: $mk3_app" >&2; exit 2;
    }
done < <(awk -F '[, ]+' '/^CFE_APP/ {print $2}' "$mk3_cpu/cf/cfe_es_startup.scr")
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

{
    echo "package=$mk3_name"
    echo "profile=$mk3_profile"
    echo "created_utc=$mk3_stamp"
    echo "repository_commit=$(git -C "$mk3_repo" rev-parse HEAD)"
    echo "repository_tracked_diff_sha256=$(git -C "$mk3_repo" diff HEAD --binary | sha256sum | awk '{print $1}')"
    echo "target=nanomind-z7030-nv3"
    echo "image=GomSpace Linux 4.1.4 (langdale); board kernel 4.14.0-adi"
    echo "toolchain=arm-gomspace-linux-gnueabi; Cortex-A9 hard-float NEON"
    echo "sysroot=$mk3_sysroot"
    echo "dynamic_loader=$mk3_interpreter"
    echo "mission_defs=cfg/mk3_defs"
    echo "queue_backend=cfg/mk3_osal/posix process-local pthread queue"
    echo "working_directory=cpu1"
    echo "compiled_modules=${mk3_expected_modules[*]}"
    echo "automatic_startup=ci_lab to_lab only"
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
    echo "submodules_begin"
    git -C "$mk3_repo" submodule status --recursive
    echo "submodules_end"
} > "$mk3_stage/MANIFEST.txt"
{
    echo "SPICEsat MK3 $mk3_profile ARM package"
    echo "Start only from cpu1, after verifying the archive and SHA256SUMS."
    echo "This build uses the MK3 process-local queue backend, not kernel POSIX mqueues."
    echo "Base and STAR reached cFE OPERATIONAL on the original no-mqueue kernel in QEMU."
    echo "Physical board execution is untested. Do not flash a QEMU-only kernel."
    if [[ "$mk3_profile" == star ]]; then
        echo "STAR modules are compiled and included but NOT auto-started."
        echo "EPS, TMP100, thermal control, and payload_if need real hardware validation."
        echo "raspberry_pi.so is legacy reference code, not the separate PayOBC service."
    fi
} > "$mk3_stage/README.txt"
(
    cd "$mk3_stage"
    find cpu1 -type f -print0 | sort -z | xargs -0 sha256sum
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
