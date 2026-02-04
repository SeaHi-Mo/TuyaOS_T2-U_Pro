/**
 * @file example_drv_ir.c
 * @author www.tuya.com
 * @brief example_drv_ir module is used to 
 * @version 0.1
 * @date 2022-09-30
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */
#include "tuya_cloud_types.h"
#include "tal_log.h"
#include "tal_system.h"
#include "tal_thread.h"

#include "tdd_ir_driver.h"
#include "tdl_ir_dev_manage.h"
#include "example_drv_ir.h"
/***********************************************************
************************macro define************************
***********************************************************/
#define USE_NEC_DEMO    1

#if 1
/* bk7231n IR hardware config */
#define IR_DEV_SEND_PIN   TUYA_GPIO_NUM_8   // PWM2
#define IR_DEV_RECV_PIN   TUYA_GPIO_NUM_6   // PWM0

#else
/* 8720cf IR hardware config */
#define IR_DEV_SEND_PIN   TUYA_GPIO_NUM_2
#define IR_DEV_RECV_PIN   TUYA_GPIO_NUM_7

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
STATIC THREAD_HANDLE sg_ir_thrd_hdl = NULL;
STATIC IR_HANDLE_T   sg_ir_dev_hdl = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/
STATIC VOID __ir_example_task(PVOID_T args)
{
    OPERATE_RET rt = OPRT_OK;
    IR_DATA_U ir_send_buffer;
    IR_DATA_U *ir_recv_buffer = NULL;

#if USE_NEC_DEMO
    ir_send_buffer.nec_data.addr = 0x807F;
    ir_send_buffer.nec_data.cmd = 0x1DE2;
    ir_send_buffer.nec_data.repeat_cnt = 1;
#else
    UINT32_T data[] = {560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 560, 1690, 1690, 1690};
    ir_send_buffer.timecode.data = data;
    ir_send_buffer.timecode.len = CNTSOF(data);
#endif

    for (;;) {
        TUYA_CALL_ERR_LOG(tdl_ir_dev_send(sg_ir_dev_hdl, 38000, ir_send_buffer, 1));

        TUYA_CALL_ERR_LOG(tdl_ir_dev_recv(sg_ir_dev_hdl, &ir_recv_buffer, 3000));
        if (OPRT_OK == rt && ir_recv_buffer) {
#if USE_NEC_DEMO
            TAL_PR_DEBUG("ir nec recv: addr:%04x, cmd:%04x, cnt:%d", ir_recv_buffer->nec_data.addr, \
                                                                     ir_recv_buffer->nec_data.cmd, \
                                                                    ir_recv_buffer->nec_data.repeat_cnt);
#else
            TAL_PR_DEBUG("ir timecode recv: addr:%p, len:%d", ir_recv_buffer, ir_recv_buffer->timecode.len);
            for (UINT_T i=0; i<ir_recv_buffer->timecode.len; i++) {
                TAL_PR_DEBUG_RAW("%d ", ir_recv_buffer->timecode.data[i]);
            }
            TAL_PR_DEBUG_RAW("\r\n");
#endif
            tdl_ir_dev_recv_release(sg_ir_dev_hdl, ir_recv_buffer);
            ir_recv_buffer = NULL;
        }

        tal_system_sleep(2000);
    }

    tal_thread_delete(sg_ir_thrd_hdl);
    sg_ir_thrd_hdl = NULL;

    return;
}

/**
 * @brief    register hardware 
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET reg_ir_hardware(CHAR_T *device_name)
{
    IR_DRV_CFG_T ir_hw_cfg = {0};

    /* IR hardware config*/
    ir_hw_cfg.send_pin = IR_DEV_SEND_PIN; 
    ir_hw_cfg.recv_pin = IR_DEV_RECV_PIN;
    ir_hw_cfg.send_timer = TUYA_TIMER_NUM_0;
    ir_hw_cfg.recv_timer = TUYA_TIMER_NUM_1;
    ir_hw_cfg.send_duty = 50;
    return tdd_ir_driver_register(device_name, IR_DRV_DUAL_TIMER, ir_hw_cfg);
}

/**
 * @brief    open driver
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET open_ir_driver(CHAR_T *device_name)
{

    OPERATE_RET rt = OPRT_OK;
    IR_DEV_CFG_T ir_device_cfg = {0};

    TUYA_CALL_ERR_RETURN(tdl_ir_dev_find(device_name, &sg_ir_dev_hdl));

    /* ir device config */
    /* ir device common config*/
    ir_device_cfg.ir_mode = IR_MODE_SEND_RECV;
    ir_device_cfg.recv_queue_num = 3;
    ir_device_cfg.recv_buf_size = 1024;
    ir_device_cfg.recv_timeout = 300; // ms

#if USE_NEC_DEMO
    /* ir device nec protocol config */
    ir_device_cfg.prot_opt = IR_PROT_NEC;
    ir_device_cfg.prot_cfg.nec_cfg.is_nec_msb = 0;
    
    ir_device_cfg.prot_cfg.nec_cfg.lead_err   = 31;
    ir_device_cfg.prot_cfg.nec_cfg.logics_err = 46;
    ir_device_cfg.prot_cfg.nec_cfg.logic0_err = 46;
    ir_device_cfg.prot_cfg.nec_cfg.logic1_err = 40;
    ir_device_cfg.prot_cfg.nec_cfg.repeat_err = 24;
#else 
    /* ir device timecode protocol config */
    ir_device_cfg.prot_opt = IR_PROT_TIMECODE;
#endif

    return tdl_ir_dev_open(sg_ir_dev_hdl, &ir_device_cfg);
}

/**
 * @brief    the example of driver function
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET example_ir_running(CHAR_T *device_name)
{
    THREAD_CFG_T ir_thrd_param = {2048, 4, "ir_thread"};

    return tal_thread_create_and_start(&sg_ir_thrd_hdl, NULL, NULL, __ir_example_task, NULL, &ir_thrd_param);   
}