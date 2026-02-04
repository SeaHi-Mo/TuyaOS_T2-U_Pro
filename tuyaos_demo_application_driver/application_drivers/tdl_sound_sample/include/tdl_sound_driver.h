/**
* @file tdl_sound_driver.h
* @author www.tuya.com
* @brief tdl_sound_driver module is used to provid sound device abstract
* @version 0.1
* @date 2022-04-01
*
* @copyright Copyright (c) tuya.inc 2022b
*
*/
 
#ifndef __TDL_SOUND_DRIVER_H__
#define __TDL_SOUND_DRIVER_H__
 
 
#ifdef __cplusplus
extern "C" {
#endif
 
/***********************************************************
*************************micro define***********************
***********************************************************/
 
 
/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef void* SOUND_DIR_HANDLE_T;

typedef struct {
    int (*open)  (SOUND_DIR_HANDLE_T handle);
    int (*close) (SOUND_DIR_HANDLE_T handle);
    int (*sample)(SOUND_DIR_HANDLE_T handle, unsigned int *data);
} SOUND_INTFS_T;  
 
/***********************************************************
***********************function define**********************
***********************************************************/
/**
* @brief           音量采集驱动注册
*
* @param[in]       dev_name           设备名字
* @param[in]       intfs              操作接口
* @param[in]       maximum            音量最大值
* @param[in]       drv                驱动句柄
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
int tdl_sound_driver_register(char *dev_name, SOUND_INTFS_T *intfs, \
                              unsigned int maximum, SOUND_DIR_HANDLE_T drv);

 
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*__TDL_SOUND_DRIVER_H__*/