/************************************************************************
** File:
**   $Id: mgr_platform_cfg.h  $
**
** Purpose:
**  Define mgr Platform Configuration Parameters
**
** Notes:
**
*************************************************************************/
#ifndef _MGR_PLATFORM_CFG_H_
#define _MGR_PLATFORM_CFG_H_

/*
** Default MGR Configuration
*/
#ifndef MGR_CFG
#define MGR_CFG_REBOOT_TIME_TIC_OFFSET 0
#define MGR_CFG_FILE_PATH              "/data/mgr.bin"
#endif

#ifndef MGR_CFG_RESTORE_LOCAL_TIME
#define MGR_CFG_RESTORE_LOCAL_TIME 1
#endif

#endif /* _MGR_PLATFORM_CFG_H_ */
