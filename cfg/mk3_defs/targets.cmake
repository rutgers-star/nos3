# NanoCom SDR MK3 ARM builds. MK3_PROFILE picks what is built:
#   base  cFS with ci_lab and to_lab
#   star  every STAR hardware module compiled and packaged, none auto-started
#   link  ci_lab, to_lab, MGR and payload_if started on the busOBC bench
# MK3_TOOLCHAIN picks the compiler: mk3-arm (GomSpace SDK or local Yocto tree, the default)
# or mk3-armhf (Debian's cross compiler, in the container `make fsw-mk3` uses).
if(NOT DEFINED MK3_PROFILE)
  if(DEFINED ENV{MK3_PROFILE})
    set(MK3_PROFILE "$ENV{MK3_PROFILE}")
  else()
    set(MK3_PROFILE base)
  endif()
endif()
if(NOT MK3_PROFILE MATCHES "^(base|star|link)$")
  message(FATAL_ERROR "MK3_PROFILE must be base, star or link")
endif()

if(MK3_PROFILE STREQUAL "star")
  set(MISSION_NAME "SPICEsatMK3STAR")
elseif(MK3_PROFILE STREQUAL "link")
  set(MISSION_NAME "SPICEsatMK3Link")
else()
  set(MISSION_NAME "SPICEsatMK3")
endif()
set(SPACECRAFT_ID 42)
set(MISSION_CPUNAMES cpu1)
set(cpu1_PROCESSORID 1)
if(NOT "$ENV{MK3_TOOLCHAIN}" STREQUAL "")
  set(cpu1_SYSTEM "$ENV{MK3_TOOLCHAIN}")
else()
  set(cpu1_SYSTEM mk3-arm)
endif()
set(cpu1_APPLIST ci_lab to_lab)
set(cpu1_FILELIST cfe_es_startup.scr)

if(MK3_PROFILE STREQUAL "star")
  # The Raspberry Pi cFS module is legacy reference code. The actual PayOBC
  # runs separate Linux services; payload_if is the busOBC-side app.
  list(APPEND cpu1_APPLIST
    hwlib
    generic_eps/fsw/cfs
    tmp100/fsw/cfs
    thermal_control/fsw/cfs
    payload_if/fsw/cfs
    raspberry_pi/fsw/cfs
    mgr/fsw/cfs
  )

  # Thermal control consumes EPS/TMP100 message definitions; these headers
  # normally come from NOS3's broad global app list, absent in this target.
  set(APPLICATION_PLATFORM_INC_LIST
    "${CMAKE_CURRENT_LIST_DIR}/../../fsw/apps/hwlib/fsw/public_inc"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/generic_eps/fsw/cfs/src"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/generic_eps/fsw/cfs/platform_inc"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/generic_eps/fsw/shared"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/tmp100/fsw/cfs/src"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/tmp100/fsw/cfs/platform_inc"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/tmp100/fsw/shared"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/mgr/fsw/cfs/src"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/mgr/fsw/cfs/platform_inc"
    # For the TO_LAB subscription table (tables/to_lab_sub.c)
    "${CMAKE_CURRENT_LIST_DIR}/../../components/thermal_control/fsw/cfs/platform_inc"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/payload_if/fsw/cfs/platform_inc"
  )
elseif(MK3_PROFILE STREQUAL "link")
  # The link startup script starts MGR and payload_if alongside the lab apps.
  list(APPEND cpu1_APPLIST hwlib mgr/fsw/cfs payload_if/fsw/cfs)
  set(cpu1_FILELIST)
  # payload_if includes MGR headers, and the TO_LAB table subscribes to MGR housekeeping.
  set(APPLICATION_PLATFORM_INC_LIST
    "${CMAKE_CURRENT_LIST_DIR}/../../fsw/apps/hwlib/fsw/public_inc"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/mgr/fsw/cfs/src"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/mgr/fsw/cfs/platform_inc"
    "${CMAKE_CURRENT_LIST_DIR}/../../components/payload_if/fsw/cfs/platform_inc"
  )
endif()
