/**
 * @file tdl_energy_monitor_manage.h
 * @author www.tuya.com
 * @brief tdl_energy_monitor_manage module is used to 
 * @version 0.1
 * @date 2023-07-13
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TDL_ENERGY_MONITOR_MANAGE_H__
#define __TDL_ENERGY_MONITOR_MANAGE_H__

#include "tuya_cloud_types.h"
#include "tdl_energy_monitor_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define ENERGY_MONITOR_NAME_LEN_MAX     (16)

typedef UINT8_T TDL_ENERGY_MONITOR_CMD_E;
#define TDL_EM_CMD_CAL_DATA_SET         0 // 校准环境设置，输入数据类型应为 ENERGY_MONITOR_CAL_DATA_T
#define TDL_EM_CMD_CAL_PARAMS_SET       1 // 校准参数设置，输入数据类型应为 ENERGY_MONITOR_CAL_PARAMS_T

#define TDL_EM_CMD_UNKNOW               (0xFFFF)

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef PVOID_T ENERGY_MONITOR_HANDLE_T;

typedef struct {
    UINT32_T voltage; // 实际电压扩大 10 倍。如：220.0v 应设置为 2200
    UINT32_T current; // 实际电流扩大 1000 倍。如：0.392A 应设置为 392
    UINT32_T power; // 实际实时功率扩大 10 倍。如：86.4w 应设置为 864
} ENERGY_MONITOR_VCP_T;

typedef VOID_T (*ENERGY_MONITOR_NOTIFY_CALLBACK)(ENERGY_MONITOR_HANDLE_T handle, TDL_ENERGY_MONITOR_CMD_E cmd, VOID_T *args);

/***********************************************************
********************function declaration********************
***********************************************************/
/**
 * @brief 查找计量设备
 *
 * @param[in] name: 设备名称
 * @param[out] handle: 设备句柄
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tdl_energy_monitor_find(CHAR_T *name, ENERGY_MONITOR_HANDLE_T *handle);

/**
 * @brief 打开电量统计设备
 *
 * @param[in] handle: 电量统计句柄
 * @param[in] ref_params: 电量统计校准时的参考电压
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tdl_energy_monitor_open(ENERGY_MONITOR_HANDLE_T handle);

/**
 * @brief 计量实时数据读取
 *
 * @param[in] handle: 设备句柄
 * @param[out] vcp_data: 当前电压，电流，功率值
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tdl_energy_monitor_read_vcp(ENERGY_MONITOR_HANDLE_T handle, ENERGY_MONITOR_VCP_T *vcp_data);

/**
 * @brief 累计电量数据读取
 *
 * @param[in] handle: 设备句柄
 * @param[out] energy_consumed: 累计电量，单位为 0.001 kWh
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tdl_energy_monitor_read_energy(ENERGY_MONITOR_HANDLE_T handle, UINT32_T *energy_consumed);

/**
 * @brief 累计电量数据读取
 *
 * @param[in] handle: 设备句柄
 * @param[in] cmd: 要操作的命令，支持的命令可以查看 TDL_ENERGY_MONITOR_CMD_E
 * @param[inout] args: 输入命令参数或返回数据
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tdl_energy_monitor_config(ENERGY_MONITOR_HANDLE_T handle, TDL_ENERGY_MONITOR_CMD_E cmd, void *args);

/**
 * @brief 获取校准参数
 *
 * @param[in] handle: 设备句柄
 * @param[in] cal_data: 校准环境值
 * @param[out] cal_params: 校准得到的参数值
 *
 * @return 校准与内部参数比较得到的误差值
 */
UINT32_T tdl_energy_monitor_calibration(ENERGY_MONITOR_HANDLE_T handle, ENERGY_MONITOR_CAL_DATA_T cal_data, OUT ENERGY_MONITOR_CAL_PARAMS_T *cal_params);

/**
 * @brief 关闭计量设备
 *
 * @param[in] handle: 设备句柄
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tdl_energy_monitor_close(ENERGY_MONITOR_HANDLE_T handle);

#ifdef __cplusplus
}
#endif

#endif /* __TDL_ENERGY_MONITOR_MANAGE_H__ */
