/**
 * @file tdd_dimmer_pwm.h
 * @author www.tuya.com
 * @brief tdd_dimmer_pwm module is used to driver led by PWM
 * @version 0.1
 * @date 2022-08-25
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#ifndef __TDD_DIMMER_PWM_H__
#define __TDD_DIMMER_PWM_H__

#include "tuya_cloud_types.h"
#include "tdl_dimmer_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
/**
 * @brief pwm frequency limit
 */
#define DIMMER_PWM_FREQ_MAX                20000   // max: 20kHz
#define DIMMER_PWM_FREQ_MIN                100     // min: 100Hz

/**
 * @brief pwm duty limit
 */
#define DIMMER_PWM_DUTY_MAX                10000   // max: 100%

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef BYTE_T DIMMER_PWM_DRV_TP_E;
#define DIMMER_PWM_DRV_TP_CW     0x00        // CW : cw & ww
#define DIMMER_PWM_DRV_TP_CCT    0x01        // CCT: cct & bright
#define DIMMER_PWM_DRV_TP_CW_NC  0x02        // CW : cw & ww, non-complementary
#define DIMMER_PWM_DRV_TP_UNUSED 0x03        // invalid

/***********************************************************
********************function declaration********************
***********************************************************/
/**
 * @brief register pwm dimmer
 *
 * @param[in] name: dimmer name
 * @param[in] p_cfg: dimmer configuration
 * @param[in] pwm_tp: pwm driver type
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
OPERATE_RET tdd_dimmer_pwm_register(IN CHAR_T *name, IN DIMMER_PWM_CFG_T *p_cfg, DIMMER_PWM_DRV_TP_E pwm_tp);

#ifdef __cplusplus
}
#endif

#endif /* __TDD_DIMMER_PWM_H__ */
