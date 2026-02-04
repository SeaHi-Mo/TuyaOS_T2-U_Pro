# 修改记录

| 版本 | 时间       | 修改内容 |
| ---- | ---------- | -------- |
| v2.1 | 2022.11.04 | 替换老的接口为 tal/tkl  |
| v2.0 | 2022.04.15 | 初版     |

# 基本功能

1、提供音频设备查找的能力

2、提供采集音频数据的能力（以原始值或者百分比形式采集）

3、提供开启/关闭采集设备的能力

# 资源依赖

| 资源                     | 大小      | 说明 |
| ------------------------ | --------- | ---- |
| 开启音量采集设备占用内存 | 104 bytes |      |

# 接口列表说明

- 音量采集操作接口，详细使用说明参考`tdl_sound_sample.h`

| 接口                                                         | 说明         |
| ------------------------------------------------------------ | ------------ |
| int tdl_sound_dev_find(char *dev_name, SOUND_HANDLE_T *handle); | 查找音频设备 |
| int  tdl_sound_dev_open(SOUND_HANDLE_T handle, S_SAMPLE_VAL_E val_tp, unsigned char val_num); | 打开设备     |
| int tdl_sound_dev_sample(SOUND_HANDLE_T handle, S_APP_DATA_E data_tp, unsigned int num, unsigned int *data); | 音量采集     |
| int  tdl_sound_dev_close(SOUND_HANDLE_T *handle);            | 关闭设备     |

# 使用说明

```C

#include "tuya_app_config.h"

#include "tuya_cloud_types.h"
#include "tal_log.h"
#include "tal_thread.h"
#include "tal_system.h"

#include "tdl_sound_sample.h"
#include "tdd_sound_adc.h"

// MIC 硬件配置
#define EXAMPLE_SOUND_NAME      "mic_dev"
#define EXAMPLES_SOUND_ADC_NUM  0
#define EXAMPLES_SOUND_ADC_PIN  23
#define EXAMPLES_SOUND_ADC_MAX  4096

static SOUND_HANDLE_T sound_handle = NULL;

static THREAD_HANDLE sound_thread_hdl = NULL;

static  VOID __sound_task(PVOID_T args)
{
    unsigned int sound_value = 0;

    for (;;) {
        tdl_sound_dev_sample(sound_handle, S_DATA_PERCENT, 1, &sound_value);
        TAL_PR_NOTICE("read sound value is %d", sound_value);
        tal_system_sleep(1000);
    }

    return;
}

void example_sound(void)
{
    OPERATE_RET rt = OPRT_OK;

    ADC_DRIVER_CONFIG_T sound_hw_cfg = {
        .pin = EXAMPLES_SOUND_ADC_PIN,
        .adc_num = EXAMPLES_SOUND_ADC_NUM,
        .adc_max = EXAMPLES_SOUND_ADC_MAX
    };
    TUYA_CALL_ERR_GOTO(tdd_sound_adc_register(EXAMPLE_SOUND_NAME, &sound_hw_cfg), __EXIT);

    TUYA_CALL_ERR_GOTO(tdl_sound_dev_find(EXAMPLE_SOUND_NAME, &sound_handle), __EXIT);

    TUYA_CALL_ERR_GOTO(tdl_sound_dev_open(sound_handle, S_VAL_MAX, 5), __EXIT);

    THREAD_CFG_T thrd_param = {
        .thrdname = "sound_sample",
        .priority = THREAD_PRIO_2,
        .stackDepth = 1024
    };
    TUYA_CALL_ERR_GOTO(tal_thread_create_and_start(&sound_thread_hdl, NULL, NULL, __sound_task, NULL, &thrd_param), __EXIT);

__EXIT:
    return;
}

```
# 相关链接

- [Tuya IoT 文档中心](https://developer.tuya.com/cn/docs/iot-device-dev/Networking-Product-Software-Development-Kit?id=Kbfjujhienrcy) 

- [TuyaOS 论坛](https://www.tuyaos.com/viewforum.php?f=11) 
