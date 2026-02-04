# tdl_dimmer_manage

## 修改记录

| 版本 | 时间 | 修改内容 |
| :--- | :--- | :--- |
| v0.1 | 2022/07/11 | 初版 |

## 名词解释

| 名词 | 解释 |
| :--- | :--- |
| CW | 涂鸦定义的一种白光 PWM 驱动方案。一路 CW 接冷白光控制电路，一路 WW 接暖白光控制电路；无论调亮度还是调色温两路都变化，调色温时两路信号互补，总功率不变。 |
| CCT | 涂鸦定义的一种白光 PWM 驱动方案。一路 Bright 控制总电流大小 (亮度)，一路 CCT 分配冷暖电流比 (色温)。 |

## 基本功能

`tdl_dimmer_manage` 组件主要用于调光器驱动管理，其基本功能如下：

- 对应用提供统一的数据类型 (通道数、输出数据)；
- 对应用提供统一的操作接口 (查找、启动、停止、输出)；
- 对驱动提供统一的注册接口和驱动接口抽象 (启动、停止、输出)；
- 对驱动提供统一的数据预处理和其他通用功能处理方案；

## 组件依赖

| 组件 | 说明 |
| :--- | :--- |
| tkl_mutex | 锁管理 |
| tkl_semaphore | 信号管理 |
| tkl_memory | 内存管理 |
| tkl_gpio | GPIO 驱动 |
| uni_log | 日志管理 |

## 资源依赖

| 资源 | 依赖情况 |
| :--- | :--- |
| 外设 | 一个调光器最多占用 1 个 GPIO 口；(控制引脚) |
| 内存 | 完成一个带控制引脚的 5 通道调光器注册并打开内存消耗约 12.5k。 |

## 目录结构

```xml
├── include
|    ├── tdl_dimmer_driver.h         /* 面向驱动接口 */
|    ├── tdl_dimmer_manage.h         /* 面向应用接口 */
|    └── tdl_dimmer_type.h           /* 通用数据类型 */
└── src
     └── tdl_dimmer_manage.c         /* 调光器驱动管理程序 */
```

## 接口列表

| 接口名称 | 接口功能 | 接口类型 |
| :--- | :--- | :--- |
| tdl_dimmer_register | 注册调光设备 | 面向驱动 |
| tdl_dimmer_find     | 查找调光设备 | 面向应用 |
| tdl_dimmer_open     | 启动调光设备 | 面向应用 |
| tdl_dimmer_close    | 停止调光设备 | 面向应用 |
| tdl_dimmmer_output  | 调光设备输出 | 面向应用 |

## 使用说明

需配合 `tdd_dimmer_driver` 组件使用。

**基本步骤**

1. 调用 `tdd_dimmer_xxxx_register` 注册 xxxx 设备；
2. 调用 `tdl_dimmer_find` 查找 xxxx 设备（确认 xxxx 设备注册成功）；
3. 调用 `tdl_dimmer_open` 启动 xxxx 设备并注册开关状态获取回调函数（准备资源）；
4. 调用 `tdl_dimmmer_output` 将设定的调光数据下发到设备驱动层输出；
5. 调用 `tdl_dimmer_close` 停止 xxxx 设备（释放资源）。

**应用示例**

这里以驱动 BP5758D IC 为例，实现每隔 1 秒点亮 1 个通道的功能。

```c
#include "app_dimmer.h"
#include "tdl_dimmer_manage.h"
#include "tdd_dimmer_bp5758d.h"
#include "tal_system.h"
#include "uni_log.h"

/**
 * @brief 调光驱动相关变量
 */
STATIC DIMMER_HANDLE sg_dimmer_handle;      /* 设备句柄 */
STATIC LIGHT_RGBCW_T sg_output_data = {0};  /* 输出数据 */
STATIC BOOL_T sg_switch_state = FALSE;      /* 开关状态 */

/**
 * @brief 调光驱动初始化
 * @param 无
 * @return 无
 */
STATIC VOID_T __dimmer_init(VOID_T)
{
    OPERATE_RET op_ret = OPRT_OK;

    /* BP5758D驱动配置 */
    DIMMER_BP5758D_CFG_T dimmer_cfg;
    /* 选择要使用的通道数 */
    dimmer_cfg.ch_num = DIMMER_CH_RGBCW;    /* 5路 */
    /* 选择各通道对应的输出通道，不使用的通道可省略配置 */
    dimmer_cfg.out_ch.r = DIMMER_CH_NUM_3;  /* R-OUT3 */
    dimmer_cfg.out_ch.g = DIMMER_CH_NUM_2;  /* G-OUT2 */
    dimmer_cfg.out_ch.b = DIMMER_CH_NUM_1;  /* B-OUT1 */
    dimmer_cfg.out_ch.c = DIMMER_CH_NUM_5;  /* C-OUT5 */
    dimmer_cfg.out_ch.w = DIMMER_CH_NUM_4;  /* W-OUT4 */
    /* 设置允许输出的最大电流，单位mA */
    dimmer_cfg.cur_tp = DIMMER_CUR_TP_MA;
    dimmer_cfg.max_cur.rgb = 40;
    dimmer_cfg.max_cur.cw = 30;
    /* 设置I2C通信引脚 */
    dimmer_cfg.i2c_pin.scl = GPIO_NUM_8;
    dimmer_cfg.i2c_pin.sda = GPIO_NUM_6;

    /* 控制引脚配置 */
    DIMMER_CTRL_PIN_T ctrl_pin;
    ctrl_pin_cfg.num = GPIO_NUM_16;
    ctrl_pin_cfg.active_lv = TUYA_GPIO_LEVEL_HIGH;

    /* 调用驱动层的接口注册设备，控制引脚不使用时传入NULL */
    op_ret = tdd_dimmer_bp5758d_register("BP5758D", dimmer_cfg, &ctrl_pin);
    if (OPRT_OK != op_ret) {
        PR_ERR("tdd_dimmer_i2c_register failed, error code: %d", op_ret);
        return;
    }
    /* 调用查找接口确认设备注册成功 */
    op_ret = tdl_dimmer_find("BP5758D", &sg_dimmer_handle);
    if (OPRT_OK != op_ret) {
        PR_ERR("tdl_dimmer_find failed, error code: %d", op_ret);
        return;
    }

    PR_NOTICE("The dimmer initialized successfully.");
}

/**
 * @brief 开关状态获取回调函数
 * @param 无
 * @return 开关状态
 */
STATIC BOOL_T __dimmer_get_switch_cb(VOID_T)
{
    return sg_switch_state;
}

/**
 * @brief 启动调光器
 * @param 无
 * @return 无
 */
STATIC VOID_T __dimmer_open(VOID_T)
{
    OPERATE_RET op_ret = OPRT_OK;

    /* 设置开光状态为打开 */
    sg_switch_state = TRUE;

    /* 调用启动接口启动设备并设置应用可写入的输出值上限、注册回调函数 */
    op_ret = tdl_dimmer_open(sg_dimmer_handle, 1000, __dimmer_get_switch_cb);
    if (OPRT_OK != op_ret) {
        PR_ERR("tdl_dimmer_open failed, error code: %d", op_ret);
        return;
    }

    PR_NOTICE("The dimmer opened.");
}

/**
 * @brief 输出数据
 * @param[in] r: R通道值
 * @param[in] g: G通道值
 * @param[in] b: B通道值
 * @param[in] c: C通道值
 * @param[in] w: W通道值
 * @return 无
 */
STATIC VOID_T __dimmer_output_data(USHORT_T r, USHORT_T g, USHORT_T b, USHORT_T c, USHORT_T w)
{
    OPERATE_RET op_ret = OPRT_OK;

    /* 设置各通道数据(0~1000)，不使用的通道可省略设置 */
    sg_output_data.red = r;
    sg_output_data.green = g;
    sg_output_data.blue = b;
    sg_output_data.cold = c;
    sg_output_data.warm = w;

    /* 调用输出接口将数据发送给调光IC */
    op_ret = tdl_dimmer_output(sg_dimmer_handle, sg_output_data);
    if (OPRT_OK != op_ret) {
        PR_ERR("tdl_dimmer_output failed, error code: %d", op_ret);
        return;
    }
}

/**
 * @brief 调光器输出
 * @param[in] step: 步骤
 * @return 无
 */
STATIC VOID_T __dimmer_output(UCHAR_T step)
{
    switch (step) {
    case 0:
        __dimmer_output_data(1000, 0, 0, 0, 0);
        break;
    case 1:
        __dimmer_output_data(0, 1000, 0, 0, 0);
        break;
    case 2:
        __dimmer_output_data(0, 0, 1000, 0, 0);
        break;
    case 3:
        __dimmer_output_data(0, 0, 0, 1000, 0);
        break;
    case 4:
        __dimmer_output_data(0, 0, 0, 0, 1000);
        break;
    default:
        break;
    }
}

/**
 * @brief 停止调光器
 * @param 无
 * @return 无
 */
STATIC VOID_T __dimmer_close(VOID_T)
{
    OPERATE_RET op_ret = OPRT_OK;

    /* 调用停止接口停止设备 */
    op_ret = tdl_dimmer_close(sg_dimmer_handle);
    if (OPRT_OK != op_ret) {
        PR_ERR("tdl_dimmer_open failed, error code: %d", op_ret);
        return;
    }

    PR_NOTICE("The dimmer closed.");
}

/**
 * @brief 调光驱动应用示例
 * @param 无
 * @return 无
 */
VOID_T app_dimmer_demo(VOID_T)
{
    __dimmer_init();
    __dimmer_open();
    tal_system_sleep(1000);
    for (UCHAR_T i = 0; i < 5; i++) {
        __dimmer_output(i);
        tal_system_sleep(1000);
    }
    __dimmer_close();
}
```

## 常见问题

暂无。

## 相关链接

- [品类方案 - 照明](https://developer.tuya.com/cn/docs/iot/lighting?id=Kaiuzc60687wl)
- [照明业务 - 照明驱动方案](https://developer.tuya.com/cn/docs/iot-device-dev/Lighting-drive-scheme?id=Kb71gj7i2nwvk)
- [TuyaOS](https://developer.tuya.com/cn/docs/iot-device-dev/TuyaOS-Overview?id=Kbfjtwjcpn1gc)
- [TuyaOS - Wi-Fi设备接入](https://developer.tuya.com/cn/docs/iot-device-dev/wifi-sdk-overview?id=Kaiuyfbaezlzt)
- [Wi-Fi 模组 SDK 二次开发教程](https://developer.tuya.com/cn/docs/iot/Module-SDK-development_tutorial?id=Kauqptzv5yo8a)

