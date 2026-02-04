/**
 * @file tuya_device.c
 * @author www.tuya.com
 * @brief tuya_device module is used to 
 * @version 0.1
 * @date 2022-09-28
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */
#include "tuya_cloud_types.h"
#include "tuya_iot_com_api.h"

#include "lwip_init.h"

#include "tal_thread.h"
#include "tal_log.h"
#include "tkl_memory.h"
#include "tkl_system.h"

#include "example_drv_button.h"
#include "example_drv_led.h"
#include "example_drv_ir.h"
#include "example_drv_sound.h"
#include "example_drv_dimmer.h"
#include "example_drv_pixels.h"
#include "example_drv_sensor.h"
#include "example_drv_ele_energy.h"
/***********************************************************
************************macro define************************
***********************************************************/
#define TY_EXAMPLE_BUTTON           "ty_button"
#define TY_EXAMPLE_LED              "ty_led"
#define TY_EXAMPLE_IR               "ty_ir"
#define TY_EXAMPLE_SOUND            "ty_sound"
#define TY_EXAMPLE_SENSOR           "ty_sensor"
#define TY_EXAMPLE_DIMMER           "ty_dimmer"
#define TY_EXAMPLE_PIXELS           "ty_pixels"
#define TY_EXAMPLE_ELE_ENERGY       "ty_ele_energy"

/*select target example driver*/
#define TY_EXAMPLE_DRIVER           TY_EXAMPLE_LED

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef OPERATE_RET (*REG_DRIVER_HARDWARE)(CHAR_T *device_name);
typedef OPERATE_RET (*OPEN_DRIVER)(CHAR_T *device_name);
typedef OPERATE_RET (*DRIVER_RUNNING)(CHAR_T *device_name);
typedef struct {
    CHAR_T                  *device_name;
    REG_DRIVER_HARDWARE      reg_drv_hardware;
    OPEN_DRIVER              open_driver;
    DRIVER_RUNNING           driver_running;
}EXAMPLE_DRV_INFO_T;

/***********************************************************
********************function declaration********************
***********************************************************/
CONST EXAMPLE_DRV_INFO_T cEXAMPLE_DRV_LIST[] = {
    {TY_EXAMPLE_BUTTON,     reg_button_hardware,     open_button_driver,     example_button_running},
    {TY_EXAMPLE_LED,        reg_led_hardware,        NULL,                   example_led_running},
    {TY_EXAMPLE_IR,         reg_ir_hardware,         open_ir_driver,         example_ir_running},
    {TY_EXAMPLE_SOUND,      reg_sound_hardware,      open_sound_driver,      example_sound_running},
    {TY_EXAMPLE_SENSOR,     reg_sensor_hardware,     open_sensor_driver,     example_sensor_running},
    {TY_EXAMPLE_DIMMER,     reg_dimmer_hardware,     open_dimmer_driver,     example_dimmer_running},
    {TY_EXAMPLE_PIXELS,     reg_pixels_hardware,     open_pixels_driver,     example_pixels_running},
    {TY_EXAMPLE_ELE_ENERGY, reg_ele_energy_hardware, open_ele_energy_driver, example_ele_energy_running},
};


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC THREAD_HANDLE ty_app_thread = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/
STATIC CONST EXAMPLE_DRV_INFO_T *__find_example_drv_info(CHAR_T *dev_name)
{
    UINT_T i = 0;

    if(NULL == dev_name) {
        return NULL;
    }

    for(i=0; i<CNTSOF(cEXAMPLE_DRV_LIST); i++) {
        if(0 == strcmp(cEXAMPLE_DRV_LIST[i].device_name, dev_name)) {
            return &cEXAMPLE_DRV_LIST[i];
        }
    }

    return NULL;
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
    OPERATE_RET rt = OPRT_OK;

    /* LwIP Initialization */
#if defined(ENABLE_LWIP) && (ENABLE_LWIP == 1)
    TUYA_LwIP_Init();
#endif

    /* TuyaOS System Service Initialization */
    TY_INIT_PARAMS_S init_param = {0};
    init_param.init_db = TRUE;
    strcpy(init_param.sys_env, TARGET_PLATFORM);
    TUYA_CALL_ERR_GOTO(tuya_iot_init_params(NULL, &init_param), TY_APP_EXIT);

    CONST EXAMPLE_DRV_INFO_T *p_example_drv = NULL;
    p_example_drv = __find_example_drv_info(TY_EXAMPLE_DRIVER);
    if(NULL == p_example_drv) {
        TAL_PR_ERR("cant find example driver");
        goto TY_APP_EXIT;
    }

    // register hardware
    if(p_example_drv->reg_drv_hardware) {
        TUYA_CALL_ERR_GOTO(p_example_drv->reg_drv_hardware(TY_EXAMPLE_DRIVER), TY_APP_EXIT);
        TAL_PR_NOTICE("register driver hardware success");
    }

    // open device
    if(p_example_drv->open_driver) {
        TUYA_CALL_ERR_GOTO(p_example_drv->open_driver(TY_EXAMPLE_DRIVER), TY_APP_EXIT);
        TAL_PR_NOTICE("open device success");
    }

    // start device example
    if(p_example_drv->driver_running) {
        TUYA_CALL_ERR_GOTO(p_example_drv->driver_running(TY_EXAMPLE_DRIVER), TY_APP_EXIT);
        TAL_PR_NOTICE("start device example success");
    }

TY_APP_EXIT:
    tal_thread_delete(ty_app_thread);
    ty_app_thread = NULL;

    return;
}


/**
 * @brief tuyaos entry function, it will call by main
 *
 * @param[in] none: 
 *
 * @return none
 */
VOID_T tuya_app_main(VOID_T)
{
    THREAD_CFG_T thrd_param = {4096, 4, "tuya_app_main"};

    tal_thread_create_and_start(&ty_app_thread, NULL, NULL, tuya_app_thread, NULL, &thrd_param);

    return;
}

