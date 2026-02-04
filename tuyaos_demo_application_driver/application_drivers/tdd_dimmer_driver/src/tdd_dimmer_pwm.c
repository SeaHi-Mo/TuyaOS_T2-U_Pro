/**
 * @file tdd_dimmer_pwm.c
 * @author www.tuya.com
 * @brief tdd_dimmer_pwm module is used to drive led by PWM
 * @version 0.1
 * @date 2022-08-26
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */
#include <string.h>

#include "tkl_pwm.h"
#include "tal_memory.h"
#include "tal_log.h"

#include "tdl_dimmer_driver.h"
#include "tdd_dimmer_pwm.h"

/***********************************************************
************************macro define************************
***********************************************************/

/***********************************************************
***********************typedef define***********************
***********************************************************/

/***********************************************************
***********************variable define**********************
***********************************************************/

/***********************************************************
***********************function define**********************
***********************************************************/
/**
 * @brief open pwm dimmer
 *
 * @param[in] drv_handle: dimmer driver handle
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
STATIC OPERATE_RET __tdd_dimmer_pwm_open(IN DIMMER_DRV_HANDLE drv_handle)
{
    OPERATE_RET rt = OPRT_OK;
    UINT_T i = 0;    
    TUYA_PWM_BASE_CFG_T pwm_cfg = {0};
    DIMMER_PWM_CFG_T *p_drv = (DIMMER_PWM_CFG_T *)drv_handle;

    if(NULL == p_drv) {
        return OPRT_INVALID_PARM;
    }

    memset((UCHAR_T *)&pwm_cfg, 0x00, SIZEOF(pwm_cfg));

    pwm_cfg.frequency = p_drv->pwm_freq;
    
    /*不是所有平台都支持极性设置，故驱动高低有效的逻辑处理由 tdd 层自行处理*/
    pwm_cfg.polarity  = TUYA_PWM_POSITIVE;
    pwm_cfg.duty      = (FALSE == p_drv->active_level) ? DIMMER_PWM_DUTY_MAX : 0;

    for (i = 0; i < DIMMER_PWM_NUM_MAX; i++) {
        if(DIMMER_PWM_ID_INVALID == p_drv->pwm_ch_arr[i]) {
            continue;
        }

        TUYA_CALL_ERR_RETURN(tkl_pwm_init(p_drv->pwm_ch_arr[i], &pwm_cfg));
    }

    return OPRT_OK;
}

/**
 * @brief close pwm dimmer
 *
 * @param[in] drv_handle: dimmer driver handle
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
STATIC OPERATE_RET __tdd_dimmer_pwm_close(IN DIMMER_DRV_HANDLE drv_handle)
{
    UINT_T i = 0;
    OPERATE_RET rt = OPRT_OK;
    DIMMER_PWM_CFG_T *p_drv = (DIMMER_PWM_CFG_T *)drv_handle;

    if(NULL == p_drv) {
        return OPRT_INVALID_PARM;
    }

    for (i = 0; i < DIMMER_PWM_NUM_MAX; i++) {
        if(DIMMER_PWM_ID_INVALID == p_drv->pwm_ch_arr[i]) {
            continue;
        }

        TUYA_CALL_ERR_RETURN(tkl_pwm_stop(p_drv->pwm_ch_arr[i]));
        TUYA_CALL_ERR_RETURN(tkl_pwm_deinit(p_drv->pwm_ch_arr[i]));
    }

    return OPRT_OK;
}


TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_pwm_multichannel_stop(TUYA_PWM_NUM_E *ch_id, UINT8_T num)
{
    return OPRT_NOT_SUPPORTED;
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_pwm_multichannel_start(TUYA_PWM_NUM_E *ch_id, UINT8_T num)
{
    return OPRT_NOT_SUPPORTED;    
}

/**
 * @brief close pwm dimmer
 *
 * @param[in] drv_handle: dimmer driver handle
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
STATIC OPERATE_RET __tdd_dimmer_pwm_multi_channel_close(IN DIMMER_DRV_HANDLE drv_handle)
{
    OPERATE_RET rt = OPRT_OK;
    UINT_T i = 0;
    DIMMER_PWM_CFG_T *p_drv = (DIMMER_PWM_CFG_T *)drv_handle;

    if(NULL == p_drv) {
        return OPRT_INVALID_PARM;
    }

    for (i = 0; i < DIMMER_PWM_NUM_MAX; i++) {
        if(DIMMER_PWM_ID_INVALID == p_drv->pwm_ch_arr[i]) {
            continue;
        }

        if(DIMMER_PWM_CH_IDX_COLD == i && DIMMER_PWM_ID_INVALID != p_drv->pwm_ch_arr[DIMMER_PWM_CH_IDX_WARM]) {
            ;
        }else if(DIMMER_PWM_CH_IDX_WARM == i && DIMMER_PWM_ID_INVALID != p_drv->pwm_ch_arr[DIMMER_PWM_CH_IDX_COLD]){
            TUYA_CALL_ERR_RETURN(tkl_pwm_multichannel_stop(&p_drv->pwm_ch_arr[DIMMER_PWM_CH_IDX_COLD], 2));
        }else {
            TUYA_CALL_ERR_RETURN(tkl_pwm_stop(p_drv->pwm_ch_arr[i]));
        }

        TUYA_CALL_ERR_RETURN(tkl_pwm_deinit(p_drv->pwm_ch_arr[i]));
    }

    return OPRT_OK;
}

/**
 * @brief control dimmer output
 *
 * @param[in] drv_handle: driver handle
 * @param[in] p_rgbcw: the value of the value
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
STATIC OPERATE_RET __tdd_dimmer_pwm_output(DIMMER_DRV_HANDLE drv_handle, LIGHT_RGBCW_U *p_rgbcw)
{
    DIMMER_PWM_CFG_T *p_drv = (DIMMER_PWM_CFG_T *)drv_handle;
    USHORT_T pwm_duty = 0, i = 0;

    if(NULL == p_drv || NULL == p_rgbcw) {
        return OPRT_INVALID_PARM;
    }

    for (i = 0; i < DIMMER_PWM_NUM_MAX; i++) {
        if(DIMMER_PWM_ID_INVALID == p_drv->pwm_ch_arr[i]) {
            continue;
        }

        pwm_duty = (TRUE == p_drv->active_level) ? p_rgbcw->array[i] :\
                                                  (DIMMER_PWM_DUTY_MAX - p_rgbcw->array[i]);
                                                  
        tkl_pwm_duty_set(p_drv->pwm_ch_arr[i],pwm_duty);

        tkl_pwm_start(p_drv->pwm_ch_arr[i]);

    }

    return OPRT_OK;
}

/**
 * @brief control dimmer output
 *
 * @param[in] drv_handle: driver handle
 * @param[in] p_rgbcw: the value of the value
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
STATIC OPERATE_RET __tdd_dimmer_pwm_multi_channel_output(DIMMER_DRV_HANDLE drv_handle, LIGHT_RGBCW_U *p_rgbcw)
{
    DIMMER_PWM_CFG_T *p_drv = (DIMMER_PWM_CFG_T *)drv_handle;
    USHORT_T pwm_duty = 0, i = 0;

    for (i = 0; i < DIMMER_PWM_NUM_MAX; i++) {
        if(DIMMER_PWM_ID_INVALID == p_drv->pwm_ch_arr[i]) {
            continue;
        }

        pwm_duty = (TRUE == p_drv->active_level) ? p_rgbcw->array[i] :\
                                                  (DIMMER_PWM_DUTY_MAX - p_rgbcw->array[i]);

        tkl_pwm_duty_set(p_drv->pwm_ch_arr[i], pwm_duty);

        if(DIMMER_PWM_CH_IDX_COLD == i && DIMMER_PWM_ID_INVALID != p_drv->pwm_ch_arr[DIMMER_PWM_CH_IDX_WARM]) {
            ;
        }else if(DIMMER_PWM_CH_IDX_WARM == i && DIMMER_PWM_ID_INVALID != p_drv->pwm_ch_arr[DIMMER_PWM_CH_IDX_COLD]) {
            tkl_pwm_multichannel_start(&p_drv->pwm_ch_arr[DIMMER_PWM_CH_IDX_COLD], 2);
        }else {
            tkl_pwm_start(p_drv->pwm_ch_arr[i]);
        }
    }

    return OPRT_OK;
}

STATIC OPERATE_RET __tdd_dimmer_pwm_config(IN DIMMER_DRV_HANDLE drv_handle, DIMMER_DRV_CMD_E cmd, VOID_T *arg)
{
    DIMMER_PWM_CFG_T *p_drv = (DIMMER_PWM_CFG_T *)drv_handle;

    if(NULL == drv_handle) {
        return OPRT_INVALID_PARM;
    }

    switch(cmd) {
        case DRV_CMD_GET_HARDWARE_CFG: {
            DIMMER_HARDWARE_CFG_U *p_cfg = (DIMMER_HARDWARE_CFG_U *)arg;
            if(NULL == p_cfg) {
                return OPRT_INVALID_PARM;
            }

            memcpy(&p_cfg->pwm_cfg, p_drv, SIZEOF(DIMMER_PWM_CFG_T));
        }
        break;
        default:
        return OPRT_NOT_SUPPORTED;
    }

    return OPRT_OK;
}


STATIC LIGHT_DRV_COLOR_CH_E __tdd_dimmer_pwm_get_color_ch(DIMMER_PWM_CFG_T *p_cfg)
{
    LIGHT_DRV_COLOR_CH_E channel = 0;
    UINT_T i = 0;

    if(NULL == p_cfg) {
        return 0;
    }

    for(i = 0;  i< DIMMER_PWM_NUM_MAX; i++) { 
        if(DIMMER_PWM_ID_INVALID != p_cfg->pwm_ch_arr[i]) {
            channel = channel | (0x01 << i);
        }
    }

    return channel;
}

/**
 * @brief register pwm dimmer
 *
 * @param[in] name: dimmer name
 * @param[in] p_cfg: dimmer configuration
 * @param[in] type: pwm driver type
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
OPERATE_RET tdd_dimmer_pwm_register(IN CHAR_T *name, IN DIMMER_PWM_CFG_T *p_cfg, DIMMER_PWM_DRV_TP_E pwm_tp)
{
    DIMMER_DRV_INTFS_T intfs;
    DIMMER_DRIVER_TYPE_E drv_tp = 0;
    DIMMER_DRV_INFO_T    drv_info;

    if ((NULL == p_cfg) || (p_cfg->pwm_freq < DIMMER_PWM_FREQ_MIN) ||\
        (p_cfg->pwm_freq > DIMMER_PWM_FREQ_MAX) || (pwm_tp >= DIMMER_PWM_DRV_TP_UNUSED)) {
        return OPRT_INVALID_PARM;
    }

    memset((UCHAR_T *)&drv_info, 0x00, SIZEOF(DIMMER_DRV_INFO_T));

    DIMMER_PWM_CFG_T *p_drv = tal_malloc(SIZEOF(DIMMER_PWM_CFG_T));
    if (NULL == p_drv) {
        return OPRT_MALLOC_FAILED;
    }
    memset(p_drv, 0, SIZEOF(DIMMER_PWM_CFG_T));

    p_drv->pwm_freq     = p_cfg->pwm_freq;
    p_drv->active_level = p_cfg->active_level;

    memcpy((UCHAR_T *)p_drv->pwm_ch_arr, (UCHAR_T *)p_cfg->pwm_ch_arr, SIZEOF(p_drv->pwm_ch_arr));

    if(pwm_tp == DIMMER_PWM_DRV_TP_CCT) {
        drv_tp = DIMMER_DRIVER_PWM_CCT;
    }else {
        drv_tp = DIMMER_DRIVER_PWM_CW;
    }

    drv_info.color_ch   = __tdd_dimmer_pwm_get_color_ch(p_cfg);
    drv_info.drv_tp     = drv_tp;
    drv_info.chip_tp    = DIMMER_CHIP_PWM;
    drv_info.resolution = DIMMER_PWM_DUTY_MAX;

    intfs.open   = __tdd_dimmer_pwm_open;
    intfs.config = __tdd_dimmer_pwm_config;
    if(pwm_tp == DIMMER_PWM_DRV_TP_CW) {
        intfs.close  = __tdd_dimmer_pwm_multi_channel_close;
        intfs.output = __tdd_dimmer_pwm_multi_channel_output;
    }else {
        intfs.close  = __tdd_dimmer_pwm_close;
        intfs.output = __tdd_dimmer_pwm_output;
    }

    return tdl_dimmer_register(name, &intfs, &drv_info, (DIMMER_DRV_HANDLE)p_drv);
}