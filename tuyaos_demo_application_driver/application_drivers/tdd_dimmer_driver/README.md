# tdd_dimmer_driver

## 修改记录

| 版本   | 时间         | 修改内容                            |
|:---- |:---------- |:------------------------------- |
| v0.2 | 2022/10/21 | 添加 `tdd_dimmer_weak.c` 文件，支持跨平台 |
| v0.1 | 2022/08/26 | 初版                              |

## 名词解释

| 名词  | 解释                                                                                 |
|:--- |:---------------------------------------------------------------------------------- |
| CW  | 涂鸦定义的一种白光 PWM 驱动方案。一路 CW 接冷白光控制电路，一路 WW 接暖白光控制电路；无论调亮度还是调色温两路都变化，调色温时两路信号互补，总功率不变。 |
| CCT | 涂鸦定义的一种白光 PWM 驱动方案。一路 Bright 控制总电流大小 (亮度)，一路 CCT 分配冷暖电流比 (色温)。                     |

## 基本功能

`tdd_dimmer_driver` 组件用于调光器驱动，其基本功能如下：

- 对应用提供统一命名规则的注册接口，具体的注册信息由驱动实例决定；
- 包含使用 I2C 协议进行通信的不同型号调光 IC 的驱动程序；
- 包含使用 PWM 驱动方案的驱动程序；

## 组件依赖

| 组件                | 说明      |
|:----------------- |:------- |
| tdl_dimmer_manage | 调光驱动管理  |
| tkl_memory        | 内存管理    |
| tkl_pwm           | PWM 驱动  |
| tkl_gpio          | GPIO 驱动 |
| uni_log           | 日志管理    |

## 资源依赖

| 资源  | 依赖情况                                                                      |
|:--- |:------------------------------------------------------------------------- |
| 外设  | >> I2C 类：一个调光器最多占用 2 个 GPIO 口；(模拟 I2C)<br/>>> PWM 类：一个调光器最多占用 5 个 PWM 通道。 |
| 内存  | 完成一个带控制引脚的 5 通道调光器注册并打开内存消耗约 12.5k。                                       |

## 目录结构

```xml
├── include
|    ├── tdd_dimmer_bp1658cj.h     /* BP1658CJ驱动接口 */
|    ├── tdd_dimmer_bp5758d.h      /* BP5758D驱动接口 */
|    ├── tdd_dimmer_common.h       /* 调光器驱动通用类型和接口 */
|    ├── tdd_dimmer_kp18058.h      /* KP18058ESP系列驱动接口 */
|    ├── tdd_dimmer_pwm.h          /* PWM类驱动接口 */
|    ├── tdd_dimmer_sm2135e.h      /* SM2135E驱动接口 */
|    ├── tdd_dimmer_sm2135eh.h     /* SM2135EH系列驱动接口 */
|    └── tdd_dimmer_sm2235egh.h    /* SM2235EGH系列驱动接口 */
└── src
     ├── tdd_dimmer_bp1658cj.c     /* BP1658CJ驱动处理 */
     ├── tdd_dimmer_bp5758d.c      /* BP5758D驱动处理 */
     ├── tdd_dimmer_common.c       /* 调光器驱动通用处理 */
|    ├── tdd_dimmer_kp18058.c      /* KP18058ESP系列驱动处理 */
     ├── tdd_dimmer_pwm.c          /* PWM类驱动处理 */
     ├── tdd_dimmer_sm2135e.c      /* SM2135E驱动处理 */
     ├── tdd_dimmer_sm2135eh.c     /* SM2135EH系列驱动处理 */
     └── tdd_dimmer_sm2235egh.c    /* SM2235EGH系列驱动处理 */
```

## 接口列表

| 接口名称                     | 接口功能          | 接口类型 |
|:------------------------ |:------------- |:---- |
| tdd_dimmer_xxxx_register | 注册 xxxx 类型调光器 | 面向应用 |

## 适配情况

已适配的调光 IC 如下：

| IC 型号        | 供应商  | 驱动方式 | 通道数 | 灰度等级 |
|:------------:|:----:|:----:|:---:|:----:|
| *BP1658CJ*   | 晶丰明源 | I2C  | 5   | 1024 |
| *BP5758D*    | 晶丰明源 | I2C  | 5   | 1024 |
| *SM2135E*    | 明微电子 | I2C  | 5   | 256  |
| *SM2135EH*   | 明微电子 | I2C  | 5   | 256  |
| *SM2135EJ*   | 明微电子 | I2C  | 5   | 256  |
| *SM2235EGH*  | 明微电子 | I2C  | 5   | 1024 |
| *SM2335EGH*  | 明微电子 | I2C  | 5   | 1024 |
| *KP18058ESP* | 必易微  | I2C  | 5   | 1024 |
| *KP18059ESP* | 必易微  | I2C  | 5   | 1024 |

## 使用说明

需配合 `tdl_dimmer_manage` 组件使用。

**基本步骤**

1. 调用 `tdd_dimmer_xxxx_register` 注册 xxxx 设备；
2. 调用 `tdl_dimmer_find` 查找 xxxx 设备（确认 xxxx 设备注册成功）；
3. 调用 `tdl_dimmer_open` 启动 xxxx 设备并注册开关状态获取回调函数（准备资源）；
4. 调用 `tdl_dimmmer_output` 将设定的调光数据下发到设备驱动层输出；
5. 调用 `tdl_dimmer_close` 停止 xxxx 设备（释放资源）。

**应用示例**

这里以实现每隔 1 秒点亮 1 个通道为例，介绍几种不同驱动的使用方法。

- 示例 1：I2C 类，OUT1~OUT5 可作为 RGBCW 中的任意通道使用。

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
VOID_T app_dimmer_init(VOID_T)
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
VOID_T app_dimmer_open(VOID_T)
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
VOID_T app_dimmer_output(UCHAR_T step)
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
VOID_T app_dimmer_close(VOID_T)
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
    app_dimmer_init();
    app_dimmer_open();
    tal_system_sleep(100);
    for (UCHAR_T i = 0; i < 5; i++) {
        app_dimmer_output(i);
        tal_system_sleep(1000);
    }
    app_dimmer_close();
}
```

- 示例 2：I2C 类，OUT1~OUT3 只可作为彩光 RGB 中的任意通道使用，OUT4~OUT5 只可作为白光 CW 中的任意通道使用。和示例 1 的主要区别是驱动配置的通道选择形式不同。

```c
/**
 * @brief 和上一种类型驱动主要的区别就是驱动配置的通道选择形式不同
 */
#include "tdd_dimmer_bp1658cj.h"

/**
 * @brief 调光驱动初始化
 * @param 无
 * @return 无
 */
VOID_T app_dimmer_init(VOID_T)
{
    /* ... */
    DIMMER_BP1658CJ_CFG_T dimmer_cfg;
    /* ... */
    dimmer_cfg.out_ch.rgb = DIMMER_RGB_CH_321;  /* R-OUT3, G-OUT2, B-OUT1 */
    dimmer_cfg.out_ch.cw = DIMMER_CW_CH_54;     /* C-OUT5, W-OUT4 */
    /* ... */
    op_ret = tdd_dimmer_bp1658cj_register("BP1658CJ", dimmer_cfg, ctrl_pin);
    /* ... */
}
```

- 示例 3：PWM 类，白光驱动方式采用 CCT 方案。和示例 1 的区别是驱动配置参数不同，以及 C/W 两路并非单独控制 LED-C/W 的亮度。接 Bright 的通道数据表示亮度调节；接 CCT 的通道数据表示色温调节，数值越大越偏向冷白光。

```c
#include "tdd_dimmer_pwm.h"

/**
 * @brief 调光驱动初始化
 * @param 无
 * @return 无
 */
VOID_T app_dimmer_init(VOID_T)
{
    OPERATE_RET op_ret = OPRT_OK;

    /* PWM驱动配置 */
    DIMMER_PWM_CFG_T dimmer_cfg;
    /* 选择白光驱动类型 */
    dimmer_cfg.cw_tp = DIMMER_CW_DRV_TP_CCT;/* CCT */
    /* 选择要使用的通道数 */
    dimmer_cfg.ch_num = DIMMER_CH_RGBCW;    /* 5路 */
    /* 选择各通道对应的PWM通道，不使用的通道可省略配置 */
    dimmer_cfg.pwm_ch.r_id = PWM_NUM_2;     /* R-PWM2 */
    dimmer_cfg.pwm_ch.g_id = PWM_NUM_1;     /* G-PWM1 */
    dimmer_cfg.pwm_ch.b_id = PWM_NUM_0;     /* B-PWM0 */
    dimmer_cfg.pwm_ch.c_id = PWM_NUM_5;     /* C-PWM5 */
    dimmer_cfg.pwm_ch.w_id = PWM_NUM_4;     /* W-PWM4 */
    /* 设置PWM相关参数 */
    dimmer_cfg.pwm_freq = 1000;             /* 输出频率：1kHz */
    dimmer_cfg.active_level = TRUE;         /* 输出极性：高电平有效 */

    /* 控制引脚配置 */
    DIMMER_CTRL_PIN_T ctrl_pin;
    ctrl_pin_cfg.num = GPIO_NUM_16;
    ctrl_pin_cfg.active_lv = TUYA_GPIO_LEVEL_HIGH;

    /* 调用驱动层的接口注册设备，控制引脚不使用时传入NULL */
    op_ret = tdd_dimmer_pwm_register("PWM_DIMMER", dimmer_cfg, &ctrl_pin);
    if (OPRT_OK != op_ret) {
        PR_ERR("tdd_dimmer_i2c_register failed, error code: %d", op_ret);
        return;
    }
    /* 调用查找接口确认设备注册成功 */
    op_ret = tdl_dimmer_find("PWM_DIMMER", &sg_dimmer_handle);
    if (OPRT_OK != op_ret) {
        PR_ERR("tdl_dimmer_find failed, error code: %d", op_ret);
        return;
    }

    PR_NOTICE("The dimmer initialized successfully.");
}

/**
 * @brief 调光器输出
 * @param[in] step: 步骤
 * @return 无
 */
VOID_T app_dimmer_output(UCHAR_T step)
{
    switch (step) {
    /* ... */
    case 3:
        __dimmer_output_data(0, 0, 0, 1000, 1000); /* 亮度最大，色温最高，即LED-C亮 */
        break;
    case 4:
        __dimmer_output_data(0, 0, 0, 1000, 0);    /* 亮度最大，色温最低，即LED-W亮 */
        break;
    default:
        break;
    }
}
```

- 示例 4：PWM 类，白光驱动方式采用 CW 方案。和示例 3 的区别是白光驱动类型的选择不同，以及 C/W 两路输出数据存在关联。接 CW 的通道数据表示冷白光比例；接 WW 的通道数据表示暖白光比例，两路数据之和表示亮度。以每一路灯光 PWM 最大占空比为 100% 为基准，互补输出两路灯光的总输出加起来占空比最大只能为 100%，即下文代码中的 `1000`；如果需要高于 100% 的功率输出，则需要配置 CW 两路为非互补输出，即配置 `cw_tp` 为 `DIMMER_CW_DRV_TP_CW_NC`。

```c
#include "tdd_dimmer_pwm.h"

/**
 * @brief 调光驱动初始化
 * @param 无
 * @return 无
 */
VOID_T app_dimmer_init(VOID_T)
{
    /* ... */
    DIMMER_PWM_CFG_T dimmer_cfg;
    /* 选择白光驱动类型 */
    dimmer_cfg.cw_tp = DIMMER_CW_DRV_TP_CW; /* CW-互补输出 */
    /* ... */
    op_ret = tdd_dimmer_pwm_register("PWM_DIMMER", dimmer_cfg, &ctrl_pin);
    /* ... */
}

/**
 * @brief 调光器输出
 * @param[in] step: 步骤
 * @return 无
 */
VOID_T app_dimmer_output(UCHAR_T step)
{
    switch (step) {
    /* ... */
    case 3:
        __dimmer_output_data(0, 0, 0, 1000, 0); /* 100%亮度，100%冷白，即LED-C亮 */
        break;
    case 4:
        __dimmer_output_data(0, 0, 0, 0, 1000); /* 100%亮度，100%暖白，即LED-W亮 */
        break;
    default:
        break;
    }
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
