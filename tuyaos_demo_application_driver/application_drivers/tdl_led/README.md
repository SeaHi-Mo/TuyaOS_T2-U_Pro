
# 修改记录

| 版本 | 时间       | 修改内容 |
| ---- | ---------- | -------- |
| v2.0 | 2022.04.20 | 初版     |



#  名词解释

| 名词 | 解释       |
| ---- | ----- |
| 闪烁 |led按照一定的规律，一会儿亮，一会儿灭|

# 使用流程

初始流程为：硬件初始化->查找设备->操作设备


# 基本功能

1. 只提供纯软件逻辑的“闪烁功能服务”。

# 资源依赖

| 资源                             | 大小   | 说明                 |
| -------------------------------- | ------ | -------------------- |
| led初始化 | 约192字节 | 初始化1路led，包含GPIO外设的内存空间 |

| 外设资源 | 大小 | 说明                 |
| -------------------------------- | ------ | -------------------- |
| ledGPIO | 约24字节 | 使用1路led所需要约24字节的IO口资源 |



# 接口列表说明

* led组件管理、控制接口，详细使用请参考
`tdl_led_manage.h `

| 名词 | 解释   |
| ---- | ----- |
| OPERATE_RET tdl_led_register_function(IN CHAR_T *dev_name, IN PVOID_T handle, IN TDL_LED_CTRL_CB cb);|注册底层驱动实现，无需关心底层对象|
| OPERATE_RET tdl_led_dev_find(IN CHAR_T *dev_name, INOUT PVOID_T *handle);| 通过TDD层注册的设备名找到对应的handle |
| OPERATE_RET tdl_led_ctrl(IN TDL_LED_HAND handle, IN TDL_LED_CONFIG_T *cfg); | 控制led接口  |
| OPERATE_RET tdl_led_set_mutex_enable(IN BOOL_T en);|指示灯互斥量|



* led驱动层接口，详细使用请参考
`tdd_led_gpio_driver.h" `

| 名词 | 解释   |
| ---- | ----- |
| OPERATE_RET tdd_led_gpio_init(CHAR_T *dev_name, IN LED_GPIO_CFG_T led_cfg); | 初始化GPIO，且与tdl层绑定led句柄 |
| OPERATE_RET tdd_led_gpio_write(IN PVOID_T handle, IN TDD_LED_STAT_E stat); |控制led亮、灭、闪烁操作  |

# 使用说明

```c
#include "tuya_cloud_types.h"
#include "tal_log.h"

#include "tdd_led_gpio_driver.h"
#include "tdl_led_manage.h"

#define LED_NAME    "led_1"
#define LED_PIN     TUYA_GPIO_NUM_0

/* 电量 LED 示例函数，须在初始化之后使用 */
STATIC VOID example_led_on(PVOID_T led_hdl)
{
    OPERATE_RET rt = OPRT_OK;

    TDL_LED_CONFIG_T led_cfg = {0};
    led_cfg.stat = TDL_LED_ON;
    TUYA_CALL_ERR_LOG(tdl_led_ctrl(led_hdl, &led_cfg));

    TAL_PR_NOTICE("led on");

    return;
}

/* 熄灭 LED 示例函数，须在初始化之后使用 */
STATIC VOID example_led_off(PVOID_T led_hdl)
{
    OPERATE_RET rt = OPRT_OK;

    TDL_LED_CONFIG_T led_cfg = {0};
    led_cfg.stat = TDL_LED_OFF;
    TUYA_CALL_ERR_LOG(tdl_led_ctrl(led_hdl, &led_cfg));

    TAL_PR_NOTICE("led off");

    return;
}

/* LED 闪烁示例函数，须在初始化之后使用 */
STATIC VOID example_led_flash(PVOID_T led_hdl)
{
    OPERATE_RET rt = OPRT_OK;

    TDL_LED_CONFIG_T led_cfg = {0};
    led_cfg.stat = TDL_LED_FLASH;
    led_cfg.start_stat = FALSE;
    led_cfg.end_stat = FALSE;
    led_cfg.flash_cnt = TDL_FLASH_FOREVER;
    led_cfg.flash_first_time = 500;
    led_cfg.flash_second_time = 500;
    TUYA_CALL_ERR_LOG(tdl_led_ctrl(led_hdl, &led_cfg));

    TAL_PR_NOTICE("led flash");

    return;
}

/* LED 初始化示例函数 */
VOID example_led_init(VOID)
{
    OPERATE_RET rt = OPRT_OK;
    PVOID_T led1_hdl = NULL;

    LED_GPIO_CFG_T led_hw_cfg = {
        .pin = LED_PIN,
        .gpio_cfg = {
            .direct = TUYA_GPIO_OUTPUT,
            .mode = TUYA_GPIO_PUSH_PULL,
            .level = TUYA_GPIO_LEVEL_LOW, // Low level turn on led
        },
    };
    TUYA_CALL_ERR_GOTO(tdd_led_gpio_init(LED_NAME, led_hw_cfg), __EXIT);
    TAL_PR_NOTICE("led init success");

    TUYA_CALL_ERR_GOTO(tdl_led_dev_find(LED_NAME, &led1_hdl), __EXIT);

    /* 点亮 LED */
    example_led_on(led1_hdl);

    tkl_system_sleep(3000);

    /* 关闭 LED */
    example_led_off(led1_hdl);

    tkl_system_sleep(3000);

    /* LED 闪烁 */
    example_led_flash(led1_hdl);

__EXIT:
    return rt;
}
```
