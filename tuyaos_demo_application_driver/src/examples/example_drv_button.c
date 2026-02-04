/**
 * @file example_drv_button.c
 * @author www.tuya.com
 * @version 0.1
 * @date 2022-09-28
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */
#include "tuya_cloud_wifi_defs.h"
#include "tal_log.h"

#include "tdd_button_gpio.h"
#include "tdl_button_manage.h"

#include "example_drv_button.h"
/***********************************************************
************************macro define************************
***********************************************************/
#define RESET_BUTTON_PIN    TUYA_GPIO_NUM_7 

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC TDL_BUTTON_HANDLE sg_button_hdl = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/
STATIC VOID __button_function_cb(IN CHAR_T *name, IN TDL_BUTTON_TOUCH_EVENT_E event, IN VOID* argc)
{
    switch (event) {
        case TDL_BUTTON_PRESS_DOWN: {
            TAL_PR_NOTICE("single click");
        }
        break;
        
        case TDL_BUTTON_LONG_PRESS_START: {
            TAL_PR_NOTICE("long press");
        }
        break;

        default:break;
    }
}

/**
 * @brief    register hardware 
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET reg_button_hardware(CHAR_T *device_name)
{
    //hardware register
    BUTTON_GPIO_CFG_T button_hw_cfg = {
        .pin   = RESET_BUTTON_PIN,
        .level = TUYA_GPIO_LEVEL_LOW,
        .mode  = BUTTON_TIMER_SCAN_MODE,
    };

    return tdd_gpio_button_register(device_name, &button_hw_cfg); 
}

/**
 * @brief    open driver
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET open_button_driver(CHAR_T *device_name)
{
    TDL_BUTTON_CFG_T button_cfg = {
        .long_start_vaild_time     = 3000,
        .long_keep_timer           = 1000,
        .button_debounce_time      = 50,
        .button_repeat_vaild_count = 3,
        .button_repeat_vaild_time  = 50
    };

    return tdl_button_create(device_name, &button_cfg, &sg_button_hdl);
}

/**
 * @brief    the example of driver function
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET example_button_running(CHAR_T *device_name)
{
    /* register button single click event, long press event */
    tdl_button_event_register(sg_button_hdl, TDL_BUTTON_PRESS_DOWN,        __button_function_cb);
    tdl_button_event_register(sg_button_hdl, TDL_BUTTON_LONG_PRESS_START,  __button_function_cb);

    return OPRT_OK;   
}
