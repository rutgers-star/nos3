# Sourced by build.sh and package.sh when using an existing GomSpace Yocto build tree.
mk3_yocto_tmp="$(cd "$MK3_YOCTO_ROOT" && pwd)/build/tmp"
mk3_yocto_sysroot="$mk3_yocto_tmp/work/nanomind_z7030_nv3-gomspace-linux-gnueabi/nanocom-sdr-image/1.0-r0/recipe-sysroot"
mk3_yocto_socketcan="$mk3_yocto_tmp/sysroots-components/cortexa9t2hf-neon/libsocketcan/usr"
