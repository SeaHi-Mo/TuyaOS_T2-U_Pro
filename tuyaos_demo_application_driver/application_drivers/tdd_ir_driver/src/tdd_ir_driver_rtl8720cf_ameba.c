/**
 * @file tdd_ir_driver_rtl8720cf_ameba.c
 * @author www.tuya.com
 * @brief tdd_ir_driver_rtl8720cf module is used to 
 * @version 0.1
 * @date 2022-06-07
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#include "tkl_pwm.h"
#include "tkl_timer.h"
#include "tkl_gpio.h"
#include "tkl_output.h"
#include "tal_log.h"
#include "tal_memory.h"

#include "tdd_ir_driver.h"

/***********************************************************
************************macro define************************
***********************************************************/

#define IR_SEND_TIMEOUT_US          (1*1000*1000)
#define IR_SEND_INTER_CODE_DELAY    0//(300*1000) // unit: us, default: 300ms
#define IR_SEND_FREQ_DEFAULT        (38000)

#define IR_SEND_TIM_HIGH_ERR_VALUE  (-10)
#define IR_SEND_TIM_LOW_ERR_VALUE   (-20)

/***********************************************************
***********************typedef define***********************
***********************************************************/

typedef struct {
    IR_MODE_E                       driver_mode;
    IR_DRIVER_TYPE_E                driver_type;
    unsigned char                   user_close_recv_flag;
    void                            *tdl_hdl;
    TUYA_PWM_NUM_E                  send_pwm_id;

    /* ir receive use irq+timer */
    volatile unsigned char          irq_edge_mode;
    volatile unsigned int           last_recv_time;

    /* ir receive use capture */
    int                             recv_pwm_id;

    /* tdl callback */
    IR_DRV_OUTPUT_FINISH_CB         output_finish_cb;
    IR_DRV_RECV_CB                  recv_value_cb;

    /* ir drive hardware config information */
    IR_DRV_CFG_T                    hw_cfg;
}IR_DRV_INFO_T;

/***********************************************************
********************function declaration********************
***********************************************************/

/* tdl adapter interface */
static int __tdd_ir_drv_open(IR_DRV_HANDLE_T drv_hdl, unsigned char mode, IR_TDL_TRANS_CB ir_tdl_cb, void *args);
static int __tdd_ir_drv_close(IR_DRV_HANDLE_T drv_hdl, unsigned char mode);
static int __tdd_ir_drv_output(IR_DRV_HANDLE_T drv_hdl, unsigned int freq, unsigned char is_active, unsigned int time_us);
static int __tdd_ir_drv_status_notification(IR_DRV_HANDLE_T drv_hdl, IR_DRIVER_STATE_E state, void *args);

static int __tdd_ir_pre_recv_process(IR_DRV_HANDLE_T drv_hdl);
static int __tdd_ir_recv_finish_process(IR_DRV_HANDLE_T drv_hdl);
static int __tdd_ir_pre_send_process(IR_DRV_HANDLE_T drv_hdl);
static int __tdd_ir_send_finish_process(IR_DRV_HANDLE_T drv_hdl);

/* hardware interface */
static int __tdd_ir_send_hw_init(IR_DRV_INFO_T *drv_info);
static void __tdd_ir_send_hw_deinit(IR_DRV_INFO_T *drv_info);
static int __tdd_ir_recv_hw_init(IR_DRV_INFO_T *drv_info);
static void __tdd_ir_recv_hw_deinit(IR_DRV_INFO_T *drv_info);

static int __tdd_ir_send_start(IR_DRV_INFO_T *drv_info);

static int __tdd_get_pwm_id(unsigned int gpio_id, TUYA_PWM_NUM_E *pwm_id);

/***********************************************************
***********************variable define**********************
***********************************************************/

static TDD_IR_INTFS_T tdd_ir_intfs = {
    .open           = __tdd_ir_drv_open,
    .close          = __tdd_ir_drv_close,
    .output         = __tdd_ir_drv_output,
    .status_notif   = __tdd_ir_drv_status_notification
};

/***********************************************************
***********************function define**********************
***********************************************************/

/**
 * @brief set pwm info and start pwm
 *
 * @param[in] pwm_id: pwm id
 * @param[in] pwm_freq: pwm frequency
 * @param[in] pwm_duty: pwm duty
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
static int __tdd_ir_pwm_start(int pwm_id, int pwm_freq, int pwm_duty)
{
    OPERATE_RET op_ret = OPRT_OK;

    if (0 == pwm_duty) {
        return tkl_pwm_stop(pwm_id);
    }

    TUYA_PWM_BASE_CFG_T pwm_cfg = {
        .polarity = TUYA_PWM_NEGATIVE,
        .duty = pwm_duty*100,
        .frequency = pwm_freq,
    };
    op_ret = tkl_pwm_info_set(pwm_id, &pwm_cfg);
    if (OPRT_OK != op_ret) {
        goto __EXIT;
    }

    op_ret = tkl_pwm_start(pwm_id);

__EXIT:
    return op_ret;
}

/**
 * @brief driver status notification function
 *
 * @param[in] drv_hdl: driver handle
 * @param[in] device_state: device state
 * @param[in] args: 
 *
 * @return none
 */
static int __tdd_ir_drv_status_notification(IR_DRV_HANDLE_T drv_hdl, IR_DRIVER_STATE_E state, void *args)
{
    if (NULL == drv_hdl) {
        return OPRT_INVALID_PARM;
    }
    IR_DRV_INFO_T *drv_info = drv_hdl;

    switch (state) {
        case IR_DRV_PRE_SEND_STATE:
            __tdd_ir_pre_send_process(drv_hdl);
        break;
        case IR_DRV_SEND_FINISH_STATE:
            __tdd_ir_send_finish_process(drv_hdl);
        break;
        case IR_DRV_PRE_RECV_STATE:
            __tdd_ir_pre_recv_process(drv_hdl);
        break;
        case IR_DRV_RECV_FINISH_STATE:
            __tdd_ir_recv_finish_process(drv_hdl);
        break;
        case IR_DRV_SEND_HW_RESET:
            __tdd_ir_send_hw_deinit(drv_hdl);
            __tdd_ir_send_hw_init(drv_hdl);
        break;
        case IR_DRV_RECV_HW_INIT:
            drv_info->user_close_recv_flag = 0;
            __tdd_ir_recv_hw_init(drv_hdl);
        break;
        case IR_DRV_RECV_HW_DEINIT:
            __tdd_ir_recv_hw_deinit(drv_hdl);
            drv_info->user_close_recv_flag = 1;
        break;
        default:break;
    }

    return OPRT_OK;
}

/**
 * @brief ir driver send callback
 *
 * @param[in] drv_handle: driver handle
 *
 * @return none
 */
static void __tdd_ir_timer_send_cb(void *drv_handle)
{
    IR_DRV_INFO_T *drv_info = NULL;

    drv_info = (IR_DRV_INFO_T *)drv_handle;
    if (NULL == drv_info) {
        tkl_log_output("send cb input is null\r\n");
        return;
    }

    /* stop timer */
    tkl_timer_stop(drv_info->hw_cfg.send_timer);

    if (NULL != drv_info->output_finish_cb) {
        drv_info->output_finish_cb(drv_info->tdl_hdl);
    }

    return;
}

/**
 * @brief gpio irq callback function
 *
 * @param[in] drv_handle: driver handle
 *
 * @return none
 */
static void __tdd_ir_irq_recv_cb(void *drv_handle)
{
    unsigned int cur_us = 0, out_us = 0;
    IR_DRV_INFO_T *drv_info = (IR_DRV_INFO_T *)drv_handle;

    if (NULL == drv_info) {
        return;
    }

    tkl_gpio_irq_disable(drv_info->hw_cfg.recv_pin);
    tkl_gpio_deinit(drv_info->hw_cfg.recv_pin);

    tkl_timer_get(drv_info->hw_cfg.recv_timer, &cur_us);

    if (drv_info->last_recv_time < cur_us) {
        out_us = cur_us - (drv_info->last_recv_time);
    } else {
        out_us = (0xffffffffUL-drv_info->last_recv_time)+cur_us+1;
    }
    drv_info->last_recv_time = cur_us;

    drv_info->recv_value_cb(drv_handle, out_us, drv_info->tdl_hdl);

    /* enable next irq interrupt */
    if (TUYA_GPIO_IRQ_FALL == drv_info->irq_edge_mode) {
        drv_info->irq_edge_mode = TUYA_GPIO_IRQ_RISE;
    } else if (TUYA_GPIO_IRQ_RISE == drv_info->irq_edge_mode) {
        drv_info->irq_edge_mode = TUYA_GPIO_IRQ_FALL;
    }

    TUYA_GPIO_IRQ_T gpio_irq_cfg = {
        .mode = drv_info->irq_edge_mode,
        .cb = __tdd_ir_irq_recv_cb,
        .arg = drv_handle
    };
    tkl_gpio_irq_init(drv_info->hw_cfg.recv_pin, &gpio_irq_cfg);
    tkl_gpio_irq_enable(drv_info->hw_cfg.recv_pin);

    return;
}

/**
 * @brief ir receive timer callback
 *
 * @param[in] args:
 *
 * @return none
 */
static void __tdd_timer_recv_cb(void *args)
{
    /* nothing to do here */
    return;
}

/**
 * @brief ir send hardware init
 *
 * @param[in] drv_info: driver info structure
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
static int __tdd_ir_send_hw_init(IR_DRV_INFO_T *drv_info)
{
    OPERATE_RET op_ret = OPRT_OK;

    if (NULL == drv_info) {
        return OPRT_INVALID_PARM;
    }

    /* PWM will also use the hardware timer, 
        you must first initialize the hardware timer before initializing the PWM, 
        otherwise the hardware timer to be used may be used by the PWM */
    /* timer init */
    TUYA_TIMER_BASE_CFG_T timer_cfg = {
        .mode = TUYA_TIMER_MODE_PERIOD,
        .cb = __tdd_ir_timer_send_cb,
        .args = drv_info
    };
    op_ret = tkl_timer_init(drv_info->hw_cfg.send_timer, &timer_cfg);
    if (OPRT_OK != op_ret) {
        tkl_log_output("timer init error\r\n");
        return op_ret;
    }

    /* pwm init */
    TUYA_PWM_BASE_CFG_T pwm_cfg = {
        .polarity = TUYA_PWM_NEGATIVE,
        .duty = drv_info->hw_cfg.send_duty*100,
        .frequency = IR_SEND_FREQ_DEFAULT,
    };
    op_ret = tkl_pwm_init(drv_info->send_pwm_id, &pwm_cfg);
    if (OPRT_OK != op_ret) {
        tkl_log_output("pwm init error\r\n");
        return op_ret;
    }

    return OPRT_OK;
}

/**
 * @brief ir send hardware deinit
 *
 * @param[in] drv_info: driver info structure
 *
 * @return none
 */
static void __tdd_ir_send_hw_deinit(IR_DRV_INFO_T *drv_info)
{
    OPERATE_RET op_ret = OPRT_OK;
    
    if (NULL == drv_info) {
        tkl_log_output("send hardware deinit error, input invalid\r\n");
        return;
    }

    /* deinit pwm */
    op_ret = tkl_pwm_stop(drv_info->send_pwm_id);
    if (op_ret != OPRT_OK) {
        tkl_log_output("pwm stop error\r\n");
    }
    op_ret = tkl_pwm_deinit(drv_info->send_pwm_id);
    if (op_ret != OPRT_OK) {
        tkl_log_output("pwm deinit error\r\n");
    }

    /* deinit timer */
    op_ret = tkl_timer_stop(drv_info->hw_cfg.send_timer);
    if (op_ret != OPRT_OK) {
        tkl_log_output("timer stop error\r\n");
    }
    op_ret = tkl_timer_deinit(drv_info->hw_cfg.send_timer);
    if (op_ret != OPRT_OK) {
        tkl_log_output("timer deinit error\r\n");
    }

    return;
}

/**
 * @brief ir receive IR_DRV_SINGLE_TIMER, IR_DRV_DUAL_TIMER init
 *
 * @param[in] drv_info: driver info structure
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
static int __tdd_ir_recv_irq_timer_init(IR_DRV_INFO_T *drv_info)
{
    OPERATE_RET op_ret = OPRT_OK;

    /* timer init */
    TUYA_TIMER_BASE_CFG_T timer_cfg = {
        .mode = TUYA_TIMER_MODE_PERIOD,
        .cb = __tdd_timer_recv_cb,
        .args = NULL
    };
    op_ret = tkl_timer_init(drv_info->hw_cfg.recv_timer, &timer_cfg);
    if (OPRT_OK != op_ret) {
        tkl_log_output("timer init err\r\n");
        op_ret = OPRT_COM_ERROR;
        goto __EXIT;
    }

    op_ret = tkl_timer_start(drv_info->hw_cfg.recv_timer, 0xFFFFFFFF);
    if (OPRT_OK != op_ret) {
        tkl_log_output("timer start err\r\n");
        op_ret = OPRT_COM_ERROR;
        goto __EXIT;
    }

    TUYA_GPIO_IRQ_T gpio_irq_cfg = {
        .mode = TUYA_GPIO_IRQ_FALL,
        .cb = __tdd_ir_irq_recv_cb,
        .arg = (void *)drv_info,
    };
    drv_info->irq_edge_mode = TUYA_GPIO_IRQ_FALL;
    op_ret = tkl_gpio_irq_init(drv_info->hw_cfg.recv_pin, &gpio_irq_cfg);
    if (OPRT_OK != op_ret) {
        tkl_log_output("gpio irq init err\r\n");
        op_ret = OPRT_COM_ERROR;
        goto __EXIT;
    }

    op_ret = tkl_gpio_irq_enable(drv_info->hw_cfg.recv_pin);
    if (OPRT_OK != op_ret) {
        tkl_log_output("enable gpio irq err\r\n");
        op_ret = OPRT_COM_ERROR;
    }

__EXIT:
    if (OPRT_OK != op_ret) {
        tkl_timer_stop(drv_info->hw_cfg.recv_timer);
        tkl_timer_deinit(drv_info->hw_cfg.recv_timer);
    }

    return OPRT_OK;
}

/**
 * @brief ir receive hardware init
 *
 * @param[in] drv_info: driver info structure
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
static int __tdd_ir_recv_hw_init(IR_DRV_INFO_T *drv_info)
{
    OPERATE_RET op_ret = OPRT_OK;

    if (NULL == drv_info) {
        return OPRT_INVALID_PARM;
    }

    if (drv_info->user_close_recv_flag) {
        tkl_log_output("user close ir recv, no init\r\n");
        return OPRT_OK;
    }

    if (IR_DRV_CAPTURE == drv_info->driver_type) {
        return OPRT_NOT_SUPPORTED;
    } else {
        op_ret = __tdd_ir_recv_irq_timer_init(drv_info);
    }

    return op_ret;
}

/**
 * @brief ir receive IR_DRV_SINGLE_TIMER, IR_DRV_DUAL_TIMER type deinit
 *
 * @param[in] drv_info: driver info structure
 *
 * @return none
 */
static void __tdd_ir_recv_irq_timer_deinit(IR_DRV_INFO_T *drv_info)
{
    tkl_gpio_irq_disable(drv_info->hw_cfg.recv_pin);
    tkl_gpio_deinit(drv_info->hw_cfg.recv_pin);
    tkl_timer_stop(drv_info->hw_cfg.recv_timer);
    tkl_timer_deinit(drv_info->hw_cfg.recv_timer);

    return;
}

/**
 * @brief ir receive hardware deinit
 *
 * @param[in] drv_info: driver info structure
 *
 * @return none
 */
static void __tdd_ir_recv_hw_deinit(IR_DRV_INFO_T *drv_info)
{
    if (NULL == drv_info) {
        tkl_log_output("ir receive hardware deinit error, input invalid\r\n");
        return;
    }

    if (IR_DRV_CAPTURE == drv_info->driver_type) {
        /* no support */
        return;
    } else {
        __tdd_ir_recv_irq_timer_deinit(drv_info);
    }

    return;
}

/**
 * @brief ir pre-send process
 *
 * @param[in] drv_info: driver info structure
 *
 * @return none
 */
static int __tdd_ir_pre_send_process(IR_DRV_HANDLE_T drv_hdl)
{
    OPERATE_RET op_ret = OPRT_OK;
    IR_DRV_INFO_T *drv_info = (IR_DRV_INFO_T *)drv_hdl;

    if (IR_DRV_SINGLE_TIMER == drv_info->driver_type && IR_MODE_SEND_RECV == drv_info->driver_mode) {
        __tdd_ir_recv_hw_deinit(drv_info);
    }

    op_ret = __tdd_ir_send_hw_init(drv_info);
    if (OPRT_OK != op_ret) {
        return op_ret;
    }

    op_ret = __tdd_ir_send_start(drv_info);
    if (OPRT_OK != op_ret) {
        tkl_log_output("tdd send start error\r\n");
        __tdd_ir_send_finish_process(drv_hdl);
        return op_ret;
    }

    return OPRT_OK;
}

/**
 * @brief ir send finish process
 *
 * @param[in] drv_info: driver info structure
 * @param[in] is_success: 1: send success, 0: send fail
 *
 * @return none
 */
static int __tdd_ir_send_finish_process(IR_DRV_HANDLE_T drv_hdl)
{
    if (NULL == drv_hdl) {
        tkl_log_output("send finish process error, input invalid\r\n");
        return OPRT_INVALID_PARM;
    }

    IR_DRV_INFO_T *drv_info = (IR_DRV_INFO_T *)drv_hdl;

    __tdd_ir_send_hw_deinit(drv_info);
    if (IR_DRV_SINGLE_TIMER == drv_info->driver_type && IR_MODE_SEND_RECV == drv_info->driver_mode) {
        __tdd_ir_recv_hw_init(drv_info);
    }

    return OPRT_OK;
}

/**
 * @brief ir pre-receive process
 *
 * @param[in] drv_info: driver info struct
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
static int __tdd_ir_pre_recv_process(IR_DRV_HANDLE_T drv_hdl)
{
    if (NULL == drv_hdl) {
        return OPRT_INVALID_PARM;
    }

    return OPRT_OK;
}

/**
 * @brief ir receive finish process
 *
 * @param[in] drv_info: driver info struct
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
static int __tdd_ir_recv_finish_process(IR_DRV_HANDLE_T drv_hdl)
{
    IR_DRV_INFO_T *drv_info = (IR_DRV_INFO_T *)drv_hdl;

    if (NULL == drv_info) {
        return OPRT_INVALID_PARM;
    }

    drv_info->last_recv_time = 0;

    return OPRT_OK;
}

/**
 * @brief start send callback
 *
 * @param[in] drv_info: driver info structure
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
static int __tdd_ir_send_start(IR_DRV_INFO_T *drv_info)
{
    OPERATE_RET op_ret = OPRT_OK;
    UINT_T  delay_time = 0;

    if (NULL == drv_info) {
        return OPRT_INVALID_PARM;
    }

    op_ret = __tdd_ir_pwm_start(drv_info->send_pwm_id, IR_SEND_FREQ_DEFAULT, 0);
    if (OPRT_OK != op_ret) {
        return op_ret;
    }

    if (IR_SEND_INTER_CODE_DELAY == 0) {
        delay_time = 300; //us
    } else {
        delay_time = IR_SEND_INTER_CODE_DELAY;
    }

    op_ret = tkl_timer_start(drv_info->hw_cfg.send_timer, delay_time);
    if (OPRT_OK != op_ret) {
        return op_ret;
    }

    return op_ret;
}

/**
 * @brief tdd open interface 
 *
 * @param[in] drv_hdl: ir driver info handle
 * @param[in] mode: ir device open mode
 * @param[in] ir_tdl_cb: ir device callback struct
 * @param[in] tdl_hdl: 
 * 
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
static int __tdd_ir_drv_open(IR_DRV_HANDLE_T drv_hdl, unsigned char mode, IR_TDL_TRANS_CB ir_tdl_cb, void *tdl_hdl)
{
    OPERATE_RET op_ret = OPRT_OK;
    IR_DRV_INFO_T *drv_info = NULL;

    if (NULL==drv_hdl || NULL==tdl_hdl) {
        return OPRT_INVALID_PARM;
    }

    if ((IR_MODE_SEND_ONLY==mode && NULL==ir_tdl_cb.output_finish_cb) || \
        (IR_MODE_RECV_ONLY==mode && NULL==ir_tdl_cb.recv_cb) || \
        (IR_MODE_SEND_RECV==mode && (NULL==ir_tdl_cb.output_finish_cb || NULL==ir_tdl_cb.recv_cb))) {
        return OPRT_INVALID_PARM;
    }

    drv_info = (IR_DRV_INFO_T *)drv_hdl;

    drv_info->output_finish_cb = ir_tdl_cb.output_finish_cb;
    drv_info->recv_value_cb = ir_tdl_cb.recv_cb;
    drv_info->tdl_hdl = tdl_hdl;
    drv_info->driver_mode = mode;

    if (IR_DRV_CAPTURE == drv_info->driver_type) {
        return OPRT_NOT_SUPPORTED;
    }

    if (IR_MODE_SEND_ONLY != mode) {
        drv_info->user_close_recv_flag = 0;
        __tdd_ir_recv_hw_deinit(drv_info);
        op_ret = __tdd_ir_recv_hw_init(drv_info);
        if (OPRT_OK != op_ret) {
            tkl_log_output("tdd recv hardware init failed\r\n");
            return op_ret;
        }
    }

    return OPRT_OK;
}

/**
 * @brief close ir driver
 *
 * @param[in] drv_hdl: ir driver info handle
 * @param[in] mode: ir device open mode
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
static int __tdd_ir_drv_close(IR_DRV_HANDLE_T drv_hdl, unsigned char mode)
{
    IR_DRV_INFO_T *drv_info = NULL;

    if (NULL == drv_hdl) {
        return OPRT_INVALID_PARM;
    }

    drv_info = (IR_DRV_INFO_T *)drv_hdl;

    __tdd_ir_send_hw_deinit(drv_info);
    if (IR_MODE_SEND_ONLY != mode) {
        __tdd_ir_recv_hw_deinit(drv_info);
    }

    return OPRT_OK;
}

/**
 * @brief ir device tdl send interface
 *
 * @param[in] drv_hdl: tdd driver handle
 * @param[in] freq: send carrier frequency
 * @param[in] is_active: 1: pwm, 0: low level
 * @param[in] time_us: time
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
static int __tdd_ir_drv_output(IR_DRV_HANDLE_T drv_hdl, unsigned int freq, unsigned char is_active, unsigned int time_us)
{
    OPERATE_RET op_ret = OPRT_OK;
    unsigned int delay_time = 0;

    if (NULL == drv_hdl) {
        return OPRT_INVALID_PARM;
    }

    IR_DRV_INFO_T *drv_info = (IR_DRV_INFO_T *)drv_hdl;

    if (is_active) {
        delay_time = ((time_us + IR_SEND_TIM_HIGH_ERR_VALUE)<=0 ? time_us : (time_us + IR_SEND_TIM_HIGH_ERR_VALUE));
        __tdd_ir_pwm_start(drv_info->send_pwm_id, freq, drv_info->hw_cfg.send_duty);
    } else {
        delay_time = ((time_us + IR_SEND_TIM_LOW_ERR_VALUE)<=0 ? time_us : (time_us + IR_SEND_TIM_LOW_ERR_VALUE));
        __tdd_ir_pwm_start(drv_info->send_pwm_id, freq, 0);
    }

    if (delay_time<50 || delay_time>IR_SEND_TIMEOUT_US) {
        op_ret = OPRT_COM_ERROR;
    } else {
        op_ret = tkl_timer_start(drv_info->hw_cfg.send_timer, delay_time);
    }

    return op_ret;
}

/**
 * @brief Convert gpio id to pwm id
 *
 * @param[in] gpio_id: gpio id, reference "tuya_cloud_types.h" file TUYA_GPIO_NUM_E
 * @param[out] pwm_id: pwm id, reference "tuya_cloud_types.h" file TUYA_PWM_NUM_E
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
static int __tdd_get_pwm_id(unsigned int gpio_id, TUYA_PWM_NUM_E *pwm_id)
{
    unsigned char i;
    const unsigned int gpio_pwm_map[6][2] = {
        {TUYA_GPIO_NUM_2,  TUYA_PWM_NUM_0},
        {TUYA_GPIO_NUM_3,  TUYA_PWM_NUM_1},
        {TUYA_GPIO_NUM_4,  TUYA_PWM_NUM_2},
        {TUYA_GPIO_NUM_17, TUYA_PWM_NUM_3},
        {TUYA_GPIO_NUM_18, TUYA_PWM_NUM_4},
        {TUYA_GPIO_NUM_19, TUYA_PWM_NUM_5}
    };

    if (NULL == pwm_id) {
        return OPRT_INVALID_PARM;
    }

    for (i = 0; i < 6; i++) {
        if (gpio_pwm_map[i][0] == gpio_id) {
            *pwm_id = (TUYA_PWM_NUM_E)gpio_pwm_map[i][1];
            break;
        }
    }

    if (i >= 6) {
        return OPRT_NOT_SUPPORTED;
    }

    return OPRT_OK;
}

/**
 * @brief register ir driver
 *
 * @param[in] dev_name: device name, maximum 16 bytes
 * @param[in] drv_cfg: driver config params
 *
 * @return OPRT_OK on success. Others on error, please refer to "tuya_error_code.h"
 */
int tdd_ir_driver_register(CHAR_T *dev_name, IR_DRIVER_TYPE_E driver_type, IR_DRV_CFG_T drv_cfg)
{
    OPERATE_RET op_ret = OPRT_OK;
    unsigned int recv_pwm_id = 0;

    IR_DRV_INFO_T *tdd_drv_info = NULL;

    if (NULL == dev_name || IR_DRV_TYPE_MAX <= driver_type) {
        return OPRT_INVALID_PARM ;
    }

    if (IR_DRV_CAPTURE == driver_type) {
        return OPRT_NOT_SUPPORTED;
    }

    tdd_drv_info = (IR_DRV_INFO_T *)tal_malloc(SIZEOF(IR_DRV_INFO_T));
    if (NULL == tdd_drv_info) {
        return OPRT_MALLOC_FAILED;
    }
    memset(tdd_drv_info, 0, SIZEOF(IR_DRV_INFO_T));

    op_ret = __tdd_get_pwm_id(drv_cfg.send_pin, &tdd_drv_info->send_pwm_id);
    if (OPRT_OK != op_ret) {
        TAL_PR_ERR("%d no support ir send", drv_cfg.send_pin);
        return op_ret;
    }
    

    if (IR_DRV_SINGLE_TIMER == driver_type) {
        drv_cfg.recv_timer = drv_cfg.send_timer;
    }
    tdd_drv_info->recv_pwm_id = recv_pwm_id;
    tdd_drv_info->driver_type = driver_type;

    memcpy(&tdd_drv_info->hw_cfg, &drv_cfg, sizeof(IR_DRV_CFG_T));

#if 0
    TAL_PR_DEBUG("************* ir use hw **********");
    TAL_PR_DEBUG("* send: pin:%d ---> PWM%d, TIMER%d", tdd_drv_info->send_pwm_id, tdd_drv_info->send_pwm_id, tdd_drv_info->hw_cfg.send_timer);
    if (IR_DRV_CAPTURE == driver_type) {
        TAL_PR_DEBUG("* recv: pin:%d ---> PWM%d", tdd_drv_info->hw_cfg.recv_pin, tdd_drv_info->recv_pwm_id);
    } else {
        TAL_PR_DEBUG("* recv: pin:%d, TIMER%d", tdd_drv_info->hw_cfg.recv_pin, tdd_drv_info->hw_cfg.recv_timer);
    }
    TAL_PR_DEBUG("**********************************");
#endif

    op_ret = tdl_ir_dev_register(dev_name, (IR_DRV_HANDLE_T)tdd_drv_info, &tdd_ir_intfs);

    return op_ret;
}
