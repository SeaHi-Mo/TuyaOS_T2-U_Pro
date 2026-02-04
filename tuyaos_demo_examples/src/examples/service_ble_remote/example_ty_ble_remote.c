/**
* @file example_ty_ble_remote.c
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
    INT_T i = 0;

    if(type == 0xff){
        TAL_PR_DEBUG("ble remote(old) send data len: %d", len);
        for ( i = 0; i < len; i++) {
            TAL_PR_DEBUG("data[%d] : %d", i, data[i]);        
        }
    }else if(type == 0x16){
        TAL_PR_DEBUG("ble remote(new) send data len: %d", len);
        for ( i = 0; i < len; i++) {
            TAL_PR_DEBUG("data[%d] : %d", i, data[i]);        
        }
    }

    return;
}

/**
* @brief ble scanf adv bind check callback
*
* @param[in] type: bind type for bluetooth remote controller
* @param[in] data: data
* @param[in] len: data len
* @return OPRT_OK
*/
STATIC OPERATE_RET __ty_remote_bind_check_callback(TUYA_BLE_BIND_TYPE type, UCHAR_T *data, UCHAR_T len)
{
    INT_T i = 0;
    
    TAL_PR_DEBUG("----------------scan adv bind check cb-----------------");
    TAL_PR_DEBUG("ble bind type : %d, data len: %d", type, len);
    for(i=0;i<len;i++) {
        TAL_PR_DEBUG("data[%d] : %d", i, data[i]);
    }

    return OPRT_OK;
}

/**
* @brief ble scan adv bind notify callback
*
* @param[in] type: bind type for bluetooth remote controller
* @return none
*/
STATIC VOID_T __ty_remote_bind_notify_callback(TUYA_BLE_BIND_TYPE type, int rslt)
{
    TAL_PR_DEBUG("------------------ble scan adv bind notify cb----------------");
    TAL_PR_DEBUG("ble bind type : %d", type);
    TAL_PR_DEBUG("result : %d", rslt);

    return ;
}

/**
* @brief  tuya bluetooth remote
*
* @param[in] param:Task parameters
* @return none
*/
VOID example_ty_ble_remote(INT_T argc, CHAR_T *argv[])
{
    OPERATE_RET rt = OPRT_OK;
    TUYA_BLE_SCAN_ADV_HANDLE_CBS ble_scan_cfg = {0};
    ble_scan_cfg.bind_check = __ty_remote_bind_check_callback;
    ble_scan_cfg.bind_notify = __ty_remote_bind_notify_callback;

    /* enable ble scan function */
    TUYA_CALL_ERR_GOTO(tuya_ble_reg_app_scan_adv_cb(__ty_remote_receive_callback), __EXIT);

    /* tuya remote bind notify */
    TUYA_CALL_ERR_GOTO(tuya_ble_reg_app_scan_adv_handle_cbs(&ble_scan_cfg), __EXIT);

    /* default 30S, If you feel the time is too short, you can invoke the interface to make modifications. */
    // tuya_ble_set_bind_window(60*3);

__EXIT:
    return;
}
#endif