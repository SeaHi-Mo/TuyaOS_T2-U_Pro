/**
 * @file tdl_nec_protocol.h
 * @author www.tuya.com
 * @brief tdl_nec_protocol module is used to 
 * @version 0.1
 * @date 2023-06-08
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TDL_NEC_PROTOCOL_H__
#define __TDL_NEC_PROTOCOL_H__

#include "tuya_cloud_types.h"

#include "tdl_ir_dev_manage.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define IR_NEC_MIN_LENGTH       (68)
#define IR_NEC_REPEAT_LENGTH    (4)

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
********************function declaration********************
***********************************************************/

/**
 * @brief NEC 构建函数，将 NEC 格式转为时间码格式，构建后需要调用 tdl_ir_nec_build_release 对 timecode 进行释放
 *
 * @param[in] is_msb: 1：高位在前，0：低位在前
 * @param[in] ir_nec_data: NEC 数据
 * @param[out] timecode: 转换得到的时间码格式
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
OPERATE_RET tdl_ir_nec_build(BOOL_T is_msb, IR_DATA_NEC_T ir_nec_data, IR_DATA_TIMECODE_T **timecode);

/**
 * @brief 释放 timecode
 *
 * @param[in] timecode: 要释放的 timecode
 *
 * @return none
 */
VOID_T tdl_ir_nec_build_release(IR_DATA_TIMECODE_T *timecode);

/**
 * @brief 初始化错误率功能，初始化后需要调用 tdl_ir_nec_err_val_init_release 对 nec_err_val 进行释放
 *
 * @param[out] nec_err_val: 初始化得到的错误率数据参数
 * @param[in] nec_cfg: 要设置的错误率参数
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
OPERATE_RET tdl_ir_nec_err_val_init(VOID_T **nec_err_val, IR_NEC_CFG_T *nec_cfg);

/**
 * @brief 释放错误率初始化得到的 nec_err_val 参数
 *
 * @param[in] nec_err_val: 要释放的 nec_err_val 参数
 *
 * @return none
 */
VOID_T tdl_ir_nec_err_val_init_release(VOID_T *nec_err_val);

/**
 * @brief 获取 NEC 引导码数据索引
 *
 * @param[in] data: 传入的数据
 * @param[in] len: 传入的数据长度
 * @param[in] nec_err_val: 错误率参数
 * @param[out] head_idx: 引导码位置索引
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
OPERATE_RET tdl_nec_get_frame_head(UINT32_T *data, UINT16_T len, VOID_T *nec_err_val, UINT16_T *head_idx);

/**
 * @brief 单次解析 NEC 数据
 *
 * @param[in] data: 传入的数据
 * @param[in] len: 传入的数据长度
 * @param[in] nec_err_val: 错误率参数
 * @param[in] is_msb: 1：高位在前，0：低位在前
 * @param[out] nec_code: 解析得到的 NEC 数据
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
UINT16_T tdl_ir_nec_parser_single(unsigned int *data, unsigned int len, void *nec_err_val, unsigned char is_msb, IR_DATA_NEC_T *nec_code);

/**
 * @brief NEC 重复码解析
 *
 * @param[in] data: 传入的数据
 * @param[in] len: 传入的数据长度
 * @param[in] nec_err_val: 错误率参数
 * @param[out] repeat_cnt: 解析得到的重复码
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
UINT16_T tdl_ir_nec_parser_repeat(UINT32_T *data, UINT32_T len, void *nec_err_val, UINT16_T *repeat_cnt);

#ifdef __cplusplus
}
#endif

#endif /* __TDL_NEC_PROTOCOL_H__ */
