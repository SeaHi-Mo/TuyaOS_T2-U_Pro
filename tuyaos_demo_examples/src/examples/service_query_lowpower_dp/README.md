

# 获取低功耗设备的 DP 缓存

## 功能介绍

对低功耗设备而言，其常规功能主要是上报设备状态和一些重要信息。但是在一些场景中，用户需要远程设置设备参数或者控制设备。此时就需要远端下发相应 DP 对设备进行控制，但 DP 命令下发时，低功耗设备可能因为**处在休眠状态而接收不到指令**。
为了解决这个问题，涂鸦针对低功耗设备提供了可先将 **DP 缓存至云端**，等待设备从休眠中醒来后**主动去云端获取指令**的方案。该方案降低了设置低功耗产品参数的失败率，同时也简化了用户操作的复杂度。

具体功能请查看文档 [低功耗设备 DP 缓存下发](https://developer.tuya.com/cn/docs/iot-device-dev/TuyaOS-iot_abi_query_lowpower_dp_cache?id=Kd1tf8sz0nq73)。



## 运行示例

- 先执行 `example_soc_init mg1fapvshzimzcsi` 命令。**mg1fapvshzimzcsi** 是一个已经在 iot 前台打开**低功耗 DP 缓存下发**能力的产品pid。

- 确保设备用该 pid 到涂鸦云激活成功后。在app 面板上点击**温标切换按钮**。
- 执行 `example_query_lp_dp`，固件会去拉温标切换 dp 的缓存。

```C
[10-27 15:20:32 ty D][lr:0x76d47] out:{"dps":[9]}
[10-27 15:20:32 ty D][lr:0x7aa7b] Post Data: {"dps":[9],"t":1698391232}
[10-27 15:20:32 ty D][lr:0x7aabb] Post URL: https://a3.tuyacn.com/d.json?a=tuya.device.dev.dp.get&devId=6c67357f7cc01b402bxtkd&et=3&t=1698391232&v=1.0&sign=9ee25f9ca37c98c6c61626178ddf5951
[10-27 15:20:32 ty D][lr:0xb2027] Connect: a3.tuyacn.com Port: 443  -->>
[10-27 15:20:32 ty D][lr:0x7e335] unw_gethostbyname a3.tuyacn.com, prio 1
[10-27 15:20:32 ty D][lr:0x7e461] use dynamic dns ip:81.69.183.170 for domain:a3.tuyacn.com
[10-27 15:20:32 ty D][lr:0xbcbb9] bind ip:c0a81cf1 port:0 ok
[10-27 15:20:32 ty D][lr:0x7eacd] MAX SECURITY_LEVEL:0, TUYA SECURITY_LEVEL:0, mode:0
[10-27 15:20:32 ty D][lr:0x7eae7] TUYA_TLS Begin Connect a3.tuyacn.com:443
[10-27 15:20:32 ty D][lr:0x7eb57] TUYA_TLS PSK Mode
[10-27 15:20:32 ty D][lr:0x7ebdd] socket fd is set. set to inner send/recv to handshake
[10-27 15:20:32 ty D][lr:0x7ec27] handshake finish for a3.tuyacn.com. set send/recv to user set
[10-27 15:20:32 ty D][lr:0x7ec59] TUYA_TLS Success Connect a3.tuyacn.com:443 Suit:TLS-PSK-WITH-AES-128-CBC-SHA256
[10-27 15:20:32 ty D][lr:0xb2041] Connect: a3.tuyacn.com Port: 443  --<< ,r:0
[10-27 15:20:33 ty D][lr:0x7b23b] Decode Rev:{"result":["{\"9\":\"f\"}"],"t":1698391232,"success":true}
[10-27 15:20:33 ty D][lr:0x82f8d] tls transporter close socket fd:9
[10-27 15:20:33 ty D][lr:0xbcc59] tcp transporter close socket fd:9
[10-27 15:20:33 ty D][lr:0x82fa7] tls transporter close tls handler:0x4178a0
[10-27 15:20:33 ty D][lr:0x7edd3] TUYA_TLS Disconnect ENTER
[10-27 15:20:33 ty D][lr:0x7ee23] TUYA_TLS Disconnect Success
[10-27 15:20:33 ty D][lr:0x7ea5f] tuya_tls_connect_destroy.
[10-27 15:20:33 ty N][lr:0x76d89] result:["{\"9\":\"f\"}"]
[10-27 15:20:33 ty D][example_query_lowpower_dp.c:85] obj dpid:9 tp:3
[10-27 15:20:33 ty D][lr:0x77d1d] rept chan:3
[10-27 15:20:33 ty D][lr:0x75bc9] dp<9> check. need_update:1 pv_stat:0 trig_t:0 dp_rept_type:0 force_send:0 prop_tp:3
[10-27 15:20:33 ty D][lr:0x75d33] is_need_update:1, is_dp_passive:0, pv_stat:2, uling_cnt:0
[10-27 15:20:33 ty D][lr:0x7665d] dp rept_type:0, data:{"9":"f"}
[10-27 15:20:33 ty D][lr:0xb7cdd] To:0 src:{"dps":{"9":"f"}} pro:4 num:21782
[10-27 15:20:33 ty D][lr:0xb7d1f] After Pack:{"protocol":4,"t":1698391233,"data":{"dps":{"9":"f"}}} offset:54
[10-27 15:20:33 ty D][lr:0xaf7e7] Prepare To Send Lan:{"dps":{"9":"f"}}, msg_len:17, out_len:69
[10-27 15:20:33 ty D][lr:0x80261] Send MQTT Msg.P:4 N:21782 Q:1 Data:{"dps":{"9":"f"}}
[10-27 15:20:33 ty D][lr:0x81b6f] packet id: 2
[10-27 15:20:33 ty D][lr:0x7656f] msg_data:{"dps":{"9":"f"}}, mqtt async dp cb:0
[10-27 15:20:33 ty D][lr:0x75967] set short dp rate rule, dpID:9, curr_t:1698391233, rept_cnt:1.
[10-27 15:20:33 ty D][lr:0x7597d] set long dp rate rule, dpID:9, curr_t:1698391233, rept_cnt:0.
[10-27 15:20:33 ty D][lr:0x75995] set all dp rate rule, dpID:9, curr_t:1698391233, rept_cnt:0.
[10-27 15:20:47 ty D][lr:0x95571] set time by rtc

```


## 技术支持

您可以通过以下方法获得涂鸦的支持:

- TuyaOS 论坛： https://www.tuyaos.com

- 开发者中心： https://developer.tuya.com

- 帮助中心： https://support.tuya.com/help

- 技术支持工单中心： https://service.console.tuya.com
