/**
 * @file tdd_sound_adc.c
 * @author www.tuya.com
 * @brief tdd_sound_adc module is used to sample sound adc val
 * @version 0.1
 * @date 2022-04-01
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */
#include "tuya_iot_config.h"

#if defined(ENABLE_ADC) && (ENABLE_ADC)
#include "tal_log.h"
#include "tkl_adc.h"
#include "tkl_pinmux.h"

#include "tdl_sound_driver.h"
#include "tdd_sound_adc.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
#define TDD_SOUND_ADC_DATA_LEN 1

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    TUYA_ADC_NUM_E adc_num;
    UINT8_T adc_chn;
} TDD_SOUND_ADC_CFG;

/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC TDD_SOUND_ADC_CFG local_adc_cfg;

/***********************************************************
***********************function define**********************
***********************************************************/
STATIC int __tdd_sound_adc_open(SOUND_DIR_HANDLE_T handle)
{
    OPERATE_RET op_ret = 0;
    TDD_SOUND_ADC_CFG *adc_cfg = ((TDD_SOUND_ADC_CFG *)handle);
    TUYA_ADC_NUM_E adc_num = adc_cfg->adc_num;
    TUYA_ADC_BASE_CFG_T adc_config;

#if 1
    adc_config.ch_list.data = 1 << adc_cfg->adc_chn;
#else 
    UINT8_T chn_list[1] = {0};
    chn_list[0] = adc_cfg->adc_chn;
    adc_config.ch_list = chn_list;
#endif
    adc_config.ch_nums = 1;

    adc_config.conv_cnt = TDD_SOUND_ADC_DATA_LEN;
    adc_config.mode = TUYA_ADC_CONTINUOUS;

    op_ret = tkl_adc_init(adc_num, &adc_config);

    return (int)op_ret;
}

STATIC int __tdd_sound_adc_close(SOUND_DIR_HANDLE_T handle)
{
    OPERATE_RET op_ret = 0;
    TUYA_ADC_NUM_E adc_num = ((TDD_SOUND_ADC_CFG *)handle)->adc_num;

    op_ret = tkl_adc_deinit(adc_num);

    return (int)op_ret;
}

STATIC int __tdd_sound_adc_sample(SOUND_DIR_HANDLE_T handle, unsigned int *data)
{
    OPERATE_RET op_ret = 0;
    TDD_SOUND_ADC_CFG *adc_cfg = ((TDD_SOUND_ADC_CFG *)handle);
    TUYA_ADC_NUM_E adc_num = adc_cfg->adc_num;
    UINT8_T adc_chn = adc_cfg->adc_chn;
    INT32_T tmp = 0;

    op_ret = tkl_adc_read_single_channel(adc_num, adc_chn, &tmp);

    *data = (unsigned int)tmp;

    return (int)op_ret;
}

int tdd_sound_adc_register(char *dev_name, ADC_DRIVER_CONFIG_T *init_param)
{
    OPERATE_RET op_ret = 0;
    SOUND_INTFS_T intfs = {0};
    UINT8_T adc_chn = 0;

    intfs.open = __tdd_sound_adc_open;
    intfs.close = __tdd_sound_adc_close;
    intfs.sample = __tdd_sound_adc_sample;

    adc_chn = (UINT8_T)tkl_io_pin_to_func(init_param->pin, TUYA_IO_TYPE_ADC);
    local_adc_cfg.adc_chn = adc_chn;
    local_adc_cfg.adc_num = init_param->adc_num;

    op_ret = tdl_sound_driver_register(dev_name, &intfs, init_param->adc_max, (SOUND_DIR_HANDLE_T *)&local_adc_cfg);
    if (op_ret != 0) {
        TAL_PR_ERR("tdl_sound_driver_register err:%d", op_ret);
        return (int)op_ret;
    }

    return (int)op_ret;
}
#endif