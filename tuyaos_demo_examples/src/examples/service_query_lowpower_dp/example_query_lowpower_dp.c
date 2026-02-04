/**
 * @file example_query_lowpower_dp.c
 * @author www.tuya.com
 * @version 0.1
 * @date 2023-10-25
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */
#include "tal_log.h"
#include "tuya_cloud_types.h"
#include "tuya_iot_wifi_api.h"

#include "smart_frame.h"

/***********************************************************
*********************** macro define ***********************
***********************************************************/
#define TEMPER_SCALE_DP     9

/***********************************************************
********************** typedef define **********************
***********************************************************/


/***********************************************************
********************** variable define *********************
***********************************************************/
CONST UCHAR_T cCACHE_DP[] = {TEMPER_SCALE_DP}; 

/***********************************************************
********************** function define *********************
***********************************************************/
/**
* @brief  SOC device format command data delivery entry
*
* @param[in] dp: obj dp info
* @return none
*/
STATIC VOID __soc_dev_obj_dp_cmd_cb(IN CONST TY_RECV_OBJ_DP_S *dp)
{
    UINT_T index = 0;
    OPERATE_RET rt = OPRT_OK;

    TAL_PR_DEBUG("SOC Rev DP Obj Cmd t1:%d t2:%d CNT:%u", dp->cmd_tp, dp->dtt_tp, dp->dps_cnt);

    for(index = 0; index < dp->dps_cnt; index++) {
        TY_OBJ_DP_S *p_dp_obj = (TY_OBJ_DP_S *)(dp->dps + index);
        TAL_PR_DEBUG("Obj DPID:%d", p_dp_obj->dpid);
    }

    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, dp->dps, dp->dps_cnt));

    return;
}

/**
 * @brief query dp cache 
 *       please use pid "mg1fapvshzimzcsi", 
 *       execute "example_soc_init mg1fapvshzimzcsi" first
 *
 * @param[in] none:
 *
 * @return none
 */
VOID example_query_lp_dp_cache(INT_T argc, CHAR_T *argv[])
{
    OPERATE_RET op_ret = OPRT_OK;
    UINT_T i = 0;
    TY_RECV_OBJ_DP_S *obj_dps = NULL; 
    TY_RECV_MULTI_RAW_DP_S *raw_dps = NULL; //只会返回一个raw型dp

    op_ret = sf_dp_low_power_query(cCACHE_DP, CNTSOF(cCACHE_DP), &obj_dps, &raw_dps);
    if(op_ret !=  OPRT_OK) {
        TAL_PR_ERR("sf_dp_low_power_query err:%d", op_ret );
        return;
    }

    /*
    * TODO：根据应用场景，处理返回的dp列表
    * 与正常的dp指令，即DEV_OBJ_DP_CMD_CB以及DEV_RAW_DP_CMD_CB的处理类似
    */
    if(obj_dps && obj_dps->dps_cnt) {
        for(i=0; i<obj_dps->dps_cnt; i++) {
            TAL_PR_DEBUG("obj dpid:%d tp:%d", obj_dps->dps[i].dpid,  obj_dps->dps[i].type);
        }

        dev_report_dp_json_async(NULL, obj_dps->dps, obj_dps->dps_cnt);
    }

    if(raw_dps && raw_dps->dps_cnt  ) {
        for(i=0; i<raw_dps->dps_cnt; i++) {
            TAL_PR_DEBUG("raw dpid:%d ", raw_dps->dps[i].dpid);

            dev_report_dp_raw_sync(NULL, raw_dps->dps[i].dpid, raw_dps->dps[i].data, raw_dps->dps[i].len, 5);
        }
    }

    if(obj_dps) {
        Free(obj_dps), obj_dps = NULL;
    }

    if(raw_dps) {
        Free(raw_dps), raw_dps= NULL;
    }


    return;
}