/**
 * @file example_drv_sound.c
 * @author www.tuya.com
 * @brief example_drv_sound module is used to
 * @version 0.1
 * @date 2022-09-28
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */
#include "tuya_cloud_types.h"

#include "tal_system.h"
#include "tal_log.h"
#include "tal_thread.h"

#if defined(ENABLE_ADC) && (ENABLE_ADC)
#include "tdd_sound_adc.h"
#endif

#include "tdl_sound_sample.h"
#include "example_drv_sound.h"

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
STATIC SOUND_HANDLE_T sg_sound_hdl = NULL;
STATIC THREAD_HANDLE sg_sound_thrd = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/
STATIC VOID_T __sound_task(PVOID_T args)
{
    UINT32_T sound_permillage = 0;

    while(1){
        tdl_sound_dev_sample(sg_sound_hdl, S_DATA_PERCENT, 1, &sound_permillage);

        TAL_PR_NOTICE("sound permillage: %d", sound_permillage);

        tal_system_sleep(500);
    }

    tal_thread_delete(sg_sound_hdl);
    sg_sound_hdl = NULL;  

    return;  
}

/**
 * @brief    register hardware 
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET reg_sound_hardware(CHAR_T *device_name)
{
#if defined(ENABLE_ADC) && (ENABLE_ADC)
    /* sample voice device hardware register */
    ADC_DRIVER_CONFIG_T hardware_cfg = {
        .adc_max = 4096,
        .adc_num = TUYA_ADC_NUM_0,
        .pin     = TUYA_IO_PIN_23,
    };

    return tdd_sound_adc_register(device_name, &hardware_cfg);
#else 
    return OPRT_NOT_SUPPORTED;
#endif
}

/**
 * @brief    open driver
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET open_sound_driver(CHAR_T *device_name)
{
    OPERATE_RET rt = OPRT_OK;

    /* find sample voice device */
    TUYA_CALL_ERR_RETURN(tdl_sound_dev_find(device_name, &sg_sound_hdl));

    /* open sample voice device */
    TUYA_CALL_ERR_RETURN(tdl_sound_dev_open(sg_sound_hdl, S_VAL_AVG, 5));

    return OPRT_OK;
}

/**
 * @brief    the example of driver function
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET example_sound_running(CHAR_T *device_name)
{
    THREAD_CFG_T task_cfg = {
        .thrdname   = "sound",
        .priority   = THREAD_PRIO_1,
        .stackDepth = 1024,
    };

    return tal_thread_create_and_start(&sg_sound_thrd, NULL, NULL, __sound_task, NULL, &task_cfg);
}