# MK3 bring-up result record

Recorded on 2026-10-13 (VM local time; package timestamp is UTC).

## Inputs

- NOS3 checkout: `main` at `53511fae6296f5d4842ea627aa43ecf44c8bc12d` before these uncommitted changes.
- Board console: `nanomind-z7030-nv3`, Linux `4.14.0-adi`, `armv7l`; `/etc/os-release` reports GomSpace Linux 4.1.4 (`langdale`).
- Local Yocto build: machine `nanomind-z7030-nv3`, distro/SDK version `4.1.4`, target `arm-gomspace-linux-gnueabi`, `cortexa9thf-neon`, hard-float ABI. Its image output is dated 2026-04-07; a board image build ID has not yet been observed.

## Observed checks

| Check | Result |
| --- | --- |
| `bitbake -e nanocom-sdr-image` target metadata | Confirmed ARM/Cortex-A9, hard-float, GomSpace 4.1.4 |
| Tiny C program compiled against local Yocto sysroot | ARM32 EABI5 hard-float ELF with `/lib/ld-linux-armhf.so.3` interpreter |
| `MK3_YOCTO_ROOT=/home/swcaskey/nanocom-sdr-sdk MK3_JOBS=4 bash scripts/mk3/build.sh` | `mission-install` passed; native host tools and ARM runtime built in separate cFE stages |
| `MK3_SYSROOT=.../nanocom-sdr-image/1.0-r0/recipe-sysroot bash scripts/mk3/package.sh` | ELF, dependencies, startup files, table, checksums, and archive passed |
| Existing NOS3 unit suite inside `ivvitc/nos3-64:20260619` | 127/127 passed |
| Existing NOS3 unit suite directly on Ubuntu 24.04 host | 126/127 passed; `coverage-es-ALL` segfaulted. The same suite passed in its pinned container. |
| Board transfer and manual cFS run | Pending; no verified board transfer path or writable deployment directory was available to this VM session |

Package: `dist/mk3/spicesat-mk3-53511fae-20261014T002509Z.tar.gz` with adjacent `.sha256`; the extracted staging tree contains `MANIFEST.txt` and `SHA256SUMS`.

## SDK generation attempt

`bitbake nanocom-sdr-image -c populate_sdk` did not finish on the Ubuntu 24.04 VM. BitBake reported `make: /bin/bash: Invalid argument` while compiling native `chrpath` and `binutils-crosssdk` tasks. Running the generated task scripts directly succeeded, which points to a BitBake worker/host environment issue rather than an ARM compiler or cFS source failure. The already built Yocto cross compiler and matching recipe sysroot were used for the successful cFS build. An installable SDK and its environment setup script are therefore not claimed as generated.

## Current remaining checks

1. Confirm the USB transfer method and install each `LOCALQ` package in a separate versioned directory under writable `/data`. Verify both archive and internal checksums, then run `core-cpu1` manually from each `cpu1` directory and capture startup and app load events.
2. Establish a clean stop procedure before repeated physical runs; Ctrl-C requested a cFE processor reset in QEMU.
3. Confirm `libsocketcan.so.2` on the board before loading STAR hardware modules. With a verified command and telemetry transport, send harmless NOOP and housekeeping requests; record events, counters, and telemetry.
4. Validate each STAR hardware module on the physical interfaces before enabling it at startup. Decide on a startup service only after a successful manual run.

## QEMU preflight, 2026-10-27

The VM's Yocto `uImage` identifies Linux `4.14.0-adi` with the same 2019-10-03 build timestamp reported by the physical board. Its root filesystem reports GomSpace Linux 4.1.4. This is a close software match, not proof that the physical board has the identical root filesystem or kernel configuration. The original desktop test directions copied the normal NOS3 x86-64 `core-cpu1` into `/usr/bin`; inspection confirmed that binary inside the image is x86-64 and cannot test the ARM target. The directions were later updated for the ARM packages.

QEMU initially failed to detect the emulated SD card because the board device tree specified `cd-gpios` on `/amba/sdhci@e0100000`. Removing that property from a **temporary copy** of the device tree allowed QEMU's `xilinx-zynq-a9` model to mount the root filesystem and reach an automatic root login. The original Yocto kernel, device tree, and rootfs were not modified. Missing physical I2C/CAN devices caused expected emulator-only probe failures.

The exact `spicesat-mk3-53511fae-20261014T002509Z.tar.gz` and `.sha256` were inserted into a disposable copy of the rootfs. Inside the ARM guest, the archive checksum and every internal `SHA256SUMS` entry passed. Running `./core-cpu1` from the extracted `cpu1` directory entered cFE core initialization, then failed when OSAL called `mq_open`: `OS_QueueCreate Error. errno = 38 (Function not implemented)`. `CFE_EVS` could not create a pipe and cFE reset. Both the guest's `/proc/config.gz` and the local Yocto kernel build configuration say `# CONFIG_POSIX_MQUEUE is not set`; `/proc/sys/fs/mqueue` is absent. Thus this kernel cannot run the current POSIX-OSAL cFS build as configured. This is **not** a successful cFS startup or app-load test.

The QEMU guest did not include `getconf` or `ldd`, so the board's libc version was checked using its dynamic loader instead. The board's queue setting was subsequently confirmed; see below.

To isolate the package from that kernel limitation, a **QEMU-only** kernel was compiled from the local Yocto kernel source in `/tmp/mk3-qemu.QAO59K/kernel-build`, using a copy of its configuration with `CONFIG_POSIX_MQUEUE=y` and `CONFIG_POSIX_MQUEUE_SYSCTL=y`. This kernel was never installed on the board or written into the original Yocto image. It booted the disposable rootfs copy with the QEMU-only device tree. The archive and all internal checksums passed again. With this test kernel, the same `core-cpu1` entered cFE `OPERATIONAL` state, loaded `ci_lab.so` and `to_lab.so`, reported CI_LAB listening on UDP port 5012, and remained active until stopped with Ctrl-C. TO_LAB initialized and awaited its output-enable command. NOOP and housekeeping traffic were not tested.

This establishes that the packaged ARM application starts when the Linux kernel supplies the required POSIX queue feature. It does **not** establish that the unmodified vendor kernel or the physical OBC can run it. Do not treat the QEMU-only kernel as a board deployment artifact.

## Physical board serial check, 2026-10-27

The complete output is preserved in `/home/swcaskey/Notes/IMAGE_INFO.txt`, copied from the desktop's `TerminalOut.txt`. The physical `nanomind-z7030-nv3` again reports GomSpace Linux 4.1.4 and kernel `4.14.0-adi` with the same build timestamp as the Yocto image. `/etc/version` is `20180309123456`. Its `/lib/ld-linux-armhf.so.3` exists and reports glibc **2.36**, satisfying the package's `GLIBC_2.34` symbol requirement. Root (`/dev/mmcblk1p2`) is read-write with 73 MB available; `/data` (`/dev/mmcblk1p5`) is read-write with about 9.2 GB available. The console was in `/data/cfs/apps`; no cFS package was transferred or run in this check.

Crucially, the board itself reports **`# CONFIG_POSIX_MQUEUE is not set`** from `/proc/config.gz`. This matches the original QEMU kernel and confirms the current POSIX-OSAL cFS build cannot create its required message queues on the installed board kernel. The QEMU-only test kernel is not an acceptable board workaround. A board-kernel or OSAL compatibility decision is required before the current package can demonstrate hardware startup. USB transfer mode and exact image build ID remain unverified.

## Separate base and STAR ARM packages, 2026-10-27

At checkout `a2f9942f` with uncommitted MK3 build configuration, two distinct ARM builds completed using the local GomSpace Yocto compiler and target sysroot. The base profile contains cFE, `ci_lab`, and `to_lab`. The STAR profile additionally compiled `hwlib`, `generic_eps`, `tmp100`, `thermal_control`, `payload_if`, `mgr`, and the legacy `raspberry_pi` module. Both startup scripts start only `ci_lab` and `to_lab`; STAR hardware modules remain inactive. The STAR `hwlib.so` links to `libsocketcan.so.2` from the Yocto `libsocketcan` component. Presence of that library on the physical board has not been checked.

`scripts/mk3/package.sh` passed ARM32 hard-float ELF, loader, target dependency, expected module, startup file, and internal checksum checks for both profiles. The copied Desktop archives passed their `.sha256` checks; archive listings contain the expected STAR `.so` modules. Files for transfer to Windows are in `/home/swcaskey/Desktop/MK3_BASE_CFS/` and `/home/swcaskey/Desktop/MK3_STAR_CFS/`. Neither refreshed package has been run in QEMU or on the physical board. The physical board's disabled `CONFIG_POSIX_MQUEUE` remains a startup blocker for both.

## MK3 process-local queue revision, 2026-10-27

The MK3 build now selects `cfg/mk3_osal/posix` through OSAL's extension path. This substitutes an in-process pthread queue for `mq_open` while leaving the upstream OSAL submodule unchanged. `bash scripts/mk3/test_localq.sh` passed in a fresh host build, exercising FIFO, poll and timed reads, full queue, deletion of a blocked reader, and cancellation. Both ARM profiles rebuilt and repackaged successfully. The final archives and copied Desktop `.sha256` files passed verification in `MK3_BASE_CFS_LOCALQ/` and `MK3_STAR_CFS_LOCALQ/`.

Both revised installed ARM trees reached cFE `OPERATIONAL` and loaded `ci_lab` and `to_lab` in QEMU using the **original** vendor kernel (`CONFIG_POSIX_MQUEUE` disabled). A final archive-level test inserted both `LOCALQ` `.tar.gz` files into a disposable copy of the QEMU rootfs. The archive hashes and internal `SHA256SUMS` matched, and each extracted archive separately reached `OPERATIONAL` and initialized `ci_lab`/`to_lab`. The STAR hardware modules remained inactive. A separate background-run check showed SIGINT/CTRL-C caused cFE to request a processor reset, run the CI/TO delete callbacks, and begin another startup. This explains why Ctrl-C did not immediately return to the shell; a clean stop procedure has not been validated. Neither revised package has run on the physical OBC. USB transfer, command/telemetry traffic, physical `libsocketcan.so.2`, and mission hardware behavior remain unverified. The older non-LOCALQ Desktop folders were moved to Trash; their archives remain in `dist/mk3/`.

## Commit candidate review, 2026-10-27

The MK3 queue backend now checks the queue object ID while holding its mutex, preventing a stale operation from using a newly allocated queue in the same slot. The package manifest now includes hashes for each MK3 source file and the full tracked diff against HEAD. Current remaining checks above replace the earlier queue-blocker guidance. `bash scripts/mk3/test_localq.sh`, both ARM `mission-install` builds, both package checks, archive hashes, and internal checksums passed. The newly packaged base archive `spicesat-mk3-base-localq-a2f9942f-20261028T011620Z.tar.gz` and STAR archive `spicesat-mk3-star-localq-a2f9942f-20261028T011623Z.tar.gz` were each verified inside a disposable QEMU guest and reached cFE `OPERATIONAL` on the original kernel. The Desktop `MK3_BASE_CFS_LOCALQ.zip` and `MK3_STAR_CFS_LOCALQ.zip` contain these exact archives and passed ZIP integrity checks. Earlier Desktop folders and ZIPs were deleted after verification; older archives remain in `dist/mk3/`.
