/**
 * @file example_product_test
 * @author www.tuya.com
 * @version 0.1
 * @date 2023-09-28
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */
#include "tuya_iot_config.h"
#include "tuya_cloud_wifi_defs.h"
#include "tal_log.h"
#include "prod_test.h"

/***********************************************************
*********************** macro define ***********************
***********************************************************/
#define APP_WF_CFG_MTHD                     GWCM_LOW_POWER 

#define PRODUCT_TEST_WIFI_1                 "tuya_mdev_test1"
#define PRODUCT_TEST_WIFI_2                 "tuya_mdev_test2"

#define PROD_TEST_WEAK_SIGNAL               -60

/***********************************************************
********************** typedef define **********************
***********************************************************/
typedef UCHAR_T PRODUCT_TEST_TYPE_E;
#define PRODUCT_TEST_TP_AGING          0x00
#define PRODUCT_TEST_TP_FUNCTION       0x01

/***********************************************************
********************** variable define *********************
***********************************************************/
STATIC CONST CHAR_T *cWIFI_SCAN_SSID_LIST[] = {
    PRODUCT_TEST_WIFI_1, 
    PRODUCT_TEST_WIFI_2, 
};

STATIC BOOL_T                sg_is_aging_test_finish = FALSE;
STATIC PRODUCT_TEST_TYPE_E   sg_product_test_type = PRODUCT_TEST_TP_AGING;

/***********************************************************
********************** extern  define *********************
***********************************************************/
/**
 * @brief  Scan Wi-Fi test.
 *
 * @param[in] wf_cfg_mthd GW_WF_CFG_MTHD_SEL mode.
 * @param[in] ssid_list List of SSIDs to scan.
 * @param[in] ssid_count Count of SSIDs to scan.
 * @param[in] scan_info_cb Callback function to handle the scan result.
 *
 * @return BOOL_T TRUE if the scan is successful, otherwise FALSE.
 */
extern BOOL_T ty_scan_test_wifi(GW_WF_CFG_MTHD_SEL wf_cfg_mthd, CONST CHAR_T **ssid_list, \
                                UINT8_T ssid_count, prodtest_app_cb_t scan_info_cb);

/***********************************************************
********************** function define *********************
***********************************************************/
STATIC prodtest_ssid_info_t *__ty_app_find_target_ssid(CHAR_T *target_ssid, prodtest_ssid_info_t *wifi_arr, UINT8_T arr_cnt)
{
    UINT8_T i = 0 ;

    if(NULL == target_ssid || NULL == wifi_arr || 0 == arr_cnt) {
        return NULL;
    }

    for(i = 0;  i<arr_cnt; i++) {
        if(0 == strcmp(wifi_arr[i].ssid, target_ssid)) {
            return &wifi_arr[i];
        }
    }

    return NULL;
}

STATIC OPERATE_RET __scan_wifi_test_info_cb(INT_T flag, prodtest_ssid_info_t *ssid_info, UINT8_T info_count)
{
    prodtest_ssid_info_t *p_ssid_info = NULL;

    if(FALSE == sg_is_aging_test_finish) {
        p_ssid_info = __ty_app_find_target_ssid(PRODUCT_TEST_WIFI_1, ssid_info, info_count);
        sg_product_test_type = PRODUCT_TEST_TP_AGING;
    }else {
        p_ssid_info = __ty_app_find_target_ssid(PRODUCT_TEST_WIFI_2, ssid_info, info_count);
        sg_product_test_type = PRODUCT_TEST_TP_FUNCTION;
    }

    //cant find target ssid 
    if(NULL == p_ssid_info) {
        return OPRT_COM_ERROR;
    }

    if(p_ssid_info->rssi < PROD_TEST_WEAK_SIGNAL) { //check signal
        TAL_PR_NOTICE("rssi:%d weak signal !", p_ssid_info->rssi);
        // todo: production signal is weak
        return OPRT_OK;
    }
    
    if(0 == flag) { 
        //todo: device not authorized 
        return OPRT_OK;
    }
    
    if(PRODUCT_TEST_TP_AGING == sg_product_test_type){
        //todo: device aging test

    }else if(PRODUCT_TEST_TP_FUNCTION == sg_product_test_type) {
        //todo: device function test
    }

    return OPRT_OK;
}

STATIC OPERATE_RET __user_product_test_cmd_cb(USHORT_T cmd, UCHAR_T *data, UINT_T len,\
                                              OUT UCHAR_T **ret_data, OUT USHORT_T *ret_len)
{
    //todo: process user product test command from production

    return OPRT_OK;
}

/**
 * @brief Function that performs the product test.
 *
 * The function scans for Wi-Fi access points specified in cWIFI_SCAN_SSID_LIST
 * and performs the product test using the __scan_wifi_test_info_cb callback function.
 * If the scan is successful and the target SSID is found, the function proceeds with the appropriate product test. 
 * The function also registers the __user_product_test_cmd_cb callback
 * to process any user product test command received from production.
 *
 * @param[in] argc Number of arguments passed to the function.
 * @param[in] argv Array of arguments passed to the function.
 *
 * @return None.
 */ 
VOID example_product_test(INT_T argc, CHAR_T *argv[])
{

#if defined(ENABLE_PRODUCT_AUTOTEST) && (ENABLE_PRODUCT_AUTOTEST == 1)
    prodtest_app_cfg_t prodtest_cfg = {.gwcm_mode  = APP_WF_CFG_MTHD,
                                       .file_name  = APP_BIN_NAME,
                                       .file_ver   = USER_SW_VER,
                                       .ssid_list  = cWIFI_SCAN_SSID_LIST,
                                       .ssid_count = CNTSOF(cWIFI_SCAN_SSID_LIST),
                                       .app_cb     = __scan_wifi_test_info_cb,
                                       .product_cb = __user_product_test_cmd_cb};

    prodtest_app_register(&prodtest_cfg);

    if (TRUE == prodtest_ssid_scan(500)) {
        //所有产测路由都要在6信道
        TAL_PR_NOTICE("enter product test !");
    }   
#else 
    if(TRUE == ty_scan_test_wifi(APP_WF_CFG_MTHD, cWIFI_SCAN_SSID_LIST, \
                                 CNTSOF(cWIFI_SCAN_SSID_LIST), __scan_wifi_test_info_cb)) {

        TAL_PR_NOTICE("enter product test !");
    }
#endif

    return;
}