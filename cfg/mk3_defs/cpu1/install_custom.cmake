# The link profile also starts payload_if, so it installs its own startup script in place of
# cpu1_cfe_es_startup.scr (targets.cmake leaves cpu1_FILELIST empty for it). build.sh exports
# MK3_PROFILE, which is how it reaches this per-architecture build.
if("$ENV{MK3_PROFILE}" STREQUAL "link")
  install(FILES ${MISSION_DEFS}/link_cfe_es_startup.scr
          DESTINATION ${TGTNAME}/${INSTALL_SUBDIR} RENAME cfe_es_startup.scr)
endif()
