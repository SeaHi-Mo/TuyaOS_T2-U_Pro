# 蓝牙遥控器

示例功能：

- example_ty_ble_remote.c
  - 打开接收涂鸦蓝牙遥控器数据功能
  - 接收涂鸦蓝牙遥控器绑定通知

- example_user_ble_remote.c

  打开接收自定义蓝牙遥控器数据功能

关于蓝牙遥控器接收功能的详细描述，请查看 [蓝牙遥控器](https://developer.tuya.com/cn/docs/iot-device-dev/TuyaOS-iot_abi_remote_bluetooth_beacon?id=Kd5s8ca4gp3xt)。

---

## 使用说明

### 使用涂鸦蓝牙遥控器

- 设备上电，先执行 `example_ty_ble_remote`，然后再执行设备初始化  `example_soc_init`。待设备联网后，框架会自动打开配对窗口。
- 让蓝牙遥控器和设备进行配对。
- 配对成功后，蓝牙遥控器可发送控制指令，设备会接收到对应的控制指令。

## 使用自定义蓝牙遥控器

- 设备上电，先执行 `example_ty_ble_remote`，然后再执行设备初始化  `example_soc_init`。待设备联网后，框架会自动打开配对窗口。
- 让自定义蓝牙遥控器发送广播数据。
- 设备会接收到对应数据。

## 运行结果

- **涂鸦蓝牙遥控器绑定数据**

```C
[07-08 16:53:27 TUYA D][lr:0x4babb] ----------------scan adv bind check cb-----------------
[07-08 16:53:27 TUYA D][lr:0x4bac3] bt type : 1
[07-08 16:53:27 TUYA D][lr:0x4bad3] data[0] : 11
[07-08 16:53:27 TUYA D][lr:0x4bad3] data[1] : 7
[07-08 16:53:27 TUYA D][lr:0x4bad3] data[2] : 1
[07-08 16:53:27 TUYA D][lr:0x4bad3] data[3] : 255
[07-08 16:53:27 TUYA D][lr:0x4bad3] data[4] : 2
[07-08 16:53:27 TUYA D][lr:0x4bad3] data[5] : 1
[07-08 16:53:27 TUYA D][lr:0x4bad3] data[6] : 255
[07-08 16:53:27 TUYA D][lr:0x4bad3] data[7] : 221
[07-08 16:53:27 TUYA D][lr:0x4bad3] data[8] : 171
[07-08 16:53:27 TUYA D][lr:0x4bad3] data[9] : 0
[07-08 16:53:27 TUYA D][lr:0x4bad3] data[10] : 177
[07-08 16:53:27 TUYA D][lr:0x4bad3] data[11] : 144
```

- **涂鸦蓝牙遥控器绑定状态发送变化，进入蓝牙事件回调函数中**

```c
[07-08 16:53:28 TUYA D][lr:0x4baf5] ------------------ble scan adv bind notify cb----------------
[07-08 16:53:28 TUYA D][lr:0x4bafd] bt type : 1
[07-08 16:53:28 TUYA D][lr:0x4bb05] bt rslt : 0
```

- **按下涂鸦蓝牙遥控器的控制键。进入数据处理回调函数中**

```c
[07-08 16:57:41 TUYA D][lr:0x4bbb3] ble remote(new) send data len: 9
[07-08 16:57:41 TUYA D][lr:0x4bbc3] data[0] : 11
[07-08 16:57:41 TUYA D][lr:0x4bbc3] data[1] : 7
[07-08 16:57:41 TUYA D][lr:0x4bbc3] data[2] : 1
[07-08 16:57:41 TUYA D][lr:0x4bbc3] data[3] : 1
[07-08 16:57:41 TUYA D][lr:0x4bbc3] data[4] : 11
[07-08 16:57:41 TUYA D][lr:0x4bbc3] data[5] : 2
[07-08 16:57:41 TUYA D][lr:0x4bbc3] data[6] : 20
[07-08 16:57:41 TUYA D][lr:0x4bbc3] data[7] : 0
[07-08 16:57:41 TUYA D][lr:0x4bbc3] data[8] : 0
```

- **涂鸦蓝牙遥控器解绑**

```c
[07-08 16:58:52 TUYA D][lr:0x4baf5] ------------------ble scan adv bind notify cb----------------
[07-08 16:58:52 TUYA D][lr:0x4bafd] bt type : 0
[07-08 16:58:52 TUYA D][lr:0x4bb05] bt rslt : 0
```

## 技术支持

您可以通过以下方法获得涂鸦的支持:

- TuyaOS 论坛： https://www.tuyaos.com
- 开发者中心： https://developer.tuya.com
- 帮助中心： https://support.tuya.com/help
- 技术支持工单中心： https://service.console.tuya.com
- TuyaOS 文档中心: https://developer.tuya.com/cn/docs/iot-device-dev
