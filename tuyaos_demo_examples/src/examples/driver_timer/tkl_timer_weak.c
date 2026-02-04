/**
 * @file tkl_timer_weak.c
 * @author www.tuya.com
 * @version 0.1
 * @date 2024-05-17
 *
 * @copyright Copyright (c) tuya.inc 2024
 *
 */

#include "tuya_iot_config.h"

#if defined(ENABLE_TIMER) && (ENABLE_TIMER)
#include "tuya_cloud_types.h"
#include "tkl_timer.h"

/***********************************************************
*********************** macro define ***********************
***********************************************************/


/***********************************************************
********************** typedef define **********************
***********************************************************/


/***********************************************************
********************** variable define *********************
***********************************************************/


/***********************************************************
********************** function define *********************
***********************************************************/
/**
 * @brief timer init
 * 
 * @param[in] timer_id timer id
 * @param[in] cfg timer configure
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_timer_init(TUYA_TIMER_NUM_E timer_id, TUYA_TIMER_BASE_CFG_T *cfg)
{
    return OPRT_NOT_SUPPORTED;     
}

/**
 * @brief timer start
 * 
 * @param[in] timer_id timer id
 * @param[in] us when to start
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_timer_start(TUYA_TIMER_NUM_E timer_id, UINT_T us)
{
    return OPRT_NOT_SUPPORTED;      
}

/**
 * @brief timer stop
 * 
 * @param[in] timer_id timer id
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_timer_stop(TUYA_TIMER_NUM_E timer_id)
{
    return OPRT_NOT_SUPPORTED;     
}

/**
 * @brief timer deinit
 * 
 * @param[in] timer_id timer id
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_timer_deinit(TUYA_TIMER_NUM_E timer_id)
{
    return OPRT_NOT_SUPPORTED;     
}

#endif