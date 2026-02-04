/**
 * @file example_service_soc.c
 * @author www.tuya.com
 * @brief example_service_soc module is used to 
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
#include "tal_thread.h"
#include "tuya_config.h"
#include "tuya_svc_netmgr.h"
#include "mqc_app.h"

/***********************************************************
************************macro define************************
***********************************************************/

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
********************extern  declaration********************
***********************************************************/

/***********************************************************
***********************variable define**********************
***********************************************************/

/***********************************************************
***********************function define**********************
***********************************************************/
// output qrcode when ENABLE_QRCODE_ACTIVE == 1 && ENABLE_WIFI_QRCODE == 0
#if (defined(ENABLE_QRCODE_ACTIVE) && (ENABLE_QRCODE_ACTIVE == 1)) && (!(defined(ENABLE_WIFI_QRCODE) && (ENABLE_WIFI_QRCODE == 1)))
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
#endif

/**
 * @brief SOC device cloud state change callback
 *
 * @param[in] status: current device status
 *
 * @return none
 */
STATIC VOID_T __soc_dev_status_changed_cb(IN CONST GW_STATUS_E status)
{
    TAL_PR_DEBUG("soc device status: %d", status);
    return;
}

/**
* @brief SOC device process device reset entry
*
* @param[in] type: gateway reset type
* @return none
*/
STATIC VOID_T __soc_dev_reset_inform_cb(GW_RESET_TYPE_E type)
{
    //todo: clear app data
    return;
}

/**
* @brief  SOC device format command data delivery entry
*
* @param[in] dp: obj dp info
* @return none
*/
STATIC VOID __soc_dev_obj_dp_cmd_cb(IN CONST TY_RECV_OBJ_DP_S *dp)
{
    UINT_T index = 0;

    TAL_PR_DEBUG("SOC Rev DP Obj Cmd t1:%d t2:%d CNT:%u", dp->cmd_tp, dp->dtt_tp, dp->dps_cnt);

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
STATIC VOID __soc_dev_raw_dp_cmd_cb(IN CONST TY_RECV_RAW_DP_S *dp)
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
STATIC VOID __soc_dev_dp_query_cb(IN CONST TY_DP_QUERY_S *dp_qry)
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
    if (tuya_svc_netmgr_linkage_is_up(LINKAGE_TYPE_DEFAULT)) {
        TAL_PR_DEBUG("linkage status changed, current status is up");
        if (get_mqc_conn_stat()) {
            TAL_PR_DEBUG("cloud is connected!");

            // UserTODO
        }
    } else {
        TAL_PR_DEBUG("linkage status changed, current status is down");

        // UserTODO
    }

    return OPRT_OK;
}

/**
 * @brief soc device init
 *
 * @param[in] none:
 *
 * @return none
 */
VOID example_soc_device_init(INT_T argc, CHAR_T *argv[])
{
    OPERATE_RET rt = OPRT_OK;
    CHAR_T *pid = NULL;

    if(argc > 1) {
        pid = argv[1];
    }else {
        pid = PID;
    }

    TAL_PR_NOTICE("device pid:%s", pid);

    ty_subscribe_event(EVENT_LINK_UP, "quickstart", __soc_dev_net_status_cb, SUBSCRIBE_TYPE_NORMAL);
    ty_subscribe_event(EVENT_LINK_DOWN, "quickstart", __soc_dev_net_status_cb, SUBSCRIBE_TYPE_NORMAL);
    ty_subscribe_event(EVENT_MQTT_CONNECTED, "quickstart", __soc_dev_net_status_cb, SUBSCRIBE_TYPE_NORMAL);

    // Initialize TuyaOS product information
    TY_IOT_CBS_S iot_cbs = {0};

    memset((UCHAR_T *)&iot_cbs, 0x00, SIZEOF(TY_IOT_CBS_S));

    iot_cbs.gw_status_cb    = __soc_dev_status_changed_cb;
    iot_cbs.gw_reset_cb     = __soc_dev_reset_inform_cb;
    iot_cbs.dev_obj_dp_cb   = __soc_dev_obj_dp_cmd_cb;
    iot_cbs.dev_raw_dp_cb   = __soc_dev_raw_dp_cmd_cb;
    iot_cbs.dev_dp_query_cb = __soc_dev_dp_query_cb;

#if (defined(ENABLE_QRCODE_ACTIVE) && (ENABLE_QRCODE_ACTIVE == 1)) && (!(defined(ENABLE_WIFI_QRCODE) && (ENABLE_WIFI_QRCODE == 1)))
    iot_cbs.active_shorturl = __qrcode_active_shourturl_cb;
#endif

#ifdef ENABLE_WIFI_SERVICE    
    TUYA_CALL_ERR_LOG(tuya_iot_wf_soc_dev_init_param(GWCM_OLD, WF_START_SMART_AP_CONCURRENT, &iot_cbs, NULL, pid, USER_SW_VER));
#ifdef ENABLE_WIRED
    // init wired linkage
    TUYA_CALL_ERR_LOG(tuya_svc_wired_init());
#endif
#else
    TUYA_CALL_ERR_LOG(tuya_iot_soc_init(&iot_cbs, PID, USER_SW_VER));
#endif

    return;
}

/**
 * @brief remove SoC device
 *
 * @param[in] none:
 *
 * @return none
 */
VOID example_soc_device_remove(INT_T argc, CHAR_T *argv[])
{
#ifdef ENABLE_WIFI_SERVICE
    tuya_iot_wf_gw_unactive();
#else
    tuya_iot_gw_unactive();
#endif
}

STATIC THREAD_HANDLE free_heap_thrd = NULL;

STATIC VOID __free_heap_task(PVOID_T args)
{
    for (;;) {
        TAL_PR_NOTICE("free heap: %d", tal_system_get_free_heap_size());
        tal_system_sleep(3000);
    }
}

VOID example_output_free_heap(INT_T argc, CHAR_T *argv[])
{
    OPERATE_RET rt = OPRT_OK;

    THREAD_CFG_T thread_cfg = {
        .thrdname = "free_heap",
        .stackDepth = 1024,
        .priority = THREAD_PRIO_5,
    };
    if (NULL == free_heap_thrd) {
        TUYA_CALL_ERR_LOG(tal_thread_create_and_start(&free_heap_thrd, NULL, NULL, __free_heap_task, NULL, &thread_cfg));
    }
    return;
}
