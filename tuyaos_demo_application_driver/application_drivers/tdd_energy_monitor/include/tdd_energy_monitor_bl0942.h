/**
 * @file tdd_ele_energy_bl0942.h
 * @author www.tuya.com
 * @brief tdd_ele_energy_bl0942 module is used to 
 * @version 0.1
 * @date 2023-07-12
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TDD_ELE_ENERGY_BL0942_H__
#define __TDD_ELE_ENERGY_BL0942_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define BL0942_DRV_MODE_E UINT8_T
#define BL0942_DRV_UART     0
#define BL0942_DRV_SPI      1

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    TUYA_UART_NUM_E id;
    UINT8_T addr; // 0~3。由硬件片选地址管脚为[A1， A2_NCS]决定，内部默认下拉。
} BL0942_DRIVER_UART_T;

/* spi 暂时未实现 */
typedef struct {
    TUYA_SPI_NUM_E id;
    TUYA_GPIO_NUM_E cs_pin;
} BL0942_DRIVER_SPI_T;

typedef struct {
    BL0942_DRV_MODE_E mode;
    union {
        BL0942_DRIVER_UART_T uart;
        BL0942_DRIVER_SPI_T  spi;
    } driver;
} BL0942_DRIVER_CONFIG_T;

/***********************************************************
********************function declaration********************
***********************************************************/

OPERATE_RET tdd_energy_driver_bl0942_register(IN CHAR_T *name, IN BL0942_DRIVER_CONFIG_T cfg);

#ifdef __cplusplus
}
#endif

#endif /* __TDD_ELE_ENERGY_BL0942_H__ */
