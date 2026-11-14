# MK3-only POSIX OSAL implementation. Keep upstream OSAL untouched; replace
# only its kernel mqueue queue source with a process-local pthread queue.
set(MK3_UPSTREAM_POSIX "${OSAL_SOURCE_DIR}/src/os/posix")
set(MK3_UPSTREAM_PORTABLE "${OSAL_SOURCE_DIR}/src/os/portable")

set(POSIX_BASE_SRCLIST
    ${MK3_UPSTREAM_POSIX}/src/os-impl-binsem.c
    ${MK3_UPSTREAM_POSIX}/src/os-impl-common.c
    ${MK3_UPSTREAM_POSIX}/src/os-impl-console.c
    ${MK3_UPSTREAM_POSIX}/src/os-impl-condvar.c
    ${MK3_UPSTREAM_POSIX}/src/os-impl-countsem.c
    ${MK3_UPSTREAM_POSIX}/src/os-impl-dirs.c
    ${MK3_UPSTREAM_POSIX}/src/os-impl-errors.c
    ${MK3_UPSTREAM_POSIX}/src/os-impl-files.c
    ${MK3_UPSTREAM_POSIX}/src/os-impl-filesys.c
    ${MK3_UPSTREAM_POSIX}/src/os-impl-heap.c
    ${MK3_UPSTREAM_POSIX}/src/os-impl-idmap.c
    ${MK3_UPSTREAM_POSIX}/src/os-impl-mutex.c
    src/os-impl-queues.c
    ${MK3_UPSTREAM_POSIX}/src/os-impl-tasks.c
    ${MK3_UPSTREAM_POSIX}/src/os-impl-timebase.c
)

set(POSIX_IMPL_SRCLIST
    ${MK3_UPSTREAM_PORTABLE}/os-impl-posix-gettime.c
    ${MK3_UPSTREAM_PORTABLE}/os-impl-console-bsp.c
    ${MK3_UPSTREAM_PORTABLE}/os-impl-bsd-select.c
    ${MK3_UPSTREAM_PORTABLE}/os-impl-posix-io.c
    ${MK3_UPSTREAM_PORTABLE}/os-impl-posix-files.c
    ${MK3_UPSTREAM_PORTABLE}/os-impl-posix-dirs.c
)

if(OSAL_CONFIG_INCLUDE_SHELL)
    list(APPEND POSIX_IMPL_SRCLIST ${MK3_UPSTREAM_POSIX}/src/os-impl-shell.c)
else()
    list(APPEND POSIX_IMPL_SRCLIST ${MK3_UPSTREAM_PORTABLE}/os-impl-no-shell.c)
endif()

if(OSAL_CONFIG_INCLUDE_DYNAMIC_LOADER)
    list(APPEND POSIX_IMPL_SRCLIST
        ${MK3_UPSTREAM_POSIX}/src/os-impl-loader.c
        ${MK3_UPSTREAM_PORTABLE}/os-impl-posix-dl-loader.c
        ${MK3_UPSTREAM_PORTABLE}/os-impl-posix-dl-symtab.c
    )
else()
    list(APPEND POSIX_IMPL_SRCLIST
        ${MK3_UPSTREAM_POSIX}/src/os-impl-no-module.c
        ${MK3_UPSTREAM_PORTABLE}/os-impl-no-loader.c
        ${MK3_UPSTREAM_PORTABLE}/os-impl-no-symtab.c
    )
endif()

if(OSAL_CONFIG_INCLUDE_NETWORK)
    list(APPEND POSIX_IMPL_SRCLIST
        ${MK3_UPSTREAM_PORTABLE}/os-impl-bsd-sockets.c
        ${MK3_UPSTREAM_PORTABLE}/os-impl-posix-network.c
    )
else()
    list(APPEND POSIX_IMPL_SRCLIST
        ${MK3_UPSTREAM_PORTABLE}/os-impl-no-network.c
        ${MK3_UPSTREAM_PORTABLE}/os-impl-no-sockets.c
    )
endif()

add_library(osal_posix_impl OBJECT ${POSIX_BASE_SRCLIST} ${POSIX_IMPL_SRCLIST})
target_include_directories(osal_posix_impl PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${MK3_UPSTREAM_POSIX}/inc
)
target_compile_definitions(osal_public_api INTERFACE _POSIX_OS_)
