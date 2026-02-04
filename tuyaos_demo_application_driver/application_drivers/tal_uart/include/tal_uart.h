/**
 * @file tal_uart.c
 * @author www.tuya.com
 * @brief tal_uart module is used to 
 * @version 0.1
 * @date 2022-11-10
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#ifndef __TAL_UART_C__
#define __TAL_UART_C__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define O_BLOCK 1 /* 阻塞读写 */
/* 通过注册回调区别是不是异步写 */
#define O_ASYNC_WRITE (1 << 1) /* 异步写 */
#define O_FLOW_CTRL (1 << 2) /* 流控使能 */

/* 用KCONFIG配置 */
#define O_TX_DMA (1 << 3) /* 用DMA发送 */
#define O_RX_DMA (1 << 4) /* 用DMA接收 */

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    UINT32_T  rx_buffer_size;
#ifdef CONFIG_TX_ASYNC
    UINT32_T  tx_buffer_size;
#endif
    UINT8_T open_mode;
    TUYA_UART_BASE_CFG_T base_cfg;
} TAL_UART_CFG_T;

/***********************************************************
********************function declaration********************
***********************************************************/

/**
 * @brief init uart
 *
 * @param[in] port_id: uart port id, id index starts from 0
 *                     in linux platform, 
 *                        high 16 bits aslo means uart type, 
 *                                  it's value must be one of the TUYA_UART_TYPE_E type
 *                        the low 16bit - means uart port id
 *                        you can input like this TUYA_UART_PORT_ID(TUYA_UART_SYS, 2)
 * @param[in] cfg: uart configure
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
OPERATE_RET tal_uart_init(TUYA_UART_NUM_E port_id, TAL_UART_CFG_T *cfg);

/**
 * @brief read data from uart
 *
 * @param[in] port_id: uart port id, id index starts from 0
 *                     in linux platform, 
 *                         high 16 bits aslo means uart type, 
 *                                   it's value must be one of the TUYA_UART_TYPE_E type
 *                         the low 16bit - means uart port id
 *                         you can input like this TUYA_UART_PORT_ID(TUYA_UART_SYS, 2)
 * @param[in] data: read data buffer
 * @param[in] len: the read size
 *
 * @return >=0, the read size; < 0, read error
 */
INT_T tal_uart_read(TUYA_UART_NUM_E port_id, UINT8_T *data, UINT32_T len);

/**
 * @brief send data by uart
 *
 * @param[in] port_id: uart port id, id index starts from 0
 *                     in linux platform, 
 *                         high 16 bits aslo means uart type, 
 *                                   it's value must be one of the TUYA_UART_TYPE_E type
 *                         the low 16bit - means uart port id
 *                         you can input like this TUYA_UART_PORT_ID(TUYA_UART_SYS, 2)
 * @param[in] data: send data buffer
 * @param[in] len: the send size
 *
 * @return >=0, the write size; < 0, write error
 */
INT_T tal_uart_write(TUYA_UART_NUM_E port_id, CONST UINT8_T *data, UINT32_T len);

/**
 * @brief deinit uart
 *
 * @param[in] port_id: uart port id, id index starts from 0
 *                     in linux platform, 
 *                         high 16 bits aslo means uart type, 
 *                                   it's value must be one of the TUYA_UART_TYPE_E type
 *                         the low 16bit - means uart port id
 *                         you can input like this TUYA_UART_PORT_ID(TUYA_UART_SYS, 2)
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
OPERATE_RET tal_uart_deinit(TUYA_UART_NUM_E port_id);


/**
 * @brief uart irq callback
 * 
 * @param[in] port_id: uart port id, id index starts from 0
 *                     in linux platform, 
 *                         high 16 bits aslo means uart type, 
 *                                   it's value must be one of the TUYA_UART_TYPE_E type
 *                         the low 16bit - means uart port id
 *                         you can input like this TUYA_UART_PORT_ID(TUYA_UART_SYS, 2)
 * @param[out] buff: data receive buff
 * @param[in] len: receive length
 *
 * @return none
 */
typedef VOID_T (*TAL_UART_IRQ_CB)(TUYA_UART_NUM_E port_id, VOID_T *buff, UINT16_T len);

/**
 * @brief enable uart rx interrupt and register interrupt callback func
 * 
 * @param[in] port_id: uart port id, id index starts from 0
 *                     in linux platform, 
 *                         high 16 bits aslo means uart type, 
 *                                   it's value must be one of the TUYA_UART_TYPE_E type
 *                         the low 16bit - means uart port id
 *                         you can input like this TUYA_UART_PORT_ID(TUYA_UART_SYS, 2)
 * @param[in] rx_cb: receive interrupt callback
 *
 * @return none
 */
VOID_T tal_uart_rx_reg_irq_cb(TUYA_UART_NUM_E port_id, TAL_UART_IRQ_CB rx_cb);

/**
 * @brief get rx data size
 * 
 * @param[in] port_num: uart port num, id index starts from 0
 *                     in linux platform, 
 *                         high 16 bits aslo means uart type, 
 *                                   it's value must be one of the TUYA_UART_TYPE_E type
 *                         the low 16bit - means uart port id
 *                         you can input like this TUYA_UART_PORT_ID(TUYA_UART_SYS, 2)
 *
 * @return rx data size
 */
INT_T tal_uart_get_rx_data_size(TUYA_UART_NUM_E port_num);

#ifdef __cplusplus
}
#endif

#endif /* __TAL_UART_C__ */



