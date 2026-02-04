/**
 * @file example_attach_ota.c
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

/***********************************************************
************************macro define************************
***********************************************************/
#define DEVICE_PID      "bkmtixnoekekcsgn"
#define FIRMWARE_KEY    "keyjmgyq7d7cam7w" 
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

STATIC VOID_T __print_firmware_data(IN CONST BYTE_T *data, IN CONST UINT_T len)
{
    UINT_T i = 0;

    if(NULL == data || 0 == len) {
        return;
    }

    for(i = 0; i < len; i++) {
        TAL_PR_DEBUG_RAW("%02X ", data[i]);
        if(0 == ((i+1)%20)) {
            TAL_PR_DEBUG_RAW("\r\n");
        }
    }

    return;
}

STATIC VOID_T __update_firmware_version(IN CONST GW_PERMIT_DEV_TP_T tp, IN CONST CHAR_T *version)
{
    GW_ATTACH_ATTR_T attr = {0};

    TAL_PR_DEBUG("update tp:%d version:%s", tp, version);

    memset((UCHAR_T *)&attr, 0x00, SIZEOF(GW_ATTACH_ATTR_T));
    attr.tp  = tp;
    strncpy(attr.ver, version, SW_VER_LEN);
    tuya_iot_dev_set_attach_attr(tp, &attr);

    return;
}

STATIC OPERATE_RET __get_attach_fw_data_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len, IN CONST UINT_T offset,
                                          IN CONST BYTE_T *data, IN CONST UINT_T len, OUT UINT_T *remain_len, IN PVOID_T pri_data)
{
    UINT_T pack_cnt = 0, i = 0, send_len = 0;

    TAL_PR_DEBUG("Rev File Data");
    TAL_PR_DEBUG("Total_len:%d ", total_len);
    TAL_PR_DEBUG("Offset:%d Len:%d", offset, len);

    pack_cnt = len/ONE_PACK_LEN;
    *remain_len = len % ONE_PACK_LEN;

    for (i=0; i<pack_cnt; i++) {
        __print_firmware_data(&data[send_len], ONE_PACK_LEN);
        send_len += ONE_PACK_LEN;
    }

    // last pack
    if (offset + len == total_len) {
        TAL_PR_DEBUG("the last fw pack");

        if ((*remain_len) > 0) {
            __print_firmware_data(&data[send_len], (*remain_len));
            send_len += (*remain_len);
            *remain_len = 0;
        }

        //update version
        __update_firmware_version(fw->tp, fw->sw_ver);
    }

    return OPRT_OK;
}

STATIC OPERATE_RET __attach_download_result_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data)
{
    TAL_PR_DEBUG("fw->tp:%d", fw->tp);
    TAL_PR_DEBUG("download result:%d", download_result);

    return OPRT_OK;
}

/**
 * @brief  attach module upgrade entry
 *
 * @param[in] fw: firmware info
 *
 * @return none
 */
STATIC INT_T __dev_rev_attatch_upgrade_info_cb(IN CONST FW_UG_S *fw)
{
    OPERATE_RET rt = OPRT_OK;

    TAL_PR_DEBUG("attach upgrade Info");
    TAL_PR_DEBUG("fw->tp:%d", fw->tp);
    TAL_PR_DEBUG("fw->fw_url:%s", fw->fw_url);
    TAL_PR_DEBUG("fw->fw_hmac:%s", fw->fw_hmac);
    TAL_PR_DEBUG("fw->sw_ver:%s", fw->sw_ver);
    TAL_PR_DEBUG("fw->file_size:%u", fw->file_size);

    TUYA_CALL_ERR_RETURN(tuya_iot_upgrade_gw(fw, __get_attach_fw_data_cb, \
                                             __attach_download_result_notify_cb, NULL));

    return OPRT_OK;
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
 * @brief main module firmware ota
 *
 * @param[in] none:
 *
 * @return none
 */
VOID example_attach_ota(INT_T argc, CHAR_T *argv[])
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

    //attach module upgrade
    iot_cbs.gw_ug_cb        = __dev_rev_attatch_upgrade_info_cb;

    GW_ATTACH_ATTR_T attach_arr[] = {
        {
            .tp = DEV_NM_NOT_ATH_SNGL,
            .ver = "1.0.0",
        },
        {
            .tp = DEV_ATTACH_MOD_1,
            .ver = "1.0.0",
        },
    };
#ifdef ENABLE_WIFI_SERVICE
    TUYA_CALL_ERR_LOG(tuya_iot_wf_dev_init(GWCM_OLD, WF_START_SMART_AP_CONCURRENT, &iot_cbs,\
                                           FIRMWARE_KEY, DEVICE_PID, USER_SW_VER, DEV_NM_ATH_SNGL,\
                                           attach_arr, CNTSOF(attach_arr)));
#else
    iot_cbs.active_shorturl = __qrcode_active_shourturl_cb;
    TUYA_CALL_ERR_LOG(tuya_iot_dev_init(&iot_cbs, FIRMWARE_KEY, DEVICE_PID, USER_SW_VER, DEV_NM_ATH_SNGL,\
                                           attach_arr, CNTSOF(attach_arr)));   
#endif

    return;
}