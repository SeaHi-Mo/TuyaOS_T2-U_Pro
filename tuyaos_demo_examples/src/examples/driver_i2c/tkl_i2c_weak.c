/**
 * @file tkl_i2c_weak.c
 * @author www.tuya.com
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) tuya.inc 2024
 *
 */
#include "tuya_iot_config.h"

#if defined(ENABLE_I2C) && (ENABLE_I2C) 
#include "tuya_cloud_types.h"
#include "tkl_i2c.h"

/***********************************************************
*********************** macro define ***********************
***********************************************************/


/***********************************************************
********************** typedef define **********************
***********************************************************/


/***********************************************************
********************** variable define *********************
***********************************************************/


/***********************************************************
********************** function define *********************
***********************************************************/
/**
 * @brief i2c init
 * 
 * @param[in] port: i2c port
 * @param[in] cfg: i2c config
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_i2c_init(TUYA_I2C_NUM_E port, CONST TUYA_IIC_BASE_CFG_T *cfg)
{
    return OPRT_NOT_SUPPORTED;    
}

/**
 * @brief i2c master send
 * 
 * @param[in] port: i2c port
 * @param[in] dev_addr: iic addrress of slave device.
 * @param[in] data: i2c data to send
 * @param[in] size: Number of data items to send
 * @param[in] xfer_pending: xfer_pending: TRUE : not send stop condition, FALSE : send stop condition.
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_i2c_master_send(TUYA_I2C_NUM_E port, UINT16_T dev_addr, CONST VOID_T *data, UINT32_T size, BOOL_T xfer_pending)
{
    return OPRT_NOT_SUPPORTED;
}

/**
 * @brief i2c master recv
 * 
 * @param[in] port: i2c port
 * @param[in] dev_addr: iic addrress of slave device.
 * @param[in] data: i2c buf to recv
 * @param[in] size: Number of data items to receive
 * @param[in] xfer_pending: TRUE : not send stop condition, FALSE : send stop condition.
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_i2c_master_receive(TUYA_I2C_NUM_E port, UINT16_T dev_addr, VOID *data, UINT32_T size, BOOL_T xfer_pending)
{
    return OPRT_NOT_SUPPORTED;
}

#endif