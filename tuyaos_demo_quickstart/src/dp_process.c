/**
 * @file dp_process.c
 * @author www.tuya.com
 * @version 0.1
 * @date 2022-09-22
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#include "tuya_cloud_types.h"
#include "tuya_iot_com_api.h"

#include "tal_log.h"
#include "tal_memory.h"
#include "tal_workq_service.h"

#include "app_led.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define DPID_SWITCH               20 /* bool */
#define DPID_MODE                 21 /* enum */
#define DPID_WHITE_BRIGHT         22 /* value */
#define DPID_COLOR                24 /* string */
#define DPID_POWER_MEMORY         33 /* raw */
#define OBJ_DP_NUM_MAX            4         

#define LIGHT_MODE_WHITE          0
#define LIGHT_MODE_COLOUR         1
#define LIGHT_MODE_SCENE          2
#define LIGHT_MODE_MUSIC          3

#define COLOR_STRING_LEN          12
#define POWER_MEMORY_RAW_LEN      12
/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct{
    UINT8_T   switch_status;
    UINT8_T   mode;
    UINT16_T  white_bright;
    CHAR_T    color[COLOR_STRING_LEN+1];
    UINT8_T   power_memory[POWER_MEMORY_RAW_LEN];
    UINT_T    power_memory_len;
}DEMO_INFO_T;

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC DEMO_INFO_T sg_demo_info = {
    .color            = {"000003e803e8"},
    .power_memory     = {0x00, 0x02, 0x00, 0x00, 0x03, 0xe8, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00},
    .power_memory_len = POWER_MEMORY_RAW_LEN,
};

/***********************************************************
***********************function define**********************
***********************************************************/
/**
 * @brief Output the received data and reply to the cloud
 *
 * @param[in] dp_data_arr: the array of recevie dp
 * @param[in] dp_cnt: the number of dp
 *
 * @return none
 */
VOID dp_obj_process(CONST TY_OBJ_DP_S *dp_data_arr, UINT_T dp_cnt)
{
    UINT32_T index = 0;
    OPERATE_RET rt = OPRT_OK;
    TY_OBJ_DP_S *dp_data = NULL;

    if(NULL == dp_data_arr || 0 == dp_cnt) {
        TAL_PR_ERR("param is err");
        return;
    }

    for (index = 0; index < dp_cnt; index++) {
        dp_data = (TY_OBJ_DP_S *)&dp_data_arr[index];

        switch (dp_data->dpid) {
            case DPID_SWITCH:
                TAL_PR_NOTICE("switch: %d", dp_data->value.dp_bool);
                sg_demo_info.switch_status = dp_data->value.dp_bool;
                set_led_status(sg_demo_info.switch_status);
            break;
            case DPID_MODE:
                TAL_PR_NOTICE("mode: %d", dp_data->value.dp_enum);
                sg_demo_info.mode = dp_data->value.dp_enum;
                //todo 

            break;
            case DPID_WHITE_BRIGHT:
                TAL_PR_NOTICE("white bright: %d", dp_data->value.dp_value);
                sg_demo_info.white_bright = dp_data->value.dp_value;
                //todo 

            break;
            case DPID_COLOR:
                TAL_PR_NOTICE("color: %s", dp_data->value.dp_str);
                if(strlen(dp_data->value.dp_str) > COLOR_STRING_LEN) {
                    TAL_PR_ERR("dp string len is over:%d", COLOR_STRING_LEN);
                    break;
                }

                memset(sg_demo_info.color, 0, COLOR_STRING_LEN+1);
                memcpy(sg_demo_info.color, dp_data->value.dp_str, COLOR_STRING_LEN);
                //todo 

            break;
            default: 
            break;
        }
    }

    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, dp_data_arr, dp_cnt));

    return;
}

/**
 * @brief Output the received raw type data and reply to the cloud
 *
 * @param[in] dpid: received raw dp id
 * @param[in] p_data: raw dp data
 * @param[in] data_len: the length of data
 * 
 * @return none
 */
VOID dp_raw_process(UINT8_T dpid, CONST UINT8_T *p_data, UINT_T data_len)
{
    OPERATE_RET rt = OPRT_OK;
    UINT_T i = 0;

    TAL_PR_NOTICE("raw dp id: %d", dpid);
    if(dpid != DPID_POWER_MEMORY) {
        TAL_PR_ERR("not support:%d", dpid);
        return;
    }

    if(NULL == p_data || 0 == data_len) {
        TAL_PR_ERR("param is err");
        return;
    }

    if(data_len > POWER_MEMORY_RAW_LEN) {
        TAL_PR_ERR("raw dp len:%d is over:%d", data_len, POWER_MEMORY_RAW_LEN);
        return;
    }

    for (i=0; i<data_len; i++) {
        TAL_PR_DEBUG_RAW("0x%02x ", p_data[i]);
    }

    memcpy(sg_demo_info.power_memory, p_data, data_len);
    sg_demo_info.power_memory_len = data_len;

    TUYA_CALL_ERR_LOG(dev_report_dp_raw_sync(NULL, DPID_POWER_MEMORY, \
                                            sg_demo_info.power_memory, sg_demo_info.power_memory_len, 5));

    return;
}

/**
 * @brief   upload swtich status
 *
 * @param[in] : state   the state of switch
 *
 * @return none
 */
VOID upload_device_switch_status(BOOL_T state)
{
    OPERATE_RET rt = OPRT_OK;
    TY_OBJ_DP_S obj_dp = {0};

    memset((UCHAR_T *)&obj_dp, 0, SIZEOF(obj_dp));

    obj_dp.dpid          = DPID_SWITCH;
    obj_dp.type          = PROP_BOOL;
    obj_dp.value.dp_bool = state;

    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, &obj_dp, 1));
    
    sg_demo_info.switch_status = TRUE;

    return;
}

/**
 * @brief report all dp to the cloud
 *
 * @param[in] none: 
 *
 * @return none
 */
VOID upload_device_all_status(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    TY_OBJ_DP_S *p_all_obj_dp = NULL;
    TY_OBJ_DP_S *p_obj_dp = NULL;

    p_all_obj_dp = (TY_OBJ_DP_S *)tal_malloc(OBJ_DP_NUM_MAX*SIZEOF(TY_OBJ_DP_S));
    if(NULL == p_all_obj_dp) {
        return;
    }
    memset((UCHAR_T *)p_all_obj_dp, 0, OBJ_DP_NUM_MAX*SIZEOF(TY_OBJ_DP_S));

    /* object type data */
    // bool type data 
    p_obj_dp = p_all_obj_dp;
    p_obj_dp->dpid          = DPID_SWITCH;
    p_obj_dp->type          = PROP_BOOL;
    p_obj_dp->value.dp_bool = sg_demo_info.switch_status;

    // enum type data 
    p_obj_dp++;
    p_obj_dp->dpid          = DPID_MODE;
    p_obj_dp->type          = PROP_ENUM;
    p_obj_dp->value.dp_enum = sg_demo_info.mode;

    // value type data
    p_obj_dp++;
    p_obj_dp->dpid           = DPID_WHITE_BRIGHT;
    p_obj_dp->type           = PROP_VALUE;
    p_obj_dp->value.dp_value = sg_demo_info.white_bright;

    // string type data
    p_obj_dp++;
    p_obj_dp->dpid          = DPID_COLOR;
    p_obj_dp->type          = PROP_STR;
    p_obj_dp->value.dp_str  = sg_demo_info.color;

    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, p_all_obj_dp, OBJ_DP_NUM_MAX));

    tal_free(p_all_obj_dp);
    p_all_obj_dp = NULL;
    p_obj_dp = NULL;

    /* raw type data */
    TUYA_CALL_ERR_LOG(dev_report_dp_raw_sync(NULL, DPID_POWER_MEMORY, \
                                             sg_demo_info.power_memory, sg_demo_info.power_memory_len, 5));

    return;
}

STATIC VOID_T __upload_device_raw_dp(VOID_T *data)
{
    OPERATE_RET rt = OPRT_OK;

    /* raw type data */
    TUYA_CALL_ERR_LOG(dev_report_dp_raw_sync(NULL, DPID_POWER_MEMORY, \
                                             sg_demo_info.power_memory, sg_demo_info.power_memory_len, 5));
}


/**
 * @brief respone all dp status when receive query from cloud or app
 *
 * @param[in] none: 
 *
 * @return none
 */
VOID_T respone_device_all_status(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    TY_OBJ_DP_S *p_all_obj_dp = NULL;
    TY_OBJ_DP_S *p_obj_dp = NULL;

    p_all_obj_dp = (TY_OBJ_DP_S *)tal_malloc(OBJ_DP_NUM_MAX*SIZEOF(TY_OBJ_DP_S));
    if(NULL == p_all_obj_dp) {
        return;
    }
    memset((UCHAR_T *)p_all_obj_dp, 0, OBJ_DP_NUM_MAX*SIZEOF(TY_OBJ_DP_S));

    /* object type data */
    // bool type data 
    p_obj_dp = p_all_obj_dp;
    p_obj_dp->dpid          = DPID_SWITCH;
    p_obj_dp->type          = PROP_BOOL;
    p_obj_dp->value.dp_bool = sg_demo_info.switch_status;

    // enum type data 
    p_obj_dp++;
    p_obj_dp->dpid          = DPID_MODE;
    p_obj_dp->type          = PROP_ENUM;
    p_obj_dp->value.dp_enum = sg_demo_info.mode;

    // value type data
    p_obj_dp++;
    p_obj_dp->dpid           = DPID_WHITE_BRIGHT;
    p_obj_dp->type           = PROP_VALUE;
    p_obj_dp->value.dp_value = sg_demo_info.white_bright;

    // string type data
    p_obj_dp++;
    p_obj_dp->dpid          = DPID_COLOR;
    p_obj_dp->type          = PROP_STR;
    p_obj_dp->value.dp_str  = sg_demo_info.color;

    TUYA_CALL_ERR_LOG(dev_query_dp_json_async(NULL, p_all_obj_dp, OBJ_DP_NUM_MAX));

    tal_free(p_all_obj_dp);
    p_all_obj_dp = NULL;
    p_obj_dp = NULL;

    /* put raw type data sync report int work queue*/
    tal_workq_schedule(WORKQ_SYSTEM, __upload_device_raw_dp, NULL);
}