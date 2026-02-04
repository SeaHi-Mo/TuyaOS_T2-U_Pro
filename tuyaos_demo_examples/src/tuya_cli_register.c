/**
 * @file tuya_cli_register.c
 * @author www.tuya.com
 * @brief tuya_cli_register module is used to 
 * @version 0.1
 * @date 2022-09-21
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#include "tuya_cloud_types.h"
#include "tuya_cli_register.h"
#include "tkl_system.h"
/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
********************function declaration********************
***********************************************************/
extern VOID example_adc(INT_T argc, CHAR_T *argv[]);
extern VOID example_pwm(INT_T argc, CHAR_T *argv[]);
extern VOID example_gpio(INT_T argc, CHAR_T *argv[]);
extern VOID example_spi(INT_T argc, CHAR_T *argv[]);
extern VOID example_timer(INT_T argc, CHAR_T *argv[]);
extern VOID example_i2c(INT_T argc, CHAR_T *argv[]);
extern VOID example_sw_i2c(INT_T argc, CHAR_T *argv[]);
extern VOID example_speaker_play(INT_T argc, CHAR_T *argv[]);
extern VOID example_recorder_init(INT_T argc, CHAR_T *argv[]);

extern VOID example_dvp(INT_T argc, CHAR_T *argv[]);
extern VOID example_dvp_stop(INT_T argc, CHAR_T *argv[]);

extern VOID example_thread(INT_T argc, CHAR_T *argv[]);
extern VOID example_sw_timer(INT_T argc, CHAR_T *argv[]);
extern VOID example_semaphore(INT_T argc, CHAR_T *argv[]);
extern VOID example_semaphore_stop(INT_T argc, CHAR_T *argv[]);
extern VOID example_queue(INT_T argc, CHAR_T *argv[]);
extern VOID example_queue_stop(INT_T argc, CHAR_T *argv[]);
extern VOID example_mutex(INT_T argc, CHAR_T *argv[]);
extern VOID example_mutex_stop(INT_T argc, CHAR_T *argv[]);

extern VOID example_feed_watchdog(INT_T argc, CHAR_T *argv[]);

extern VOID example_ble_central(INT_T argc, CHAR_T *argv[]);
extern VOID example_ble_peripheral(INT_T argc, CHAR_T *argv[]);
extern VOID example_ty_ble_remote(INT_T argc, CHAR_T *argv[]);
extern VOID example_user_ble_remote(INT_T argc, CHAR_T *argv[]);

extern VOID example_kv(INT_T argc, CHAR_T *argv[]);
extern VOID example_uf_file(INT_T argc, CHAR_T *argv[]);

extern VOID example_wifi_ap(INT_T argc, CHAR_T *argv[]);
extern VOID example_wifi_sta(INT_T argc, CHAR_T *argv[]);
extern VOID example_wifi_scan(INT_T argc, CHAR_T *argv[]);
extern VOID example_wifi_low_power(INT_T argc, CHAR_T *argv[]);

extern VOID example_product_test(INT_T argc, CHAR_T *argv[]);

extern VOID example_ffc_master_init(INT_T argc, CHAR_T *argv[]);
extern VOID example_ffc_master_send(INT_T argc, CHAR_T *argv[]);
extern VOID example_ffc_slave_init(INT_T argc, CHAR_T *argv[]);

extern VOID example_output_free_heap(INT_T argc, CHAR_T *argv[]);
extern VOID example_soc_device_remove(INT_T argc, CHAR_T *argv[]);
extern VOID example_soc_device_init(INT_T argc, CHAR_T *argv[]);
extern VOID example_query_lp_dp_cache(INT_T argc, CHAR_T *argv[]);

extern VOID example_tcp_server(INT_T argc, CHAR_T *argv[]);
extern VOID example_tcp_client(INT_T argc, CHAR_T *argv[]);

extern VOID example_health_manage(INT_T argc, CHAR_T *argv[]);

extern VOID example_http(INT_T argc, CHAR_T *argv[]);
extern VOID example_iot_http(INT_T argc, CHAR_T *argv[]);
extern VOID example_http_download(INT_T argc, CHAR_T *argv[]);

extern VOID example_attach_ota(INT_T argc, CHAR_T *argv[]);
extern VOID example_main_ota_custom(INT_T argc, CHAR_T *argv[]);
extern VOID example_time_service(INT_T argc, CHAR_T *argv[]);

extern VOID command_list_output(INT_T argc, CHAR_T *argv[]);

VOID example_reboot(INT_T argc, CHAR_T *argv[]);

/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC CLI_CMD_T __cmd_register_list[] = {
#if defined(ENABLE_ADC) && (ENABLE_ADC == 1)
    {"example_adc", "Output TUYA_ADC_NUM_0 Sample value", example_adc},
#endif

#if defined(ENABLE_GPIO) && (ENABLE_GPIO == 1)
    {"example_gpio", "init gpio TUYA_GPIO_NUM_14 and read write every 2 seconds", example_gpio},
    {"example_sw_i2c", "init software i2c, read the value of temperature and humidity, chip:sht3x ", example_sw_i2c},
#endif

#if defined(ENABLE_I2C) && (ENABLE_I2C == 1)
    {"example_i2c", "i2c example", example_i2c},
#endif

#if defined(ENABLE_PWM) && (ENABLE_PWM == 1)
    {"example_pwm", "init pwm TUYA_PWM_NUM_3 and change the duty and frequency every 2 seconds", example_pwm},
#endif

#if defined(ENABLE_SPI) && (ENABLE_SPI == 1)
    {"example_spi", "init and start spi0", example_spi},
#endif

#if defined(ENABLE_MEDIA) && (ENABLE_MEDIA == 1)
    {"example_speaker_play", "play mp3 file", example_speaker_play},
    {"example_recorder_init", "Record sound using mic when key is pressed, play recorded sound after releasing key", example_recorder_init},
    {"example_dvp", "DVP example", example_dvp},
    {"example_dvp_stop", "Stop DVP example", example_dvp_stop},
#endif

#if defined(ENABLE_TIMER) && (ENABLE_TIMER == 1)
    {"example_timer", "Init and start timer0, enter callback 5 times to stop the timer", example_timer},
#endif

    {"example_thread", "creat a thread, then delete", example_thread},
    {"example_sw_timer", "creat a software timer", example_sw_timer},
    {"example_semaphore", "semaphore example, you can execute the \"example_semaphore_stop\" command to stop it", example_semaphore},
    {"example_semaphore_stop", "stop semaphore example", example_semaphore_stop},
    {"example_queue", "queue example, you can execute the \"example_queue_stop\" command to stop it", example_queue},
    {"example_queue_stop", "stop queue example", example_queue_stop},
    {"example_mutex", "mutex example, you can execute the \"example_mutex_stop\" command to stop it", example_mutex},
    {"example_mutex_stop", "stop mutex example", example_mutex_stop},

#if defined(ENABLE_BT_SERVICE) && (ENABLE_BT_SERVICE == 1)
    {"example_ble_central", "Ble central example", example_ble_central},
    {"example_ble_peripheral", "Ble peripheral example", example_ble_peripheral},
#endif

#if defined(ENABLE_BT_REMOTE_CTRL) && (ENABLE_BT_REMOTE_CTRL == 1)
    {"example_ty_ble_remote", "enable accept tuya ble remote data function. then you should execute \"example_soc_init\"", example_ty_ble_remote},
    {"example_user_ble_remote", "enable accept user ble remote data function. then you should execute \"example_soc_init\"", example_user_ble_remote},
#endif

    /* kv database */
    {"example_kv", "Key-value database example", example_kv},

    {"example_uf_file", "uf file example", example_uf_file},

#if defined(ENABLE_WIFI_SERVICE) && (ENABLE_WIFI_SERVICE == 1)
    {"example_wifi_ap", "SoftAP, SSID: \"my-wifi\", password: \"12345678\"", example_wifi_ap},
    {"example_wifi_sta", "Wi-Fi station connect, Please enter the SSID and password in the code.", example_wifi_sta},
    {"example_wifi_scan", "Scan for nearby Wi-Fi", example_wifi_scan},
    {"example_wifi_low_power", "Wi-Fi low power", example_wifi_low_power},
    {"example_query_lp_dp", "query lowpower dp cache, you need to first call \"example_soc_init mg1fapvshzimzcsi\".", example_query_lp_dp_cache},
    {"example_product_test", "scan target ssid enter product test ", example_product_test},
#endif

#if defined(TUYA_WIFI_FFC_MASTER) && (TUYA_WIFI_FFC_MASTER == 1)
    {"example_ffc_master_init", "ffc master init and bind", example_ffc_master_init},
    {"example_ffc_master_send", "ffc master send data, 0x00-0x09", example_ffc_master_send},
#endif

#if defined(TUYA_WIFI_FFC_SLAVER) && (TUYA_WIFI_FFC_SLAVER == 1)
    {"example_ffc_slave_init", "ffc slave init and bind", example_ffc_slave_init},
#endif

    {"example_output_free_heap", "Output free heap every 3 seconds.", example_output_free_heap},
    {"example_soc_init", "After initialization, you can use the Tuya Smart App to operate.", example_soc_device_init},
    {"example_soc_remove", "Remove the device to reconnect it to the network.", example_soc_device_remove},

    {"example_tcp_server", "tcp server", example_tcp_server},
    {"example_tcp_client", "tcp client", example_tcp_client},

#if defined(ENABLE_WATCHDOG) && (ENABLE_WATCHDOG == 1)
    {"example_feed_watchdog", "Feed watchdog, you need to first call \"example_soc_init\".", example_feed_watchdog},
#endif

    {"example_health_manage", "Health manage add item example, you need to first call \"example_soc_init\".", example_health_manage},

    {"example_http", "http post/get example, please connect to the network first.", example_http},
    {"example_iot_http", "tuya iot http post/get example, please connect to the network first.", example_iot_http},
    {"example_http_download", "Download a file, please connect to the network first.", example_http_download},

    {"example_attach_fw_ota", "Download attach firmware.", example_attach_ota},
    {"example_main_fw_ota", "Register main-firmware-ota handler to replace the default one.", example_main_ota_custom},
    {"example_get_local_time", "Get device local time.", example_time_service},

    {"reboot", "Reboot device.", example_reboot},
    {"help", "Output examples list.", command_list_output},
};

/***********************************************************
***********************function define**********************
***********************************************************/
VOID example_reboot(INT_T argc, CHAR_T *argv[])
{
    tkl_system_reset();
}

VOID examples_cli_register(VOID_T)
{
    tuya_cli_cmd_register(__cmd_register_list, CNTSOF(__cmd_register_list));
}
