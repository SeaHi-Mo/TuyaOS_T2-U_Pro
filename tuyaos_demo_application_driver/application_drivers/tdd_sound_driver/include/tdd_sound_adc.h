/**
 * @file tdd_sound_adc.h
 * @author www.tuya.com
 * @brief tdd_sound_adc module is used to sample sound driver (adc)
 * @version 0.1
 * @date 2022-04-01
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#ifndef __TDD_SOUND_ADC_H__
#define __TDD_SOUND_ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tkl_adc.h"
#include "tkl_pinmux.h"
/***********************************************************
*************************micro define***********************
***********************************************************/

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    TUYA_ADC_NUM_E adc_num;
    unsigned int adc_max;
    unsigned int pin;
} ADC_DRIVER_CONFIG_T;

/***********************************************************
***********************variable define**********************
***********************************************************/

/***********************************************************
***********************function define**********************
***********************************************************/
/**
 * @brief               音量adc采集设备注册
 *
 * @param[in]            dev_name         设备名字
 * @param[in]            port             adc端口号
 * @param[in]            adc_max          adc最大值
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
int tdd_sound_adc_register(char *dev_name, ADC_DRIVER_CONFIG_T *init_param);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*__TDD_SOUND_ADC_H__*/