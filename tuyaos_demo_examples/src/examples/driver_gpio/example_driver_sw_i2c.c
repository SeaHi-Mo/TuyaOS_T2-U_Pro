/**
 * @file example_driver_sw_i2c.c
 * @author www.tuya.com
 * @version 0.1
 * @date 2023-10-20
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_iot_config.h"

#if defined(ENABLE_GPIO) && (ENABLE_GPIO) 
#include "tal_system.h"
#include "tal_thread.h"
#include "tal_log.h"
#include "tdd_sw_i2c.h"

/***********************************************************
*********************** macro define ***********************
***********************************************************/
/**
 * @brief CRC result
 */
#define CRC_OK                      (0)
#define CRC_ERR                     (-1)


/**
 * @brief pin define
 */
#define SHT30_SCL_PIN               TUYA_GPIO_NUM_20
#define SHT30_SDA_PIN               TUYA_GPIO_NUM_22
#define SHT30_ALT_PIN               TUYA_GPIO_NUM_14

#define SHT3X_ADDR                  0x44
#define SHT3X_CMD_R_FETCH_H         0xE0  /* readout measurements for periodic mode */
#define SHT3X_CMD_R_FETCH_L         0x00  

#define I2C_NUM_ID                  SW_I2C_PORT_NUM_0

#define TASK_IIC_PRIORITY           THREAD_PRIO_1
#define TASK_IIC_SIZE               1024

#define I2C_WRITE_BUFLEN            2u
#define I2C_READ_BUFLEN             6u
/***********************************************************
********************** typedef define **********************
***********************************************************/


/***********************************************************
********************** variable define *********************
***********************************************************/
STATIC THREAD_CFG_T sg_task = {
    .priority   = TASK_IIC_PRIORITY,
    .stackDepth = TASK_IIC_SIZE,
    .thrdname   = "sw_i2c"
};

STATIC THREAD_HANDLE sg_i2c_handle;

/***********************************************************
********************** function define *********************
***********************************************************/
/**
 * @brief get CRC8 value for sht3x
 *
 * @param[in] data: data to be calculated
 * @param[in] len: data length
 *
 * @return CRC8 value
 */
STATIC UCHAR_T __sht3x_get_crc8(IN CONST UCHAR_T *data, IN USHORT_T len)
{
    UCHAR_T i;
    UCHAR_T crc = 0xFF;

    while (len--) {
        crc ^= *data;

        for (i = 8; i > 0; --i) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc = (crc << 1);
            }
        }
        data++;
    }

    return crc;
}

/**
 * @brief check CRC8
 *
 * @param[in] data: data to be checked
 * @param[in] len: data length
 * @param[in] crc_val: crc value
 *
 * @return check result
 */
STATIC INT_T __sht3x_check_crc8(IN CONST UCHAR_T *data, IN CONST USHORT_T len, IN CONST UCHAR_T crc_val)
{
    if (__sht3x_get_crc8(data, len) != crc_val) {
        return CRC_ERR;
    }
    return CRC_OK;
}


/**
* @brief i2c task
*
* @param[in] param:Task parameters
* @return none
*/
VOID __sw_i2c_task(VOID* param)
{
    OPERATE_RET  op_ret = OPRT_OK;
    SW_I2C_MSG_T i2c_msg = {0};
    UCHAR_T      read_buf[I2C_READ_BUFLEN] = {0};
    UCHAR_T      write_buff[I2C_WRITE_BUFLEN] = {SHT3X_CMD_R_FETCH_H, SHT3X_CMD_R_FETCH_L};
    USHORT_T     temper = 0, humi = 0;

    /*i2c init*/
    SW_I2C_GPIO_T sw_i2c_gpio = {
        .scl = SHT30_SCL_PIN,
        .sda = SHT30_SDA_PIN,
    };
    op_ret = tdd_sw_i2c_init(I2C_NUM_ID, sw_i2c_gpio);
    if(OPRT_OK != op_ret) {
        TAL_PR_ERR("err<%d>,i2c init fail!", op_ret);
    }

    while (1) {
        /*IIC write data*/
        i2c_msg.addr  = SHT3X_ADDR;
        i2c_msg.flags = SW_I2C_FLAG_WR;
        i2c_msg.buff  = write_buff;
        i2c_msg.len   = I2C_WRITE_BUFLEN;
        tdd_sw_i2c_xfer(I2C_NUM_ID, &i2c_msg);

        /*IIC read data*/
        i2c_msg.addr  = SHT3X_ADDR;
        i2c_msg.flags = SW_I2C_FLAG_RD;
        i2c_msg.buff  = read_buf;
        i2c_msg.len   = I2C_READ_BUFLEN;
        tdd_sw_i2c_xfer(I2C_NUM_ID, &i2c_msg);

        /*check read data*/
        if ((CRC_ERR == __sht3x_check_crc8(read_buf,   2, read_buf[2])) ||\
            (CRC_ERR == __sht3x_check_crc8(read_buf+3, 2, read_buf[5]))) {
            TAL_PR_ERR("[SHT3x] The received temp_humi data can't pass the CRC8 check.");
        }else {
            temper = ((USHORT_T)read_buf[0] << 8) | read_buf[1];
            humi   = ((USHORT_T)read_buf[3] << 8) | read_buf[4];

            TAL_PR_NOTICE("temper = %d humi = %d", temper, humi);
        }

        tal_system_sleep(2000);
    }   
}

/**
* @brief software i2c example
*
* @return none
*/
VOID example_sw_i2c(INT_T argc, CHAR_T *argv[])
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CALL_ERR_LOG(tal_thread_create_and_start(&sg_i2c_handle, NULL, NULL, __sw_i2c_task, NULL, &sg_task));

    return;
}



#endif