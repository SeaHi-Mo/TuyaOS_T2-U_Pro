/**
 * @file tdd_energy_monitor_bl0937_hlw8012.h
 * @author www.tuya.com
 * @brief tdl_energy_monitor_bl0937_hlw8012 module is used to 
 * @version 0.1
 * @date 2023-07-14
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TDD_ENERGY_MONITOR_BL0937_HLW8012_H__
#define __TDD_ENERGY_MONITOR_BL0937_HLW8012_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    TUYA_TIMER_NUM_E timer_id;
    TUYA_GPIO_NUM_E sel_pin;
/* sel 电压有效电平值:
 * sel_level 设置为 TUYA_GPIO_LEVEL_LOW：sel_pin 输入为 低电平 时，cf1_pin 输出电压有效值；sel_pin 输入为 高电平 时，cf1_pin 输出电流有效值。
 * sel_level 设置为 TUYA_GPIO_LEVEL_HIGH：sel_pin 输入为 高电平 时，cf1_pin 输出电压有效值；sel_pin 输入为 低电平 时，cf1_pin 输出电流有效值。
*/
    TUYA_GPIO_LEVEL_E sel_level;
    TUYA_GPIO_NUM_E cf_pin; // 有功功率
    TUYA_GPIO_NUM_E cf1_pin; // 电流或电压
}HLW8012_DRIVER_CONFIG_T, BL0937_DRIVER_CONFIG_T;

/***********************************************************
********************function declaration********************
***********************************************************/

OPERATE_RET tdd_energy_driver_hlw8012_register(IN CHAR_T *name, IN HLW8012_DRIVER_CONFIG_T cfg);

OPERATE_RET tdd_energy_driver_bl0937_register(IN CHAR_T *name, IN BL0937_DRIVER_CONFIG_T cfg);

#ifdef __cplusplus
}
#endif

#endif /* __TDD_ENERGY_MONITOR_BL0937_HLW8012_H__ */
