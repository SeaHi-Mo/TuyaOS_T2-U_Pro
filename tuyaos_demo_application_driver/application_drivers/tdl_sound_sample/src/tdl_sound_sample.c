/**
* @file tdl_sound_sample.c
* @author www.tuya.com
* @brief tdl_sound_sample module is used to sample sound 
* @version 0.1
* @date 2022-04-01
*
* @copyright Copyright (c) tuya.inc 2022
*
*/
#include <string.h>

#include "tal_mutex.h"
#include "tal_log.h"
#include "tal_memory.h"

#include "tdl_sound_sample.h"
#include "tdl_sound_driver.h" 
/***********************************************************
*************************micro define***********************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct sound_dev_list{
    struct sound_dev_list *next;
    CHAR_T                 name[SOUND_DEV_NAME_MAX_LEN+1];
    MUTEX_HANDLE           mutex;

    UCHAR_T                is_open;
    S_SAMPLE_VAL_E         val_tp;
    UCHAR_T                val_num;        

    UINT_T                 maximum;
    SOUND_DIR_HANDLE_T     drv;
    SOUND_INTFS_T          intf;
}SOUND_DEV_NODE_T, SOUND_DEV_LIST_T;

/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC SOUND_DEV_LIST_T g_sound_dev_list = {
    .next = NULL,
};

/***********************************************************
***********************function define**********************
***********************************************************/
STATIC SOUND_DEV_NODE_T *__tdl_sound_find_dev(char *name)
{
    SOUND_DEV_NODE_T *node = NULL;

    node = g_sound_dev_list.next;
    while(node) {
        if(0 == strcmp(node->name, name)) {
            return node;
        }

        node = node->next;
    }

    return NULL;
}

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
                                unsigned int maximum, SOUND_DIR_HANDLE_T drv)
{
    OPERATE_RET op_ret = OPRT_OK;
    SOUND_DEV_NODE_T *device = NULL, *last_node = NULL;

    if(NULL == dev_name || NULL == intfs || 0 == maximum) {
        return OPRT_INVALID_PARM;
    }

    if(NULL == intfs->sample) {
        return OPRT_INVALID_PARM;
    }

    if(NULL != __tdl_sound_find_dev(dev_name)) {
        TAL_PR_ERR("device:%s is already exist", dev_name);
        return OPRT_COM_ERROR;
    }

    device = (SOUND_DEV_NODE_T *)tal_malloc(sizeof(SOUND_DEV_NODE_T));
    if(NULL == device) {
        return OPRT_MALLOC_FAILED;
    }
    memset((unsigned char *)device, 0x00, sizeof(SOUND_DEV_NODE_T));

    op_ret = tal_mutex_create_init(&device->mutex);
    if(OPRT_OK != op_ret) {
        TAL_PR_ERR("tal_mutex_create_init err:%d", op_ret);
        tal_free(device);
        return op_ret;
    }

    strncpy(device->name, dev_name, SOUND_DEV_NAME_MAX_LEN);
    memcpy((unsigned char *)&device->intf, (unsigned char *)intfs, sizeof(SOUND_INTFS_T));
    device->maximum = maximum;
    device->drv = drv;

    //find the last node
    last_node = &g_sound_dev_list;
    while(last_node->next) {
        last_node = last_node->next;
    }

    //add node
    last_node->next = device;

    return OPRT_OK;
}

/**
* @brief          查找音频设备
*
* @param[in]      dev_name    设备名字
* @param[out]     handle      设备句柄
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
int tdl_sound_dev_find(char *dev_name, SOUND_HANDLE_T *handle)
{
    SOUND_DEV_NODE_T *device = NULL;

    if(NULL == dev_name || NULL == handle) {
        return OPRT_INVALID_PARM;
    }

    device = __tdl_sound_find_dev(dev_name);
    if(NULL == device) {
        TAL_PR_ERR("cant find device:%s", dev_name);
        return OPRT_INVALID_PARM;
    }

    *handle = (SOUND_HANDLE_T)device;

    return OPRT_OK;
}

/**
* @brief        打开设备
*
* @param[in]    handle        设备句柄
* @param[in]    val_tp        采集数值类型
* @param[in]    val_num       一次性采集的个数
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
int  tdl_sound_dev_open(SOUND_HANDLE_T handle, S_SAMPLE_VAL_E val_tp, unsigned char val_num)
{
    OPERATE_RET op_ret = OPRT_OK;
    SOUND_DEV_NODE_T *device = NULL;

    if(NULL == handle || val_tp > S_VAL_AVG || 0 == val_num) {
        return OPRT_INVALID_PARM;
    }

    device = (SOUND_DEV_NODE_T *)handle;

    if(1 == device->is_open) {
        TAL_PR_WARN("device is already open");
        return OPRT_COM_ERROR;
    }

    tal_mutex_lock(device->mutex);

    device->val_tp = val_tp;
    device->val_num = val_num;

    if(device->intf.open) {
        op_ret = device->intf.open(device->drv);
        if(op_ret != OPRT_OK) {
            TAL_PR_ERR("tdd driver open err:%d", op_ret);
            tal_mutex_unlock(device->mutex);
            return OPRT_MALLOC_FAILED;
        }
    }

    device->is_open = 1;

    tal_mutex_unlock(device->mutex);

    return OPRT_OK;
}

STATIC INT_T __tdl_sound_sample_val(SOUND_DEV_NODE_T *device, unsigned int *data)
{
    INT_T op_ret = 0;
    UINT_T i = 0, tmp = 0, sum = 0, sample_data = 0;

    for(i=0; i<device->val_num; i++) {
        op_ret = device->intf.sample(device->drv, &sample_data);
        if(op_ret != 0) {
            TAL_PR_ERR("tdd sample err:%d", op_ret);
            return op_ret;
        }

        if(S_VAL_MAX == device->val_tp) {
            tmp = (sample_data > tmp) ? sample_data : tmp;
        }else {
            sum += sample_data;
            tmp = sum / (i+1);
        }
    }

    *data = tmp;

    return OPRT_OK;
}

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
int tdl_sound_dev_sample(SOUND_HANDLE_T handle, S_APP_DATA_E data_tp, unsigned int num, unsigned int *data)
{
    int op_ret = 0;
    UINT_T i = 0, tmp = 0;
    SOUND_DEV_NODE_T *device = NULL;

    if(NULL == handle || data_tp > S_DATA_PERCENT || \
        0 == num || NULL == data) {
            return OPRT_INVALID_PARM;
    }

    device = (SOUND_DEV_NODE_T *)handle;

    if(0 == device->is_open) {
        TAL_PR_ERR("device is not open");
        return OPRT_COM_ERROR;
    }

    tal_mutex_lock(device->mutex);

    for(i = 0; i <num; i++) {
        op_ret = __tdl_sound_sample_val(device, &tmp);
        if(op_ret !=  OPRT_OK) {
            tal_mutex_unlock(device->mutex);
            return op_ret;
        }
        if(S_DATA_PERCENT == data_tp) {
            data[i] = tmp*1000 / device->maximum;
        }else {
            data[i] = tmp;
        }
    }

    tal_mutex_unlock(device->mutex);

    return OPRT_OK;
}

/**
* @brief        关闭设备
*
* @param[in]    handle        设备句柄指针
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
int  tdl_sound_dev_close(SOUND_HANDLE_T *handle)
{
    OPERATE_RET op_ret = OPRT_OK;
    SOUND_DEV_NODE_T *device = NULL;

    if(NULL == handle || NULL == *handle) {
            return OPRT_INVALID_PARM;
    }

    device = (SOUND_DEV_NODE_T *)(*handle);

    if(0 == device->is_open) {
        TAL_PR_NOTICE("device is already close");
        return OPRT_OK;
    }

    tal_mutex_lock(device->mutex);  

    if(device->intf.close) {
        op_ret = device->intf.close(device->drv);
        if(op_ret != 0) {
            TAL_PR_ERR("tdd close err:%d", op_ret);
            tal_mutex_unlock(device->mutex);
            return op_ret;
        }
        device->drv = NULL;
    }

    device->val_tp = 0;
    device->val_num = 0;
    device->is_open = 0;

    *handle = NULL;

    tal_mutex_unlock(device->mutex);

    return OPRT_OK;
}
