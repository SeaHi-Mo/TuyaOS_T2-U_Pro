

# TIME SERVICE

##  简介

这个项目将会介绍如何使用 `tuyaos 3 time service` 相关接口，创建一个软件定时器，定时查询本地时间。

想要获取到准确的设备时间，需要等设备连云成功与云端校时后。



**更多详细描述请查看 [时间时区服务](https://developer.tuya.com/cn/docs/iot-device-dev/TuyaOS-iot_abi_time_service?id=Kcspzns4pmryl)。**



## 运行结果

**需要先运行 `example_soc_init` 指令，等待设备连云校时后才能获取正确时间**。

```c
[04-23 10:16:42 ty N][example_time.c:58] get local time
[04-23 10:16:42 ty N][example_time.c:59] year: 2024
[04-23 10:16:42 ty N][example_time.c:60] month: 4
[04-23 10:16:42 ty N][example_time.c:61] day: 23
[04-23 10:16:42 ty N][example_time.c:62] week: 2
[04-23 10:16:42 ty N][example_time.c:63] 10 : 16 : 42
```


## 技术支持
您可以通过以下方法获得涂鸦的支持:
* [开发者中心](https://developer.tuya.com)
* [帮助中心](https://support.tuya.com/help)
* [技术支持帮助中心](https://service.console.tuya.com)
* [Tuya os](https://developer.tuya.com/cn/tuyaos)
