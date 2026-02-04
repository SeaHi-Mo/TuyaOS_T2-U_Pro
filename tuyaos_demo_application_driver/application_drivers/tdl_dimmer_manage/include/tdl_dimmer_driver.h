/**
 * @file tdl_dimmer_driver.h
 * @author www.tuya.com
 * @brief tdl_dimmer_driver module is used to provid a unified
 *        dimmer driver access standard for the driver layer
 * @version 0.1
 * @date 2022-06-01
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#ifndef __TDL_DIMMER_DRIVER_H__
#define __TDL_DIMMER_DRIVER_H__

#include "tdu_light_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define DIMMER_PWM_ID_INVALID               0xFE

#define DIMMER_PWM_NUM_MAX                  5u

#define DIMMER_PWM_CH_IDX_RED               0
#define DIMMER_PWM_CH_IDX_GREEN             1
#define DIMMER_PWM_CH_IDX_BLUE              2
#define DIMMER_PWM_CH_IDX_COLD              3  //cct: bright
#define DIMMER_PWM_CH_IDX_WARM              4  //cct: temper

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef UCHAR_T DIMMER_DRIVER_TYPE_E;
#define DIMMER_DRIVER_PWM_CW                      0
#define DIMMER_DRIVER_PWM_CCT                     1
#define DIMMER_DRIVER_IIC                         2

typedef UCHAR_T DIMMER_CHIP_TYPE_E;
#define DIMMER_CHIP_PWM                           0
#define DIMMER_CHIP_SM16726B                      1 
#define DIMMER_CHIP_SM2135E                       2 
#define DIMMER_CHIP_SM2135EH                      3 
#define DIMMER_CHIP_SM2135EJ                      4 
#define DIMMER_CHIP_BP1658CJ                      5
#define DIMMER_CHIP_BP5758D                       6
#define DIMMER_CHIP_SM2235EGH                     7
#define DIMMER_CHIP_SM2335EGH                     8
#define DIMMER_CHIP_KP18058ESPA                   9
#define DIMMER_CHIP_KP18059ESPA                   10
  
#define DIMMER_CHIP_IIC_START                     DIMMER_CHIP_SM16726B
#define DIMMER_CHIP_IIC_END                       DIMMER_CHIP_KP18059ESPA

typedef UCHAR_T DIMMER_DRV_CMD_E;
#define DRV_CMD_GET_HARDWARE_CFG                  0x01

typedef struct {
    TUYA_GPIO_NUM_E scl;
    TUYA_GPIO_NUM_E sda;    
}DIMMER_SW_I2C_CFG_T;

typedef struct {
    UINT_T                  pwm_freq;       // pwm frequency (Hz)
    BOOL_T                  active_level;   // true means active high, false means active low
    TUYA_PWM_NUM_E          pwm_ch_arr[DIMMER_PWM_NUM_MAX];   // pwm id of each channel
} DIMMER_PWM_CFG_T;

typedef union {
    DIMMER_SW_I2C_CFG_T     i2c_cfg;
    DIMMER_PWM_CFG_T        pwm_cfg;
}DIMMER_HARDWARE_CFG_U;

/**
 * @brief driver handle
 */
typedef VOID_T* DIMMER_DRV_HANDLE;

/**
 * @brief driver interfaces abstraction
 */
typedef struct {
    OPERATE_RET (*open  )(IN DIMMER_DRV_HANDLE drv_handle);
    OPERATE_RET (*close )(IN DIMMER_DRV_HANDLE drv_handle);
    OPERATE_RET (*output)(IN DIMMER_DRV_HANDLE drv_handle, LIGHT_RGBCW_U *p_rgbcw);
    OPERATE_RET (*config)(IN DIMMER_DRV_HANDLE drv_handle, DIMMER_DRV_CMD_E cmd, VOID_T *arg);
} DIMMER_DRV_INTFS_T;

typedef struct {
    USHORT_T              resolution;
    DIMMER_DRIVER_TYPE_E  drv_tp;
    DIMMER_CHIP_TYPE_E    chip_tp;
    LIGHT_DRV_COLOR_CH_E  color_ch;
}DIMMER_DRV_INFO_T;

/***********************************************************
********************function declaration********************
***********************************************************/
/**
 * @brief register dimmer
 *
 * @param[in] name: dimmer name
 * @param[in] p_intfs: interfaces for operating the dimmer
 * @param[in] p_info: dimmer driver information
 * @param[in] drv_handle: dimmer driver handle
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tdl_dimmer_register(IN CHAR_T *name, IN DIMMER_DRV_INTFS_T *p_intfs, DIMMER_DRV_INFO_T *p_info, \
                                IN DIMMER_DRV_HANDLE drv_handle);

#ifdef __cplusplus
}
#endif

#endif /* __TDL_DIMMER_DRIVER_H__ */
