/**
* @file example_user_ble_remote.c
* @author www.tuya.com
* @version 0.1
* @date 2022-05-20
*
* @copyright Copyright (c) tuya.inc 2022
*
*/

#include "tuya_cloud_types.h"

#if defined(ENABLE_BT_REMOTE_CTRL) && (ENABLE_BT_REMOTE_CTRL==1)
#include "tal_log.h"
#include "tuya_bt.h"

/***********************************************************
*************************micro define***********************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/

/**
* @brief 
*
* @param[in] data: data
* @param[in] len: data len
* @param[in] type: type
* @param[in] mac: device mac
* @return none
*/
STATIC VOID __ty_remote_receive_callback(UCHAR_T *data, UCHAR_T len,  UCHAR_T type, UCHAR_T* mac)
{
    //if do not support tuya remote, you can do nothing, return directly
    //if support tuya remote, you can refer to the example_ty_ble_remote.c

    return;
}

STATIC VOID __user_remote_receive_callback(TAL_BLE_ADV_REPORT_T *scan_info)
{
    TAL_PR_DEBUG("----------------user remote data cb-----------------");
    TAL_PR_DEBUG("adv_type:%d", scan_info->adv_type);
    TAL_PR_DEBUG("rssi:%d", scan_info->rssi);
    TAL_PR_DEBUG("data len:%d", scan_info->data_len );

    for (UINT_T i = 0; i < scan_info->data_len; i++) {
        TAL_PR_DEBUG_RAW("data[%d] : 0X%X", i, scan_info->p_data[i]);
        if((i+1)%20 == 0) {
            TAL_PR_DEBUG_RAW("\r\n");
        }     
    }

    return;
}

/**
* @brief  tuya bluetooth remote
*
* @param[in] param:Task parameters
* @return none
*/
VOID example_user_ble_remote(INT_T argc, CHAR_T *argv[])
{
    OPERATE_RET rt = OPRT_OK;

    /* enable ble scan function */
    TUYA_CALL_ERR_GOTO(tuya_ble_reg_app_scan_adv_cb(__ty_remote_receive_callback), __EXIT);

    /* register user remote */
    TUYA_CALL_ERR_GOTO(tuya_ble_reg_raw_scan_adv_cb(__user_remote_receive_callback), __EXIT);


__EXIT:
    return;
}
#endif