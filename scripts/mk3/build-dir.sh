# Sourced by build.sh and package.sh: sets mk3_build, the build directory for mk3_profile and
# mk3_toolchain. Each toolchain gets its own directory, because CMake caches the compiler.
case "$mk3_profile" in
    base) mk3_build="$mk3_repo/fsw/build-mk3-localq" ;;
    star) mk3_build="$mk3_repo/fsw/build-mk3-star-localq" ;;
    link) mk3_build="$mk3_repo/fsw/build-mk3-link-localq" ;;
    *) echo "MK3_PROFILE must be base, star or link" >&2; exit 2 ;;
esac
case "$mk3_toolchain" in
    mk3-arm) ;;
    mk3-armhf) mk3_build="$mk3_build-armhf" ;;
    *) echo "MK3_TOOLCHAIN must be mk3-arm or mk3-armhf" >&2; exit 2 ;;
esac
