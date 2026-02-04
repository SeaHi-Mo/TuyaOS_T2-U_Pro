/**
 * @file tdd_energy_monitor_hlw8032.h
 * @author www.tuya.com
 * @brief tdl_energy_monitor_hlw8032 module is used to 
 * @version 0.1
 * @date 2023-07-14
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TDD_ENERGY_MONITOR_HLW8032_H__
#define __TDD_ENERGY_MONITOR_HLW8032_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define UART_BUFFER_SIZE_MAX    (24*4)

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    TUYA_UART_NUM_E uart_id;
}HLW8032_DRIVER_CONFIG_T;

/***********************************************************
********************function declaration********************
***********************************************************/

OPERATE_RET tdd_energy_driver_hlw8032_register(IN CHAR_T *name, IN HLW8032_DRIVER_CONFIG_T cfg);

#ifdef __cplusplus
}
#endif

#endif /* __TDD_ENERGY_MONITOR_HLW8032_H__ */
