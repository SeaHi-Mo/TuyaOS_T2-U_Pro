/**
 * @file dp_process.h
 * @author www.tuya.com
 * @brief dp_process module is used to 
 * @version 0.1
 * @date 2022-10-28
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#ifndef __DP_PROCESS_H__
#define __DP_PROCESS_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
********************function declaration********************
***********************************************************/
/**
 * @brief Output the received data and reply to the cloud
 *
 * @param[in] dp_data_arr: the array of recevie dp
 * @param[in] dp_cnt: the number of dp
 *
 * @return none
 */
VOID dp_obj_process(CONST TY_OBJ_DP_S *dp_data_arr, UINT_T dp_cnt);

/**
 * @brief Output the received raw type data and reply to the cloud
 *
 * @param[in] dpid: received raw dp id
 * @param[in] p_data: raw dp data
 * @param[in] data_len: the length of data
 * 
 * @return none
 */
VOID dp_raw_process(UINT8_T dpid, CONST UINT8_T *p_data, UINT_T data_len);

/**
 * @brief   upload swtich status
 *
 * @param[in] : state   the state of switch
 *
 * @return none
 */
VOID upload_device_switch_status(BOOL_T state);

/**
 * @brief report all dp to the cloud
 *
 * @param[in] none: 
 *
 * @return none
 */
VOID upload_device_all_status(VOID_T);

/**
 * @brief respone all dp status when receive query from cloud or app
 *
 * @param[in] none: 
 *
 * @return none
 */
VOID_T respone_device_all_status(VOID_T);


#ifdef __cplusplus
}
#endif

#endif /* __DP_PROCESS_H__ */
