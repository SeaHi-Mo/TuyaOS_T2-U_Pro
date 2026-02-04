/**
 * @file tdl_led_manage.h
 * @author www.tuya.com
 * @brief tdl_led module is used to
 * @version 1.0.0
 * @date 2022-05-16
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#ifndef __TDL_LED_MANAGE_H__
#define __TDL_LED_MANAGE_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
*************************micro define***********************
***********************************************************/
// #define TDL_LED_HANDLE        PVOID_T       // LED控制句柄
#define TDL_FLASH_FOREVER     (0xFFFFFFFFu) // 永久闪烁
#define TDL_FLASH_CNT_BASE(x) ((x) << 1)    // 获取闪烁次数

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef enum {
    TDL_LED_OFF,    // 熄灭
    TDL_LED_ON,     // 打开
    TDL_LED_FLASH,  // 闪烁
    TDL_LED_FINISH, // 不能使用
} TDL_LED_STAT_E;

typedef struct {
    TDL_LED_STAT_E stat; // 设置指示灯指示状态
    BOOL_T start_stat;   // 起始状态，在闪烁下有效. false-代表 TDL_LED_OFF
    BOOL_T end_stat;     // 结束后状态，在闪烁下有效

    UINT_T flash_cnt;         // 闪烁次数， TDL_FLASH_FOREVER代表永久闪烁，2代表亮灭一次
    UINT_T flash_first_time;  // 闪烁一次前半周期时间 ms 最小单位：50ms
    UINT_T flash_second_time; // 闪烁一次后半周期时间 ms 最小单位：50ms
} TDL_LED_CONFIG_T;

// 用于TDD底层实现的回调函数
typedef OPERATE_RET (*TDL_LED_CTRL_CB)(PVOID_T handle, TDL_LED_STAT_E stat);

/***********************************************************
***********************variable define**********************
***********************************************************/

/***********************************************************
***********************function define**********************
***********************************************************/
/**
 * @brief 设置指示灯互斥量
 *
 * @param[in] en TRUE - 使能(默认是使能无需调用)；  FALSE - 取消互斥量
 * @return OPRT_OK 代表成功。如果是其他错误码，请参考 tuya_error_code.h
 */
OPERATE_RET tdl_led_set_mutex_enable(IN BOOL_T en);

/**
 * @brief 对led进行控制
 *
 * @param[in] handle 句柄，通过句柄查找资源
 * @param[in] cfg 配置表
 * @return OPRT_OK 代表成功。如果是其他错误码，请参考 tuya_error_code.h
 */
OPERATE_RET tdl_led_ctrl(IN PVOID_T handle, IN TDL_LED_CONFIG_T *cfg);

/**
 * @brief 通过设备名找到对应的handle
 *
 * @param[in] dev_name  led name
 * @param[out] handle 操作的句柄对象
 * @return 
*/
OPERATE_RET tdl_led_dev_find(IN CHAR_T *dev_name, INOUT PVOID_T *handle);

/**
 * @brief 注册底层驱动实现，用于可无需关心具体驱动实现，只需关心控制对象
 *
 * @param[in] dev_name  led name
 * @param[in] handle 操作的句柄对象
 * @param[in] cb led驱动实现
 * @return OPRT_OK 代表成功。如果是其他错误码，请参考 tuya_error_code.h
 */
OPERATE_RET tdl_led_register_function(IN CHAR_T *dev_name, IN PVOID_T handle, IN TDL_LED_CTRL_CB cb);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__TDL_LED_H__*/
