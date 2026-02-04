/**
 * @file app_led.c
 * @author www.tuya.com
 * @brief app_led module is used to 
 * @version 0.1
 * @date 2022-10-28
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#include "tal_log.h"
#include "tkl_gpio.h"
#include "tal_thread.h"
#include "app_led.h"
#include "dp_process.h"
#include "tuya_svc_netmgr.h"

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
STATIC TUYA_GPIO_NUM_E sg_led_pin = 0;
STATIC UINT8_T cur_led_status = LED_OFF;

THREAD_HANDLE blink_task_handle = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/

/**
 * @brief set led
 *
 * @param[in] led_status: LED_ON: led turn on, LED_OFF: led turn off
 * @return none
 */
VOID_T set_led_status(LED_STATUS_E led_status)
{
    if (LED_ON == led_status) {
        tkl_gpio_write(sg_led_pin, TUYA_GPIO_LEVEL_HIGH);
    }
    else {
        tkl_gpio_write(sg_led_pin, TUYA_GPIO_LEVEL_LOW);
    }

    cur_led_status = led_status;
    return;
}

/**
 * @brief get led status, on or off
 *
 * @param[in] none
 * @return current led status
 */
LED_STATUS_E get_led_status(void)
{
    return cur_led_status;
}

/**
 * @brief Blinking LED when WiFi is in a wiring state
 *
 * @param[in] args: blink interval
 * @return none
 */
STATIC VOID_T status_display_task(VOID_T *args)
{
    for(;;) {
        GW_WORK_STAT_T stat = get_gw_active();
        BOOL_T network_is_connected = tuya_svc_netmgr_linkage_is_up(LINKAGE_TYPE_DEFAULT);
        TAL_PR_DEBUG("network_is_connected: %d", network_is_connected);
        if (network_is_connected == 0 && stat != ACTIVATED) { // network is not connected and not activated
            set_led_status(LED_ON);
            tal_system_sleep(1000);
            set_led_status(LED_OFF);
            tal_system_sleep(1000);
        } else {
            tal_system_sleep(3000);
        }
    }

    return;
}



/**
 * @brief led init
 *
 * @param[in] pin_id: led pin id
 * 
 * @return none
 */
VOID_T app_led_init(TUYA_GPIO_NUM_E pin_id)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_GPIO_BASE_CFG_T led_cfg = {
        .mode = TUYA_GPIO_PUSH_PULL,
        .direct = TUYA_GPIO_OUTPUT,
        .level = TUYA_GPIO_LEVEL_HIGH
    };
    TUYA_CALL_ERR_LOG(tkl_gpio_init(pin_id, &led_cfg));
    sg_led_pin = pin_id;
    set_led_status(LED_OFF);

    THREAD_CFG_T blink_thread_cfg = {
        .thrdname = "led_task",
        .priority = THREAD_PRIO_6,
        .stackDepth = 4096
    };
    TUYA_CALL_ERR_LOG(tal_thread_create_and_start(&blink_task_handle, NULL, NULL, status_display_task, NULL, &blink_thread_cfg));
    return;
}
