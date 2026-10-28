# NanoCom SDR MK3 cFS bring-up

This workflow targets the observed `nanomind-z7030-nv3` board running GomSpace Linux 4.1.4 (`langdale`), kernel `4.14.0-adi`, on 32-bit ARMv7. Its Yocto configuration selects Cortex-A9, hard-float ABI, NEON, and the `arm-gomspace-linux-gnueabi` toolchain. The board identification is recorded in `/home/swcaskey/Notes/NanoCom_SDR_MK3_Image_Record.md` on the VM.

## Build on the NOS3 VM

The MK3 target uses `cfg/mk3_defs` and its process-local queue backend in `cfg/mk3_osal/posix`. `MK3_PROFILE=base` builds in `fsw/build-mk3-localq`; `MK3_PROFILE=star` builds in `fsw/build-mk3-star-localq`. Both load only `ci_lab` and `to_lab` at startup. STAR also compiles `hwlib`, `generic_eps`, `tmp100`, `thermal_control`, `payload_if`, `mgr`, and legacy `raspberry_pi` as inactive modules. The legacy Raspberry Pi module is not the separate PayOBC Linux service. The cFE host utilities run with the VM compiler while the runtime is cross-compiled. The regular NOS3 configuration and `fsw/build` are separate.

Preferred path after a GomSpace 4.1.4 SDK installer is available:

```sh
MK3_SDK_ENV=/absolute/path/to/environment-setup-armv7at2hf-neon-gomspace-linux-gnueabi \
  bash scripts/mk3/build.sh
```

Use the actual installed environment filename. The script checks that the setup file supplies `CC`, `CXX`, and `SDKTARGETSYSROOT`; it does not source the SDK into the caller's shell.

The current VM has a local GomSpace Yocto tree at `/home/swcaskey/nanocom-sdr-sdk` with a built Z7030 NV3 image. The following command succeeded using that tree's compiler and target sysroot:

```sh
MK3_YOCTO_ROOT=/home/swcaskey/nanocom-sdr-sdk MK3_JOBS=4 \
  bash scripts/mk3/build.sh
```

Set `MK3_PROFILE=star` on the same command to build STAR. The local Yocto STAR build also uses its staged `libsocketcan` development files. The two build directories stay separate.

`mission-install` places the complete base runtime in `fsw/build-mk3-localq/exe/cpu1`: `core-cpu1`, `cf/cfe_es_startup.scr`, `ci_lab.so`, `to_lab.so`, `cfe_assert.so`, and `to_lab_sub.tbl`. The `pc-linux` PSP maps the current directory's `./cf` to cFE's `/cf`, so start the core from the `cpu1` directory.

## Validate and package

Use the sysroot corresponding to the compiler used for the build. For the local Yocto build:

```sh
MK3_SYSROOT=/home/swcaskey/nanocom-sdr-sdk/build/tmp/work/nanomind_z7030_nv3-gomspace-linux-gnueabi/nanocom-sdr-image/1.0-r0/recipe-sysroot \
  bash scripts/mk3/package.sh
```

For STAR, prefix that package command with `MK3_PROFILE=star MK3_SOCKETCAN_ROOT=/home/swcaskey/nanocom-sdr-sdk/build/tmp/sysroots-components/cortexa9t2hf-neon/libsocketcan/usr`. The generated base and STAR archives are in `dist/mk3/`; the reviewed files to copy to Windows are in `/home/swcaskey/Desktop/MK3_BASE_CFS_LOCALQ/` and `/home/swcaskey/Desktop/MK3_STAR_CFS_LOCALQ/`. The older folders without `LOCALQ` used kernel POSIX queues, retained the known startup failure, and were moved to Trash after the revised archives passed QEMU testing.

For an installed SDK, `MK3_SYSROOT` is the value of its `SDKTARGETSYSROOT`. The package script checks ARM32 hard-float ELF files, the `/lib/ld-linux-armhf.so.3` interpreter, each declared target shared library, startup app files, and the TO_LAB table. It archives the whole `cpu1` tree with file permissions, a repository/submodule/configuration manifest, per-file MK3 source hashes, and SHA-256 checksums. Output is under `dist/mk3/` and is ignored by Git.

The local build's core and app modules are ARM EABI5 hard-float ELF files. The core needs `libpthread.so.0`, `librt.so.1`, and `libc.so.6`; all were found in the matching local Yocto sysroot. Its highest required glibc symbol version is `GLIBC_2.34`. The physical board's dynamic loader reports glibc 2.36, satisfying this version requirement.

## Stage and manually run on the board

The physical board serial check on 2026-10-27 confirmed `/data` is mounted read-write with about 9.2 GB available and the ARM loader reports glibc 2.36. It also confirmed **`# CONFIG_POSIX_MQUEUE is not set`** on the installed kernel. The older POSIX-mqueue archives failed at startup in QEMU with that kernel setting. The revised `LOCALQ` builds reached cFE `OPERATIONAL` using the original kernel in QEMU, but have **not** run on the physical board. Ctrl-C caused cFE to request a processor reset and begin another startup, so it is not a demonstrated clean stop procedure. Do not flash or upload the QEMU-only kernel. The USB transfer method remains unverified. For a future board test, choose a versioned installation directory under `/data` without replacing an earlier release. Use SCP only when SSH and networking actually work; otherwise use a verified vendor-supported transfer path. Transfer both the `.tar.gz` and its `.sha256` file without changing their names.

On the board, from the directory containing the transferred files, set `MK3_ARCHIVE` to the actual archive filename and `MK3_INSTALL_BASE` to the verified writable parent directory:

```sh
sha256sum -c "$MK3_ARCHIVE.sha256"
MK3_RELEASE=${MK3_ARCHIVE%.tar.gz}
mkdir -p "$MK3_INSTALL_BASE/$MK3_RELEASE"
tar -xzf "$MK3_ARCHIVE" -C "$MK3_INSTALL_BASE/$MK3_RELEASE"
cd "$MK3_INSTALL_BASE/$MK3_RELEASE"
sha256sum -c SHA256SUMS
cd cpu1
./core-cpu1
```

Capture the console output, startup events, and `cf` load results. For the first physical test, arrange a verified stop procedure; Ctrl-C requested a cFE processor reset in QEMU. A service should be considered only after a clean manual run and confirmation of the board's init system and runtime user. To roll back, stop this core and start the previous release from its own `cpu1` directory; the package does not modify an earlier release or the vendor image.

`ci_lab` uses UDP port 5012 for incoming commands on CPU1; `to_lab` needs an output-enable command before sending telemetry. Use the project's verified ground transport to demonstrate a NOOP and housekeeping. Core startup alone is a distinct milestone if that transport is not yet configured. The STAR archive includes mission modules, but does not start them or validate their physical UART, I2C, CAN, radio, actuator, or payload interfaces.

## Current result

See [MK3_BRINGUP_RESULTS.md](MK3_BRINGUP_RESULTS.md) for the observed build and test record. The revised `LOCALQ` ARM builds reached `OPERATIONAL` and loaded `ci_lab`/`to_lab` in QEMU on the original kernel. Physical startup, clean shutdown, NOOP, housekeeping, and hardware app behavior remain unverified.
