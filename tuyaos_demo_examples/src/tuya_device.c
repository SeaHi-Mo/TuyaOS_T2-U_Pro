/**
 * @file tuya_device.c
 * @author www.tuya.com
 * @brief tuya_device module is used to 
 * @version 0.1
 * @date 2022-09-20
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#include "tuya_iot_config.h"
#include "tuya_cli_register.h"
#include "tuya_cloud_types.h"
#include "tuya_iot_com_api.h"
#include "tal_thread.h"
#include "tal_log.h"
#include "tal_system.h"
#include "tuya_config.h"
#include "tuya_ws_db.h"
#include  <stdlib.h>

#ifdef ENABLE_WIFI_SERVICE
#include "tuya_iot_wifi_api.h"
#else
#include "tuya_iot_base_api.h"
#endif
#if defined(ENABLE_LWIP) && (ENABLE_LWIP == 1)
#include "lwip_init.h"
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
extern VOID example_mf_init(VOID);

extern VOID example_os_event_subscribe(VOID_T);

/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC THREAD_HANDLE ty_app_thread = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/
/**
 * @brief Initialization device
 *
 * @param[in] none: 
 *
 * @return none
 */
STATIC VOID_T user_main(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    // base utilities init, such as log system, etc.
    tuya_base_utilities_init();

    tal_log_set_manage_attr(TAL_LOG_LEVEL_DEBUG);

    /* LwIP Initialization */
#if defined(ENABLE_LWIP) && (ENABLE_LWIP == 1)
    TUYA_LwIP_Init();
#endif

    /* TuyaOS event subscribe example */
    example_os_event_subscribe();

    /* TuyaOS System Service Initialization */
#if OPERATING_SYSTEM == SYSTEM_LINUX    
    TUYA_CALL_ERR_LOG(system("mkdir -p ./tuya_db_files/"));
    TUYA_CALL_ERR_LOG(tuya_iot_init_params("./tuya_db_files/", NULL));
#else
    TY_INIT_PARAMS_S init_param = {0};
    init_param.init_db = TRUE;
    strcpy(init_param.sys_env, TARGET_PLATFORM);
    TUYA_CALL_ERR_LOG(tuya_iot_init_params(NULL, &init_param));
#endif

    TAL_PR_NOTICE("sdk_info:%s", tuya_iot_get_sdk_info());                        /* print SDK information */
    TAL_PR_NOTICE("name:%s:%s", APP_BIN_NAME, USER_SW_VER);                       /* print the firmware name and version */
    TAL_PR_NOTICE("firmware compiled at %s %s", __DATE__, __TIME__);              /* print firmware compilation time */
    TAL_PR_NOTICE("system reset reason:[%d]", tal_system_get_reset_reason(NULL)); /* print system reboot causes */

#ifdef UUID
    /* Write authorization information to flash */
    ws_db_init_mf();
#ifdef ENABLE_WIFI_SERVICE
    WF_GW_PROD_INFO_S prod_info = {UUID, AUTHKEY};
    TUYA_CALL_ERR_LOG(tuya_iot_set_wf_gw_prod_info(&prod_info));
#else
    GW_PROD_INFO_S prod_info = {UUID, AUTHKEY};
    TUYA_CALL_ERR_LOG(tuya_iot_set_gw_prod_info(&prod_info));
#endif
#else
    /* Authorization using the host computer */
    example_mf_init();
#endif

    /* Examples function register */
    examples_cli_register();

    /* CLI service initialized and started */
    TUYA_CALL_ERR_LOG(tuya_cli_init_with_uart(TUYA_UART_NUM_0));

    return;
}

/**
 * @brief tuyaos Initialization thread
 *
 * @param[in] arg: Parameters when creating a task
 *
 * @return none
 */
STATIC VOID_T tuya_app_thread(VOID_T *arg)
{
    user_main();

    tal_thread_delete(ty_app_thread);
    ty_app_thread = NULL;
}

/**
 * @brief tuyaos entry function, it will call by main
 *
 * @param[in] none: 
 *
 * @return none
 */
#if OPERATING_SYSTEM == SYSTEM_LINUX
INT_T main(INT_T argc, CHAR_T **argv)
#else
VOID_T tuya_app_main(VOID)
#endif
{
#if defined(ENABLE_EXT_RAM) && (ENABLE_EXT_RAM == 1)
    extern VOID_T tkl_system_psram_malloc_force_set(BOOL_T enable);
    tkl_system_psram_malloc_force_set(TRUE);
#endif

    THREAD_CFG_T thrd_param = {4096, 4, "tuya_app_main"};
    tal_thread_create_and_start(&ty_app_thread, NULL, NULL, tuya_app_thread, NULL, &thrd_param);

#if OPERATING_SYSTEM == SYSTEM_LINUX
    while (1)
    {
        tal_system_sleep(1000);
    }
    
#endif
}

#if defined(ENABLE_EXT_RAM) && (ENABLE_EXT_RAM == 1)
void tuya_gui_pause(void)
{
    //! TODO:
}

void tuya_gui_resume(void)
{
    //! TODO:

}

VOID_T tuya_gui_main(VOID_T)
{
    
}
#endif
