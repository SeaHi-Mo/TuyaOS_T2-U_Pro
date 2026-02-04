/**
 * @file example_drv_ele_energy.c
 * @author www.tuya.com
 * @brief example_drv_ele_energy module is used to 
 * @version 0.1
 * @date 2022-11-10
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */
#include "tal_log.h"
#include "tal_system.h"
#include "tal_thread.h"
#include "tkl_timer.h"

#include "tdd_energy_monitor_bl0937_hlw8012.h"
#include "tdd_energy_monitor_bl0942.h"
#include "tdd_energy_monitor_hlw8032.h"
#include "tdl_energy_monitor_manage.h"
#include "example_drv_ele_energy.h"
/***********************************************************
************************macro define************************
***********************************************************/
// energy monitor chip
#define ENABLE_BL0937_DRV                   1
#define ENABLE_HLW8012_DRV                  2
#define ENABLE_BL0942_DRV                   3
#define ENABLE_HLW8032_DRV                  4

//! select energy monitor chip
#define ENABLE_ENERGY_MONITOR_DRV           ENABLE_HLW8032_DRV

#define ENABLE_ENERGY_MONITOR_CALIBRATION   0

/* bl0937&hlw8012 hardware config */
#if (ENABLE_ENERGY_MONITOR_DRV == ENABLE_BL0937_DRV) || (ENABLE_ENERGY_MONITOR_DRV == ENABLE_HLW8012_DRV)
#define EXAMPLE_ELE_PIN         TUYA_GPIO_NUM_7  // CF  sample power
#define EXAMPLE_IV_PIN          TUYA_GPIO_NUM_8  // CF1 sample voltage or current
#define EXAMPLE_SEL_PIN         TUYA_GPIO_NUM_6  // SEL select voltage or current
#define EXAMPLE_HW_TIMER_ID     TUYA_TIMER_NUM_0
#endif

/* bl0942&hlw8012 hardware config */
#if (ENABLE_ENERGY_MONITOR_DRV == ENABLE_BL0942_DRV) || (ENABLE_ENERGY_MONITOR_DRV == ENABLE_HLW8032_DRV)
#define EXAMPLE_UART_ID         TUYA_UART_NUM_0
#endif

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/

#if (ENABLE_ENERGY_MONITOR_DRV == ENABLE_BL0937_DRV)
STATIC BL0937_DRIVER_CONFIG_T sg_bl0937_cfg = {
    .timer_id  = EXAMPLE_HW_TIMER_ID,
    .sel_pin   = EXAMPLE_SEL_PIN,
    .sel_level = TUYA_GPIO_LEVEL_HIGH,
    .cf1_pin   = EXAMPLE_IV_PIN,
    .cf_pin    = EXAMPLE_ELE_PIN,
};
#endif

#if (ENABLE_ENERGY_MONITOR_DRV == ENABLE_HLW8012_DRV)
STATIC HLW8012_DRIVER_CONFIG_T sg_hlw8012_cfg = {
    .timer_id  = EXAMPLE_HW_TIMER_ID,
    .sel_pin   = EXAMPLE_SEL_PIN,
    .sel_level = TUYA_GPIO_LEVEL_HIGH,
    .cf1_pin   = EXAMPLE_IV_PIN,
    .cf_pin    = EXAMPLE_ELE_PIN,
};
#endif

#if (ENABLE_ENERGY_MONITOR_DRV == ENABLE_BL0942_DRV)
STATIC BL0942_DRIVER_CONFIG_T sg_bl0942_cfg = {
    .mode = BL0942_DRV_UART,
    .driver.uart = {
        .id   = EXAMPLE_UART_ID,
        .addr = 0,
    }
};
#endif

#if (ENABLE_ENERGY_MONITOR_DRV == ENABLE_HLW8032_DRV)
STATIC HLW8032_DRIVER_CONFIG_T sg_hlw8032_cfg = {
    .uart_id = EXAMPLE_UART_ID,
};
#endif

STATIC THREAD_HANDLE           sg_ele_energy_thrd = NULL;
STATIC ENERGY_MONITOR_HANDLE_T sg_ele_energy_hdl  = NULL;
/***********************************************************
***********************function define**********************
***********************************************************/
STATIC VOID __energy_monitor_task(PVOID_T args)
{
    ENERGY_MONITOR_VCP_T vcp = {0,0,0};
    UINT_T energy = 0;
    UINT_T _cnt = 0;

    for (;;) {
        tdl_energy_monitor_read_vcp(sg_ele_energy_hdl, &vcp);
        /* pwr: 0.1W, volt: 0.1V, curr: 0.01A */
        TAL_PR_DEBUG("curr pvie data, P: %d, V: %d, I: %d", vcp.power, vcp.voltage, vcp.current);

        _cnt++;
        if (_cnt++ > 20*3) { // 3 minutes
            _cnt = 0;
            /* get electric quantity*/
            tdl_energy_monitor_read_energy(sg_ele_energy_hdl, &energy);
            TAL_PR_DEBUG("E:%d", energy);
        }

        tal_system_sleep(3000);
    }

    return;
}

/**
 * @brief    register hardware 
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET reg_ele_energy_hardware(CHAR_T *device_name)
{
    OPERATE_RET rt = OPRT_OK;

#if (ENABLE_ENERGY_MONITOR_DRV == ENABLE_BL0937_DRV)
    TUYA_CALL_ERR_RETURN(tdd_energy_driver_bl0937_register(device_name, sg_bl0937_cfg));
#elif (ENABLE_ENERGY_MONITOR_DRV == ENABLE_HLW8012_DRV)
    TUYA_CALL_ERR_RETURN(tdd_energy_driver_hlw8012_register(device_name, sg_hlw8012_cfg));
#elif (ENABLE_ENERGY_MONITOR_DRV == ENABLE_BL0942_DRV)
    TUYA_CALL_ERR_RETURN(tdd_energy_driver_bl0942_register(device_name, sg_bl0942_cfg));
#elif (ENABLE_ENERGY_MONITOR_DRV == ENABLE_HLW8032_DRV)
    TUYA_CALL_ERR_RETURN(tdd_energy_driver_hlw8032_register(device_name, sg_hlw8032_cfg));
#endif

    return OPRT_OK;
}

/**
 * @brief    open driver
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET open_ele_energy_driver(CHAR_T *device_name)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CALL_ERR_RETURN(tdl_energy_monitor_find(device_name, &sg_ele_energy_hdl));

#if ENABLE_ENERGY_MONITOR_CALIBRATION
    ENERGY_MONITOR_CAL_DATA_T   cal_env;
    ENERGY_MONITOR_CAL_PARAMS_T cal_params;

    //set environmental parameters
    // Voltage, current and power values of 560R/100W/1% ceramic resistor at 220 V environment
    cal_env.voltage = 2200; // 220.0v
    cal_env.current = 392;  // 0.392A
    cal_env.power   = 864;  // 86.4w
    cal_env.resval  = 1;
    UINT32_T percent_err = tdl_energy_monitor_calibration(sg_ele_energy_hdl, cal_env, &cal_params);
    if (percent_err < 30) {
        // After calibration, you can store these two data in flash to avoid calibration every time you power on
        // cal_env -> flash
        // cal_params -> flash
        TAL_PR_DEBUG("calibration success, percent_err:%d", percent_err);
        // flash -> cal_env
        // flash -> cal_params
        tdl_energy_monitor_config(sg_ele_energy_hdl, TDL_EM_CMD_CAL_DATA_SET, &cal_env);
        tdl_energy_monitor_config(sg_ele_energy_hdl, TDL_EM_CMD_CAL_PARAMS_SET, &cal_params);
    } else {
        TAL_PR_ERR("calibration failed, percent_err:%d", percent_err);
    }
#endif

    /*open*/
    TUYA_CALL_ERR_RETURN(tdl_energy_monitor_open(sg_ele_energy_hdl));

    return OPRT_OK;
}

/**
 * @brief    the example of driver function
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET example_ele_energy_running(CHAR_T *device_name)
{
    OPERATE_RET rt = OPRT_OK;

    THREAD_CFG_T task_cfg = {
        .thrdname = "ele_demo",
        .priority = THREAD_PRIO_1,
        .stackDepth = 1024*2,
    };

    TUYA_CALL_ERR_RETURN(tal_thread_create_and_start(&sg_ele_energy_thrd, NULL, NULL, __energy_monitor_task, NULL, &task_cfg));

    return OPRT_OK;
}