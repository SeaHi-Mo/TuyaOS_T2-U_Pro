/**
 * @file tdd_led_gpio_driver.h
 * @author www.tuya.com
 * @brief tdd_led_gpio module is used to
 * @version 1.0.0
 * @date 2022-05-16
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#ifndef __TDD_LED_GPIO_DRIVER_H__
#define __TDD_LED_GPIO_DRIVER_H__

#include "tuya_cloud_types.h"
#include "tdl_led_manage.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
*************************micro define***********************
***********************************************************/
#define TDD_LED_STAT_E UCHAR_T
#define TDD_LED_OFF    0 // 熄灭
#define TDD_LED_ON     1 // 亮
#define TDD_LED_FLASH  2 // 闪烁

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    TUYA_GPIO_NUM_E pin;
    TUYA_GPIO_MODE_E mode;
    TUYA_GPIO_LEVEL_E level;
} LED_GPIO_CFG_T;

/***********************************************************
***********************variable define**********************
***********************************************************/

/***********************************************************
***********************function define**********************
***********************************************************/
/**
 * @brief TDD初始化GPIO类型
 *
 * @param[in] handle 与tdl绑定handle
 * @param[in] led_cfg gpio引脚及配置项
 * @return OPRT_OK 代表成功。如果是其他错误码，请参考 tuya_error_code.h
 */
OPERATE_RET tdd_led_gpio_init(CHAR_T *dev_name, IN LED_GPIO_CFG_T led_cfg);

/**
 * @brief TDD对GPIO操作
 *
 * @param[in] handle TDD管理句柄
 * @param[in] stat 状态 支持开、关、闪烁
 * @return OPRT_OK 代表成功。如果是其他错误码，请参考 tuya_error_code.h
 */
OPERATE_RET tdd_led_gpio_write(IN PVOID_T handle, IN TDD_LED_STAT_E stat);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__TDD_LED_GPIO_H__*/