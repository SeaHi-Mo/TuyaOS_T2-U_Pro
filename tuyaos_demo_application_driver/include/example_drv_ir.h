/**
 * @file example_drv_ir.h
 * @author www.tuya.com
 * @version 0.1
 * @date 2023-10-27
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __EXAMPLE_DRV_IR_H__
#define __EXAMPLE_DRV_IR_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
*********************** macro define ***********************
***********************************************************/


/***********************************************************
********************** typedef define **********************
***********************************************************/


/***********************************************************
******************* function declaration *******************
***********************************************************/
/**
 * @brief    register hardware 
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET reg_ir_hardware(CHAR_T *device_name);

/**
 * @brief    open driver
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET open_ir_driver(CHAR_T *device_name);

/**
 * @brief    the example of driver function
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET example_ir_running(CHAR_T *device_name);


#ifdef __cplusplus
}
#endif

#endif /* __EXAMPLE_DRV_IR_H__ */
