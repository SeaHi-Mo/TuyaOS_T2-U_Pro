/**
 * @file example_drv_sensor.c
 * @author www.tuya.com
 * @version 0.1
 * @date 2022-10-12
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#include <string.h>
#include "tal_log.h"

#include "tdl_sensor_hub.h"

#include "tdd_sensor_temp_humi.h"
#include "example_drv_sensor.h"
/***********************************************************
************************macro define************************
***********************************************************/
/**
 * @brief pin define
 */
#define SHT30_SCL_PIN   TUYA_GPIO_NUM_26
#define SHT30_SDA_PIN   TUYA_GPIO_NUM_6
#define SHT30_ALT_PIN   TUYA_GPIO_NUM_14

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/

/***********************************************************
***********************function define**********************
***********************************************************/
/**
 * @brief sensor data ready inform
 *
 * @param[in] name: device name
 * @param[in] ele_num: number of elements
 * @param[in] ele_data: element data
 *
 * @return none
 */
STATIC VOID_T __example_sensor_data_cb(CHAR_T* name, UCHAR_T ele_num, SR_ELE_BUFF_T *ele_data)
{
    TAL_PR_INFO("---------- sensor data callback ----------");


    TAL_PR_INFO("Temp: %.1f", ele_data[0].val[0].sr_float);
    TAL_PR_INFO("Humi: %.1f", ele_data[1].val[0].sr_float);

    return;
}

/**
 * @brief    register hardware 
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET reg_sensor_hardware(CHAR_T *device_name)
{
    OPERATE_RET rt = OPRT_OK;

    SR_I2C_GPIO_T i2c_gpio = {
        .scl = SHT30_SCL_PIN,
        .sda = SHT30_SDA_PIN
    };
    SR_TH_I2C_CFG_T sht30_i2c_cfg = {
        .port = 0,
        .addr = SR_I2C_ADDR_SHT3X_A,
        .gpio = i2c_gpio
    };
    SR_TH_MEAS_CFG_T sht30_meas_cfg = {
        .prec = SR_TH_PREC_HIGH,
        .freq = SR_TH_FREQ_1HZ
    };

    TUYA_CALL_ERR_RETURN(tdd_sensor_sht3x_register(device_name, sht30_i2c_cfg, sht30_meas_cfg));

    return OPRT_OK;
}

/**
 * @brief    open driver
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET open_sensor_driver(CHAR_T *device_name)
{
    OPERATE_RET rt = OPRT_OK;
    SENSOR_HANDLE_T sensor_hdl = NULL;

    TUYA_CALL_ERR_RETURN(tdl_sensor_dev_find(device_name, &sensor_hdl));

    // set param
    SR_DEV_CFG_T sht3x_cfg;
    sht3x_cfg.mode.trig_mode = SR_MODE_POLL_SOFT_TM;
    sht3x_cfg.mode.poll_intv_ms = 1000;
    sht3x_cfg.inform_cb.ele = __example_sensor_data_cb;
    sht3x_cfg.fifo_size = 1;
    sht3x_cfg.ele_sub = NULL;
    TUYA_CALL_ERR_RETURN(tdl_sensor_dev_open(sensor_hdl, &sht3x_cfg));    

    return OPRT_OK;
}

/**
 * @brief    the example of driver function
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET example_sensor_running(CHAR_T *device_name)
{
    OPERATE_RET rt = OPRT_OK;
    SENSOR_HANDLE_T sensor_hdl = NULL;
    SR_TH_STATUS_U  sht3x_status = {.word = 0};

    // read status
    TUYA_CALL_ERR_RETURN(tdl_sensor_dev_find(device_name, &sensor_hdl));
    TUYA_CALL_ERR_RETURN(tdl_sensor_dev_config(sensor_hdl, SR_TH_CMD_GET_STATUS, &sht3x_status));

    TAL_PR_NOTICE("Status <last command>: %d.", sht3x_status.bit.cmd_status);

    //wait sensor(tdl) inform callback

    return OPRT_OK;
}