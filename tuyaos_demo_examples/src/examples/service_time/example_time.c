/**
 * @file example_sw_timer.c
 * @author www.tuya.com
 * @brief example_sw_timer module is used to 
 * @version 0.1
 * @date 2022-11-21
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#include "tuya_cloud_types.h"
#include "tal_log.h"
#include "tal_sw_timer.h"
#include "tal_time_service.h"

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
STATIC TIMER_ID sw_timer_id = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/
/**
 * @brief software timer callback
 *
 * @param[in] : 
 *
 * @return none
 */
STATIC VOID_T __timer_cb(TIMER_ID timer_id, VOID_T *arg)
{
    OPERATE_RET rt = OPRT_OK;
    POSIX_TM_S local_tm;

    if(OPRT_OK == tal_time_check_time_sync()) {
        memset((UCHAR_T *)&local_tm, 0x00, SIZEOF(POSIX_TM_S));

        rt = tal_time_get_local_time_custom(0, &local_tm);
        if(OPRT_OK == rt) {
            TAL_PR_NOTICE("get local time");
            TAL_PR_NOTICE("year: %d", local_tm.tm_year+1900);
            TAL_PR_NOTICE("month: %d", local_tm.tm_mon+1);
            TAL_PR_NOTICE("day: %d", local_tm.tm_mday);
            TAL_PR_NOTICE("week: %d", local_tm.tm_wday);
            TAL_PR_NOTICE("%d:%d:%d", local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
        }else {
            TAL_PR_ERR("get local time error ret:%d", rt);
        }

        tal_sw_timer_start(sw_timer_id, 60*1000, TAL_TIMER_ONCE);
    }else {
        TAL_PR_NOTICE("device time is not yet synchronized with the cloud");

        tal_sw_timer_start(sw_timer_id, 10000, TAL_TIMER_ONCE);
    }
}

/**
 * @brief time service example
 *
 * @param[in] : 
 *
 * @return none
 */
VOID example_time_service(INT_T argc, CHAR_T *argv[])
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CALL_ERR_GOTO(tal_sw_timer_create(__timer_cb, NULL, &sw_timer_id), __EXIT);
    TUYA_CALL_ERR_LOG(tal_sw_timer_start(sw_timer_id, 3000, TAL_TIMER_ONCE));

__EXIT:
    return;
}