/**
 * @file tkl_spi_weak.c
 * @author www.tuya.com
 * @version 0.1
 * @date 2024-05-17
 *
 * @copyright Copyright (c) tuya.inc 2024
 *
 */

#include "tuya_iot_config.h"

#if defined(ENABLE_SPI) && (ENABLE_SPI) 
#include "tuya_cloud_types.h"
#include "tkl_spi.h"

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
 * @brief spi init
 * 
 * @param[in] port: spi port
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_spi_init(TUYA_SPI_NUM_E port, CONST TUYA_SPI_BASE_CFG_T *cfg)
{
    return OPRT_NOT_SUPPORTED; 
}

/**
 * @brief spi deinit
 * 
 * @param[in] port: spi port
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_spi_deinit(TUYA_SPI_NUM_E port)
{
    return OPRT_NOT_SUPPORTED;     
}

/**
 * Spi send
 *
 * @param[in]  port      the spi device
 * @param[in]  data     spi send data
 * @param[in]  size     spi send data size
 *
 * @return  OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_spi_send(TUYA_SPI_NUM_E port, VOID_T *data, UINT16_T size)
{
    return OPRT_NOT_SUPPORTED;    
}

#endif