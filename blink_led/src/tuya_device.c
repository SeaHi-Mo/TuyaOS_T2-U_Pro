#include "tuya_iot_config.h"
#include "tuya_cloud_types.h"
#include "tuya_cloud_com_defs.h"
#include "tal_thread.h"
#include "tal_log.h"
#include "tal_system.h"
#include "tkl_gpio.h"
#include "tuya_iot_wifi_api.h" // wifi相关接口

#define SSID "*****" // wifi名称
#define PASSWORD "12345678" // wifi密码

BOOL_T user_wifi_status = FALSE; // wifi状态
STATIC VOID_T user_wifi_event_cb(WF_EVENT_E event, VOID_T *arg)
{
    switch (event)
    {
    case WFE_CONNECTED:
        TAL_PR_INFO("wifi connected");
        user_wifi_status = TRUE;
        break;
    case WFE_CONNECT_FAILED:
        TAL_PR_INFO("wifi connect failed");
        user_wifi_status = FALSE;
        break;
    case WFE_DISCONNECTED:
        TAL_PR_INFO("wifi disconnected");
        user_wifi_status = FALSE;
        break;
    default:
        break;
    }
}
STATIC VOID user_main(VOID_T)
{
    tuya_base_utilities_init();                   // 初始化基础组件
    tal_log_set_manage_attr(TAL_LOG_LEVEL_DEBUG); // 设置日志级别
    // 配置LED引脚
    TUYA_GPIO_BASE_CFG_T led_cfg = {0};
    led_cfg.mode = TUYA_GPIO_PUSH_PULL;        // 推挽输出
    led_cfg.level = TUYA_GPIO_LEVEL_LOW;       // 默认低电平
    led_cfg.direct = TUYA_GPIO_OUTPUT;         // 输出方向
    tkl_gpio_init(TUYA_GPIO_NUM_26, &led_cfg); // 初始化GPIO26

#if defined(ENABLE_LWIP) && (ENABLE_LWIP == 1)
    TUYA_LwIP_Init();
#endif

    tkl_wifi_init(user_wifi_event_cb);         // 初始化wifi
    tkl_wifi_set_work_mode(WWM_STATION);       // 设置wifi工作模式为STA
    tkl_wifi_set_country_code(COUNTRY_CODE_CN);
    tkl_wifi_station_connect(SSID, PASSWORD); // 连接wifi

    while (1)
    {
        if (user_wifi_status == 1){
            tal_system_sleep(2000);
             tkl_gpio_write(TUYA_GPIO_NUM_26, TUYA_GPIO_LEVEL_LOW);
            tal_system_sleep(80 );
            tkl_gpio_write(TUYA_GPIO_NUM_26, TUYA_GPIO_LEVEL_HIGH);
            tal_system_sleep(80);
        } 
        tkl_gpio_write(TUYA_GPIO_NUM_26, TUYA_GPIO_LEVEL_LOW);
        tal_system_sleep(user_wifi_status == 1 ? 80 : 200);
        tkl_gpio_write(TUYA_GPIO_NUM_26, TUYA_GPIO_LEVEL_HIGH);
        tal_system_sleep(user_wifi_status == 1 ? 80 : 200);
    }
    return;
}

THREAD_HANDLE ty_app_thread = NULL;
STATIC VOID tuya_app_thread(VOID_T *arg)
{
    user_main();

    tal_thread_delete(ty_app_thread);
    ty_app_thread = NULL;
}

/**
 * @brief user entry function
 *
 * @param[in] none
 *
 * @return none
 */
#if OPERATING_SYSTEM == SYSTEM_LINUX
INT_T main(INT_T argc, CHAR_T **argv)
#else
VOID_T tuya_app_main(VOID)
#endif
{
    THREAD_CFG_T thrd_param = {4096, 4, "tuya_app_main"};
    tal_thread_create_and_start(&ty_app_thread, NULL, NULL, tuya_app_thread, NULL, &thrd_param);
#if OPERATING_SYSTEM == SYSTEM_LINUX
    while (1)
    {
        tal_system_sleep(1000);
    }
#endif
}
