/**
 * @file example_os_event.c
 * @author www.tuya.com
 * @brief example_os_event module is used to 
 * @version 0.1
 * @date 2023-10-07
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_cloud_types.h"

#include "tal_log.h"

#include "base_event.h"
#include "base_event_info.h"

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/


/***********************************************************
***********************function define**********************
***********************************************************/

STATIC OPERATE_RET db_init_start_cb(VOID_T *data)
{
    TAL_PR_DEBUG("---> db init start");
    return OPRT_OK;
}

STATIC OPERATE_RET db_init_finish_cb(VOID_T *data)
{
    TAL_PR_DEBUG("---> db init finish");
    return OPRT_OK;
}

VOID example_os_event_subscribe(VOID_T)
{
    ty_subscribe_event(EVENT_SDK_EARLY_INIT_OK, "db init start", db_init_start_cb, SUBSCRIBE_TYPE_NORMAL);
    ty_subscribe_event(EVENT_SDK_DB_INIT_OK, "db init finish", db_init_finish_cb, SUBSCRIBE_TYPE_NORMAL);
    return;
}
