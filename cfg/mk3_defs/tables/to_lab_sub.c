/*
** TO_LAB subscription table for the MK3 builds (cfg/mk3_defs).
**
** TO_LAB's own default table lists none of the STAR apps, so without this one the ground never
** sees payload_if. An app's packets are listed only when its message IDs are on the include
** path, which depends on MK3_PROFILE, so one table serves base, star and link.
*/
#include "cfe_tbl_filedef.h"
#include "to_lab_sub_table.h"
#include "to_lab_msgids.h"
#include "ci_lab_msgids.h"

#if __has_include("payload_if_msgids.h")
#include "payload_if_msgids.h"
#endif
#if __has_include("generic_eps_msgids.h")
#include "generic_eps_msgids.h"
#endif
#if __has_include("tmp100_msgids.h")
#include "tmp100_msgids.h"
#endif
#if __has_include("thermal_control_msgids.h")
#include "thermal_control_msgids.h"
#endif
#if __has_include("mgr_msgids.h")
#include "mgr_msgids.h"
#endif

TO_LAB_Subs_t TO_LAB_Subs = {
    .Subs = {
        /* Lab apps */
        {CFE_SB_MSGID_WRAP_VALUE(TO_LAB_HK_TLM_MID), {0, 0}, 4},
        {CFE_SB_MSGID_WRAP_VALUE(TO_LAB_DATA_TYPES_MID), {0, 0}, 4},
        {CFE_SB_MSGID_WRAP_VALUE(CI_LAB_HK_TLM_MID), {0, 0}, 4},

        /* cFE core */
        {CFE_SB_MSGID_WRAP_VALUE(CFE_ES_HK_TLM_MID), {0, 0}, 4},
        {CFE_SB_MSGID_WRAP_VALUE(CFE_EVS_HK_TLM_MID), {0, 0}, 4},
        {CFE_SB_MSGID_WRAP_VALUE(CFE_SB_HK_TLM_MID), {0, 0}, 4},
        {CFE_SB_MSGID_WRAP_VALUE(CFE_TBL_HK_TLM_MID), {0, 0}, 4},
        {CFE_SB_MSGID_WRAP_VALUE(CFE_TIME_HK_TLM_MID), {0, 0}, 4},
        {CFE_SB_MSGID_WRAP_VALUE(CFE_TIME_DIAG_TLM_MID), {0, 0}, 4},
        {CFE_SB_MSGID_WRAP_VALUE(CFE_SB_STATS_TLM_MID), {0, 0}, 4},
        {CFE_SB_MSGID_WRAP_VALUE(CFE_TBL_REG_TLM_MID), {0, 0}, 4},
        {CFE_SB_MSGID_WRAP_VALUE(CFE_EVS_LONG_EVENT_MSG_MID), {0, 0}, 32},

#ifdef PAYLOAD_IF_HK_TLM_MID
        /* Payload link: housekeeping, and every packet from the PayOBC inside its wrapper */
        {CFE_SB_MSGID_WRAP_VALUE(PAYLOAD_IF_HK_TLM_MID), {0, 0}, 32},
        {CFE_SB_MSGID_WRAP_VALUE(PAYLOAD_IF_DEVICE_TLM_MID), {0, 0}, 32},
        {CFE_SB_MSGID_WRAP_VALUE(PAYLOAD_IF_PAYOBC_TLM_MID), {0, 0}, 32},
#endif
#ifdef GENERIC_EPS_HK_TLM_MID
        {CFE_SB_MSGID_WRAP_VALUE(GENERIC_EPS_HK_TLM_MID), {0, 0}, 32},
#endif
#ifdef TMP100_HK_TLM_MID
        {CFE_SB_MSGID_WRAP_VALUE(TMP100_HK_TLM_MID), {0, 0}, 4},
        {CFE_SB_MSGID_WRAP_VALUE(TMP100_DEVICE_TLM_MID), {0, 0}, 4},
#endif
#ifdef THERMAL_HK_TLM_MID
        {CFE_SB_MSGID_WRAP_VALUE(THERMAL_HK_TLM_MID), {0, 0}, 4},
#endif
#ifdef MGR_HK_TLM_MID
        {CFE_SB_MSGID_WRAP_VALUE(MGR_HK_TLM_MID), {0, 0}, 32},
#endif
    }};

CFE_TBL_FILEDEF(TO_LAB_Subs, TO_LAB_APP.TO_LAB_Subs, TO Lab Sub Tbl, to_lab_sub.tbl)
