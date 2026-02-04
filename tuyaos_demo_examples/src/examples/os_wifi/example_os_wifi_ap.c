/**
* @file examples_os_wifi_ap.c
* @author www.tuya.com
* @brief examples_os_wifi_ap module is used to 
* @version 0.1
* @date 2022-05-20
*
* @copyright Copyright (c) tuya.inc 2022
*
*/

#include "tuya_cloud_types.h"

#include "tal_log.h"
#include "tal_wifi.h"

#include "tal_sw_timer.h"
#include "tal_network.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
#define DEFAULT_WIFI_CHANNEL                 5

#define AP_SSID "my-wifi"
#define AP_PASSWD "12345678"

#define AP_IP   "192.168.1.123"
#define AP_MASK "255.255.255.0"
#define AP_GW   "192.168.1.1"

// udp broadcast
#define UDP_BROADCAST_PORT  6666

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
TIMER_ID timer_id = NULL;
INT_T udp_broadcast_fd = 0;

/***********************************************************
***********************function define**********************
***********************************************************/
/**
* @brief wifi Related event callback function
*
* @param[in] event:event
* @param[in] arg:parameter
* @return none
*/
STATIC VOID_T wifi_event_callback(WF_EVENT_E event, VOID_T *arg)
{
    TAL_PR_DEBUG("-------------event callback-------------");
}

VOID_T udp_broadcast_func(TIMER_ID timer_id, VOID_T *arg)
{
    CHAR_T send_data[] = "hello world";
    TUYA_IP_ADDR_T broadcast_addr;

#ifdef ipaddr4
    broadcast_addr = TY_IPADDR_BROADCAST;
#else
    broadcast_addr = 0xFFFFFFFF;
#endif

    tal_net_send_to(udp_broadcast_fd, send_data, strlen(send_data), broadcast_addr, UDP_BROADCAST_PORT);
    TAL_PR_DEBUG("send data:%s", send_data);

    return;
}

/**
* @brief WiFi AP task
*
* @param[in] param:Task parameters
* @return none
*/
VOID example_wifi_ap(INT_T argc, CHAR_T *argv[])
{
    OPERATE_RET rt = OPRT_OK;
    const char ap_ssid[] = AP_SSID;        // ssid
    const char ap_password[] = AP_PASSWD;     // password
    
#ifdef nwipstr    
    NW_IP_S nw_ip = {
        .nwipstr   = AP_IP,
        .nwmaskstr = AP_MASK,
        .nwgwstr   = AP_GW
    };
#else 
    NW_IP_S nw_ip = {
        .ip   = AP_IP,
        .mask = AP_MASK,
        .gw   = AP_GW,
    };
#endif

    TAL_PR_NOTICE("------ wifi ap example start ------");

    /*wifi init*/
    TUYA_CALL_ERR_GOTO(tal_wifi_init(wifi_event_callback), __EXIT);

    /*Set WiFi mode to AP*/
    TUYA_CALL_ERR_LOG(tal_wifi_set_work_mode(WWM_SOFTAP));

    WF_AP_CFG_IF_S wifi_cfg = {
        .s_len = strlen(ap_ssid),       //ssid length
        .p_len = strlen(ap_password),   //password length
        .chan = DEFAULT_WIFI_CHANNEL,   //wifi channel
        .md = WAAM_WPA2_PSK,             //encryption type
        .ip = nw_ip,                    //ip information
        .ms_interval = 100,             //broadcast interval,Unit(ms)
        .max_conn = 3                   //max sta connect numbers 
    };

    strcpy((char *)wifi_cfg.ssid, ap_ssid);   //ssid
    strcpy((char *)wifi_cfg.passwd, ap_password); //password
    TUYA_CALL_ERR_LOG(tal_wifi_ap_start(&wifi_cfg));

    /* udp broadcast */
#if 1
    INT_T status = 0;
    udp_broadcast_fd = tal_net_socket_create(PROTOCOL_UDP);
    if (udp_broadcast_fd < 0) {
        TAL_PR_ERR("create udp socket failed");
        goto __EXIT;
    }

    TUYA_IP_ADDR_T ip_addr = tal_net_str2addr(AP_IP);
    TAL_PR_DEBUG("ip_addr:%d", ip_addr);

    status = tal_net_bind(udp_broadcast_fd, ip_addr, UDP_BROADCAST_PORT);
    if (status < 0) {
        TAL_PR_ERR("bind fail:%d", tal_net_get_errno());
        goto __EXIT;
    }

    tal_net_set_broadcast(udp_broadcast_fd);

    tal_sw_timer_create(udp_broadcast_func, NULL, &timer_id);
    tal_sw_timer_start(timer_id, 5*1000, TAL_TIMER_CYCLE);
#endif

__EXIT:
    return;
}
