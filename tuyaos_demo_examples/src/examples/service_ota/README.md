# 固件升级



## 示例说明

### example_attach_ota.c

该示例介绍了如何注册附属模组（MCU模组 和 自定义模组），以及如何实现附属模组 OTA 。

更多详细描述请查看 [附属固件升级](https://developer.tuya.com/cn/docs/iot-device-dev/TuyaOS-iot_abi_attach_fw_ota?id=Kd5s93f0gmwwy)。

### example_main_ota_custom.c

该示例介绍了如何实现自己处理主联网固件的OTA。

**主联网固件的升级一般不用开发者处理，整个OTA流程框架都会自行处理。该示例只是适用开发者想自己接管主联网固件的OTA的情况。**

更多详细描述请查看 [联网固件升级](https://developer.tuya.com/cn/docs/iot-device-dev/TuyaOS-iot_abi_device_ota?id=Kd5s90hhrjuhn)。



## 使用说明

### example_attach_ota.c

1. 设备上电后，执行 `example_attach_fw_ota` 指令。
2. 用涂鸦 App 对设备配网激活。
3. 点进设备面板，可收到升级提醒。（例程中的内置的产品已经配置好升级了。如果你自己配置升级可替换成自己的产品。）
4. 点击升级
5. 升级完成后，MCU 版本 和 自定义模组版本均变为 2.0.0。

### example_main_ota_custom.c

 1. 开发者将示例中的 PID和 FIRMWARE_KEY 替换成自己的产品 PID 和自己的固件 key。

 2. 编译烧录固件，同时编译个更高版本的升级固件。

 3. 设备上电后，执行 `example_main_fw_ota` 指令。

 4. 用涂鸦 App 对设备进行配网激活。

 5. 在涂鸦 IOT 平台上配置好固件升级。

 6. 设备进入升级状态。

 7. 现在处理函数只是有些信息打印，并没有实际处理，**故 App 面板上会显示升级超时**。这些处理函数需要开发者按需填充。

    

## 运行结果

### example_attach_ota.c

- **接收到进入升级流程的通知**

  ```
  [10:37:35:478][12-21 10:37:35 TUYA D][example_attach_ota.c:128] attach upgrade Info
  [10:37:35:478][12-21 10:37:35 TUYA D][example_attach_ota.c:129] fw->tp:9
  [10:37:35:478][12-21 10:37:35 TUYA D][example_attach_ota.c:130] fw->fw_url:https://fireware-ttls.tuyacn.com:2443/smart/firmware/upgrade/bay1606729651139eUrx/1703070099bd736c37d95.bin
  [10:37:35:478][12-21 10:37:35 TUYA D][example_attach_ota.c:131] fw->fw_hmac:C8E9907AC928073E50D59F1D8E9EFA0751F9FA709488A7DFE17868787F09D79A
  [10:37:35:487][12-21 10:37:35 TUYA D][example_attach_ota.c:132] fw->sw_ver:2.0.0
  [10:37:35:522][12-21 10:37:35 TUYA D][example_attach_ota.c:133] fw->file_size:15698
  ```

- **开始拉包升级**

  ```
  [10:37:40:311][12-21 10:37:40 TUYA D][example_attach_ota.c:76] Rev File Data
  [10:37:40:311][12-21 10:37:40 TUYA D][example_attach_ota.c:77] Total_len:15698 
  [10:37:40:311][12-21 10:37:40 TUYA D][example_attach_ota.c:78] Offset:0 Len:1024
  [10:37:40:312][12-21 10:37:40 TUYA D][example_attach_ota.c:97] fw pack
  [10:37:40:355]4D 43 55 20 4F 54 41 20 54 45 53 54 20 4D 43 55 20 4F 54 41 
  [10:37:40:355]20 54 45 53 54 20 4D 43 55 20 4F 54 41 20 54 45 53 54 20 4D 
  [10:37:40:355]43 55 20 4F 54 41 20 54 45 53 54 20 4D 43 55 20 4F 54 41 20 
  [10:37:40:355]54 45 53 54 20 4D 43 55 20 4F 54 41 20 54 45 53 54 20 4D 43 
  [10:37:40:355]55 20 4F 54 41 20 54 45 53 54 20 4D 43 55 20 4F 54 41 20 54 
  [10:37:40:355]45 53 54 20 4D 43 55 20 4F 54 41 20 54 45 53 54 20 4D 43 55 
  [10:37:40:355]20 4F 54 41 20 54 45 53 54 20 4D 43 55 20 4F 54 41 20 54 45 
  [10:37:40:355]53 54 20 4D 43 55 20 4F 54 41 20 54 45 53 54 0D 0A 4D 43 55 
  ...
  ```

- **收到最后一包升级包**

  ```JS
  [10:37:49:237][12-21 10:37:48 TUYA D][example_attach_ota.c:76] Rev File Data
  [10:37:49:244][12-21 10:37:48 TUYA D][example_attach_ota.c:77] Total_len:15698 
  [10:37:49:275][12-21 10:37:48 TUYA D][example_attach_ota.c:78] Offset:15360 Len:338
  [10:37:49:275][12-21 10:37:49 TUYA D][example_attach_ota.c:81] the last fw pack
  [10:37:49:275]43 55 20 4F 54 41 20 54 45 53 54 20 4D 43 55 20 4F 54 41 20 
  [10:37:49:276]54 45 53 54 0D 0A 4D 43 55 20 4F 54 41 20 54 45 53 54 20 4D 
  [10:37:49:276]43 55 20 4F 54 41 20 54 45 53 54 20 4D 43 55 20 4F 54 41 20 
  [10:37:49:276]54 45 53 54 20 4D 43 55 20 4F 54 41 20 54 45 53 54 20 4D 43 
  [10:37:49:276]55 20 4F 54 41 20 54 45 53 54 20 4D 43 55 20 4F 54 41 20 54 
  [10:37:49:276]45 53 54 20 4D 43 55 20 4F 54 41 20 54 45 53 54 20 4D 43 55
  [10:37:49:317]20 4F 54 41 20 54 45 53 54 20 4D 43 55 20 4F 54 41 20 54 45 
  [10:37:49:318]53 54 20 4D 43 55 20 4F 54 41 20 54 45 53 54 20 4D 43 55 20 
  [10:37:49:318]4F 54 41 20 54 45 53 54 20 4D 43 55 20 4F 54 41 20 54 45 53 
  [10:37:49:318]54 0D 0A 4D 43 55 20 4F 54 41 20 54 45 53 54 20 4D 43 55 20 
  [10:37:49:318]4F 54 41 20 54 45 53 54 20 4D 43 55 20 4F 54 41 20 54 45 53 
  [10:37:49:318]54 20 4D 43 55 20 4F 54 41 20 54 45 53 54 20 4D 43 55 20 4F 
  [10:37:49:318]54 41 20 54 45 53 54 20 4D 43 55 20 4F 54 41 20 54 45 53 54 
  [10:37:49:319]20 4D 43 55 20 4F 54 41 20 54 45 53 54 20 4D 43 55 20 4F 54 
  [10:37:49:362]41 20 54 45 53 54 20 4D 43 55 20 4F 54 41 20 54 45 53 54 20 
  [10:37:49:363]4D 43 55 20 4F 54 41 20 54 45 53 54 20 4D 43 55 20 4F 54 41 
  [10:37:49:363]20 54 45 53 54 20 4D 43 55 20 4F 54 41 20 54 45 53 54 
  ```

- **更新版本号**

  ```JS
  [12-21 11:08:46 TUYA D][example_attach_ota.c:63] update tp:9 version:2.0.0
  [11:08:47:002][12-21 11:08:46 TUYA D][lr:0x79247] This Time Download 15698 Byte. Offset:0
  ```

- **接收升级结果通知**

  ```
  [10:37:49:409][12-21 10:37:49 TUYA D][lr:0x87cb7] file hmac check success
  [10:37:49:450][12-21 10:37:49 TUYA D][example_attach_ota.c:111] fw->tp:9
  [10:37:49:450][12-21 10:37:49 TUYA D][example_attach_ota.c:112] download result:0
  ```

### example_main_ota_custom.c

- **接收到进入升级流程的通知**

  ```
  [11:21:57:169][04-25 11:21:56 ty D][lr:0xb2e05] Connect: a3.tuyacn.com Port: 443  --<< ,r:0
  [11:21:57:551][04-25 11:21:57 ty D][lr:0x7bf5b] Decode Rev:{"result":{"cdnUrl":"https://images.tuyacn.com/smart/firmware/upgrade/bay1606729651139eUrx/1670471380841caa9aad6.bin","execTime":0,"hasMore":false,"hmac":"01A92790AFE331266BF25C92B1F2A784EF196119FA99C7B62969D3[04-25 11:21:57 ty D][lr:0x83cb5] tls transporter close socket fd:4
  [11:21:57:574][04-25 11:21:57 ty D][lr:0xbda1d] tcp transporter close socket fd:4
  [11:21:57:586][04-25 11:21:57 ty D][lr:0x83ccf] tls transporter close tls handler:0x4186a4
  [11:21:57:588][04-25 11:21:57 ty D][lr:0x7faf3] TUYA_TLS Disconnect ENTER
  [11:21:57:596][04-25 11:21:57 ty D][lr:0x7fb43] TUYA_TLS Disconnect Success
  [11:21:57:598][04-25 11:21:57 ty D][lr:0x7f77f] tuya_tls_connect_destroy.
  [11:21:57:607][04-25 11:21:57 ty D][lr:0x8e0d9] hmac:01A92790AFE331266BF25C92B1F2A784EF196119FA99C7B62969D39D8810FCBC, md5:ac8fcfb3566fc48a4496ecfb072671d1, ver:1.0.1,url:https://fireware-ttls.tuyacn.com:2443/smart/firmware/upgrade/bay1606729651139eUrx/1670471380841ca[04-25 11:21:57 ty D][lr:0x964db] set gw ext_stat:3
  [11:21:57:630][04-25 11:21:57 ty D][example_main_ota_custom.c:112] pre dev ug, tp:0, url:https://fireware-ttls.tuyacn.com:2443/smart/firmware/upgrade/bay1606729651139eUrx/1670471380841caa9aad6.bin, hmac:01A92790AFE331266BF25C92B1F2A784EF196119FA99C7B62969D39D8810FCBC,[04-25 11:21:57 ty I][tal_thread.c:184] thread_create name:dev_upgrade_func,stackDepth:4096,totalstackDepth:40960,priority:2
  [11:21:57:662][04-25 11:21:57 ty D][lr:0x8d989] pre gw ug is set:1
  [11:21:57:669][04-25 11:21:57 ty D][tal_thread.c:203] Thread:dev_upgrade_fun Exec Start. Set to Running Stat
  [11:21:57:678][04-25 11:21:57 ty D][lr:0x8db3b] download begin, set upgrade stat:2
  [11:21:57:680][04-25 11:21:57 ty D][lr:0x7b79b] Post Data: {"upgradeStatus":2,"type":0,"t":1714015317}
  [11:21:57:696][04-25 11:21:57 ty D][lr:0x7b7db] Post URL: https://a3.tuyacn.com/d.json?a=tuya.device.upgrade.status.update&devId=6c3fa1e908382c3d0ax518&et=3&t=1714015317&v=4.1&sign=1f15e67fcba0d9cde944821538123cbb
  [11:21:57:715][04-25 11:21:57 ty D][lr:0xb2deb] Connect: a3.tuyacn.com Port: 443  -->>
  [11:21:57:718][04-25 11:21:57 ty D][lr:0x7f055] unw_gethostbyname a3.tuyacn.com, prio 1
  ```

- **开始拉包升级**

  ```
  [11:21:59:245][04-25 11:21:58 ty D][lr:0x8e3fb] mqtt report download percent:0
  [11:21:59:246][04-25 11:21:58 ty D][lr:0x8288f] packet id: 2
  [11:21:59:269][04-25 11:21:58 ty D][example_main_ota_custom.c:92] total_len:634400 offset:0, data_len:375
  [11:21:59:271][04-25 11:21:58 ty D][example_main_ota_custom.c:92] total_len:634400 offset:375, data_len:1
  [11:21:59:318][04-25 11:21:58 ty D][example_main_ota_custom.c:92] total_len:634400 offset:376, data_len:1024
  [11:21:59:356][04-25 11:21:58 ty D][example_main_ota_custom.c:92] total_len:634400 offset:1400, data_len:1024
  [11:21:59:404][04-25 11:21:58 ty D][example_main_ota_custom.c:92] total_len:634400 offset:2424, data_len:1024
  [11:21:59:445][04-25 11:21:58 ty D][example_main_ota_custom.c:92] total_len:634400 offset:3448, data_len:1024
  [11:21:59:508][04-25 11:21:58 ty D][example_main_ota_custom.c:92] total_len:634400 offset:4472, data_len:1024
  ```

- **接收升级结果通知**

  ```
  [11:22:33:278][04-25 11:22:32 ty D][lr:0x7c819] download file finish
  [11:22:33:287][04-25 11:22:32 ty D][lr:0x8dc11] target file hmac:01A92790AFE331266BF25C92B1F2A784EF196119FA99C7B62969D39D8810FCBC
  [11:22:33:290][04-25 11:22:32 ty D][lr:0x8dc17] sdk calc file hmac:
  [11:22:33:302]01a92790afe331266bf25c92b1f2a784ef196119fa99c7b62969d39d8810fcbc
  [11:22:33:304][04-25 11:22:32 ty D][lr:0x8dc5b] file hmac check success
  [11:22:33:326][04-25 11:22:32 ty D][example_main_ota_custom.c:79] fw->tp:0
  [11:22:33:326][04-25 11:22:32 ty D][example_main_ota_custom.c:80] download result:0
  [11:22:33:326][04-25 11:22:32 ty D][lr:0x964db] set gw ext_stat:2
  [11:22:33:329][04-25 11:22:32 ty D][tal_thread.c:244] Del Thrd:dev_upgrade_fun
  ```

  

## 技术支持

您可以通过以下方法获得涂鸦的支持:

- TuyaOS 论坛： https://www.tuyaos.com
- 开发者中心： https://developer.tuya.com
- 帮助中心： https://support.tuya.com/help
- 技术支持工单中心： https://service.console.tuya.com
- TuyaOS 文档中心: https://developer.tuya.com/cn/docs/iot-device-dev
