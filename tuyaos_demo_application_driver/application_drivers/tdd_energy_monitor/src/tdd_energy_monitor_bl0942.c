/**
 * @file tdd_energy_monitor_bl0942.c
 * @author www.tuya.com
 * @brief tdd_energy_monitor_bl0942 module is used to 
 * @version 0.1
 * @date 2023-07-12
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tdd_energy_monitor_bl0942.h"
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
#define TDD_BL0942_SAMPLE_RES   (1)

// hlw8032 数据更新时间，建议最小 300ms
#define SW_TIMER_CYCLE_TIMER        (900) // 900ms

#define UART_BUFFER_SIZE_MAX        (96)
#define UART_DEFAULT_BAUDRATE       (4800U)

#define UART_WRITE_FLAG             (0xA0)
#define UART_READ_FLAG              (0x50)

/**
 *  默认硬件环境为：
 *  V_ref: 1.218v; R1: 510 Ω; R2: 390 000 Ω * 5; RL: 1mΩ
 *  下面的 V_COE, I_COE, P_COE, PF_UNIT 不同于其他 3 个计量平台，它们是芯片手册上的系数，不是校准环境下的脉冲个数
 */

/*
    由 V = V_RMS * V_ref * (R2+R1) / (73989*R1*1000) 推算得到。
    V_ref 单位为 伏（V），R1，R2 单位为 KΩ，得到的电压的单位为伏（V）。
*/
#define V_COE   (15883U)
/*
    由 I = I_RMS * V_ref / (305978*RL) 推算得到。
    V_ref 单位为 伏（V），RL 单位为毫欧，得到的电流的单位为安培（A）。
    这里计算得到的实际数值应为 251213，但下面转换为 mA 需要 I_RMS * 1000，
    为了避免I_RMS * 1000 产生溢出，I_COE/10，这样 I_RMS * 100 就不会产生溢出。
*/
#define I_COE   (25121U)

/*
    由 P = WATT * V_ref^2 * (R2+R1) / (3537*RL*R1*1000) 推算得到。
    V_ref 单位为 伏（V），R1，R2 单位为 KΩ，RL 单位为毫欧，得到的功率的单位为瓦（w）。
    这里计算得到的实际数值应为 623，为了兼容老代码提高测量进度这里 P_COE*10。
*/
#define P_COE   (6230U)
/*
    每个电能脉冲电量 = 1638.4*256*V_ref^2 * (R2+R1) / 3600000*3537*RL*R1*1000 度
*/
// 1 度脉冲个数
// bl0942 0.001 度的脉冲个数太小了，精度损失太严重，这里计算 1 度的脉冲
// 后面转换的时候将测量得到的脉冲数 *1000 然后进行转换，这样与其他测量芯片保持一致，对上提供的都是 0.001 度，也不会损失精度
#define PF_UNIT (5474)

/**
 *  bl0942 芯片寄存器地址
 */
// 电参量寄存器（只读）
#define REG_I_WAVE          0x01 // 电流波形寄存器，有符号
#define REG_V_WAVE          0x02 // 电压波形寄存器，有符号
#define REG_I_RMS           0x03 // 电流有效值寄存器，无符号
#define REG_V_RMS           0x04 // 电压有效值寄存器，无符号
#define REG_I_FAST_RMS      0x05 // 电流快速有效值寄存器，无符号
#define REG_WATT            0x06 // 有功功率寄存器，有符号
#define REG_CF_CNT          0x07 // 有功电能脉冲计数寄存器，无符号
#define	REG_FREQ            0x08 // 线电压频率寄存器，无符号
#define REG_STATUS          0x09 // 状态寄存器，无符号
// 用户操作寄存器（读写）
#define REG_I_RMSOS         0x12 // 电流有效值小信号校正寄存器
#define REG_WA_CREEP        0x14 // 有功功率防潜寄存器
#define REG_FAST_RMS_TH     0x15 // 电流快速有效值阈值寄存器
#define REG_FAST_RMS_CYC    0x16 // 电流快速有效值刷新周期寄存器
#define REG_FREQ_CYC        0x17 // 线电压频率刷新周期寄存器
#define REG_OT_FUNX         0x18 // 输出配置寄存器
#define REG_MODE            0x19 // 用户模式选择寄存器
#define REG_GAIN_CR         0x1A // 电流通道增益控制寄存器
#define REG_SOFT_RESET      0x1C // 写入 0x5A5A5A 时，用户区寄存器复位
#define REG_USR_WRPROT      0x1D // 用户写保护设置寄存器。写入 0x55 后，用户操作寄存器可以写入；写入其他值，用户操作寄存器区域不可写入


/***********************************************************
***********************typedef define***********************
***********************************************************/
// uart data
// uart send W/R register
#pragma pack(1)
typedef struct {
    UINT8_T byte_l;
    UINT8_T byte_m;
    UINT8_T byte_h;
} REG_DATA_T;

typedef struct {
    UINT8_T cmd;
    UINT8_T register_addr;
    REG_DATA_T reg_data;
    UINT8_T checksum;
} BL0942_UART_DATA_T;

// all params 
typedef struct {
    UINT8_T HEAD;
    REG_DATA_T I_RMS;
    REG_DATA_T V_RMS;
    REG_DATA_T I_FAST_RMS;
    REG_DATA_T WATT;
    REG_DATA_T CF_CNT;
    REG_DATA_T FREQ;
    REG_DATA_T STATUS;
    UINT8_T CHECKSUM;
} BL0942_UART_ALL_DATA_T;

#pragma pack()

typedef struct{
    UINT8_T is_enable;
    UINT8_T vol_alarm_flag;
    UINT8_T cur_alarm_flag;
    UINT32_T over_voltage;
    UINT32_T less_voltage;
    UINT32_T over_current;
} FAULT_DATA_T;

typedef struct {
    BL0942_DRIVER_CONFIG_T hw_cfg;
    UINT8_T drv_addr;

    TIMER_ID timer_id;
    MUTEX_HANDLE mutex_hdl;

    ENERGY_MONITOR_CAL_DATA_T cal_data;
    ENERGY_MONITOR_CAL_PARAMS_T cal_params;

    UINT32_T last_pf;
    UINT32_T pf_cache;

    TDD_ENERGY_MONITOR_DATA_T monitor_data;
} BL0942_DRIVER_DATA_T;


/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/


/***********************************************************
***********************function define**********************
***********************************************************/
STATIC UINT8_T __tdd_bl0942_checksum(UINT8_T cmd, UINT8_T *data, UINT32_T length) {
    UINT8_T checksum = 0;
    UINT32_T i = 0;
    
    // TAL_PR_DEBUG("checksum debug");

    checksum += cmd;

    // TAL_PR_DEBUG_RAW("0x%02x ", cmd);

    for (i = 0; i < length; i++) {
        checksum += data[i];
        // TAL_PR_DEBUG_RAW("0x%02x ", data[i]);
        checksum &= 0xFF;  // 保持校验和为8位
    }

    // TAL_PR_DEBUG_RAW("\r\n");

    checksum = ~(checksum & 0xFF);  // 取反并截断为8位

    return checksum;
}

STATIC OPERATE_RET __tdd_bl0942_reg_uart_write(BL0942_DRIVER_DATA_T *tdd_hdl, UINT8_T reg_addr, REG_DATA_T reg_data)
{
    OPERATE_RET rt = OPRT_OK;

    BL0942_UART_DATA_T write_date = {0};

    TUYA_CHECK_NULL_RETURN(tdd_hdl, OPRT_INVALID_PARM);

    if (tdd_hdl->hw_cfg.mode != BL0942_DRV_UART) {
        return OPRT_NOT_SUPPORTED;
    }

    write_date.cmd = (UART_WRITE_FLAG | (tdd_hdl->drv_addr & 0x0F));
    write_date.register_addr = reg_addr;
    memcpy(&write_date.reg_data, &reg_data, SIZEOF(REG_DATA_T));
    write_date.checksum = __tdd_bl0942_checksum(write_date.cmd, &write_date.register_addr, SIZEOF(BL0942_UART_DATA_T) - SIZEOF(UINT8_T)- SIZEOF(UINT8_T));
    TAL_PR_HEXDUMP_DEBUG("write reg", (UINT8_T *)&write_date, SIZEOF(BL0942_UART_DATA_T));

    tal_uart_write(tdd_hdl->hw_cfg.driver.uart.id, (UINT8_T *)&write_date, SIZEOF(BL0942_UART_DATA_T));

    return rt;
}

STATIC OPERATE_RET __tdd_bl0942_reg_uart_read(BL0942_DRIVER_DATA_T *tdd_hdl, UINT8_T reg_addr, REG_DATA_T *reg_data)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T read_cnt = 0;
    UINT8_T checksum = 0;
    BL0942_UART_DATA_T read_date = {0};

    TUYA_CHECK_NULL_RETURN(tdd_hdl, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(reg_data, OPRT_INVALID_PARM);

    if (tdd_hdl->hw_cfg.mode != BL0942_DRV_UART) {
        return OPRT_NOT_SUPPORTED;
    }

    // clear uart read
    while (tal_uart_get_rx_data_size(tdd_hdl->hw_cfg.driver.uart.id) > 0) {
        memset(&read_date, 0, SIZEOF(BL0942_UART_DATA_T));
        tal_uart_read(tdd_hdl->hw_cfg.driver.uart.id, (UINT8_T *)&read_date, SIZEOF(BL0942_UART_DATA_T));
        read_cnt++;
        if (read_cnt >= (UART_BUFFER_SIZE_MAX/SIZEOF(BL0942_UART_DATA_T)+1)) {
            return OPRT_COM_ERROR;
        }
    }

    read_date.cmd = (UART_READ_FLAG | (tdd_hdl->drv_addr & 0x0F));
    read_date.register_addr = reg_addr;
    TAL_PR_HEXDUMP_DEBUG("read reg", (UINT8_T *)&read_date, SIZEOF(BL0942_UART_DATA_T));

    tal_uart_write(tdd_hdl->hw_cfg.driver.uart.id, (UINT8_T *)&read_date, 2*SIZEOF(UINT8_T));

    tal_system_sleep(50);
    if (tal_uart_get_rx_data_size(tdd_hdl->hw_cfg.driver.uart.id) >= 4) {
        // read register data, checksum
        TUYA_CALL_ERR_RETURN(tal_uart_read(tdd_hdl->hw_cfg.driver.uart.id, (UINT8_T *)&read_date.reg_data, SIZEOF(REG_DATA_T)+1));
    } else {
        TAL_PR_ERR("read data to few");
        return OPRT_COM_ERROR;
    }

    TAL_PR_HEXDUMP_DEBUG("read reg", (UINT8_T *)&read_date, SIZEOF(BL0942_UART_DATA_T));

    // checksum
    checksum = __tdd_bl0942_checksum(read_date.cmd, &read_date.register_addr, SIZEOF(BL0942_UART_DATA_T)-SIZEOF(UINT8_T)-SIZEOF(UINT8_T));
    if (checksum != read_date.checksum) {
        TAL_PR_ERR("checksum fail, %d, %d", checksum, read_date.checksum);
        return OPRT_COM_ERROR;
    }

    memcpy(reg_data, &read_date.reg_data, SIZEOF(REG_DATA_T));

    return rt;
}

STATIC OPERATE_RET __tdd_bl0942_reg_uart_soft_reset(BL0942_DRIVER_DATA_T *tdd_hdl)
{
    OPERATE_RET rt = OPRT_OK;
    REG_DATA_T reg_data = {0};

    // 关闭用户寄存器写保护，可以写入用户寄存器
    memset(&reg_data, 0, SIZEOF(REG_DATA_T));
    reg_data.byte_l = 0x55;
    TUYA_CALL_ERR_RETURN(__tdd_bl0942_reg_uart_write(tdd_hdl, REG_USR_WRPROT, reg_data));

    // 用户区寄存器复位
    reg_data.byte_h = 0x5A;
    reg_data.byte_m = 0x5A;
    reg_data.byte_l = 0x5A;
    TUYA_CALL_ERR_RETURN(__tdd_bl0942_reg_uart_write(tdd_hdl, REG_SOFT_RESET, reg_data));

    // 打开用户寄存器写保护
    memset(&reg_data, 0, SIZEOF(REG_DATA_T));
    TUYA_CALL_ERR_RETURN(__tdd_bl0942_reg_uart_write(tdd_hdl, REG_USR_WRPROT, reg_data));

    return rt;
}

STATIC OPERATE_RET __tdd_bl0942_all_reg_uart_read(BL0942_DRIVER_DATA_T *tdd_hdl, BL0942_UART_ALL_DATA_T *data)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T read_cnt = 0, read_len = 0;
    UINT8_T checksum = 0;
    UINT8_T write_data[2] = {0};

    TUYA_CHECK_NULL_RETURN(tdd_hdl, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(data, OPRT_INVALID_PARM);

    // clear uart read
    read_cnt = 0;
    while (tal_uart_get_rx_data_size(tdd_hdl->hw_cfg.driver.uart.id) > 0) {
        memset(data, 0, SIZEOF(BL0942_UART_ALL_DATA_T));
        read_len = tal_uart_read(tdd_hdl->hw_cfg.driver.uart.id, (UINT8_T *)data, SIZEOF(BL0942_UART_ALL_DATA_T));
        read_cnt++;
        if (read_cnt >= (UART_BUFFER_SIZE_MAX/SIZEOF(BL0942_UART_ALL_DATA_T)+1) || read_len < 0) {
            return OPRT_COM_ERROR;
        }
    }

    // send read all data cmd
    write_data[0] = UART_READ_FLAG | (tdd_hdl->drv_addr & 0x0F);
    write_data[1] = 0xAA;
    tal_uart_write(tdd_hdl->hw_cfg.driver.uart.id, write_data, 2*SIZEOF(UINT8_T));

    // wait data
    read_cnt = 0;
    while (tal_uart_get_rx_data_size(tdd_hdl->hw_cfg.driver.uart.id) < SIZEOF(BL0942_UART_ALL_DATA_T)) {
        tal_system_sleep(10);
        read_cnt++;
        if (read_cnt > 8) {
            TAL_PR_DEBUG("wait bl0942 data timeout");
            return OPRT_COM_ERROR;
        }
    }
    // read data
    memset(data, 0, SIZEOF(BL0942_UART_ALL_DATA_T));
    read_len = tal_uart_read(tdd_hdl->hw_cfg.driver.uart.id, (UINT8_T *)data, SIZEOF(BL0942_UART_ALL_DATA_T));
    if (read_len < SIZEOF(BL0942_UART_ALL_DATA_T)) {
        return OPRT_COM_ERROR;
    }
    // TAL_PR_HEXDUMP_DEBUG("read all data", (UINT8_T *)data, SIZEOF(BL0942_UART_ALL_DATA_T));

    // checksum
    checksum = __tdd_bl0942_checksum(write_data[0], (UINT8_T *)data, SIZEOF(BL0942_UART_ALL_DATA_T)-SIZEOF(UINT8_T));
    if (checksum != data->CHECKSUM) {
        TAL_PR_ERR("all data check sum error, %d ,%d", checksum, data->CHECKSUM);
        return OPRT_COM_ERROR;
    }

    return rt;
}

STATIC OPERATE_RET __tdd_energy_monitor_bl0942_convert(BL0942_DRIVER_DATA_T *tdd_data, BL0942_UART_ALL_DATA_T reg_data, TDD_ENERGY_MONITOR_DATA_T *monitor_data)
{
    OPERATE_RET rt = OPRT_OK;
    UINT32_T cur_pf = 0;
    UINT32_T temp_val = 0;

    if (0 >= tdd_data->cal_params.voltage_period) {
        tdd_data->cal_params.voltage_period = V_COE;
    }
    if (0 >= tdd_data->cal_params.current_period) {
        tdd_data->cal_params.current_period = I_COE;
    }
    if (0 >= tdd_data->cal_params.power_period) {
        tdd_data->cal_params.power_period = P_COE;
    }
    if (0 >= tdd_data->cal_params.pf_num) {
        tdd_data->cal_params.pf_num = PF_UNIT;
    }

    // 电压转换
    temp_val = 0;
    temp_val = reg_data.V_RMS.byte_h<<16 | reg_data.V_RMS.byte_m<<8 | reg_data.V_RMS.byte_l;
    monitor_data->voltage = temp_val*10 / tdd_data->cal_params.voltage_period;
    // 电流转换
    temp_val = 0;
    temp_val = reg_data.I_RMS.byte_h<<16 | reg_data.I_RMS.byte_m<<8 | reg_data.I_RMS.byte_l;
    monitor_data->current = temp_val*100 / tdd_data->cal_params.current_period;
    // 实时功率转换
    // watt 寄存器的 bit[23] 为符号位
    temp_val = 0;
    temp_val = reg_data.WATT.byte_h<<16 | reg_data.WATT.byte_m<<8 | reg_data.WATT.byte_l;
    if (reg_data.WATT.byte_h & 0x80) {
        // 负功
        temp_val = ((~temp_val)&0x00FFFFFF) + 1;
    }
    // TAL_PR_DEBUG("WATT: %d", temp_val);
    monitor_data->power = temp_val*100 / tdd_data->cal_params.power_period;

    // TAL_PR_DEBUG("v: %d, c: %d, p: %d", monitor_data->voltage, monitor_data->current, monitor_data->power);


    cur_pf = reg_data.CF_CNT.byte_h<<16 | reg_data.CF_CNT.byte_m<<8 | reg_data.CF_CNT.byte_l;
    if (cur_pf >= tdd_data->last_pf) {
        // 这里 *1000 是为了方便下面计算 0.001 度的电量，并保证计量精度
        tdd_data->pf_cache += (cur_pf - tdd_data->last_pf) * 1000;
    } else {
        TAL_PR_ERR("energy monitor chip reset or pf number overflow");
    }
    tdd_data->last_pf = cur_pf;
    if (tdd_data->pf_cache >= tdd_data->cal_params.pf_num) {
        monitor_data->energy_consumed += tdd_data->pf_cache / tdd_data->cal_params.pf_num;
        tdd_data->pf_cache = tdd_data->pf_cache % tdd_data->cal_params.pf_num;
    }
    // TAL_PR_DEBUG("CF_CNT: %d, pf_cache: %d, energy_consumed: %d", cur_pf, tdd_data->pf_cache, monitor_data->energy_consumed);

    return rt;
}

STATIC VOID_T __tdd_energy_monitor_bl0942_timer_cb(TIMER_ID timer_id, VOID_T *arg)
{
    OPERATE_RET rt = OPRT_OK;
    STATIC UINT8_T first_cnt = 1;
    BL0942_DRIVER_DATA_T *tdd_data = NULL;
    BL0942_UART_ALL_DATA_T reg_data = {0};

    if (NULL == arg) {
        return;
    }
    tdd_data = (BL0942_DRIVER_DATA_T *)arg;

    if (first_cnt*SW_TIMER_CYCLE_TIMER < 1000) {
        // 1s 延时是为了保证芯片上电第一包采样正确
        first_cnt++;
        return;
    }

    tal_mutex_lock(tdd_data->mutex_hdl);

    rt = __tdd_bl0942_all_reg_uart_read(tdd_data, &reg_data);
    if (OPRT_OK != rt) {
        TAL_PR_DEBUG("read bl0942 data error");
        goto __EXIT;
    }
    tdd_data->monitor_data.voltage = 0;
    tdd_data->monitor_data.current = 0;
    tdd_data->monitor_data.power = 0;
    TUYA_CALL_ERR_GOTO(__tdd_energy_monitor_bl0942_convert(tdd_data, reg_data, &tdd_data->monitor_data), __EXIT);

__EXIT:
    tal_mutex_unlock(tdd_data->mutex_hdl);

    return;
}

STATIC OPERATE_RET __tdd_energy_monitor_open(TDD_EM_DRV_HANDLE handle)
{
    OPERATE_RET rt = OPRT_OK;
    TAL_UART_CFG_T uart_cfg = {0};
    BL0942_DRIVER_DATA_T *tdd_data = NULL;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    tdd_data = (BL0942_DRIVER_DATA_T *)handle;

    TAL_PR_DEBUG("bl0942 coe. v: %d, c: %d, p: %d, energy: %d", tdd_data->cal_params.voltage_period, tdd_data->cal_params.current_period, tdd_data->cal_params.power_period, tdd_data->cal_params.pf_num);

    if (tdd_data->hw_cfg.mode == BL0942_DRV_UART) {
        uart_cfg.rx_buffer_size = UART_BUFFER_SIZE_MAX;
        uart_cfg.base_cfg.baudrate = UART_DEFAULT_BAUDRATE;
        uart_cfg.base_cfg.databits = TUYA_UART_DATA_LEN_8BIT;
        uart_cfg.base_cfg.stopbits = TUYA_UART_STOP_LEN_1BIT;
        uart_cfg.base_cfg.flowctrl = TUYA_UART_FLOWCTRL_NONE;
        TUYA_CALL_ERR_RETURN(tal_uart_init(tdd_data->hw_cfg.driver.uart.id, &uart_cfg));

        tdd_data->drv_addr = 0x08 | (tdd_data->hw_cfg.driver.uart.addr & 0x03);
        TAL_PR_DEBUG("BL0942 uart driver addr: 0x%02x", tdd_data->drv_addr);

        __tdd_bl0942_reg_uart_soft_reset(tdd_data);
    } else {
        return OPRT_NOT_SUPPORTED;
    }

    TUYA_CALL_ERR_RETURN(tal_sw_timer_create(__tdd_energy_monitor_bl0942_timer_cb, handle, &tdd_data->timer_id));

    TUYA_CALL_ERR_RETURN(tal_sw_timer_start(tdd_data->timer_id, SW_TIMER_CYCLE_TIMER, TAL_TIMER_CYCLE));

    TUYA_CALL_ERR_RETURN(tal_mutex_create_init(&tdd_data->mutex_hdl));

    return rt;
}

STATIC OPERATE_RET __tdd_energy_monitor_close(TDD_EM_DRV_HANDLE handle)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    
    /* nothing todo */

    return rt;
}

STATIC OPERATE_RET __tdd_energy_monitor_read(TDD_EM_DRV_HANDLE handle, TDD_ENERGY_MONITOR_DATA_T *monitor_data)
{
    OPERATE_RET rt = OPRT_OK;
    BL0942_DRIVER_DATA_T *tdd_data = NULL;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    tdd_data = (BL0942_DRIVER_DATA_T *)handle;

    tal_mutex_lock(tdd_data->mutex_hdl);

    memcpy(monitor_data, &tdd_data->monitor_data, SIZEOF(TDD_ENERGY_MONITOR_DATA_T));
    tdd_data->monitor_data.energy_consumed = 0;

    // 低功率下测量不准，避免无负载的状态下还有功率
    if (monitor_data->power < 20) {
        monitor_data->power = 0;
        monitor_data->current = 0;
    } else if (monitor_data->current == 0) {
        monitor_data->power = 0;
    }

    tal_mutex_unlock(tdd_data->mutex_hdl);

    return rt;
}

STATIC OPERATE_RET __tdd_energy_monitor_bl0942_calibration(BL0942_DRIVER_DATA_T *tdd_data, ENERGY_MONITOR_CAL_PARAMS_T *cal_params)
{
    OPERATE_RET rt = OPRT_OK;
    UINT32_T power_sample_ms = 0;
    UINT32_T temp_val = 0;
    TDD_ENERGY_MONITOR_DATA_T tmp_data;
    BL0942_UART_ALL_DATA_T reg_data = {0};

    if (0 == tdd_data->cal_data.voltage || 0 == tdd_data->cal_data.current || 0 == tdd_data->cal_data.power) {
        return OPRT_INVALID_PARM;
    }

    tal_mutex_lock(tdd_data->mutex_hdl);

    tal_sw_timer_stop(tdd_data->timer_id);

    // 计算 0.0001 度需要的时间
    power_sample_ms =  1000 * 3600 / tdd_data->cal_data.power;
    TAL_PR_DEBUG("0.0001 kWh need %u ms", power_sample_ms);
    tal_system_sleep(power_sample_ms);

    memset(&reg_data, 0, SIZEOF(BL0942_UART_ALL_DATA_T));
    TUYA_CALL_ERR_GOTO(__tdd_bl0942_all_reg_uart_read(tdd_data, &reg_data), __EXIT);

    // 转换为国际单位
    TUYA_CALL_ERR_GOTO(__tdd_energy_monitor_bl0942_convert(tdd_data, reg_data, &tmp_data), __EXIT);

    cal_params->voltage_period = tmp_data.voltage;
    cal_params->current_period = tmp_data.current;
    cal_params->power_period   = tmp_data.power;

    // bl0942 pf 计数太小了，使用实际时间内计数 pf 会导致偏差过大
    temp_val = reg_data.WATT.byte_h<<16 | reg_data.WATT.byte_m<<8 | reg_data.WATT.byte_l;
    if (temp_val > 0 && tdd_data->cal_data.power > 0) {
        cal_params->pf_num = ((FLOAT_T)3600000/16384)*(temp_val * 100 / tdd_data->cal_data.power)/256;
    } else {
        cal_params->pf_num = 0;
    }

    TAL_PR_DEBUG("cal params: %d,  %d, %d, %d", cal_params->voltage_period, cal_params->current_period, cal_params->power_period, cal_params->pf_num);

__EXIT:

    tal_mutex_unlock(tdd_data->mutex_hdl);
    tal_sw_timer_start(tdd_data->timer_id, SW_TIMER_CYCLE_TIMER, TAL_TIMER_CYCLE);

    return rt;
}

STATIC OPERATE_RET __tdd_energy_monitor_config(TDD_EM_DRV_HANDLE handle, TDD_ENERGY_MONITOR_CMD_E cmd, VOID_T *arg)
{
    OPERATE_RET rt = OPRT_OK;
    BL0942_DRIVER_DATA_T *tdd_data = NULL;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    tdd_data = (BL0942_DRIVER_DATA_T *)handle;


    switch (cmd) {
        case (TDD_ENERGY_CMD_CAL_DATA_SET) : {
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            memcpy(&tdd_data->cal_data, arg, SIZEOF(ENERGY_MONITOR_CAL_DATA_T));
        } break;
        case (TDD_ENERGY_CMD_CAL_PARAMS_SET) : {
            ENERGY_MONITOR_CAL_PARAMS_T *temp_params = (ENERGY_MONITOR_CAL_PARAMS_T *)arg;
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            tdd_data->cal_params.voltage_period = V_COE;
            tdd_data->cal_params.current_period = I_COE;
            tdd_data->cal_params.power_period   = P_COE;
            tdd_data->cal_params.pf_num         = temp_params->pf_num;
        } break;
        case (TDD_ENERGY_CMD_CAL_PARAMS_GET) : {
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            TUYA_CALL_ERR_RETURN(__tdd_energy_monitor_bl0942_calibration(tdd_data, (ENERGY_MONITOR_CAL_PARAMS_T *)arg));
        } break;
        case (TDD_ENERGY_CMD_DEF_DATA_GET) :{
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            ENERGY_MONITOR_CAL_DATA_T *temp_data = (ENERGY_MONITOR_CAL_DATA_T *)arg;
            temp_data->voltage = tdd_data->cal_data.voltage;
            temp_data->current = tdd_data->cal_data.current;
            temp_data->power   = tdd_data->cal_data.power;
            temp_data->resval  = 1;
        } break;
        case (TDD_ENERGY_CMD_DEF_PARAMS_GET) : {
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            ENERGY_MONITOR_CAL_PARAMS_T *temp_params = (ENERGY_MONITOR_CAL_PARAMS_T *)arg;
            temp_params->voltage_period = tdd_data->cal_data.voltage;
            temp_params->current_period = tdd_data->cal_data.current;
            temp_params->power_period   = tdd_data->cal_data.power;
            temp_params->pf_num         = PF_UNIT;
        }break;
        default : return OPRT_NOT_SUPPORTED;
    }

    return rt;
}

OPERATE_RET tdd_energy_driver_bl0942_register(IN CHAR_T *name, IN BL0942_DRIVER_CONFIG_T cfg)
{
    OPERATE_RET rt = OPRT_OK;
    BL0942_DRIVER_DATA_T *tdd_data = NULL;
    ENERGY_DRV_INTFS_T intfs = {0};

    TUYA_CHECK_NULL_RETURN(name, OPRT_INVALID_PARM);

    tdd_data = (BL0942_DRIVER_DATA_T *)tal_malloc(SIZEOF(BL0942_DRIVER_DATA_T));
    TUYA_CHECK_NULL_RETURN(tdd_data, OPRT_MALLOC_FAILED);
    memset(tdd_data, 0, SIZEOF(BL0942_DRIVER_DATA_T));

    memcpy(&tdd_data->hw_cfg, &cfg, SIZEOF(BL0942_DRIVER_CONFIG_T));

    tdd_data->cal_data.voltage = TDD_ENERGY_220_REF_V;
    tdd_data->cal_data.current = TDD_ENERGY_220_REF_I;
    tdd_data->cal_data.power   = TDD_ENERGY_220_REF_P;
    tdd_data->cal_data.resval  = TDD_BL0942_SAMPLE_RES;

    tdd_data->cal_params.voltage_period = V_COE;
    tdd_data->cal_params.current_period = I_COE;
    tdd_data->cal_params.power_period   = P_COE;
    tdd_data->cal_params.pf_num = PF_UNIT; // 1度电 pf 个数

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
