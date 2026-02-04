# 修改记录

| 版本   | 时间         | 修改内容 |
| ---- | ---------- | ---- |
| v2.1 | 2023.03.09 | 更新示例   |
| v2.0 | 2022.04.15 | 初版   |

# 名词解释

# 基本功能

1、提供查找设备（灯带）、启动设备和停止设备的功能。通过设备名字查找已注册设备，开启设备或停止设备

2、提供像素数据刷新显示功能，在灯带上显示相应的颜色

3、提供单一颜色显示、多种颜色显示、在背景色上设置前景色功能等

4、提供像素点颜色的左右平移、镜像平移功能等

# 组件依赖

| sdk组件依赖         | 说明     |
| --------------- | ------ |
| tal_mutex.h     | 锁      |
| tal_semaphore.h | 信号量    |
| tal_thread.h    | 线程     |
| tal_memory.h    | 内存池    |
| tal_log.h       | 日志管理   |
| tal_system.h    | 系统相关接口 |

# 资源依赖

以 ws2812 设备为例，驱动灯珠数为100颗

| 资源              | 大小    | 说明                               |
| --------------- | ----- | -------------------------------- |
| 完成设备注册并打开设备内存消耗 | 约7.6k | 每增加（减少）10颗灯珠，RAM消耗增加（减少）300bytes |

# 接口列表说明

- 像素点驱动管理接口，详细使用说明参考`tdl_pixel_dev_manage.h`

| 接口                                                                                   | 说明                |
| ------------------------------------------------------------------------------------ | ----------------- |
| int tdl_pixel_dev_find(char *name, OUT PIXEL_HANDLE_T *handle);                      | 查找幻彩灯带设备          |
| int tdl_pixel_dev_open(PIXEL_HANDLE_T handle, PIXEL_DEV_CONFIG_T* config);           | 启动设备              |
| int tdl_pixel_dev_refresh(PIXEL_HANDLE_T handle);                                    | 刷新所有像素显存的数据到驱动端显示 |
| int tdl_pixel_dev_config(PIXEL_HANDLE_T handle, PIXEL_DEV_CFG_CMD_E cmd, void *arg); | 配置设备参数            |
| int tdl_pixel_dev_close(PIXEL_HANDLE_T *handle);                                     | 停止设备              |

- 像素点驱动颜色操作接口，详细使用说明参考`tdl_pixel_color_manage.h`

| 接口                                                                                                                                                          | 说明           |
| ----------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------ |
| int tdl_pixel_set_single_color(PIXEL_HANDLE_T handle, UINT_T index_start, UINT_T pixel_num, PIXEL_COLOR_T *color);                                          | 设置像素段颜色（单一）  |
| int tdl_pixel_set_multi_color(PIXEL_HANDLE_T handle, UINT_T index_start, UINT_T pixel_num, PIXEL_COLOR_T *color_arr);                                       | 设置像素段颜色（多种）  |
| int tdl_pixel_set_single_color_with_backcolor(PIXEL_HANDLE_T handle, UINT_T index_start, UINT_T pixel_num, PIXEL_COLOR_T *backcolor, PIXEL_COLOR_T *color); | 在背景色上设置像素段颜色 |
| int tdl_pixel_cycle_shift_color(PIXEL_HANDLE_T handle, PIXEL_SHIFT_DIR_T dir, UINT_T index_start, UINT_T index_end, UINT_T move_step);                      | 循环平移像素颜色     |
| int tdl_pixel_mirror_cycle_shift_color(PIXEL_HANDLE_T handle, PIXEL_M_SHIFT_DIR_T dir, UINT_T index_start, UINT_T index_end, UINT_T move_step);             | 镜像循环移动像素颜色   |
| int tdl_pixel_get_color(PIXEL_HANDLE_T handle, UINT_T index,  PIXEL_COLOR_T *color);                                                                        | 获得像素颜色       |

# 使用说明

```c
#include "tuya_app_config.h"

#include "tuya_cloud_types.h"
#include "tal_log.h"
#include "tal_system.h"
#include "tal_thread.h"

#include "tdd_pixel_ws2812.h"
#include "tdl_pixel_dev_manage.h"
#include "tdl_pixel_color_manage.h"

// 幻彩灯带硬件配置
#define EXAMPLE_LEDS_PIXEL_NAME         "light_dev"
#define EXAMPLE_LEDS_PIXEL_PORT         0
#define EXAMPLE_LEDS_PIXEL_LINE_SEQ     2
#define EXAMPLE_LEDS_PIXEL_NUM          200
#define EXAMPLE_LEDS_PIXEL_RESOLUTION   1000

#define COLOR_ARR_NUM      3

static PIXEL_HANDLE_T pixel_hdl = NULL;
static THREAD_HANDLE pixel_thrd = NULL;

static PIXEL_COLOR_T pixel_color[COLOR_ARR_NUM] = {
    {
        .red = EXAMPLE_LEDS_PIXEL_RESOLUTION,
        .green = 0,
        .blue = 0,
        .cold = 0,
        .warm = 0
    },
    {
        .red = 0,
        .green = EXAMPLE_LEDS_PIXEL_RESOLUTION,
        .blue = 0,
        .cold = 0,
        .warm = 0
    },
    {
        .red = 0,
        .green = 0,
        .blue = EXAMPLE_LEDS_PIXEL_RESOLUTION,
        .cold = 0,
        .warm = 0
    },

};

static void __leds_pixel_task(void *arg)
{
    uint8_t color_index = 0;

    for (;;) {
        tdl_pixel_set_single_color(pixel_hdl, 0, EXAMPLE_LEDS_PIXEL_NUM, &pixel_color[color_index]);
        // 刷新后上面设置的颜色才会生效
        tdl_pixel_dev_refresh(pixel_hdl);
        color_index = (color_index + 1) % COLOR_ARR_NUM;
        tal_system_sleep(1000);
    }

    return;
}

void example_leds_pixel(void)
{
    OPERATE_RET rt = OPRT_OK;

    PIXEL_DRIVER_CONFIG_T dev_init_cfg = {
        .port = EXAMPLE_LEDS_PIXEL_PORT,
        .line_seq = EXAMPLE_LEDS_PIXEL_LINE_SEQ,
    };
    TUYA_CALL_ERR_GOTO(tdd_ws2812_driver_register(EXAMPLE_LEDS_PIXEL_NAME, &dev_init_cfg), __EXIT);

    TUYA_CALL_ERR_GOTO(tdl_pixel_dev_find(EXAMPLE_LEDS_PIXEL_NAME, &pixel_hdl), __EXIT);

    PIXEL_DEV_CONFIG_T pixel_cfg = {
        .pixel_num = EXAMPLE_LEDS_PIXEL_NUM,
        .pixel_resolution = EXAMPLE_LEDS_PIXEL_RESOLUTION,
    };
    TUYA_CALL_ERR_GOTO(tdl_pixel_dev_open(pixel_hdl, &pixel_cfg), __EXIT);

    THREAD_CFG_T thrd_param = {
        .thrdname = "leds_pixel",
        .priority = THREAD_PRIO_2,
        .stackDepth = 1024
    };
    TUYA_CALL_ERR_GOTO(tal_thread_create_and_start(&pixel_thrd, NULL, NULL, __leds_pixel_task, NULL, &thrd_param), __EXIT);

__EXIT:
    return;
}
```

# 相关链接

- [Tuya IoT 文档中心](https://developer.tuya.com/cn/docs/iot-device-dev/Networking-Product-Software-Development-Kit?id=Kbfjujhienrcy) 

- [TuyaOS 论坛](https://www.tuyaos.com/viewforum.php?f=11) 