# 器件驱动包

## 驱动列表

| 器件驱动名称 | 依赖组件                                                                                      | 备注                                    |
|:------:|:-----------------------------------------------------------------------------------------:|:-------------------------------------:|
| 按键     | `tdl_button_manage`<br/>`tdd_button_driver`                                               |                                       |
| LED    | `tdl_led`                                                                                 |                                       |
| 幻彩     | `tdl_leds_pixel_manage`<br/>`tdd_leds_pixel`                                              |                                       |
| 全彩     | `tdl_dimmer_manage`<br/>`tdd_dimmer_driver`                                               |                                       |
| 红外     | `tdl_ir_device`<br/>`tdd_ir_driver`                                                       |                                       |
| 传感器    | `tdl_sensor_hub`<br/>`tdd_sensor_imu`<br/>`tdd_sensor_temp_humi`<br/>`tdd_sensor_voltage` |  |
| 声音     | `tdl_sound_sample`<br/>`tdd_sound_driver`                                                 |                    |
| 电量统计     | `tdl_ele_energy`<br/>`tdd_ele_energy`                                                 |  |

## 目录结构

```bash
.
├── application_drivers     //器件驱动源码
│   ├── tal_uart
│   ├── tdd_button_driver
│   ├── tdd_dimmer_driver
│   ├── tdd_energy_monitor
│   ├── tdd_ir_driver
│   ├── tdd_leds_pixel
│   ├── tdd_sensor_imu
│   ├── tdd_sensor_temp_humi
│   ├── tdd_sensor_voltage
│   ├── tdd_sound_driver
│   ├── tdl_button_manage
│   ├── tdl_dimmer_manage
│   ├── tdl_energy_monitor
│   ├── tdl_ir_device
│   ├── tdl_led
│   ├── tdl_leds_pixel_manage
│   ├── tdl_sensor_hub
│   ├── tdl_sound_sample
│   └── tdu_light_types
├── build                
├── include              //头文件
├── local.mk            
├── README.md        
└── src                 //入口源文件
    ├── examples        //器件驱动示例
    │   ├── example_drv_button.c
    │   ├── example_drv_dimmer.c
    │   ├── example_drv_ele_energy.c
    │   ├── example_drv_ir.c
    │   ├── example_drv_led.c
    │   ├── example_drv_pixels.c
    │   ├── example_drv_sensor.c
    │   └── example_drv_sound.c
    └── tuya_device.c   //应用入口

```

## 运行示例方法

1.进入 `tuya_device.c` 中通过修改 `TY_EXAMPLE_DRIVER` 来指定运行的目标驱动，目前默认是 `LED`。

```
/***********************************************************
************************macro define************************
***********************************************************/
#define TY_EXAMPLE_BUTTON           "ty_button"
#define TY_EXAMPLE_LED              "ty_led"
#define TY_EXAMPLE_IR               "ty_ir"
#define TY_EXAMPLE_SOUND            "ty_sound"
#define TY_EXAMPLE_SENSOR           "ty_sensor"
#define TY_EXAMPLE_DIMMER           "ty_dimmer"
#define TY_EXAMPLE_PIXELS           "ty_pixels"
#define TY_EXAMPLE_ELE_ENERGY       "ty_ele_energy"

/*select target example driver*/
#define TY_EXAMPLE_DRIVER           TY_EXAMPLE_LED
```

2.进入目标驱动示例文件 `src->example->example_drv_xxx.c`, 根据自己的硬件接线，修改硬件引脚等配置。

3.编译烧录。

## 驱动调用三部曲

1. 注册硬件驱动， reg_drv_hardware。
2. 打开驱动设备，open_driver。
3. 运行驱动，driver_running。

## 支持与帮助

在开发过程遇到问题，您可以登录 TuyaOS 开发者论坛 [联网单品开发版块](https://www.tuyaos.com/viewforum.php?f=11) 进行沟通咨询。

## 更多资料

[**TuyaOS 开发者文档中心**](https://developer.tuya.com/cn/docs/iot-device-dev/TuyaOS-course?id=Kbxa3zfh6ovn8)