/**
 * @file tdl_energy_monitor_driver.h
 * @author www.tuya.com
 * @brief tdl_energy_monitor_driver module is used to 
 * @version 0.1
 * @date 2023-07-13
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TDL_ENERGY_MONITOR_DRIVER_H__
#define __TDL_ENERGY_MONITOR_DRIVER_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
typedef UINT8_T TDD_ENERGY_MONITOR_CMD_E;
#define TDD_ENERGY_CMD_CAL_DATA_SET         0
#define TDD_ENERGY_CMD_CAL_PARAMS_SET       1
#define TDD_ENERGY_CMD_CAL_PARAMS_GET       2
#define TDD_ENERGY_CMD_DEF_DATA_GET         3
#define TDD_ENERGY_CMD_DEF_PARAMS_GET       4

/**
 *  calibration data
 *  涂鸦默认 220v 下校准环境
 */
#define TDD_ENERGY_220_REF_V            2200 // 220v，扩大 10 倍
#define TDD_ENERGY_220_REF_I            392 // 0.392A，扩大 1000 倍
#define TDD_ENERGY_220_REF_P            864 // 86.4w，扩大 10 倍数

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef PVOID_T TDD_EM_DRV_HANDLE;

typedef struct {
    UINT32_T voltage; // 实际电压扩大 10 倍。如：220.0v 应设置为 2200
    UINT32_T current; // 实际电流扩大 1000 倍。如：0.392A 应设置为 392
    UINT32_T power; // 实际实时功率扩大 10 倍。如：86.4w 应设置为 864
    UINT32_T resval; // 电量采样电阻，单位 1mR
}ENERGY_MONITOR_CAL_DATA_T;
typedef struct {
    UINT32_T voltage_period;
    UINT32_T current_period;
    UINT32_T power_period;
    UINT32_T pf_num; // 0.001 度电需要的 pf 脉冲个数
}ENERGY_MONITOR_CAL_PARAMS_T;

typedef struct{
    UINT32_T voltage; // 扩大 10 倍。如：220.0v 为 2200
    UINT32_T current; // 单位 mA。如：0.392A  为 392
    UINT32_T power; // 扩大 10 倍。如：86.4w 为 864
    UINT32_T energy_consumed; // 单位 0.001 度电
} TDD_ENERGY_MONITOR_DATA_T;

typedef struct {
    OPERATE_RET (*open)(TDD_EM_DRV_HANDLE handle);
    OPERATE_RET (*close)(TDD_EM_DRV_HANDLE handle);
    OPERATE_RET (*read)(TDD_EM_DRV_HANDLE handle, TDD_ENERGY_MONITOR_DATA_T *monitor_data);
    OPERATE_RET (*config)(TDD_EM_DRV_HANDLE handle, TDD_ENERGY_MONITOR_CMD_E cmd, VOID_T *arg);
}ENERGY_DRV_INTFS_T;

typedef VOID_T (*TDD_ENERGY_MONITOR_NOTIFY_CB)(PVOID_T tdl_handle, TDD_ENERGY_MONITOR_CMD_E cmd, VOID_T *args);

/***********************************************************
********************function declaration********************
***********************************************************/
OPERATE_RET tdl_energy_monitor_driver_register(IN CHAR_T *name, IN ENERGY_DRV_INTFS_T *intfs, IN TDD_EM_DRV_HANDLE tdd_hdl);

#ifdef __cplusplus
}
#endif

#endif /* __TDL_ENERGY_MONITOR_DRIVER_H__ */
