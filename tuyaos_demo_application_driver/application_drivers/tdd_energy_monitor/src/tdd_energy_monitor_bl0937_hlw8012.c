/**
 * @file tdd_energy_monitor_bl0937_hlw8012.c
 * @author www.tuya.com
 * @brief tdd_energy_monitor_bl0937_hlw8012 module is used to 
 * @version 0.1
 * @date 2023-07-14
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tdd_energy_monitor_bl0937_hlw8012.h"
#include "tdl_energy_monitor_driver.h"

#include "tal_log.h"
#include "tal_memory.h"
#include "tal_system.h"

#include "tkl_gpio.h"
#include "tkl_timer.h"
#include "tkl_output.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define __DEBUG     0
#define DEBUG_PR    bk_printf

/**
 * 电流采样电阻，bl0937 hlw8012 可按照 1mR 或 2mR；bl0942 和 hlw8032默认1mR
 */
#define TDD_ELE_HLW8012_SAMPLE_RES      (1)
#define TDD_ELE_BL0937_SAMPLE_RES       (1) // 1mR，单位：毫欧

/* CF1 引脚采样类型 */
typedef UINT8_T CF1_SAMPLE_TYPES_E;
#define SAMPLE_VOLTAGE                  0 // 采样电压
#define SAMPLE_CURRENT                  1 // 采样电流
#define SAMPLE_IDLE                     2

typedef UINT8_T CONVERT_TYPE_E;
#define CONVERT_VOLTAGE                 0 // 电压转换
#define CONVERT_CURRENT                 1 // 电流转换
#define CONVERT_POWER                   2 // 功率转换

#define SAMPLE_CACHE_NUMBER             (5)

/* 硬件定时器定时时间 */
#define TIMER_TIMEOUT_US                (1*1000U)

/* 单周期采样大于该时间，就进行一次转换 */
#define SINGLE_CONVERT_US               (100*1000U)

// 等待电压采样时间
#define VOLTAGE_SAMPLE_DELAY_US         (700*1000U)
/**
 *  电压采样时间：1000ms，hlw8012 最小可采集电流为 0.35v, bl0937 最小为 0.1v
 */
#define VOLTAGE_SAMPLE_US               (1000*1000U)

// 电流采样等待时间
#define CURRENT_SAMPLE_DELAY_US         (1*1000*1000U)
/**
 *  电流采样时间最大为 5s，最小可采集电流为 0.002A ~ 0.003A
 */
#define CURRENT_SAMPLE_US_DEF           (5*1000*1000U)

#define CURRENT_SAMPLE_US_MAX           (10*1000*1000U)

// 功率采样时间
/**
 *  功率采样时间：5s，hlw8012 最小可采集功率约为 1.6w; bl0937 最小为 2.9w
 */
#define POWER_SAMPLE_US                 (5*1000*1000U)

#define POWER_FAST_SAMPLE_US            (3*1000*1000U)

#define ENERGY_CONSUMED_LOW_LIMIT       30 // 实时功率低于该值，不进行计量
/**
 *  单次转换上限，单位 wh
 *  220v, 16A 一小时度数为 3.52 度(kWh)，5s 预计消耗 220*16/1000/3600*5 0.0049 度
 *  所以这里设置为 10，也就是 0.010 度（220v, 32A）不会影响设备正常使用。
 */
#define ENERGY_CONSUMED_SINGLE_MAX      10

/**
 *  hlw8012 calibration parameters
 * 
 *  220.0v, 0.392A, 86.4w 采样电阻为 1mR 下得到的校准参数
 */
#define TDD_ELE_HLW8012_V_REF           (1680U)
#define TDD_ELE_HLW8012_I_REF           (41093U)
#define TDD_ELE_HLW8012_P_REF           (115560U)
#define TDD_ELE_HLW8012_E_REF           (350U)

/**
 *  bl0937 calibration parameters
 * 
 *  220.0v, 0.392A, 86.4w 采样电阻为 1mR 下得到的校准参数
 */
#define TDD_ELE_BL0937_V_REF            (586U)
#define TDD_ELE_BL0937_I_REF            (28928U)
#define TDD_ELE_BL0937_P_REF            (16929U)
#define TDD_ELE_BL0937_E_REF            (2820U)

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct{
    UINT8_T is_enable;
    UINT8_T vol_alarm_flag;
    UINT8_T cur_alarm_flag;

    SYS_TIME_T vol_fault_start_ms;
    SYS_TIME_T cur_fault_start_ms;

    UINT32_T over_voltage;
    UINT32_T less_voltage;
    UINT32_T over_current;
} FAULT_DATA_T;

typedef struct{
    UINT_T last_time;
    UINT8_T convert_flag;
    UINT8_T index;
    UINT32_T period[SAMPLE_CACHE_NUMBER];
    UINT32_T edge_count; // 边沿个数

    UINT32_T period_overflow_cnt;

    UINT32_T actual_sample_us; // 实际采样时间
    UINT32_T desired_sample_us;

    UINT8_T convert_cnt; // 转换次数，用于过渡使用
}PULSE_SAMPLE_DATA_T;

typedef struct {
    HLW8012_DRIVER_CONFIG_T hw_cfg;

    volatile CF1_SAMPLE_TYPES_E cf1_sample_type;

    volatile UINT8_T is_cal;
    ENERGY_MONITOR_CAL_DATA_T cal_data;
    ENERGY_MONITOR_CAL_PARAMS_T cal_params;

    volatile PULSE_SAMPLE_DATA_T voltage_sample;
    volatile PULSE_SAMPLE_DATA_T current_sample;
    volatile PULSE_SAMPLE_DATA_T power_sample;
    volatile UINT32_T pf_number;

    // 采样转换后的数据
    volatile TDD_ENERGY_MONITOR_DATA_T monitor_data;
} PULSE_DRIVER_DATA_T;

/***********************************************************
********************function declaration********************
***********************************************************/
STATIC VOID_T __tdd_energy_monitor_cf1_cb(VOID_T *args);
STATIC VOID_T __tdd_energy_monitor_cf_cb(VOID_T *args);
STATIC VOID_T __tdd_energy_monitor_timer_cb(VOID_T *args);

/***********************************************************
***********************variable define**********************
***********************************************************/


/***********************************************************
***********************function define**********************
***********************************************************/
STATIC OPERATE_RET __tdd_energy_cf1_mode_sel(PULSE_DRIVER_DATA_T *tdd_data, CF1_SAMPLE_TYPES_E type)
{
    OPERATE_RET rt = OPRT_OK;
    TUYA_GPIO_IRQ_T gpio_isr = {0};
    UINT8_T tmp_convert_cnt = 0;
    TUYA_GPIO_LEVEL_E level = TUYA_GPIO_LEVEL_LOW;
    volatile PULSE_SAMPLE_DATA_T *sample_data = NULL;

    // stop cf1 sample
    rt = tkl_gpio_irq_disable(tdd_data->hw_cfg.cf1_pin);
    if (OPRT_OK != rt) {
        tkl_log_output("cf1 disable fail\r\n");
        return rt;
    }

    level = (SAMPLE_VOLTAGE == type) ? (tdd_data->hw_cfg.sel_level) : (!tdd_data->hw_cfg.sel_level);
    rt = tkl_gpio_write(tdd_data->hw_cfg.sel_pin, level);
    if (OPRT_OK != rt) {
        tkl_log_output("cf1 mode sel %d fail\r\n", level);
        return rt;
    }

    tdd_data->cf1_sample_type = type;

    // clear sample data
    sample_data  = (SAMPLE_VOLTAGE == type) ? (&tdd_data->voltage_sample) : (&tdd_data->current_sample);
    tmp_convert_cnt = sample_data->convert_cnt;
    memset((void *)(sample_data), 0, SIZEOF(PULSE_SAMPLE_DATA_T));
    sample_data->convert_cnt = tmp_convert_cnt;
    sample_data->desired_sample_us = (SAMPLE_VOLTAGE == type) ? (VOLTAGE_SAMPLE_US) : (CURRENT_SAMPLE_US_MAX);

    // start cf1 sample
    gpio_isr.mode = TUYA_GPIO_IRQ_FALL;
    gpio_isr.arg = tdd_data;
    gpio_isr.cb = __tdd_energy_monitor_cf1_cb;
    rt = tkl_gpio_irq_init(tdd_data->hw_cfg.cf1_pin, &gpio_isr);
    if (OPRT_OK != rt) {
        tkl_log_output("cf1 init fail\r\n");
        return rt;
    }
    rt = tkl_gpio_irq_enable(tdd_data->hw_cfg.cf1_pin);
    if (OPRT_OK != rt) {
        tkl_log_output("cf1 enable fail\r\n");
        return rt;
    }

    return rt;
}

/**
 * @brief 将脉冲数转换成标准单位
 *
 * @param[in] tdd_data: 
 * @param[in] convert_type: 0: voltage, 1: current, 2 power
 *
 * @return none
 */
STATIC VOID_T __tdd_energy_monitor_pulse_convert(PULSE_DRIVER_DATA_T *tdd_data, CONVERT_TYPE_E convert_type)
{
    volatile PULSE_SAMPLE_DATA_T *sample_data = NULL;
    UINT32_T period = 0, period_num = 0;
    volatile UINT32_T *convert_data = 0, cal_data = 0, cal_param = 0;
    UINT32_T temp_data = 0, temp_percent = 0, cur_data = 0;
    UINT32_T temp_index = 0;

    switch (convert_type) {
        case (CONVERT_VOLTAGE) :{
            sample_data = &tdd_data->voltage_sample;
            convert_data = &tdd_data->monitor_data.voltage;
            cal_data = tdd_data->cal_data.voltage;
            cal_param = tdd_data->cal_params.voltage_period;
#if __DEBUG
            DEBUG_PR("v:t_s:%d,%d ", sample_data->actual_sample_us, sample_data->edge_count);
#endif
        } break;
        case (CONVERT_CURRENT) :{
            sample_data = &tdd_data->current_sample;
            convert_data = &tdd_data->monitor_data.current;
            cal_data = tdd_data->cal_data.current;
            cal_param = tdd_data->cal_params.current_period;
#if __DEBUG
            DEBUG_PR("c:t_s:%d,%d ", sample_data->actual_sample_us, sample_data->edge_count);
#endif
        } break;
        case (CONVERT_POWER) :{
            sample_data = &tdd_data->power_sample;
            convert_data = &tdd_data->monitor_data.power;
            cal_data = tdd_data->cal_data.power;
            cal_param = tdd_data->cal_params.power_period;
#if __DEBUG
            DEBUG_PR("p:t_s:%d,%d ", sample_data->actual_sample_us, sample_data->edge_count);
#endif
        } break;
        default : return;
    }

    if (sample_data->edge_count < 2) {
        // 没有采样到数据直接退出
        *convert_data = 0;
#if __DEBUG
        DEBUG_PR("\r\n");
#endif
        goto __EXIT;
    }

    if (sample_data->actual_sample_us >= 300*1000) { // 避免采样时间过短导致转换数据不准
        temp_index = (sample_data->index>0) ? (sample_data->index-1) : (SAMPLE_CACHE_NUMBER-1);
        period_num = sample_data->edge_count - 1;
        if (sample_data->actual_sample_us / period_num > 100*1000 && \
            sample_data->period[temp_index] > 100*1000) {
            period = sample_data->period[temp_index];
        } else {
            period = sample_data->actual_sample_us / period_num;
        }
    } else {
#if __DEBUG
        DEBUG_PR("\r\n");
#endif
        goto __EXIT;
    }
#if __DEBUG
    DEBUG_PR("perd: %d\r\n", period);
#endif

    if (period>0) {
        // 如果当前采样数据和上一次差距过大，则再等待一个采样周期
        // 这样可以避免继电器关闭后电流或功率上报数据不正确
        if (*convert_data > 0 && sample_data->convert_cnt < 1) {
            cur_data = cal_data * cal_param / period;
            if (cur_data > 0) {
                temp_data = ((*convert_data) > (cur_data)) ? ((*convert_data)-cur_data) : (cur_data-(*convert_data));
                temp_percent = (temp_data*100) / (((*convert_data) > (cur_data)) ? (*convert_data) : (cur_data));
            } else {
                temp_percent = 0;
            }
            if (temp_percent > 80) {
                // 上一次值和当前采样数据偏差过大，超过 80%
                sample_data->convert_cnt += 1;
            } else {
                *convert_data = cal_data * cal_param / period;
                sample_data->convert_cnt = 0;
            }
        } else {
            *convert_data = cal_data * cal_param / period;
            sample_data->convert_cnt = 0;
        }
    } else {
        *convert_data = 0;
        sample_data->convert_cnt = 0;
    }

__EXIT:
    // convert energy_consumed
    if (convert_type == CONVERT_POWER) {
        if (tdd_data->monitor_data.power > ENERGY_CONSUMED_LOW_LIMIT && \
            tdd_data->power_sample.edge_count < ENERGY_CONSUMED_SINGLE_MAX * tdd_data->cal_params.pf_num) {
                tdd_data->pf_number += (tdd_data->power_sample.edge_count-1);
                if (tdd_data->pf_number > tdd_data->cal_params.pf_num) {
                    tdd_data->monitor_data.energy_consumed += tdd_data->pf_number / tdd_data->cal_params.pf_num;
                    tdd_data->pf_number = tdd_data->pf_number % tdd_data->cal_params.pf_num;
                }
        }
    }

    return;
}

STATIC VOID_T __tdd_energy_monitor_timer_cb(VOID_T *args)
{
    STATIC UINT32_T tick_cnt = 0;
    UINT8_T tmp_convert_cnt = 0;
    PULSE_DRIVER_DATA_T *tdd_data = NULL;

    STATIC UINT32_T delay_time = VOLTAGE_SAMPLE_DELAY_US;

    if (NULL == args) {
        tkl_log_output("energy monitor timer cb input error\r\n");
        return;
    }
    tdd_data = (PULSE_DRIVER_DATA_T *)args;

    tdd_data->power_sample.period_overflow_cnt++;
    tdd_data->power_sample.actual_sample_us += TIMER_TIMEOUT_US;
    if (tdd_data->cf1_sample_type == SAMPLE_VOLTAGE) {
        tdd_data->voltage_sample.period_overflow_cnt++;
        tdd_data->voltage_sample.actual_sample_us += TIMER_TIMEOUT_US;
    } else if (tdd_data->cf1_sample_type == SAMPLE_CURRENT) {
        tdd_data->current_sample.period_overflow_cnt++;
        tdd_data->current_sample.actual_sample_us += TIMER_TIMEOUT_US;
    }

    if (tdd_data->is_cal) {
        // 产测中，无需下面的电压，电流采样切换和数据转换的功能
        return;
    }

    switch (tdd_data->cf1_sample_type) {
        case (SAMPLE_IDLE): {
            if (delay_time == VOLTAGE_SAMPLE_DELAY_US) {
                // 电压采样等待时间时间结束，开始采样
                if (tick_cnt>=delay_time/TIMER_TIMEOUT_US) {
                    // 开始电压采样
                    tdd_data->cf1_sample_type = SAMPLE_VOLTAGE;
                    // 清除采样等待时间的数据
                    tmp_convert_cnt = tdd_data->voltage_sample.convert_cnt;
                    memset((void *)(&tdd_data->voltage_sample), 0, SIZEOF(PULSE_SAMPLE_DATA_T));
                    tdd_data->voltage_sample.convert_cnt = tmp_convert_cnt;
                    // 设置采样时间
                    tdd_data->voltage_sample.desired_sample_us = VOLTAGE_SAMPLE_US;
                    tick_cnt=0;
                }
            } else if (delay_time == CURRENT_SAMPLE_DELAY_US) {
                // 电流采样等待时间结束，开始电流采样
                if (tick_cnt>=delay_time/TIMER_TIMEOUT_US) {
                    tdd_data->cf1_sample_type = SAMPLE_CURRENT;
                    tmp_convert_cnt = tdd_data->current_sample.convert_cnt;
                    memset((void *)(&tdd_data->current_sample), 0, SIZEOF(PULSE_SAMPLE_DATA_T));
                    tdd_data->current_sample.convert_cnt = tmp_convert_cnt;
                    tdd_data->current_sample.desired_sample_us = CURRENT_SAMPLE_US_MAX;
                    tick_cnt=0;
                }
            }
        } break;
        case (SAMPLE_VOLTAGE) : {
            if (tdd_data->voltage_sample.actual_sample_us >= tdd_data->voltage_sample.desired_sample_us) {
                // 电压转换为国际单位
                tdd_data->voltage_sample.convert_flag=1;

                // 设置电流采样等待时间
                __tdd_energy_cf1_mode_sel(tdd_data, SAMPLE_CURRENT);
                tdd_data->cf1_sample_type = SAMPLE_IDLE;
                delay_time = CURRENT_SAMPLE_DELAY_US;
                tick_cnt=0;
            }
        } break;
        case (SAMPLE_CURRENT) : {
            if (tdd_data->current_sample.actual_sample_us >= CURRENT_SAMPLE_US_DEF) {
                if (tdd_data->current_sample.edge_count < 50) {
                    // 电流很小需要更长的采样周期
                    tdd_data->current_sample.desired_sample_us = CURRENT_SAMPLE_US_MAX;
                } else {
                    tdd_data->current_sample.desired_sample_us = CURRENT_SAMPLE_US_DEF;
                }
            }
            if (tdd_data->current_sample.actual_sample_us >= tdd_data->current_sample.desired_sample_us) {
                // 电流转换为国际单位
                tdd_data->current_sample.convert_flag=1;

                // 设置电压采样等待时间
                __tdd_energy_cf1_mode_sel(tdd_data, SAMPLE_VOLTAGE);
                tdd_data->cf1_sample_type = SAMPLE_IDLE;
                delay_time = VOLTAGE_SAMPLE_DELAY_US;
                tick_cnt=0;
            }
        } break;
        default :  break;
    }

    // 判断功率是否到转换时间
    if (tdd_data->power_sample.actual_sample_us >= tdd_data->power_sample.desired_sample_us) {
        tdd_data->power_sample.convert_flag=1;
    }

    if (tdd_data->voltage_sample.convert_flag) {
        tdd_data->voltage_sample.convert_flag=0;
        __tdd_energy_monitor_pulse_convert(tdd_data, CONVERT_VOLTAGE);
    }

    if (tdd_data->current_sample.convert_flag) {
        tdd_data->current_sample.convert_flag=0;
        __tdd_energy_monitor_pulse_convert(tdd_data, CONVERT_CURRENT);
    }

    if (tdd_data->power_sample.convert_flag) {
        tdd_data->power_sample.convert_flag=0;
        __tdd_energy_monitor_pulse_convert(tdd_data, CONVERT_POWER);
        tdd_data->power_sample.edge_count = 1;
        tdd_data->power_sample.actual_sample_us = 0;
        tdd_data->power_sample.index = 0;
        memset((VOID_T *)&tdd_data->power_sample.period, 0, SAMPLE_CACHE_NUMBER*SIZEOF(UINT32_T));
    }

    tick_cnt++;

    return;
}

/**
 * @brief 电压/电流采样
 *
 * @param[in] : 
 *
 * @return none
 */
STATIC VOID_T __tdd_energy_monitor_cf1_cb(VOID_T *args)
{
    OPERATE_RET rt = OPRT_OK;
    UINT_T cur_time = 0;
    UINT_T cur_period = 0;
    PULSE_SAMPLE_DATA_T *sample_data = NULL;
    PULSE_DRIVER_DATA_T *tdd_data = NULL;

    if (NULL == args) {
        tkl_log_output("energy monitor cf1 cb input error\r\n");
        return;
    }
    tdd_data = (PULSE_DRIVER_DATA_T *)args;

    if (SAMPLE_IDLE == tdd_data->cf1_sample_type) {
        return;
    }

    // 获取当前时间
    rt = tkl_timer_get(tdd_data->hw_cfg.timer_id, &cur_time);
    if (OPRT_OK != rt) {
        sample_data->edge_count = 0;
        return;
    }

    // 判断当前是测量电压还是测量电流
    if (SAMPLE_VOLTAGE == tdd_data->cf1_sample_type) {
        sample_data = (PULSE_SAMPLE_DATA_T *)(&tdd_data->voltage_sample);
    } else {
        sample_data = (PULSE_SAMPLE_DATA_T *)(&tdd_data->current_sample);
    }

    if (0 == sample_data->edge_count) { // 第一次获取数据，没有上一次数据无法计算周期，直接退出
        sample_data->edge_count++;
        sample_data->last_time = cur_time;
        sample_data->actual_sample_us = 0;
        sample_data->period_overflow_cnt = 0;
        return;
    }
    sample_data->edge_count++;

    if (cur_time > sample_data->last_time && sample_data->period_overflow_cnt == 0) {
        cur_period = cur_time - sample_data->last_time;
    } else if (sample_data->period_overflow_cnt > 0) {
        cur_period = (TIMER_TIMEOUT_US-sample_data->last_time) + ((sample_data->period_overflow_cnt-1)*TIMER_TIMEOUT_US) + cur_time;
        sample_data->period_overflow_cnt = 0;
    } else {
        /* 这种情况可能会在定时器刚溢出，立马来一个irq 中断导致 period_overflow_cnt 未来得及更新 */
        sample_data->last_time = cur_time;
        sample_data->period_overflow_cnt = 0;
        return;
    }

    sample_data->period[sample_data->index] = cur_period;

    // 更新索引
    sample_data->index = (sample_data->index + 1) % SAMPLE_CACHE_NUMBER;
    // 更新上一次时间
    sample_data->last_time = cur_time;

    return;
}

/**
 * @brief 实时功率采样
 *
 * @param[in] : 
 *
 * @return none
 */
STATIC VOID_T __tdd_energy_monitor_cf_cb(VOID_T *args)
{
    OPERATE_RET rt = OPRT_OK;
    UINT_T cur_time = 0;
    UINT_T cur_period = 0;
    PULSE_DRIVER_DATA_T *tdd_data = NULL;

    if (NULL == args) {
        tkl_log_output("energy monitor cf cb input error\r\n");
        return;
    }
    tdd_data = (PULSE_DRIVER_DATA_T *)args;

    rt = tkl_timer_get(tdd_data->hw_cfg.timer_id, &cur_time);
    if (OPRT_OK != rt) {
        tdd_data->power_sample.edge_count = 0;
#if __DEBUG
        DEBUG_PR("---> time get fail \r\n");
#endif
        return;
    }

    if (0 == tdd_data->power_sample.edge_count) { // 第一次获取数据，没有上一次数据无法计算周期，直接退出
        tdd_data->power_sample.edge_count++;
        tdd_data->power_sample.last_time = cur_time;
#if __DEBUG
        DEBUG_PR("---> first get time\r\n");
#endif
        return;
    }

    tdd_data->power_sample.edge_count++;
#if __DEBUG
    DEBUG_PR("---> p ");
#endif

    if (cur_time > tdd_data->power_sample.last_time && tdd_data->power_sample.period_overflow_cnt == 0) {
        cur_period = cur_time - tdd_data->power_sample.last_time;
    } else if (tdd_data->power_sample.period_overflow_cnt > 0) {
        cur_period = (TIMER_TIMEOUT_US-tdd_data->power_sample.last_time) + ((tdd_data->power_sample.period_overflow_cnt-1)*TIMER_TIMEOUT_US) + cur_time;
        tdd_data->power_sample.period_overflow_cnt = 0;
    } else {
        tdd_data->power_sample.last_time = cur_time;
        tdd_data->power_sample.period_overflow_cnt = 0;
        return;
    }

    tdd_data->power_sample.period[tdd_data->power_sample.index] = cur_period;

    // 更新索引
    tdd_data->power_sample.index++;
    tdd_data->power_sample.index = tdd_data->power_sample.index % SAMPLE_CACHE_NUMBER;
    // 更新上一次时间
    tdd_data->power_sample.last_time = cur_time;

    if (tdd_data->power_sample.edge_count > 500 && tdd_data->power_sample.actual_sample_us > POWER_FAST_SAMPLE_US) {
        // 不是小功率 3s 转换一次
        tdd_data->power_sample.convert_flag=1;
    }

    return;
}

STATIC OPERATE_RET __tdd_energy_monitor_hardware_init(PULSE_DRIVER_DATA_T *tdd_data)
{
    OPERATE_RET rt = OPRT_OK;
    TUYA_TIMER_BASE_CFG_T timer_cfg = {0};
    TUYA_GPIO_BASE_CFG_T gpio_cfg = {0};
    TUYA_GPIO_IRQ_T gpio_isr = {0};

    // sel_pin 初始化，默认采集电压
    gpio_cfg.direct = TUYA_GPIO_OUTPUT;
    gpio_cfg.mode = TUYA_GPIO_PUSH_PULL;
    gpio_cfg.level = tdd_data->hw_cfg.sel_level;
    tkl_gpio_init(tdd_data->hw_cfg.sel_pin, &gpio_cfg);
    tdd_data->cf1_sample_type = SAMPLE_IDLE;

    // 硬件定时器初始化
    timer_cfg.mode = TUYA_TIMER_MODE_PERIOD;
    timer_cfg.cb = __tdd_energy_monitor_timer_cb;
    timer_cfg.args = tdd_data;
    TUYA_CALL_ERR_RETURN(tkl_timer_init(tdd_data->hw_cfg.timer_id, &timer_cfg));
    TUYA_CALL_ERR_RETURN(tkl_timer_start(tdd_data->hw_cfg.timer_id, TIMER_TIMEOUT_US));

    // 电流或电压脚初始（cf1）
    gpio_isr.mode = TUYA_GPIO_IRQ_FALL;
    gpio_isr.arg = tdd_data;
    gpio_isr.cb = __tdd_energy_monitor_cf1_cb;
    TUYA_CALL_ERR_RETURN(tkl_gpio_irq_init(tdd_data->hw_cfg.cf1_pin, &gpio_isr));
    TUYA_CALL_ERR_RETURN(tkl_gpio_irq_enable(tdd_data->hw_cfg.cf1_pin));

    // 有功功率初始化（cf）
    gpio_isr.cb = __tdd_energy_monitor_cf_cb;
    TUYA_CALL_ERR_RETURN(tkl_gpio_irq_init(tdd_data->hw_cfg.cf_pin, &gpio_isr));
    TUYA_CALL_ERR_RETURN(tkl_gpio_irq_enable(tdd_data->hw_cfg.cf_pin));

    return rt;
}

STATIC OPERATE_RET __tdd_energy_monitor_hardware_deinit(PULSE_DRIVER_DATA_T *tdd_data)
{
    OPERATE_RET rt = OPRT_OK;

    tkl_gpio_irq_disable(tdd_data->hw_cfg.cf1_pin);
    tkl_gpio_irq_disable(tdd_data->hw_cfg.cf_pin);
    tkl_gpio_deinit(tdd_data->hw_cfg.sel_pin);
    tkl_timer_stop(tdd_data->hw_cfg.timer_id);
    tkl_timer_deinit(tdd_data->hw_cfg.timer_id);

    return rt;
}

STATIC OPERATE_RET __tdd_energy_monitor_open(TDD_EM_DRV_HANDLE handle)
{
    OPERATE_RET rt = OPRT_OK;
    PULSE_DRIVER_DATA_T *tdd_data = NULL;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);

    tdd_data = (PULSE_DRIVER_DATA_T *)handle;

    tdd_data->voltage_sample.desired_sample_us = VOLTAGE_SAMPLE_US;
    tdd_data->current_sample.desired_sample_us = CURRENT_SAMPLE_US_MAX;
    tdd_data->power_sample.desired_sample_us = POWER_SAMPLE_US;

    TUYA_CALL_ERR_RETURN(__tdd_energy_monitor_hardware_init(tdd_data));

#if 1
    TAL_PR_DEBUG("------ energy calibration info ------");
    TAL_PR_DEBUG("| vol: %u, cur: %u, power: %u, res: %d", tdd_data->cal_data.voltage, tdd_data->cal_data.current,\
                                    tdd_data->cal_data.power, tdd_data->cal_data.resval);
    TAL_PR_DEBUG("| period: V: %u, I: %u, P: %u, 0.001 kWh E number: %u", tdd_data->cal_params.voltage_period,\
                    tdd_data->cal_params.current_period, tdd_data->cal_params.power_period, tdd_data->cal_params.pf_num);
    TAL_PR_DEBUG("------------------------------------");
#endif

    return rt;
}

STATIC OPERATE_RET __tdd_energy_monitor_close(TDD_EM_DRV_HANDLE handle)
{
    OPERATE_RET rt = OPRT_OK;
    PULSE_DRIVER_DATA_T *tdd_data = NULL;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    tdd_data = (PULSE_DRIVER_DATA_T *)handle;

    __tdd_energy_monitor_hardware_deinit(tdd_data);

    memset((void *)(&tdd_data->voltage_sample), 0, SIZEOF(PULSE_SAMPLE_DATA_T));
    memset((void *)(&tdd_data->current_sample), 0, SIZEOF(PULSE_SAMPLE_DATA_T));
    memset((void *)(&tdd_data->power_sample), 0, SIZEOF(PULSE_SAMPLE_DATA_T));
    tdd_data->pf_number = 0;

    return rt;
}

STATIC OPERATE_RET __tdd_energy_monitor_read(TDD_EM_DRV_HANDLE handle, TDD_ENERGY_MONITOR_DATA_T *monitor_data)
{
    OPERATE_RET rt = OPRT_OK;
    PULSE_DRIVER_DATA_T *tdd_data = NULL;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(monitor_data, OPRT_INVALID_PARM);
    tdd_data = (PULSE_DRIVER_DATA_T *)handle;

    monitor_data->voltage         = tdd_data->monitor_data.voltage;
    monitor_data->current         = tdd_data->monitor_data.current;
    monitor_data->power           = tdd_data->monitor_data.power;
    monitor_data->energy_consumed = tdd_data->monitor_data.energy_consumed;
    tdd_data->monitor_data.energy_consumed = 0;

    // 低功率下不展示电流，功率
    if (monitor_data->power < ENERGY_CONSUMED_LOW_LIMIT) {
        monitor_data->current = 0;
        monitor_data->power = 0;
    }

    return rt;
}

STATIC OPERATE_RET __tdd_energy_monitor_calibration(PULSE_DRIVER_DATA_T *tdd_data, ENERGY_MONITOR_CAL_PARAMS_T *cal_params)
{
    OPERATE_RET rt = OPRT_OK;
    UINT32_T power_sample_ms = 0;

    tdd_data->is_cal = 1;

    // 计算 0.0001 度需要的时间
    power_sample_ms =  1000 * 3600 / tdd_data->cal_data.power;
    TAL_PR_DEBUG("0.0001 kWh need %u ms", power_sample_ms);

    memset(cal_params, 0, sizeof(ENERGY_MONITOR_CAL_PARAMS_T));

    // voltage calibration
    __tdd_energy_cf1_mode_sel(tdd_data, SAMPLE_VOLTAGE);
    tal_system_sleep(VOLTAGE_SAMPLE_DELAY_US/1000);
    memset((void *)(&tdd_data->voltage_sample), 0, SIZEOF(PULSE_SAMPLE_DATA_T));
    while (tdd_data->voltage_sample.actual_sample_us < VOLTAGE_SAMPLE_US) {
        // nothing to do;
        tal_system_sleep(1);
    }
    TAL_PR_DEBUG("vol cal: %d, %d", tdd_data->voltage_sample.actual_sample_us, tdd_data->voltage_sample.edge_count);
    if (tdd_data->voltage_sample.period[tdd_data->voltage_sample.index] > 100*1000) {
        // 单周期采样
        cal_params->voltage_period = tdd_data->voltage_sample.period[tdd_data->voltage_sample.index];
    } else if (tdd_data->voltage_sample.edge_count > 1) {
        cal_params->voltage_period = tdd_data->voltage_sample.actual_sample_us / tdd_data->voltage_sample.edge_count;
    } else {
        cal_params->voltage_period = 0;
        TAL_PR_ERR("voltage calibration fail");
        goto __EXIT;
    }
    // current calibration
    __tdd_energy_cf1_mode_sel(tdd_data, SAMPLE_CURRENT);
    tal_system_sleep(CURRENT_SAMPLE_DELAY_US/1000);
    memset((void *)(&tdd_data->current_sample), 0, SIZEOF(PULSE_SAMPLE_DATA_T));
    memset((void *)(&tdd_data->power_sample), 0, SIZEOF(PULSE_SAMPLE_DATA_T));

    while (tdd_data->power_sample.actual_sample_us < power_sample_ms*1000) {
        // nothing to do;
        tal_system_sleep(1);
    }

    // current
    TAL_PR_DEBUG("cur cal: %d, %d", tdd_data->current_sample.actual_sample_us, tdd_data->current_sample.edge_count);
    if (tdd_data->current_sample.period[tdd_data->current_sample.index] > 100*1000) {
        // 单周期采样
        cal_params->current_period = tdd_data->current_sample.period[tdd_data->current_sample.index];
    } else if (tdd_data->current_sample.edge_count > 1) {
        cal_params->current_period = tdd_data->current_sample.actual_sample_us / tdd_data->current_sample.edge_count;
    } else {
        cal_params->current_period = 0;
        TAL_PR_DEBUG("current calibration fail");
        goto __EXIT;
    }
    TAL_PR_DEBUG("power cal: %d, %d", tdd_data->power_sample.actual_sample_us, tdd_data->power_sample.edge_count);
    // 这里 x10 得到 0.001 度电量 pf 个数
    if (tdd_data->power_sample.edge_count > 1) {
        cal_params->pf_num = (tdd_data->power_sample.edge_count - 1) * 10;
    }
    // power
    if (tdd_data->power_sample.period[tdd_data->power_sample.index] > 100*1000) {
        // 单周期采样
        cal_params->power_period = tdd_data->power_sample.period[tdd_data->power_sample.index];
    } else if (tdd_data->power_sample.edge_count > 1) {
        cal_params->power_period = tdd_data->power_sample.actual_sample_us / tdd_data->power_sample.edge_count;
    } else {
        cal_params->power_period = 0;
        TAL_PR_DEBUG("power calibration fail");
        goto __EXIT;
    }

__EXIT:
    tdd_data->cf1_sample_type = SAMPLE_IDLE;
    memset((void *)(&tdd_data->voltage_sample), 0, SIZEOF(PULSE_SAMPLE_DATA_T));
    memset((void *)(&tdd_data->current_sample), 0, SIZEOF(PULSE_SAMPLE_DATA_T));
    memset((void *)(&tdd_data->power_sample), 0, SIZEOF(PULSE_SAMPLE_DATA_T));
    tdd_data->pf_number = 0;

    tdd_data->is_cal = 0;

    return rt;
}

STATIC OPERATE_RET __tdd_energy_monitor_bl0937_config(TDD_EM_DRV_HANDLE handle, TDD_ENERGY_MONITOR_CMD_E cmd, VOID_T *arg)
{
    OPERATE_RET rt = OPRT_OK;
    PULSE_DRIVER_DATA_T *tdd_data = NULL;
    static UINT8_T cal_params_update = 0;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    tdd_data = (PULSE_DRIVER_DATA_T *)handle;

    switch (cmd) {
        case (TDD_ENERGY_CMD_CAL_DATA_SET) : {
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            memcpy(&tdd_data->cal_data, arg, SIZEOF(ENERGY_MONITOR_CAL_DATA_T));
            if (0 == cal_params_update) {
                if (tdd_data->cal_data.voltage > 0) {
                    tdd_data->cal_params.voltage_period = TDD_ENERGY_220_REF_V * TDD_ELE_BL0937_V_REF / tdd_data->cal_data.voltage;
                }
                if (tdd_data->cal_data.current > 0 && tdd_data->cal_data.resval > 0) {
                    tdd_data->cal_params.current_period = TDD_ENERGY_220_REF_I * TDD_ELE_BL0937_I_REF / tdd_data->cal_data.current / tdd_data->cal_data.resval;
                }
                if (tdd_data->cal_data.power > 0 && tdd_data->cal_data.resval > 0) {
                    tdd_data->cal_params.power_period = TDD_ENERGY_220_REF_P * TDD_ELE_BL0937_P_REF / tdd_data->cal_data.power  / tdd_data->cal_data.resval;
                }
                tdd_data->cal_params.pf_num = TDD_ELE_BL0937_E_REF * tdd_data->cal_data.resval;
            }
        } break;
        case (TDD_ENERGY_CMD_CAL_PARAMS_SET) : {
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            memcpy(&tdd_data->cal_params, arg, SIZEOF(ENERGY_MONITOR_CAL_PARAMS_T));
            cal_params_update = 1;
        } break;
        case (TDD_ENERGY_CMD_CAL_PARAMS_GET) : {
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            TUYA_CALL_ERR_RETURN(__tdd_energy_monitor_calibration((tdd_data), (ENERGY_MONITOR_CAL_PARAMS_T *)arg));
        } break;
        case (TDD_ENERGY_CMD_DEF_DATA_GET) :{
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            ENERGY_MONITOR_CAL_DATA_T *temp_data = (ENERGY_MONITOR_CAL_DATA_T *)arg;
            temp_data->voltage = TDD_ENERGY_220_REF_V;
            temp_data->current = TDD_ENERGY_220_REF_I;
            temp_data->power = TDD_ENERGY_220_REF_P;
            temp_data->resval = TDD_ELE_BL0937_SAMPLE_RES;
        } break;
        case (TDD_ENERGY_CMD_DEF_PARAMS_GET) : {
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            ENERGY_MONITOR_CAL_PARAMS_T *temp_params = (ENERGY_MONITOR_CAL_PARAMS_T *)arg;
            temp_params->voltage_period = TDD_ELE_BL0937_V_REF;
            temp_params->current_period = TDD_ELE_BL0937_I_REF;
            temp_params->power_period   = TDD_ELE_BL0937_P_REF;
            temp_params->pf_num         = TDD_ELE_BL0937_E_REF;
        }break;
        default : return OPRT_NOT_SUPPORTED;
    }

    return rt;
}

STATIC OPERATE_RET __tdd_energy_monitor_hlw8012_config(TDD_EM_DRV_HANDLE handle, TDD_ENERGY_MONITOR_CMD_E cmd, VOID_T *arg)
{
    OPERATE_RET rt = OPRT_OK;
    PULSE_DRIVER_DATA_T *tdd_data = NULL;
    static UINT8_T cal_params_update = 0;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    tdd_data = (PULSE_DRIVER_DATA_T *)handle;

    switch (cmd) {
        case (TDD_ENERGY_CMD_CAL_DATA_SET) : {
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            memcpy(&tdd_data->cal_data, arg, SIZEOF(ENERGY_MONITOR_CAL_DATA_T));
            if (0 == cal_params_update) {
                if (tdd_data->cal_data.voltage > 0) {
                    tdd_data->cal_params.voltage_period = TDD_ENERGY_220_REF_V * TDD_ELE_HLW8012_V_REF / tdd_data->cal_data.voltage;
                }
                if (tdd_data->cal_data.current > 0 && tdd_data->cal_data.resval > 0) {
                    tdd_data->cal_params.current_period = TDD_ENERGY_220_REF_I * TDD_ELE_HLW8012_I_REF / tdd_data->cal_data.current / tdd_data->cal_data.resval;
                }
                if (tdd_data->cal_data.power > 0 && tdd_data->cal_data.resval > 0) {
                    tdd_data->cal_params.power_period = TDD_ENERGY_220_REF_P * TDD_ELE_HLW8012_P_REF / tdd_data->cal_data.power  / tdd_data->cal_data.resval;
                }
                tdd_data->cal_params.pf_num = TDD_ELE_HLW8012_E_REF * tdd_data->cal_data.resval;
            }
        } break;
        case (TDD_ENERGY_CMD_CAL_PARAMS_SET) : {
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            memcpy(&tdd_data->cal_params, arg, SIZEOF(ENERGY_MONITOR_CAL_PARAMS_T));
            cal_params_update = 1;
        } break;
        case (TDD_ENERGY_CMD_CAL_PARAMS_GET) : {
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            TUYA_CALL_ERR_RETURN(__tdd_energy_monitor_calibration((tdd_data), (ENERGY_MONITOR_CAL_PARAMS_T *)arg));
        } break;
        case (TDD_ENERGY_CMD_DEF_DATA_GET) :{
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            ENERGY_MONITOR_CAL_DATA_T *temp = (ENERGY_MONITOR_CAL_DATA_T *)arg;
            temp->voltage = TDD_ENERGY_220_REF_V;
            temp->current = TDD_ENERGY_220_REF_I;
            temp->power = TDD_ENERGY_220_REF_P;
            temp->resval = TDD_ELE_HLW8012_SAMPLE_RES;
        } break;
        case (TDD_ENERGY_CMD_DEF_PARAMS_GET) : {
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            ENERGY_MONITOR_CAL_PARAMS_T *temp_params = (ENERGY_MONITOR_CAL_PARAMS_T *)arg;
            temp_params->voltage_period = TDD_ELE_HLW8012_V_REF;
            temp_params->current_period = TDD_ELE_HLW8012_I_REF;
            temp_params->power_period   = TDD_ELE_HLW8012_P_REF;
            temp_params->pf_num         = TDD_ELE_HLW8012_E_REF;
        }break;
        default : return OPRT_NOT_SUPPORTED;
    }

    return rt;
}

OPERATE_RET tdd_energy_driver_bl0937_register(IN CHAR_T *name, IN BL0937_DRIVER_CONFIG_T cfg)
{
    OPERATE_RET rt = OPRT_OK;
    ENERGY_DRV_INTFS_T intfs = {0};
    PULSE_DRIVER_DATA_T *tdd_data = NULL;

    TUYA_CHECK_NULL_RETURN(name, OPRT_INVALID_PARM);
    tdd_data = tal_malloc(sizeof(PULSE_DRIVER_DATA_T));
    TUYA_CHECK_NULL_RETURN(tdd_data, OPRT_MALLOC_FAILED);
    memset(tdd_data, 0, sizeof(PULSE_DRIVER_DATA_T));
    memcpy(&tdd_data->hw_cfg, &cfg, sizeof(BL0937_DRIVER_CONFIG_T));

    intfs.open = __tdd_energy_monitor_open;
    intfs.close = __tdd_energy_monitor_close;
    intfs.read = __tdd_energy_monitor_read;
    intfs.config = __tdd_energy_monitor_bl0937_config;

    tdd_data->cal_data.voltage          = TDD_ENERGY_220_REF_V;
    tdd_data->cal_data.current          = TDD_ENERGY_220_REF_I;
    tdd_data->cal_data.power            = TDD_ENERGY_220_REF_P;
    tdd_data->cal_data.resval           = TDD_ELE_BL0937_SAMPLE_RES;
    tdd_data->cal_params.voltage_period = TDD_ELE_BL0937_V_REF;
    tdd_data->cal_params.current_period = TDD_ELE_BL0937_I_REF;
    tdd_data->cal_params.power_period   = TDD_ELE_BL0937_P_REF;
    tdd_data->cal_params.pf_num         = TDD_ELE_BL0937_E_REF;

    TUYA_CALL_ERR_GOTO(tdl_energy_monitor_driver_register(name, &intfs, (TDD_EM_DRV_HANDLE)tdd_data), __ERR);

    return rt;

__ERR:
    if (NULL != tdd_data) {
        tal_free(tdd_data);
        tdd_data = NULL;
    }

    return rt;
}

OPERATE_RET tdd_energy_driver_hlw8012_register(IN CHAR_T *name, IN HLW8012_DRIVER_CONFIG_T cfg)
{
    OPERATE_RET rt = OPRT_OK;
    ENERGY_DRV_INTFS_T intfs = {0};
    PULSE_DRIVER_DATA_T *tdd_data = NULL;

    TUYA_CHECK_NULL_RETURN(name, OPRT_INVALID_PARM);
    tdd_data = tal_malloc(sizeof(PULSE_DRIVER_DATA_T));
    TUYA_CHECK_NULL_RETURN(tdd_data, OPRT_MALLOC_FAILED);
    memset(tdd_data, 0, sizeof(PULSE_DRIVER_DATA_T));
    memcpy(&tdd_data->hw_cfg, &cfg, sizeof(HLW8012_DRIVER_CONFIG_T));

    intfs.open = __tdd_energy_monitor_open;
    intfs.close = __tdd_energy_monitor_close;
    intfs.read = __tdd_energy_monitor_read;
    intfs.config = __tdd_energy_monitor_hlw8012_config;

    tdd_data->cal_data.voltage          = TDD_ENERGY_220_REF_V;
    tdd_data->cal_data.current          = TDD_ENERGY_220_REF_I;
    tdd_data->cal_data.power            = TDD_ENERGY_220_REF_P;
    tdd_data->cal_data.resval           = TDD_ELE_HLW8012_SAMPLE_RES;
    tdd_data->cal_params.voltage_period = TDD_ELE_HLW8012_V_REF;
    tdd_data->cal_params.current_period = TDD_ELE_HLW8012_I_REF;
    tdd_data->cal_params.power_period   = TDD_ELE_HLW8012_P_REF;
    tdd_data->cal_params.pf_num         = TDD_ELE_HLW8012_E_REF;

    TUYA_CALL_ERR_GOTO(tdl_energy_monitor_driver_register(name, &intfs, (TDD_EM_DRV_HANDLE)tdd_data), __ERR);

    return rt;

__ERR:
    if (NULL != tdd_data) {
        tal_free(tdd_data);
        tdd_data = NULL;
    }

    return rt;
}
