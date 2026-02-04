/**
 * @file example_drv_dimmer.c
 * @author www.tuya.com
 * @brief example_drv_dimmer module is used to 
 * @version 0.1
 * @date 2022-10-12
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */
#include <string.h>

#include "tuya_cloud_types.h"
#include "tal_log.h"
#include "tal_system.h"
#include "tal_thread.h"

#if defined(ENABLE_PWM) && (ENABLE_PWM)
#include "tdd_dimmer_pwm.h"
#endif

#include "tdd_dimmer_bp1658cj.h"
#include "tdd_dimmer_bp5758d.h"
#include "tdd_dimmer_kp1805x.h"
#include "tdd_dimmer_sm2x35egh.h"
#include "tdd_dimmer_sm2135e.h"
#include "tdd_dimmer_sm2135ex.h"

#include "tdl_dimmer_manage.h"
#include "example_drv_dimmer.h"
/***********************************************************
************************macro define************************
***********************************************************/
#define LIGHT_COLOR_VALUE              1000u

/***********************************************************
*********************** const define ***********************
***********************************************************/

/***********************************************************
***********************variable define**********************
***********************************************************/
#if defined(ENABLE_PWM) && (ENABLE_PWM)
STATIC DIMMER_PWM_CFG_T sg_pwm_cfg = {
    .active_level = TRUE,
    .pwm_freq  = 1000,
    .pwm_ch_arr = {
        TUYA_PWM_NUM_5,
        TUYA_PWM_NUM_4,
        TUYA_PWM_NUM_0,
        TUYA_PWM_NUM_2,
        TUYA_PWM_NUM_1,
    },
};
#endif

// STATIC TDD_I2C_PIN_T sg_i2c_pin = {
//     .scl = TUYA_GPIO_NUM_26,
//     .sda = TUYA_GPIO_NUM_6,
// };

// STATIC DIMMER_CH_CFG_U sg_channel_cfg = {
//     .r = DIMMER_CH_ID_3,
//     .g = DIMMER_CH_ID_2,
//     .b = DIMMER_CH_ID_1,
//     .c = DIMMER_CH_ID_5,
//     .w = DIMMER_CH_ID_4,
// };

// STATIC BP1658CJ_CFG_T sg_bp1658cj_cfg = {
//     .max_curr_code.color = 10,
//     .max_curr_code.white = 10,
// };

// STATIC BP5758D_CFG_T sg_bp5758d_cfg = {
//     .max_curr_code.r = 10,
//     .max_curr_code.g = 10,
//     .max_curr_code.b = 10,
//     .max_curr_code.c = 10,   
//     .max_curr_code.w = 10,
// };

// STATIC DIMMER_KP1805X_CFG_T sg_kp18058_cfg = {
//     .max_curr_code.color = 2,
//     .max_curr_code.white = 4,
// };

// STATIC DIMMER_KP1805X_CFG_T sg_kp18059_cfg = {
//     .max_curr_code.color = 2,
//     .max_curr_code.white = 4,
// };

// STATIC SM2X35EGH_CFG_T sg_sm2x35egh_cfg = {
//     .max_curr_code.color = 2,
//     .max_curr_code.white = 4,
// };

// STATIC SM2135E_CFG_T sg_sm2135e_cfg = {
//     .max_curr_code.color = 2,
//     .max_curr_code.white = 4,
// };

// STATIC SM2135EX_CFG_T sg_sm2135ex_cfg = {
//     .max_curr_code.color = 2,
//     .max_curr_code.white = 4,
// };

STATIC THREAD_HANDLE     sg_dimmer_thrd = NULL;
STATIC TDL_DIMMER_HANDLE sg_dimmer_hdl  = NULL;
STATIC LIGHT_RGBCW_U sg_color = {
    .s.red = LIGHT_COLOR_VALUE,
};

/***********************************************************
***********************function define**********************
***********************************************************/
STATIC VOID_T __dimmer_task(void *arg)
{
    OPERATE_RET rt =  OPRT_OK;
    UINT_T idx = 0;
    DIMMER_DRIVER_TYPE_E type = 0;

    // get driver type
    tdl_dimmer_config(sg_dimmer_hdl, DIMMER_CMD_GET_DRV_TYPE, &type);

    for (;;) {

        if(DIMMER_DRIVER_PWM_CCT == type) {
            /* cct driver: cold channel -> light brightness channel
                           warm channel -> light temperature channel */
            if(COLOR_CH_IDX_COLD == idx) {
                //cold light
                sg_color.array[COLOR_CH_IDX_COLD] = LIGHT_COLOR_VALUE; //brightness max value
                sg_color.array[COLOR_CH_IDX_WARM] = LIGHT_COLOR_VALUE; //temperature max value
            }else if(COLOR_CH_IDX_WARM == idx) {
                //warm light
                sg_color.array[COLOR_CH_IDX_COLD] = LIGHT_COLOR_VALUE; //brightness max value
                sg_color.array[COLOR_CH_IDX_WARM] = 0;                 //temperature min value
            }else {
                sg_color.array[idx] = LIGHT_COLOR_VALUE; 
            }
        }else {
            sg_color.array[idx] = LIGHT_COLOR_VALUE; 
        }


        TAL_PR_NOTICE("array[%d] = %d", idx, sg_color.array[idx]);
        TUYA_CALL_ERR_LOG(tdl_dimmer_output(sg_dimmer_hdl, &sg_color.s));

        tal_system_sleep(2000);

        //关
        memset((UCHAR_T *)sg_color.array, 0x00, SIZEOF(LIGHT_RGBCW_U));
        TUYA_CALL_ERR_LOG(tdl_dimmer_output(sg_dimmer_hdl, &sg_color.s));

        tal_system_sleep(1000);

        //切换下一个颜色
        idx = ((idx+1) % LIGHT_COLOR_CHANNEL_MAX);
    }

    return;
}

/**
 * @brief    register hardware 
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET reg_dimmer_hardware(CHAR_T *device_name)
{
    OPERATE_RET rt = OPRT_OK;

#if defined(ENABLE_PWM) && (ENABLE_PWM)
    //cw
    // TUYA_CALL_ERR_RETURN(tdd_dimmer_pwm_register(device_name, &sg_pwm_cfg, DIMMER_PWM_DRV_TP_CW));

    //cct
     TUYA_CALL_ERR_RETURN(tdd_dimmer_pwm_register(device_name, &sg_pwm_cfg, DIMMER_PWM_DRV_TP_CCT));
#endif


    // TUYA_CALL_ERR_RETURN(tdd_dimmer_bp1658cj_register(device_name, &sg_i2c_pin, &sg_channel_cfg, &sg_bp1658cj_cfg));

    // TUYA_CALL_ERR_RETURN(tdd_dimmer_bp5758d_register(device_name, &sg_i2c_pin, &sg_channel_cfg, &sg_bp5758d_cfg)); 

    // TUYA_CALL_ERR_RETURN(tdd_dimmer_kp18058_register(device_name, &sg_i2c_pin, &sg_channel_cfg, &sg_kp18058_cfg));

    // TUYA_CALL_ERR_RETURN(tdd_dimmer_kp18059_register(device_name, &sg_i2c_pin, &sg_channel_cfg, &sg_kp18059_cfg));

    // TUYA_CALL_ERR_RETURN(tdd_dimmer_sm2235egh_register(device_name, &sg_i2c_pin, &sg_channel_cfg, &sg_sm2x35egh_cfg));

    // TUYA_CALL_ERR_RETURN(tdd_dimmer_sm2335egh_register(device_name, &sg_i2c_pin, &sg_channel_cfg, &sg_sm2x35egh_cfg));

    // TUYA_CALL_ERR_RETURN(tdd_dimmer_sm2135e_register(device_name, &sg_i2c_pin, &sg_channel_cfg, &sg_sm2135e_cfg));

    // TUYA_CALL_ERR_RETURN(tdd_dimmer_sm2135ej_register(device_name, &sg_i2c_pin, &sg_channel_cfg, &sg_sm2135ex_cfg));

    // TUYA_CALL_ERR_RETURN(tdd_dimmer_sm2135eh_register(device_name, &sg_i2c_pin, &sg_channel_cfg, &sg_sm2135ex_cfg));

    return OPRT_OK;
}

/**
 * @brief    open driver
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET open_dimmer_driver(CHAR_T *device_name)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CALL_ERR_RETURN(tdl_dimmer_find(device_name, &sg_dimmer_hdl));

    TUYA_CALL_ERR_RETURN(tdl_dimmer_open(sg_dimmer_hdl, LIGHT_COLOR_VALUE));

    return OPRT_OK;
}

/**
 * @brief    the example of driver function
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET example_dimmer_running(CHAR_T *device_name)
{
    THREAD_CFG_T thrd_param = {
        .thrdname = "dimmer_drv",
        .priority = THREAD_PRIO_2,
        .stackDepth = 1024
    };

    return tal_thread_create_and_start(&sg_dimmer_thrd, NULL, NULL, __dimmer_task, NULL, &thrd_param);
}