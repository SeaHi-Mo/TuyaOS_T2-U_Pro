/**
 * @file example_drv_led.c
 * @author www.tuya.com
 * @brief example_drv_led module is used to 
 * @version 0.1
 * @date 2022-09-28
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */
#include "tuya_cloud_types.h"
#include "tal_log.h"
#include "tal_system.h"

#include "tdd_led_gpio_driver.h"
#include "tdl_led_manage.h"
#include "example_drv_led.h"
/***********************************************************
************************macro define************************
***********************************************************/
#define LED_PIN     TUYA_GPIO_NUM_26

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
/**
 * @brief    register hardware 
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET reg_led_hardware(CHAR_T *device_name)
{
    LED_GPIO_CFG_T led_hw_cfg = {
        .pin   = LED_PIN,
        .mode  = TUYA_GPIO_PUSH_PULL,
        .level = TUYA_GPIO_LEVEL_LOW,
    };

    return tdd_led_gpio_init(device_name, led_hw_cfg);
}

/**
 * @brief    the example of driver function
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET example_led_running(CHAR_T *device_name)
{
    OPERATE_RET rt = OPRT_OK;
    PVOID_T led_hdl = NULL;
    TDL_LED_CONFIG_T led_cfg = {0};

    TUYA_CALL_ERR_RETURN(tdl_led_dev_find(device_name, &led_hdl));

    // turn on LED
    led_cfg.stat = TDL_LED_ON;
    TUYA_CALL_ERR_RETURN(tdl_led_ctrl(led_hdl, &led_cfg));
    TAL_PR_NOTICE("led on");

    tal_system_sleep(3000);

    // turn off LED
    led_cfg.stat = TDL_LED_OFF;
    TUYA_CALL_ERR_RETURN(tdl_led_ctrl(led_hdl, &led_cfg));
    TAL_PR_NOTICE("led off");

    tal_system_sleep(3000);

    // LED flash
    led_cfg.stat       = TDL_LED_FLASH;
    led_cfg.start_stat = FALSE;
    led_cfg.end_stat   = FALSE;
    led_cfg.flash_cnt  = TDL_FLASH_FOREVER;
    led_cfg.flash_first_time  = 500;
    led_cfg.flash_second_time = 500;
    TUYA_CALL_ERR_RETURN(tdl_led_ctrl(led_hdl, &led_cfg));
    TAL_PR_NOTICE("led flash");

    return OPRT_OK;
}
