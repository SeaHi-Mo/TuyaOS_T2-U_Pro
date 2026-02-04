/**
* @file tdl_sound_sample.h
* @author www.tuya.com
* @brief tdl_sound_sample module is used to sample sound
* @version 0.1
* @date 2022-04-01
*
* @copyright Copyright (c) tuya.inc 2022
*
*/
 
#ifndef __TDL_SOUND_SAMPLE_H__
#define __TDL_SOUND_SAMPLE_H__
 
 
#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_types.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
#define SOUND_DEV_NAME_MAX_LEN                16u 
 
/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef void* SOUND_HANDLE_T;

typedef unsigned char S_SAMPLE_VAL_E;
#define S_VAL_MAX            0x00
#define S_VAL_AVG            0x01

typedef unsigned char S_APP_DATA_E;
#define S_DATA_RAW           0x00            //原始值
#define S_DATA_PERCENT       0x01            //千分比， 0-1000

/***********************************************************
***********************function define**********************
***********************************************************/
/**
* @brief          查找音频设备
*
* @param[in]      dev_name    设备名字
* @param[out]     handle      设备句柄
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
int tdl_sound_dev_find(char *dev_name, SOUND_HANDLE_T *handle);

/**
* @brief        打开设备
*
* @param[in]    handle        设备句柄
* @param[in]    val_tp        采集数值类型
* @param[in]    val_num       一次性采集的个数
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
int  tdl_sound_dev_open(SOUND_HANDLE_T handle, S_SAMPLE_VAL_E val_tp, unsigned char val_num);

/**             音量采集
* @brief 
*
* @param[in]    handle        设备句柄
* @param[in]    data_tp       音量数据类型
* @param[in]    num           音量数据组数
* @param[out]   data          音量数据
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
int tdl_sound_dev_sample(SOUND_HANDLE_T handle, S_APP_DATA_E data_tp, unsigned int num, unsigned int *data);

/**
* @brief        关闭设备
*
* @param[in]    handle        设备句柄指针
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
int  tdl_sound_dev_close(SOUND_HANDLE_T *handle);


 
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*__TDL_SOUND_SAMPLE_H__*/