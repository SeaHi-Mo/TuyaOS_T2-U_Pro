/**
* @file examples_driver_gpio.c
* @author www.tuya.com
* @brief 一个简单的tkl gpio接口使用演示程序，可以通过命令行执行
* @version 0.1
* @date 2022-05-20
*
* @copyright Copyright (c) tuya.inc 2022
*
*/

#include "tuya_iot_config.h"

#include "tuya_cloud_types.h"
#include "tal_log.h"
#include "tal_thread.h"
#include "tal_system.h"
#include "tkl_gpio.h"
#include "tkl_output.h"

/***********************************************************
*************************micro define***********************
***********************************************************/

/*user pin*/
/*please modify according to the platform*/
#define GPIO_OUT_PIN            TUYA_GPIO_NUM_15
#define GPIO_IN_PIN             TUYA_GPIO_NUM_17
#define GPIO_IRQ_PIN            TUYA_GPIO_NUM_9

#define TASK_GPIO_PRIORITY      THREAD_PRIO_2
#define TASK_GPIO_SIZE          1024

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC THREAD_HANDLE sg_gpio_handle;
STATIC volatile UINT_T sg_irq_trig_cnt = 0;

/***********************************************************
***********************function define**********************
***********************************************************/
/**
* @brief interrupt callback function
*
* @param[in] args:parameters
* @return none
*/
STATIC VOID_T __gpio_irq_callback(VOID_T *args)
{
    sg_irq_trig_cnt ++;
}

/**
* @brief gpio task
*
* @param[in] param:Task parameters
* @return none
*/
STATIC VOID __example_gpio_task(VOID* param)
{
    OPERATE_RET rt = OPRT_OK;
    UCHAR_T i = 0;
    TUYA_GPIO_LEVEL_E read_level = 0;
    UINT_T last_irq_trig_cnt = 0;

    /*GPIO output init*/
    TUYA_GPIO_BASE_CFG_T out_pin_cfg = {
        .mode = TUYA_GPIO_PUSH_PULL,
        .direct = TUYA_GPIO_OUTPUT,
        .level = TUYA_GPIO_LEVEL_LOW
    };
    TUYA_CALL_ERR_LOG(tkl_gpio_init(GPIO_OUT_PIN, &out_pin_cfg));

    /*GPIO input init*/
    TUYA_GPIO_BASE_CFG_T in_pin_cfg = {
        .mode = TUYA_GPIO_PUSH_PULL,
        .direct = TUYA_GPIO_INPUT,
    };
    TUYA_CALL_ERR_LOG(tkl_gpio_init(GPIO_IN_PIN, &in_pin_cfg));

    /*GPIO irq init*/
    TUYA_GPIO_IRQ_T irq_cfg = {
        .cb = __gpio_irq_callback,
        .arg = NULL,
        .mode = TUYA_GPIO_IRQ_RISE,
    };
    TUYA_CALL_ERR_LOG(tkl_gpio_irq_init(GPIO_IRQ_PIN, &irq_cfg));

    /*irq enable*/
    TUYA_CALL_ERR_LOG(tkl_gpio_irq_enable(GPIO_IRQ_PIN));

    while (1) {
        /* GPIO output */
        if (i == 0) {
            tkl_gpio_write(GPIO_OUT_PIN, TUYA_GPIO_LEVEL_HIGH);
            TAL_PR_DEBUG("pin output high");
        } else {
            tkl_gpio_write(GPIO_OUT_PIN, TUYA_GPIO_LEVEL_LOW);
            TAL_PR_DEBUG("pin output low");
        }
        i = i^1;

        /* GPIO read */
        TUYA_CALL_ERR_LOG(tkl_gpio_read(GPIO_IN_PIN, &read_level));
        if(read_level == 1) {
            TAL_PR_DEBUG("GPIO read high level");
        } else {
            TAL_PR_DEBUG("GPIO read low level");
        }

        /* GPIO irq */
        if(last_irq_trig_cnt != sg_irq_trig_cnt) {
            last_irq_trig_cnt = sg_irq_trig_cnt;
            TAL_PR_DEBUG("GPIO trig irq cnt:%d", last_irq_trig_cnt);
        }

        tal_system_sleep(2000);
    }
}

/**
* @brief examples_adc_init_and_read
*
* @return none
*/
VOID example_gpio(INT_T argc, CHAR_T *argv[])
{
    OPERATE_RET rt = OPRT_OK;

    STATIC THREAD_CFG_T thrd_param = {
        .priority = TASK_GPIO_PRIORITY,
        .stackDepth = TASK_GPIO_SIZE,
        .thrdname = "gpio"
    };
    TUYA_CALL_ERR_LOG(tal_thread_create_and_start(&sg_gpio_handle, NULL, NULL, __example_gpio_task, NULL, &thrd_param));

    return;
}
