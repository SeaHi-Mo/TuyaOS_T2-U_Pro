/**
 * @file tdd_led_gpio.c
 * @author www.tuya.com
 * @brief tdd_led_gpio module is used to
 * @version 1.0.0
 * @date 2022-05-16
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#include "tdd_led_gpio_driver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tal_memory.h"
#include "tkl_gpio.h"
#include "tal_log.h"
#include "tuya_list.h"

/***********************************************************
*************************micro define***********************
***********************************************************/

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    UCHAR_T pin;
    BOOL_T is_high_drv;
    struct tuya_list_head node; // 节点
} TDD_LED_GPIO_MNG_T;

/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC LIST_HEAD(sg_tdd_list_head);

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
OPERATE_RET tdd_led_gpio_init(CHAR_T *dev_name, IN LED_GPIO_CFG_T led_cfg)
{
    OPERATE_RET rt = OPRT_OK;
    TUYA_GPIO_BASE_CFG_T gpio_cfg = {0};

    gpio_cfg.direct = TUYA_GPIO_OUTPUT;
    gpio_cfg.mode = led_cfg.mode;
    // tkl_gpio_init 初始化设置有效电平，取反使初始化成非有效状态
    gpio_cfg.level = !led_cfg.level;
    TUYA_CALL_ERR_RETURN(tkl_gpio_init(led_cfg.pin, &gpio_cfg));

    TDD_LED_GPIO_MNG_T *tdd_led_cfg = (TDD_LED_GPIO_MNG_T *)tal_malloc(sizeof(TDD_LED_GPIO_MNG_T));
    TUYA_CHECK_NULL_RETURN(tdd_led_cfg, OPRT_MALLOC_FAILED);

    tdd_led_cfg->pin = led_cfg.pin;
    tdd_led_cfg->is_high_drv = led_cfg.level;
    tuya_list_add(&tdd_led_cfg->node, &sg_tdd_list_head);

    TUYA_CALL_ERR_GOTO(tdl_led_register_function(dev_name, (PVOID_T)tdd_led_cfg, tdd_led_gpio_write), __ERR);

    return rt;

__ERR:
    if (NULL != tdd_led_cfg) {
        tal_free(tdd_led_cfg);
        tdd_led_cfg = NULL;
    }

    return rt;
}

/**
 * @brief 遍历查找所有LED链表
 *
 * @param[in] handle
 * @return 查找成功: 返回led handle  查找失败：返回NULL
 */
STATIC TDD_LED_GPIO_MNG_T *__find_led_handler(PVOID_T handle)
{
    TDD_LED_GPIO_MNG_T *entry = NULL;
    struct tuya_list_head *pos = NULL;
    tuya_list_for_each(pos, &sg_tdd_list_head)
    {
        entry = tuya_list_entry(pos, TDD_LED_GPIO_MNG_T, node);
        if (handle == entry) {
            return entry;
        }
    }
    return NULL;
}

/**
 * @brief TDD对GPIO操作
 *
 * @param[in] handle TDD管理句柄
 * @param[in] stat 状态 支持开、关、闪烁
 * @return OPRT_OK 代表成功。如果是其他错误码，请参考 tuya_error_code.h
 */
OPERATE_RET tdd_led_gpio_write(IN PVOID_T handle, IN TDD_LED_STAT_E stat)
{
    if ((NULL == handle) || (stat > TDD_LED_FLASH)) {
        return OPRT_INVALID_PARM;
    }

    TDD_LED_GPIO_MNG_T *tmp = __find_led_handler(handle);
    if (NULL == tmp) {
        return OPRT_NOT_FOUND;
    }

    BOOL_T set_stat = FALSE;
    if (stat == TDD_LED_OFF) {
        set_stat = FALSE;
    } else if (stat == TDD_LED_ON) {
        set_stat = TRUE;
    }
    
    return tkl_gpio_write(tmp->pin, (tmp->is_high_drv) ? set_stat : !set_stat);
}