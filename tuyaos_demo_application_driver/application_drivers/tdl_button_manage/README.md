# 修改记录

| 版本 | 时间       | 修改内容 |
| ---- | ---------- | -------- |
| v2.1 | 2022.11.04 | 支持 TuyaOS 3.5.1，修改老的接口为 tal/tkl 接口 |
| v2.0 | 2022.04.18 | 初版     |

# 基本功能

1、提供按键管理机制，支持短按、长按、连续按

2、支持轮询和中断两种按键检测机制

# 资源依赖

| 资源               | 大小   | 说明 |
| ------------------ | ------ | ---- |
| 完成一个按键的注册 | 约2.3k |      |

# 接口列表说明

- 按键管理接口，详细使用说明参考`tdl_button_manage.h`

| 接口                                                         | 说明             |
| ------------------------------------------------------------ | ---------------- |
| OPERATE_RET tdl_button_create(IN CHAR_T *name, IN TDL_BUTTON_CFG_T *button_cfg, OUT TDL_BUTTON_HANDLE *handle); | 创建一个按键实例 |
| OPERATE_RET tdl_button_delete(IN TDL_BUTTON_HANDLE handle);  | 删除一个按键     |
| VOID tdl_button_event_register(IN TDL_BUTTON_HANDLE handle, IN TDL_BUTTON_TOUCH_EVENT_E event, IN TDL_BUTTON_EVENT_CB cb); | 按键事件注册     |
| OPERATE_RET tdl_button_deep_sleep_ctrl(BOOL_T enable);       | 使能按键         |

# 使用说明

```C
#include "tdd_button_gpio.h"
#include "tdl_button_manage.h"

#define BUTTON_DEV    "my_device"

TDL_BUTTON_HANDLE  button_handle;

/* 按键回调函数，注册的事件触发后会通过该回调进行通知 */
STATIC VOID tdl_button_cb(IN CHAR_T *name, IN TDL_BUTTON_TOUCH_EVENT_E event, IN VOID* argc)
{
    static int key_count = 0;
    PR_NOTICE("key_count %d", key_count);
    key_count++;
}

void tuya_button_dev_demo(void)
{
	int op_ret = 0;
	
	BUTTON_GPIO_CFG_T gpio_cfg;
    TDL_BUTTON_CFG_T button_cfg;	
    
    gpio_cfg.pin = 9;
    gpio_cfg.level = 1;
    gpio_cfg.mode = BUTTON_TIMER_SCAN_MODE;
    
    button_cfg.long_start_vaild_time = 3000;
    button_cfg.long_keep_timer = 1000;
    button_cfg.button_debounce_time = 50;
    button_cfg.button_repeat_vaild_count = 3;
    button_cfg.button_repeat_vaild_time = 50;

    /* 注册按钮 */
    op_ret = tdd_gpio_button_register(BUTTON_DEV, &gpio_cfg);
    if (OPRT_OK != op_ret) {
        PR_ERR("tdd_gpio_button_register err:%d", op_ret);
        return;
    } 

    /* 创建按钮，开始检查按键 */
    op_ret = tdl_button_create(BUTTON_DEV, &button_cfg, &button_handle);
    if(OPRT_OK != op_ret) {
        PR_ERR("tdl_button_create err:%d", op_ret);
        return;
    }

    /* 注册短按事件，只有注册的事件才会在回调中进行通知 */
    tdl_button_event_register(button_handle, TDL_BUTTON_PRESS_SINGLE_CLICK, tdl_button_cb);
}
```
