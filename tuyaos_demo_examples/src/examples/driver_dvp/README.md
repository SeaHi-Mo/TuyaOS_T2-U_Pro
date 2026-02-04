# DVP

该实例演示了如何从摄像头中筛选出来 I 帧，然后通过 ffmpeg 转换成 jpg 格式，使开发者可以看到摄像头采集到的信息。

## 硬件准备

+ T5AI 开发板
+ SD 卡
+ DVP 摄像头

## 使用步骤

调用 `example_dvp` 摄像头开始采集数据，然后存放到 tf 卡中。在将摄像头采集到的数据存放到 tf 卡中会有以下日志：

```
    [06-03 11:38:28 ty N][3fd][example_driver_dvp.c:209] File /sdcard/11_38_28 created successfully
    [06-03 11:38:28 ty N][3fd][example_driver_dvp.c:219] Data written to file /sdcard/11_38_28 successfully, length: 10248 bytes
```

采集结束后可以调用 `example_dvp_stop` 停止数据采样，并且推出 tf 卡。

之后需要借助于 ffmpeg 工具将采集到的数据转换成 jpg 格式。如果没有安装 ffmpeg 工具，可以使用以下命令安装：

```bash
sudo apt install ffmpeg -y
```

可以使用以下命令将采集到的数据转换成 jpg 格式：

```bash
ffmpeg -i 13_39_07 -vframes 1 -q:v 2 output.jpg
```

13_39_07 是采集到的数据文件名，output.jpg 是输出的 jpg 文件名。

## 技术支持

您可以通过以下方法获得涂鸦的支持:

- TuyaOS 论坛： https://www.tuyaos.com

- 开发者中心： https://developer.tuya.com

- 帮助中心： https://support.tuya.com/help

- 技术支持工单中心： https://service.console.tuya.com
