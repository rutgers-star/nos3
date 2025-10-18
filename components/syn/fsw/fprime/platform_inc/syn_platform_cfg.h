/************************************************************************
** File:
**   $Id: syn_platform_cfg.h  $
**
** Purpose:
**  Define syn Platform Configuration Parameters
**
** Notes:
**
*************************************************************************/
#ifndef _SYN_PLATFORM_CFG_H_
#define _SYN_PLATFORM_CFG_H_

/*
** Default SYN Configuration
*/
#ifndef SYN_CFG
    /* Notes: 
    **   NOS3 uart requires matching handle and bus number
    */
    #define SYN_CFG_STRING           "usart_29"
    #define SYN_CFG_HANDLE           29 
    #define SYN_CFG_BAUDRATE_HZ      115200
    #define SYN_CFG_MS_TIMEOUT       50            /* Max 255 */

    /* SYNOPSIS Example Defines*/    
    #define SYN_DATABASE_PATH "/data/owls/db_new.db"
    #define SYN_DATABASE_CLEAN_PATH "/data/owls/db_blank.db"

    #define NUMDPS 8  // This is a dumb counter for this example.  Users will need to handle how data is utilized within Synopsis.
    #define MAX_DL_SIZE 100
    #define MIN_DL_SIZE 0

    /* Note: Debug flag disabled (commented out) by default */
    //#define SYN_CFG_DEBUG
#endif

#endif /* _SYN_PLATFORM_CFG_H_ */
