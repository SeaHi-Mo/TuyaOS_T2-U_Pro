# tdl_energy_monitor 组件介绍

这两将使用串口的两款芯片 BL0942, HLW8032 记为串口型计量芯片，这两个芯片可以免校准；BL0937, HLW8012 是通过单位时间内脉冲计数从而转换成国际标准单位，这里称之为脉冲型计量芯片，这两款芯片需要校准才能得到较为准确的值。

## 一、修改记录

| 版本 | 时间       | 修改内容 |
| ---- | ---------- | -------- |
| 1.0.0 | 2024.02.01 | 首版   |


## 二、基本功能

1. 支持 BL0937, HLW8012, BL0942, HLW8032 计量芯片；
2. 支持获取实时电压、电流、功率采样，获取累计电量；
3. 在参数校准之后，参数的采样精度大约 3%；
4. 支持计量芯片产测、校准。

## 三、组件依赖
| 应用组件依赖          | 说明                                  |
| --------------------- | ------------------------------------- |
| tal_uart           | BL0942, HLW8032 两个串口型计量芯片使用 |
| tdd_energy_monitor |BL0937, HLW8012, BL0942, HLW8032 计量芯片驱动层 |

## 四、接口介绍

### 计量设备管理头文件

[tdl_energy_monitor_manage.h](./src/tdl_energy_monitor_manage.h)

#### 函数说明

> OPERATE_RET tdl_energy_monitor_find(CHAR_T *name, ENERGY_MONITOR_HANDLE_T *handle);

计量设备查找接口，传入设备名字 `name` ，如果查找发现了 `name` 设备，会通过 `handle` 将设备句柄返回，后续操作都将通过 `handle` 进行操作。

在调用该接口之前应先通过 `tdd_energy_monitor` 组件中提供的 `tdd_energy_driver_xxx_register() ` 硬件设备注册接口对计量设备硬件进行注册。

+ 返回值

  `OPERATE_RET` 的返回值类型可以在 `TuyaOS/include/base/include/tuya_error_code.h` 文件中进行查看，下面是对于该接口常见的返回错误值的介绍：

  + `OPRT_INVALID_PARM` : 传入参数 `name` 或 `handle` 为 `NULL`；
  + `OPRT_NOT_FOUND` : 没有发现设备 `name` ，可能没有注册或注册失败。

+ 入参

  + `name` : 要查找的设备名字；
  + `handle` : 查找到 `name` 后传出的设备句柄。



> OPERATE_RET tdl_energy_monitor_open(ENERGY_MONITOR_HANDLE_T handle);

打开电量统计设备，启动电量统计。

+ 返回值

  `OPERATE_RET` 的返回值类型可以在 `TuyaOS/include/base/include/tuya_error_code.h` 文件中进行查看，下面是对于该接口常见的返回错误值的介绍：

  + `OPRT_INVALID_PARM` : 传入参数  `handle` 为 `NULL` 或 tdd_energy_monitor  中 open 设备时返回的错误；
  + `OPRT_COM_ERROR` : tdd_energy_monitor 层未注册 open；

  提示：在 `tdl_energy_monitor_open()` 发生的错误更多的可能是在 `tdd_energy_monitor` 中出现的错误，可以在 `tdd_energy_monitor/src/tdd_energy_monitor_xxx.c` 中的 `__tdd_energy_monitor_open()` 中判断出现问题的原因。

+ 入参

  + `handle` : 设备句柄，由 `tdl_energy_monitor_find()` 得到。



> OPERATE_RET tdl_energy_monitor_read_vcp(ENERGY_MONITOR_HANDLE_T handle, ENERGY_MONITOR_VCP_T *vcp_data);

读取实时电压、电流、功率值。

+ 返回值

  `OPERATE_RET` 的返回值类型可以在 `TuyaOS/include/base/include/tuya_error_code.h` 文件中进行查看，下面是对于该接口常见的返回错误值的介绍：

  + `OPRT_INVALID_PARM` : 传入参数  `handle` 或 `vcp_data` 为 `NULL` 。

+ 入参

  + `handle` : 设备句柄，由 `tdl_energy_monitor_find()` 得到；

  + `vcp_data` : 通过该入参将实时电压、电流、功率值传出。

    `vcp_data` 的数据结构为：

    ```c
    typedef struct {
        UINT32_T voltage; // 实际电压扩大 10 倍。如：220.0v 应设置为 2200
        UINT32_T current; // 实际电流扩大 1000 倍。如：0.392A 应设置为 392
        UINT32_T power; // 实际实时功率扩大 10 倍。如：86.4w 应设置为 864
    } ENERGY_MONITOR_VCP_T;
    ```



> OPERATE_RET tdl_energy_monitor_read_energy(ENERGY_MONITOR_HANDLE_T handle, UINT32_T *energy_consumed);

读取累计电量值，读取后 `tdl_energy_monitor` 层的累计电量会被清零。

提示：在 0s 的时候 open 了计量设备，2s 的时候调用了 `tdl_energy_monitor_read_energy()` 函数，那么此时读取得到的是 0 - 2s 的累计电量值，同时会将所记录累计电量清零；之后在 10s 再次调用了 `tdl_energy_monitor_read_energy()` 函数，那么在 10s 时读取得到的是 2-10s 这段时间所累计电量值，并将累计电量清零。

+ 返回值

  `OPERATE_RET` 的返回值类型可以在 `TuyaOS/include/base/include/tuya_error_code.h` 文件中进行查看，下面是对于该接口常见的返回错误值的介绍：

  + `OPRT_INVALID_PARM` : 传入参数  `handle` 或 `energy_consumed` 为 `NULL` 。

+ 入参

  + `handle` : 设备句柄，由 `tdl_energy_monitor_find()` 得到；
  + `energy_consumed` : 累计电量，单位为 0.001 kWh。读取得到 1，表示 0.001 kWh；1000表示1度电。



> OPERATE_RET tdl_energy_monitor_config(ENERGY_MONITOR_HANDLE_T handle, TDL_ENERGY_MONITOR_CMD_E cmd, void *args);

计量配置函数，通过该函数进行计量功能进行一些特殊配置，支持扩展。目前该接口主要用来配合产测、计量校准和需校准的计量芯片(bl0937和hlw8012)初始化使用。

+ 返回值

  `OPERATE_RET` 的返回值类型可以在 `TuyaOS/include/base/include/tuya_error_code.h` 文件中进行查看，下面是对于该接口常见的返回错误值的介绍：

  + `OPRT_INVALID_PARM` : 传入参数  `handle` 或 `energy_consumed` 为 `NULL` 。

+ 入参

  + `handle` : 设备句柄，由 `tdl_energy_monitor_find()` 得到；

  + `cmd` : 配置命令，支持以下命令：

    + `TDL_EM_CMD_CAL_DATA_SET`：校准环境参数设置。传入产测时连接负载后的电压、电流、功率和硬件实际采样电阻的大小，使用该命令时应通过 `args` 参数传入数据结构为 `ENERGY_MONITOR_CAL_DATA_T` 的形参。`ENERGY_MONITOR_CAL_DATA_T` 的数据结构如下：

        ```c
        typedef struct {
            UINT32_T voltage; // 实际电压扩大 10 倍。如：220.0v 应设置为 2200
            UINT32_T current; // 实际电流扩大 1000 倍。如：0.392A 应设置为 392
            UINT32_T power; // 实际实时功率扩大 10 倍。如：86.4w 应设置为 864
            UINT32_T resval; // 电量采样电阻，单位 1mR
        }ENERGY_MONITOR_CAL_DATA_T;
        ```

    + `TDL_EM_CMD_CAL_PARAMS_SET`：校准参数设置。主要用于需要校准的计量芯片配置校准参数以获取更准确的测量值，使用该命令时应通过 `args` 参数传入数据结构为 `ENERGY_MONITOR_CAL_PARAMS_T` 的形参。`ENERGY_MONITOR_CAL_PARAMS_T` 的数据结构如下：

        ```c
        typedef struct {
            UINT32_T voltage_period;
            UINT32_T current_period;
            UINT32_T power_period;
            UINT32_T pf_num; // 0.001 度电需要的 pf 脉冲个数
        }ENERGY_MONITOR_CAL_PARAMS_T;
        ```
        
        上面两个命令 `TDL_EM_CMD_CAL_DATA_SET` 和 `TDL_EM_CMD_CAL_PARAMS_SET` ，这两个命令的具体用法会在下文中的正常使用示例中进行演示。
    
  + `args`：配置参数。配合 `cmd` 示例。



> UINT32_T tdl_energy_monitor_calibration(ENERGY_MONITOR_HANDLE_T handle, ENERGY_MONITOR_CAL_DATA_T cal_data, OUT ENERGY_MONITOR_CAL_PARAMS_T *cal_params);

计量校准函数。

+ 返回值

  校准得到的参数值与内置参数的最大误差值。脉冲型计量在 tdd 层驱动会内置一个参数，当调用计量校准函数后会将得到的校准值与内置参数进行比较，分别比较电压参数、电流参数、功率参数和电量参数，然后返回误差最大的参数百分比。

+ 入参

  + `handle` : 设备句柄；
  + `cal_data`：校准时的环境参数；
  + `cal_params`：校准得到的校准参数。

该函数的使用示例可以参考下文中的校准示例。

## 五、示例代码

### 5.1 产测示例

这里以 BL0937 这款计量芯片为例演示如何进行产测、获取校准系数。

```c
#include "tdd_energy_monitor_bl0937_hlw8012.h"
#include "tdl_energy_monitor_manage.h"

...

#define ENERGY_NAME "energy_monitor"

STATIC ENERGY_MONITOR_HANDLE_T sg_ele_energy_hdl  = NULL;
    
void example_energy_monitor_cal(void)
{
    OPERATE_RET rt = OPRT_OK;
    
    // 关闭所有继电器
    ...
    
    // 注册硬件
    BL0937_DRIVER_CONFIG_T bl0937_cfg = {
        .timer_id  = TUYA_TIMER_NUM_0,
        .sel_pin   = TUYA_GPIO_NUM_2,
        .sel_level = TUYA_GPIO_LEVEL_HIGH,
        .cf1_pin   = TUYA_GPIO_NUM_11,
        .cf_pin    = TUYA_GPIO_NUM_7,
    };
    rt = tdd_energy_driver_bl0937_register(ENERGY_NAME, bl0937_cfg)
    if (OPRT_OK != rt) {
        TAL_PR_ERR("bl0937 register error: %d", rt);
        return;
    }
    
    // 查找计量设备
    rt = tdl_energy_monitor_find(device_name, &sg_ele_energy_hdl);
    if (OPRT_OK != rt) {
        TAL_PR_ERR("bl0937 open error: %d", rt);
        return;
    }
    // 打开所有继电器
    ...

    // 开启计量校准
	ENERGY_MONITOR_CAL_DATA_T   cal_env;
    // 当计量插座上接上负载时，实际电压为 220.0v，实际电流为 0.392A，实际功率为 86.4w
    // 计量插座硬件电路实际采样电阻为 1mR
    cal_env.voltage = 2200; // 220.0v
    cal_env.current = 392;  // 0.392A
    cal_env.power   = 864;  // 86.4w
    cal_env.resval  = 1;
    ENERGY_MONITOR_CAL_PARAMS_T cal_params;
    UINT32_T err_percent = tdl_energy_monitor_calibration(sg_ele_energy_hdl, cal_env, &cal_params);
    // 这里会将计量得到的实际脉冲值与内置
    if (err_percent >= 30) {
        TAL_PR_ERR("bl0937 calibration fail: %d", err_percent);
    }
    
    // 这里应将校准环境和校准成功的参数存放到 flash 中
    // cal_env->flash
    // cal_params->flash
    ...

    return;
}
```

### 5.2 使用示例

这里以 BL0937 为例演示如何在校准后正常使用计量功能。这里默认设备已经经过校准，并将校准数据存放到了 flash 中。

```c
#include "tdd_energy_monitor_bl0937_hlw8012.h"
#include "tdl_energy_monitor_manage.h"

...

#define ENERGY_NAME "energy_monitor"

STATIC ENERGY_MONITOR_HANDLE_T sg_ele_energy_hdl  = NULL;
    
void example_energy_monitor_app(void)
{
    OPERATE_RET rt = OPRT_OK;
    
    // 注册硬件
    BL0937_DRIVER_CONFIG_T bl0937_cfg = {
        .timer_id  = TUYA_TIMER_NUM_0,
        .sel_pin   = TUYA_GPIO_NUM_2,
        .sel_level = TUYA_GPIO_LEVEL_HIGH,
        .cf1_pin   = TUYA_GPIO_NUM_11,
        .cf_pin    = TUYA_GPIO_NUM_7,
    };
    rt = tdd_energy_driver_bl0937_register(ENERGY_NAME, bl0937_cfg)
    if (OPRT_OK != rt) {
        TAL_PR_ERR("bl0937 register error: %d", rt);
        return;
    }
    
    // 查找计量设备
    rt = tdl_energy_monitor_find(ENERGY_NAME, &sg_ele_energy_hdl);
    if (OPRT_OK != rt) {
        TAL_PR_ERR("bl0937 open error: %d", rt);
        return;
    }
    
    // 从 flash 中读取校准参数
    ENERGY_MONITOR_CAL_DATA_T   cal_env;
    ENERGY_MONITOR_CAL_PARAMS_T cal_params;
    // cal_env -> flash
    // cal_params -> flash
    ...

    // 传入产测参数
    // 这里如果不设置会使用内置参数，得到的测量结果会不准确，误差较大。免校准的计量芯片可以不进行设置。
	tdl_energy_monitor_config(sg_ele_energy_hdl, TDL_EM_CMD_CAL_DATA_SET,   &cal_env);
    tdl_energy_monitor_config(sg_ele_energy_hdl, TDL_EM_CMD_CAL_PARAMS_SET, &cal_factor);
    
    // 开启计量设备
    tdl_energy_monitor_open(sg_ele_energy_hdl);
    
    // 没 3s 读取一次计量数据
    ENERGY_MONITOR_VCP_T vcp = {0,0,0};
    UINT_T energy = 0;
    UINT_T _cnt = 0;
    while (1) {
        tdl_energy_monitor_read_vcp(sg_ele_energy_hdl, &vcp);
        /* pwr: 0.1W, volt: 0.1V, curr: 0.01A */
        TAL_PR_DEBUG("curr pvie data, P: %d, V: %d, I: %d", vcp.power, vcp.voltage, vcp.current);

        _cnt++;
        if (_cnt++ > 20*3) { // 3 minutes
            _cnt = 0;
            /* get electric quantity*/
            tdl_energy_monitor_read_energy(sg_ele_energy_hdl, &energy);
            TAL_PR_DEBUG("E:%d", energy);
        }

        tal_system_sleep(3000);
    }

    return;
}
```

