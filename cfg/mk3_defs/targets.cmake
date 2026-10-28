# NanoCom SDR MK3 ARM builds. Both profiles retain a safe startup script;
# STAR hardware modules are compiled and packaged but never auto-started.
if(NOT DEFINED MK3_PROFILE)
  if(DEFINED ENV{MK3_PROFILE})
    set(MK3_PROFILE "$ENV{MK3_PROFILE}")
  else()
    set(MK3_PROFILE base)
  endif()
endif()
if(NOT MK3_PROFILE STREQUAL "base" AND NOT MK3_PROFILE STREQUAL "star")
  message(FATAL_ERROR "MK3_PROFILE must be base or star")
endif()

if(MK3_PROFILE STREQUAL "star")
  set(MISSION_NAME "SPICEsatMK3STAR")
else()
  set(MISSION_NAME "SPICEsatMK3")
endif()
set(SPACECRAFT_ID 42)
set(MISSION_CPUNAMES cpu1)
set(cpu1_PROCESSORID 1)
set(cpu1_SYSTEM mk3-arm)
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
  )
endif()
