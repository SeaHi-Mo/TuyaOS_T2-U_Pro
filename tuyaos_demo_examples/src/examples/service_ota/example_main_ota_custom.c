/**
 * @file example_main_ota_custom.c
 * @author www.tuya.com
 * @version 0.1
 * @date 2022-09-20
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#include "tuya_cloud_types.h"
#ifdef ENABLE_WIFI_SERVICE
#include "tuya_cloud_wifi_defs.h"
#include "tuya_iot_wifi_api.h"
#else
#include "tuya_iot_base_api.h"
#endif
#include "tuya_iot_com_api.h"

#include "tal_system.h"
#include "tal_log.h"
#include "tuya_config.h"
#include "tuya_svc_netmgr.h"
#include "tuya_devos_utils.h"
#include "tuya_svc_upgrade.h"
#include "tuya_svc_upgrade_http.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define FIRMWARE_KEY    "keyxskkas8m4s3m8" 
/***********************************************************
***********************typedef define***********************
***********************************************************/
#define ONE_PACK_LEN     512    

/***********************************************************
********************extern  declaration********************
***********************************************************/

/***********************************************************
***********************variable define**********************
***********************************************************/

/***********************************************************
***********************function define**********************
***********************************************************/

// qrcode打印
extern INT_T qrcode_exec(INT_T argc, CHAR_T **argv);
STATIC INT_T __qrcode_printf(CHAR_T *msg)
{
    CHAR_T *qrcode_argv[] = {
        "qrcode_exec", "-m", "3", "-t", "ansiutf8", msg
    };

    return qrcode_exec(sizeof(qrcode_argv)/sizeof(qrcode_argv[0]), qrcode_argv);
}

// TuyaOS获取到短链接之后调用此接口输出qrcode打印
STATIC VOID __qrcode_active_shourturl_cb(CONST CHAR_T *shorturl)
{
    if (NULL == shorturl) {
        return;
    }

    TAL_PR_DEBUG("shorturl : %s", shorturl);
    ty_cJSON *item = ty_cJSON_Parse(shorturl);
    __qrcode_printf(ty_cJSON_GetObjectItem(item, "shortUrl")->valuestring);
    ty_cJSON_Delete(item);

    return;
}

// main netlink module upgrade end notify
STATIC OPERATE_RET __main_module_upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data)
{

    TAL_PR_DEBUG("fw->tp:%d", fw->tp);
    TAL_PR_DEBUG("download result:%d", download_result);

    //todo:process result and soft reset。
    //After software reboot, the device will report the current firmware version once it is connected to the network.

    return OPRT_OK;
}


STATIC OPERATE_RET __main_module_upgrage_process_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len,
                                            IN CONST UINT_T offset, IN CONST BYTE_T *data,
                                            IN CONST UINT_T len, OUT UINT_T *remain_len, IN PVOID_T pri_data)
{
    TAL_PR_DEBUG("total_len:%d offset:%d, data_len:%d", total_len, offset, len);

     //todo: user can process firmware data

    return OPRT_OK;
}


// main netlink module upgrade start inform
STATIC VOID_T __main_module_ug_inform_cb(INOUT BOOL_T *handled, IN CONST FW_UG_S *fw)
{
    OPERATE_RET rt = OPRT_OK;

    // only support DEV_NM_ATH_SNGL upgrade
    if (fw->tp != DEV_NM_ATH_SNGL) {
        *handled = FALSE;
        return;
    }

    TAL_PR_DEBUG("pre dev ug, tp:%d, url:%s, hmac:%s, ver:%s, file_size:%d, diff:%d",
                 fw->tp, fw->fw_url, fw->fw_hmac, fw->sw_ver, fw->file_size, fw->diff_ota);

    *handled = TRUE;    


    //start pull firmware
    TUYA_CALL_ERR_GOTO(tuya_svc_upgrade_start(NULL, fw, __main_module_upgrage_process_cb, NULL, __main_module_upgrade_notify_cb, TRUE, 0), ERR_EXIT);

    return;

ERR_EXIT:
    set_gw_ext_stat(EXT_NORMAL_S);

    /*非局域网ota，才执行云端上报*/
    if (fw->type != UPGRADE_TYPE_LAN) {
        http_device_upgrade_stat_update(NULL, fw->tp, TUS_UPGRD_EXEC);
    }

    return;
}


/**
 * @brief  device cloud state change callback
 *
 * @param[in] status: current device status
 *
 * @return none
 */
STATIC VOID_T __dev_status_changed_cb(IN CONST GW_STATUS_E status)
{
    TAL_PR_DEBUG(" device status: %d", status);
    return;
}

/**
* @brief  device process device reset entry
*
* @param[in] type: gateway reset type
* @return none
*/
STATIC VOID_T __dev_reset_inform_cb(GW_RESET_TYPE_E type)
{
    //todo: clear app data

    return;
}

/**
* @brief  device format command data delivery entry
*
* @param[in] dp: obj dp info
* @return none
*/
STATIC VOID __dev_obj_dp_cmd_cb(IN CONST TY_RECV_OBJ_DP_S *dp)
{
    UINT_T index = 0;

    TAL_PR_DEBUG(" Rev DP Obj Cmd t1:%d t2:%d CNT:%u", dp->cmd_tp, dp->dtt_tp, dp->dps_cnt);

    for(index = 0; index < dp->dps_cnt; index++) {
        TAL_PR_DEBUG("Obj DP ID:%d", dp->dps[index].dpid);
    }

    dev_report_dp_json_async(NULL, dp->dps, dp->dps_cnt);

    return;
}


/**
 * @brief called after the cloud sends data of type raw
 *
 * @param[in] dp: raw dp info
 *
 * @return none
 */
STATIC VOID __dev_raw_dp_cmd_cb(IN CONST TY_RECV_RAW_DP_S *dp)
{
    UINT_T i = 0;

    TAL_PR_DEBUG("SOC Rev DP Raw Cmd t1:%d t2:%d dpid:%d len:%u", dp->cmd_tp, dp->dtt_tp, dp->dpid, dp->len);

    for (i=0; i<dp->len; i++) {
        TAL_PR_DEBUG_RAW("0x%02x ", dp->data[i]);
    }

    dev_report_dp_raw_sync(NULL, dp->dpid, dp->data, dp->len, 5);

    return;
}

/**
* @brief query device dp status
*
* @param[in] dp_qry: query info
*
* @return none
*/
STATIC VOID __dev_dp_query_cb(IN CONST TY_DP_QUERY_S *dp_qry)
{
    TAL_PR_DEBUG("SOC Rev DP Query Cmd");

    if(dp_qry->cnt == 0) {
        TAL_PR_DEBUG("soc rev all dp query");
        // UserTODO
    }else {
        TAL_PR_DEBUG("soc rev dp query cnt:%d", dp_qry->cnt);
        UINT_T index = 0;
        for(index = 0; index < dp_qry->cnt; index++) {
            TAL_PR_DEBUG("rev dp query:%d", dp_qry->dpid[index]);
            // UserTODO
        }
    }
}

/**
 * @brief SOC external network status change callback
 * 
 * @param[in/out] data 
 * @return STATIC 
 */
STATIC OPERATE_RET __soc_dev_net_status_cb(VOID *data)
{
    TAL_PR_DEBUG("network status changed!");
    netmgr_linkage_t *default_linkage = tuya_svc_netmgr_linkage_get(LINKAGE_TYPE_DEFAULT);
    if (default_linkage) {
        TAL_PR_DEBUG("linkage status changed, current active is [%s], status is up[%d]", default_linkage->type, tuya_svc_netmgr_linkage_is_up(LINKAGE_TYPE_DEFAULT));
    } else {
        TAL_PR_DEBUG("no active linkage, status is up[0]");
    }
    
    // UserTODO

    return OPRT_OK;
}

/**
 * @brief main netlink module firmware ota
 *
 * @param[in] none:
 *
 * @return none
 */
VOID example_main_ota_custom(INT_T argc, CHAR_T *argv[])
{
    OPERATE_RET rt = OPRT_OK;

    ty_subscribe_event(EVENT_LINK_UP, "quickstart", __soc_dev_net_status_cb, SUBSCRIBE_TYPE_NORMAL);
    ty_subscribe_event(EVENT_LINK_DOWN, "quickstart", __soc_dev_net_status_cb, SUBSCRIBE_TYPE_NORMAL);
    ty_subscribe_event(EVENT_MQTT_CONNECTED, "quickstart", __soc_dev_net_status_cb, SUBSCRIBE_TYPE_NORMAL);

    // Initialize TuyaOS product information
    TY_IOT_CBS_S iot_cbs = {0};
    iot_cbs.gw_status_cb    = __dev_status_changed_cb;
    iot_cbs.gw_reset_cb     = __dev_reset_inform_cb;
    iot_cbs.dev_obj_dp_cb   = __dev_obj_dp_cmd_cb;
    iot_cbs.dev_raw_dp_cb   = __dev_raw_dp_cmd_cb;
    iot_cbs.dev_dp_query_cb = __dev_dp_query_cb;

    // register main netlink module upgrade inform callback
    tuya_svc_upgrade_register_pre_cb(__main_module_ug_inform_cb);

#ifdef ENABLE_WIFI_SERVICE
    TUYA_CALL_ERR_LOG(tuya_iot_wf_dev_init(GWCM_OLD, WF_START_SMART_AP_CONCURRENT, &iot_cbs,\
                                           FIRMWARE_KEY, PID, USER_SW_VER, DEV_NM_ATH_SNGL,\
                                           NULL, 0));
#else
    iot_cbs.active_shorturl = __qrcode_active_shourturl_cb;
    TUYA_CALL_ERR_LOG(tuya_iot_dev_init(&iot_cbs, FIRMWARE_KEY, PID, USER_SW_VER, DEV_NM_ATH_SNGL,\
                                        NULL, 0));   
#endif

    return;
}