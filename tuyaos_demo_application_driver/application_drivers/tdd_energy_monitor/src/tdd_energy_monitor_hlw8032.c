/**
 * @file tdd_energy_monitor_hlw8032.c
 * @author www.tuya.com
 * @brief tdd_energy_monitor_hlw8032 module is used to 
 * @version 0.1
 * @date 2023-07-14
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tdd_energy_monitor_hlw8032.h"
#include "tdl_energy_monitor_driver.h"

#include "tal_log.h"
#include "tal_memory.h"
#include "tal_uart.h"
#include "tal_sw_timer.h"
#include "tal_system.h"
#include "tal_mutex.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define HLW8032_FRAME_LEN           24

#define TDD_ELE_HLW8032_SAMPLE_RES  (1) // 1mR，单位：毫欧

// 电压系数
#define VOLTAGE_K                   (188) // VOLTAGE_K = R2 / R1 = (470*4)/10， 和手册上比扩大了100倍
// 电流系数
#define CURRENT_K                   (1) // CURRENT_K = 1 / R(=0.0001R) * 1000 = 1

// hlw8032 数据更新时间
#define SW_TIMER_CYCLE_TIMER        (100)

#define READ_TIMEOUT_MS             (1000)

/***********************************************************
***********************typedef define***********************
***********************************************************/
#pragma pack(1)
typedef union {
    UINT8_T byte;
    struct {
        UINT8_T param_state : 1;
        UINT8_T power_state : 1;
        UINT8_T current_state : 1;
        UINT8_T voltage_state : 1;
        UINT8_T reserved : 4;
    } reg;
} STATE_REG_U;

typedef struct {
    UINT8_T high_byte;
    UINT8_T middle_byte;
    UINT8_T low_byte;
} DATA_REG_T;

typedef struct {
    UINT8_T reserved : 4;
    UINT8_T power : 1;
    UINT8_T current : 1;
    UINT8_T voltage : 1;
    UINT8_T pf_carry : 1;
} DATA_UPDATE_REG_T;

typedef struct {
    STATE_REG_U state_reg;
    UINT8_T check_reg; // 0x5A
    DATA_REG_T voltage_params_reg;
    DATA_REG_T voltage_reg;
    DATA_REG_T current_params_reg;
    DATA_REG_T current_reg;
    DATA_REG_T power_params_reg;
    DATA_REG_T power_reg;
    DATA_UPDATE_REG_T data_update_reg;
    UINT16_T pf_reg;
    UINT8_T checksum_reg;
} HLW8032_UART_DATA_T;
#pragma pack()

typedef struct{
    UINT8_T is_enable;
    UINT8_T vol_alarm_flag;
    UINT8_T cur_alarm_flag;
    UINT32_T over_voltage;
    UINT32_T less_voltage;
    UINT32_T over_current;
} FAULT_DATA_T;

typedef struct{
    UINT8_T vol;
    UINT8_T cur;
    UINT8_T power;
} EM_ERR_CNT_T;

typedef struct {
    HLW8032_DRIVER_CONFIG_T hw_cfg;

    ENERGY_MONITOR_CAL_DATA_T cal_data;
    ENERGY_MONITOR_CAL_PARAMS_T cal_params;
    UINT8_T cal_params_enable;

    TIMER_ID timer_id;
    MUTEX_HANDLE mutex_hdl;

    EM_ERR_CNT_T err_cnt;
    TDD_ENERGY_MONITOR_DATA_T monitor_data;
    UINT32_T def_pf_unit;
    UINT32_T pf_num;
    UINT32_T last_pf;
    UINT8_T last_pf_carry;
} HLW8032_DRIVER_DATA_T;

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/


/***********************************************************
***********************function define**********************
***********************************************************/

UINT8_T __tdd_hlw8032_checksum(UINT8_T *data, UINT32_T length) {
    UINT8_T checksum = 0;
    UINT32_T i = 0;
    
    for (i = 0; i < length; i++) {
        checksum += data[i];
        checksum &= 0xFF;  // 保持校验和为8位
    }

    return checksum;
}

STATIC OPERATE_RET __tdd_hlw8032_uart_read(TUYA_UART_NUM_E uart_id, HLW8032_UART_DATA_T *data)
{
    OPERATE_RET rt = OPRT_OK;
    UINT32_T read_len = 0;
    UINT8_T rx_buff[HLW8032_FRAME_LEN] = {0};
    UINT8_T checksum = 0;

    TUYA_CHECK_NULL_RETURN(data, OPRT_INVALID_PARM);

    read_len = tal_uart_get_rx_data_size(uart_id);
    if (read_len < HLW8032_FRAME_LEN) {
        // TAL_PR_NOTICE("recv data too few");
        return OPRT_COM_ERROR;
    }

    // find head
    while (1) {
        if (1 == tal_uart_read(uart_id, &rx_buff[0], 1)) {
            if (rx_buff[0] == 0xAA || rx_buff[0] == 0x55 || (rx_buff[0] & 0xF0) == 0xF0) {
                if (1 == tal_uart_read(uart_id, &rx_buff[1], 1)) {
                    if (rx_buff[1] == 0x5A) {
                        // find head
                        break;
                    }
                }
            }
        }
    }

    if (22 != tal_uart_read(uart_id, &rx_buff[2], 22)) {
        TAL_PR_ERR("hlw8032 frame read data fail");
        return OPRT_COM_ERROR;
    }

    // checksum
    checksum = __tdd_hlw8032_checksum(&rx_buff[2], 21);
    if (checksum != rx_buff[23]) {
        TAL_PR_ERR("hlw8032 checksum fail, checksum: %d, %d", checksum, rx_buff[23]);
        return OPRT_COM_ERROR;
    }

    memcpy(data, rx_buff, SIZEOF(HLW8032_UART_DATA_T));

    return rt;
}

// return 1:need update, 0:not need update
STATIC UINT8_T __tdd_energy_monitor_hlw8032_convert_voltage(HLW8032_UART_DATA_T reg_data, UINT32_T *voltage)
{
    UINT32_T reg_params_v = 0, reg_v = 0;

    reg_params_v = reg_data.voltage_params_reg.high_byte<<16 | reg_data.voltage_params_reg.middle_byte<<8 | reg_data.voltage_params_reg.low_byte;

    if (0xF8 == (reg_data.state_reg.byte & 0xF8)) {
        reg_v = 0;
    } else if (reg_data.data_update_reg.voltage) {
        reg_v = reg_data.voltage_reg.high_byte<<16 | reg_data.voltage_reg.middle_byte<<8 | reg_data.voltage_reg.low_byte;
    } else {
        TAL_PR_DEBUG("hlw8032 voltage data not update");
        return 0;
    }

    if (reg_v > 0) {
        *voltage = (reg_params_v * VOLTAGE_K)  / reg_v  / 10; // x10
    } else {
        *voltage = 0;
    }

    return 1;
}

STATIC UINT8_T __tdd_energy_monitor_hlw8032_convert_current(HLW8032_UART_DATA_T reg_data, UINT32_T *current)
{
    UINT32_T reg_params_i = 0, reg_i = 0;

    reg_params_i = reg_data.current_params_reg.high_byte<<16 | reg_data.current_params_reg.middle_byte<<8 | reg_data.current_params_reg.low_byte;

    if (0xF4 == (reg_data.state_reg.byte & 0xF4)) {
        reg_i = 0;
    } else if (reg_data.data_update_reg.current) {
        reg_i = reg_data.current_reg.high_byte<<16 | reg_data.current_reg.middle_byte<<8 | reg_data.current_reg.low_byte;
    } else {
        TAL_PR_DEBUG("hlw8032 current data not update");
        return 0;
    }

    if (reg_i > 0) {
        *current = (reg_params_i*100) / reg_i * 10 * CURRENT_K; // mA
    } else {
        *current = 0;
    }

    return 1;
}

// 返回：1：转换成功，0：转换失败
STATIC UINT8_T __tdd_energy_monitor_hlw8032_convert_power(HLW8032_UART_DATA_T reg_data, UINT32_T *power)
{
    UINT32_T reg_params_p = 0, reg_p = 0;

    reg_params_p = reg_data.power_params_reg.high_byte<<16 | reg_data.power_params_reg.middle_byte<<8 | reg_data.power_params_reg.low_byte;

    if (0xF2 == (reg_data.state_reg.byte & 0xF2)) {
        reg_p = 0;
    } else if (reg_data.data_update_reg.power) {
        reg_p = reg_data.power_reg.high_byte<<16 | reg_data.power_reg.middle_byte<<8 | reg_data.power_reg.low_byte;
    } else {
        return 0;
    }

    if (reg_p > 0) {
        *power = (reg_params_p*VOLTAGE_K) / reg_p * CURRENT_K / 10; // x10
    } else {
        *power = 0;
    }

    return 1;
}

STATIC VOID_T __tdd_energy_monitor_timer_cb(TIMER_ID timer_id, VOID_T *arg)
{
    OPERATE_RET rt = OPRT_OK;
    HLW8032_DRIVER_DATA_T *tdd_data = NULL;
    HLW8032_UART_DATA_T reg_data = {{0}};
    UINT8_T is_pf_overflow = 0;
    UINT32_T cur_pf_cnt = 0, pf_add_cnt = 0;
    UINT32_T tmp_data = 0;

    if (NULL == arg) {
        return;
    }
    tdd_data = (HLW8032_DRIVER_DATA_T *)arg;

    tal_mutex_lock(tdd_data->mutex_hdl);

    rt = __tdd_hlw8032_uart_read(tdd_data->hw_cfg.uart_id, &reg_data);
    if (OPRT_OK != rt) {
        tdd_data->err_cnt.vol++;
        tdd_data->err_cnt.cur++;
        tdd_data->err_cnt.power++;
        goto __EXIT;
    }

    if (reg_data.state_reg.byte == 0xAA || reg_data.state_reg.byte == 0xF1) {
        TAL_PR_ERR("hlw8032 state params reg fail");
        tdd_data->err_cnt.vol++;
        tdd_data->err_cnt.cur++;
        tdd_data->err_cnt.power++;
        goto __EXIT;
    }

    if (__tdd_energy_monitor_hlw8032_convert_voltage(reg_data, &tmp_data)) {
        tdd_data->monitor_data.voltage = tmp_data;
        tdd_data->err_cnt.vol = 0;
    } else {
        tdd_data->err_cnt.vol++;
    }
    if (__tdd_energy_monitor_hlw8032_convert_current(reg_data, &tmp_data)) {
        tdd_data->monitor_data.current = tmp_data;
        tdd_data->err_cnt.cur = 0;
    } else {
        tdd_data->err_cnt.cur++;
    }
    if (__tdd_energy_monitor_hlw8032_convert_power(reg_data, &tmp_data)) {
        tdd_data->monitor_data.power = tmp_data;
        tdd_data->err_cnt.power = 0;
    } else {
        tdd_data->err_cnt.power++;
    }

    // 获取累计 pf_num
    cur_pf_cnt = UNI_HTONS(reg_data.pf_reg);
    is_pf_overflow = (reg_data.data_update_reg.pf_carry == tdd_data->last_pf_carry) ? (0) : (1);
    tdd_data->last_pf_carry = reg_data.data_update_reg.pf_carry;

    if (is_pf_overflow) {
        pf_add_cnt = (0xFFFF - tdd_data->last_pf) + cur_pf_cnt;
        // 1500 为经验值，它是在 220v, 60A 的情况下 100ms 大约产生的 pf_cnt 个数
        if (pf_add_cnt > 1500) {
            TAL_PR_ERR("energy monitor chip hlw8032 reset");
            pf_add_cnt = 0;
            tdd_data->pf_num = 0;
        }
    } else {
        if (cur_pf_cnt >= tdd_data->last_pf) {
            pf_add_cnt = cur_pf_cnt - tdd_data->last_pf;
        } else {
            TAL_PR_ERR("energy monitor chip hlw8032 reset");
            pf_add_cnt = 0;
            tdd_data->pf_num = 0;
        }
    }
    tdd_data->last_pf = cur_pf_cnt;
    tdd_data->pf_num += pf_add_cnt;

    // 消耗电量
    if (tdd_data->cal_params_enable) {
        if (tdd_data->pf_num >= tdd_data->cal_params.pf_num && tdd_data->cal_params.pf_num > 0) {
            tdd_data->monitor_data.energy_consumed += tdd_data->pf_num / tdd_data->cal_params.pf_num;
            tdd_data->pf_num = tdd_data->pf_num % tdd_data->cal_params.pf_num;
        }
    } else {
        if (tdd_data->pf_num >= tdd_data->def_pf_unit && tdd_data->def_pf_unit>0) {
            tdd_data->monitor_data.energy_consumed += tdd_data->pf_num / tdd_data->def_pf_unit;
            tdd_data->pf_num = tdd_data->pf_num % tdd_data->def_pf_unit;
        }
    }

__EXIT:
    if (tdd_data->err_cnt.vol * SW_TIMER_CYCLE_TIMER > READ_TIMEOUT_MS) {
        tdd_data->err_cnt.vol = 0;
        tdd_data->monitor_data.voltage = 0;
    }
    if (tdd_data->err_cnt.cur * SW_TIMER_CYCLE_TIMER > READ_TIMEOUT_MS) {
        tdd_data->err_cnt.cur = 0;
        tdd_data->monitor_data.current = 0;
    }
    if (tdd_data->err_cnt.power * SW_TIMER_CYCLE_TIMER > READ_TIMEOUT_MS) {
        tdd_data->err_cnt.power = 0;
        tdd_data->monitor_data.power = 0;
    }

    tal_mutex_unlock(tdd_data->mutex_hdl);

    return;
}

STATIC OPERATE_RET __tdd_energy_monitor_open(TDD_EM_DRV_HANDLE handle)
{
    OPERATE_RET rt = OPRT_OK;
    HLW8032_DRIVER_DATA_T *tdd_data = NULL;
    TAL_UART_CFG_T uart_cfg = {0};
    HLW8032_UART_DATA_T reg_data;
    UINT8_T fail_cnt = 0;
    UINT32_T reg_params_p = 0;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    tdd_data = (HLW8032_DRIVER_DATA_T *)handle;

    uart_cfg.rx_buffer_size = UART_BUFFER_SIZE_MAX;
    uart_cfg.base_cfg.baudrate = 4800;
    uart_cfg.base_cfg.databits = TUYA_UART_DATA_LEN_8BIT;
    uart_cfg.base_cfg.stopbits = TUYA_UART_STOP_LEN_1BIT;
    uart_cfg.base_cfg.flowctrl = TUYA_UART_FLOWCTRL_NONE;
    TUYA_CALL_ERR_RETURN(tal_uart_init(tdd_data->hw_cfg.uart_id, &uart_cfg));

    // pf_num init
    do {
        rt = __tdd_hlw8032_uart_read(tdd_data->hw_cfg.uart_id, &reg_data);
        if (OPRT_OK != rt || reg_data.state_reg.byte == 0xaa || reg_data.state_reg.byte == 0xF1) {
            fail_cnt++;
            if (fail_cnt >= 5) {
                TAL_PR_ERR("cal params init fail");
                return OPRT_COM_ERROR;
            } else {
                tal_system_sleep(100);
            }
        } else {
            break;
        }
    } while (1);
    // 将 hlw8032 当前电量状态设置为初始状态
    tdd_data->last_pf = UNI_HTONS(reg_data.pf_reg);
    tdd_data->last_pf_carry = reg_data.data_update_reg.pf_carry;
    // 芯片内部 0.001 度电量 pf 个数
    reg_params_p = reg_data.power_params_reg.high_byte<<16 | reg_data.power_params_reg.middle_byte<<8 | reg_data.power_params_reg.low_byte;
    tdd_data->def_pf_unit = 3600 * 1000 / VOLTAGE_K* CURRENT_K *100 / (reg_params_p/1000); // 0.001 度电量 pf 个数

    TAL_PR_DEBUG("hlw8032 init params: %d, %d, %d", tdd_data->last_pf, tdd_data->last_pf_carry, tdd_data->def_pf_unit);

    if (NULL == tdd_data->timer_id) {
        TUYA_CALL_ERR_RETURN(tal_sw_timer_create(__tdd_energy_monitor_timer_cb, handle, &tdd_data->timer_id));
    }
    TUYA_CALL_ERR_RETURN(tal_sw_timer_start(tdd_data->timer_id, SW_TIMER_CYCLE_TIMER, TAL_TIMER_CYCLE));

    TUYA_CALL_ERR_RETURN(tal_mutex_create_init(&tdd_data->mutex_hdl));

    TAL_PR_DEBUG("hlw8032 open success");

    return rt;
}

STATIC OPERATE_RET __tdd_energy_monitor_close(TDD_EM_DRV_HANDLE handle)
{
    OPERATE_RET rt = OPRT_OK;
    HLW8032_DRIVER_DATA_T *tdd_data = NULL;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    tdd_data = (HLW8032_DRIVER_DATA_T *)handle;

    TUYA_CALL_ERR_RETURN(tal_sw_timer_stop(tdd_data->timer_id));

    TUYA_CALL_ERR_RETURN(tal_uart_deinit(tdd_data->hw_cfg.uart_id));

    return rt;
}

STATIC OPERATE_RET __tdd_energy_monitor_read(TDD_EM_DRV_HANDLE handle, TDD_ENERGY_MONITOR_DATA_T *monitor_data)
{
    OPERATE_RET rt = OPRT_OK;
    HLW8032_DRIVER_DATA_T *tdd_data = NULL;

    TUYA_CHECK_NULL_RETURN(monitor_data, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    tdd_data = (HLW8032_DRIVER_DATA_T *)handle;

    tal_mutex_lock(tdd_data->mutex_hdl);

    memcpy(monitor_data, &tdd_data->monitor_data, SIZEOF(TDD_ENERGY_MONITOR_DATA_T));
    // 读取后清零
    tdd_data->monitor_data.energy_consumed = 0;

    // 功耗过低测量不准确
    if (monitor_data->power < 20) {
        monitor_data->power = 0;
        monitor_data->current = 0;
    } else if (monitor_data->power < 70 && monitor_data->current > 0) {
        monitor_data->current = monitor_data->power * 1000 / monitor_data->voltage;
    }

    tal_mutex_unlock(tdd_data->mutex_hdl);

    return rt;
}

STATIC OPERATE_RET __tdd_energy_monitor_calibration(HLW8032_DRIVER_DATA_T *tdd_data, ENERGY_MONITOR_CAL_PARAMS_T *cal_params)
{
    OPERATE_RET rt = OPRT_OK;
    UINT32_T power_sample_ms = 0;
    SYS_TIME_T start_time = 0, cur_time = 0;
    UINT32_T cur_pf = 0;
    UINT32_T reg_params_p = 0;
    UINT32_T tmp_conv_data = 0;

    HLW8032_UART_DATA_T hlw8032_data = {{0}}, temp_data = {{0}};

    // 这里校准，只用判断当前校准环境和测量得到的数值是否一致，以及测量 0.001 度电量 pf 个数

    memset(cal_params, 0, SIZEOF(ENERGY_MONITOR_CAL_PARAMS_T));

    // 计算 0.0001 度需要的时间
    power_sample_ms =  1000 * 3600 / tdd_data->cal_data.power;
    TAL_PR_DEBUG("0.0001 kWh need %u ms", power_sample_ms);

    if (NULL != tdd_data->timer_id) {
        tal_sw_timer_stop(tdd_data->timer_id);
    }

    TAL_PR_DEBUG("wait hlw8032 data");
    while (1) {
        memset(&hlw8032_data, 0, SIZEOF(HLW8032_UART_DATA_T));
        rt = __tdd_hlw8032_uart_read(tdd_data->hw_cfg.uart_id, &hlw8032_data);
        if (rt == OPRT_OK) { break; }
        tal_system_sleep(10);
    }
    TAL_PR_DEBUG("recv hlw8032 data");

    // 保存pf
    tdd_data->last_pf = UNI_HTONS(hlw8032_data.pf_reg);
    tdd_data->last_pf_carry = hlw8032_data.data_update_reg.pf_carry;

    TAL_PR_DEBUG("last pf number: %d", tdd_data->last_pf);

    start_time = tal_system_get_millisecond();
    cur_time = start_time;
    while (cur_time <= start_time+power_sample_ms) {
        tal_system_sleep(1);
        cur_time = tal_system_get_millisecond();
        memset(&temp_data, 0, SIZEOF(HLW8032_UART_DATA_T));
        rt = __tdd_hlw8032_uart_read(tdd_data->hw_cfg.uart_id, &temp_data);
        if (rt == OPRT_OK) {
            memcpy(&hlw8032_data, &temp_data, SIZEOF(HLW8032_UART_DATA_T));
            if (hlw8032_data.state_reg.byte == 0xAA || hlw8032_data.state_reg.byte == 0xF1) {
                continue;
            }
            if (hlw8032_data.data_update_reg.voltage) {
                if (__tdd_energy_monitor_hlw8032_convert_voltage(hlw8032_data, &tmp_conv_data)) {
                    cal_params->voltage_period = tmp_conv_data;
                }
            }
            if (hlw8032_data.data_update_reg.current) {
                if (__tdd_energy_monitor_hlw8032_convert_current(hlw8032_data, &tmp_conv_data)) {
                    cal_params->current_period = tmp_conv_data;
                }
            }
            if (hlw8032_data.data_update_reg.power) {
                if (__tdd_energy_monitor_hlw8032_convert_power(hlw8032_data, &tmp_conv_data)) {
                    cal_params->power_period = tmp_conv_data;
                }
            }
        }
    }

    TAL_PR_DEBUG("time: %d=%d-%d", cur_time-start_time, cur_time, start_time);
    TAL_PR_DEBUG("cal params: %d, %d, %d", cal_params->voltage_period, cal_params->current_period, cal_params->power_period);
    TAL_PR_DEBUG("cur pf number: %d", cur_pf);

    //  0.0001 度电量 pf 个数
    cur_pf = UNI_HTONS(hlw8032_data.pf_reg);
    if (tdd_data->last_pf_carry == hlw8032_data.data_update_reg.pf_carry) {
        cal_params->pf_num = cur_pf - tdd_data->last_pf;
    } else {
        cal_params->pf_num = (0xFFFF - tdd_data->last_pf) + cur_pf;
    }
    // 得到 0.001 度电量 pf 个数
    cal_params->pf_num = cal_params->pf_num * 10;

    TAL_PR_DEBUG("0.001 pf %d",cal_params->pf_num);

    // 计算默认 0.001 度电量 pf 个数
    reg_params_p = hlw8032_data.power_params_reg.high_byte<<16 | hlw8032_data.power_params_reg.middle_byte<<8 | hlw8032_data.power_params_reg.low_byte;
    tdd_data->def_pf_unit = 3600 * 1000 / VOLTAGE_K* CURRENT_K *100 / (reg_params_p/1000); // 0.001 度电量 pf 个数

    // 采样结束将当前累计电量清零
    tdd_data->pf_num = 0;

    tdd_data->last_pf = UNI_HTONS(hlw8032_data.pf_reg);
    tdd_data->last_pf_carry = hlw8032_data.data_update_reg.pf_carry;

    if (NULL != tdd_data->timer_id) {
        tal_sw_timer_start(tdd_data->timer_id, SW_TIMER_CYCLE_TIMER, TAL_TIMER_CYCLE);
    }

    return OPRT_OK;
}

STATIC OPERATE_RET __tdd_energy_monitor_config(TDD_EM_DRV_HANDLE handle, TDD_ENERGY_MONITOR_CMD_E cmd, VOID_T *arg)
{
    OPERATE_RET rt = OPRT_OK;
    HLW8032_DRIVER_DATA_T *tdd_data = NULL;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    tdd_data = (HLW8032_DRIVER_DATA_T *)handle;

    switch (cmd) {
        case (TDD_ENERGY_CMD_CAL_DATA_SET) : {
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            memcpy(&tdd_data->cal_data, arg, SIZEOF(ENERGY_MONITOR_CAL_DATA_T));
        } break;
        case (TDD_ENERGY_CMD_CAL_PARAMS_SET) : {
            // nothing todo
        } break;
        case (TDD_ENERGY_CMD_CAL_PARAMS_GET) : {
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            TUYA_CALL_ERR_RETURN(__tdd_energy_monitor_calibration(tdd_data, (ENERGY_MONITOR_CAL_PARAMS_T *)arg));
        } break;
        case (TDD_ENERGY_CMD_DEF_DATA_GET) :{
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            ENERGY_MONITOR_CAL_DATA_T *temp_data = (ENERGY_MONITOR_CAL_DATA_T *)arg;
            temp_data->voltage  = tdd_data->cal_data.voltage;
            temp_data->current  = tdd_data->cal_data.current;
            temp_data->power    = tdd_data->cal_data.power;
            temp_data->resval   = 1;
        } break;
        case (TDD_ENERGY_CMD_DEF_PARAMS_GET) : {
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            ENERGY_MONITOR_CAL_PARAMS_T *temp_params = (ENERGY_MONITOR_CAL_PARAMS_T *)arg;
            temp_params->voltage_period = tdd_data->cal_data.voltage;
            temp_params->current_period = tdd_data->cal_data.current;
            temp_params->power_period   = tdd_data->cal_data.power;
            temp_params->pf_num         = tdd_data->def_pf_unit;
        }break;
        default : return OPRT_NOT_SUPPORTED;
    }

    return rt;
}

OPERATE_RET tdd_energy_driver_hlw8032_register(IN CHAR_T *name, IN HLW8032_DRIVER_CONFIG_T cfg)
{
    OPERATE_RET rt = OPRT_OK;
    HLW8032_DRIVER_DATA_T *tdd_data = NULL;
    ENERGY_DRV_INTFS_T intfs = {0};

    TUYA_CHECK_NULL_RETURN(name, OPRT_INVALID_PARM);
    tdd_data = (HLW8032_DRIVER_DATA_T *)tal_malloc(SIZEOF(HLW8032_DRIVER_DATA_T));
    TUYA_CHECK_NULL_RETURN(tdd_data, OPRT_MALLOC_FAILED);
    memset(tdd_data, 0, SIZEOF(HLW8032_DRIVER_DATA_T));
    memcpy(&tdd_data->hw_cfg, &cfg, SIZEOF(HLW8032_DRIVER_CONFIG_T));

    intfs.open = __tdd_energy_monitor_open;
    intfs.close = __tdd_energy_monitor_close;
    intfs.read = __tdd_energy_monitor_read;
    intfs.config = __tdd_energy_monitor_config;

    TUYA_CALL_ERR_GOTO(tdl_energy_monitor_driver_register(name, &intfs, (TDD_EM_DRV_HANDLE)tdd_data), __ERR);

    return rt;

__ERR:
    if (NULL != tdd_data) {
        tal_free(tdd_data);
        tdd_data = NULL;
    }

    return rt;
}
