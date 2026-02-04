# Speaker

## 简介

Speaker（扬声器）是一种常见的输出设备，用于将电信号转换为声音信号。本 demo 演示了如何从代码数组和内部 flash 中读取 mp3 文件，然后每 3s 会进行一次 MP3 解码播放。使用的 MP3 音频文件应和设置的 spk_sample 一致， 可以使用 https://convertio.co/zh/ 网站在线进行音频格式和频率转换。

## 制作文件系统

制作文件系统的工具位于 `tuyaos_demo_examples/tools/littlefs` 目录下。具体使用方法可以查看 `tuyaos_demo_examples/tools/littlefs` 下的使用说明和如何烧录文件系统镜像文件文件。这里需要注意的是，在 TuyaOS 中 T5 内部 flash 的文件系统起始于 `0x600000` ，结束地址位于  `0x7cb000` ，长度为 `1880064`。

所以这里进行文件系统的打包命令为（mklittlefs 是一个 linux 工具）：

```shell
./mklittlefs -c tuya/ -b 4096 -p 256 -s 1880064 tuya.bin
```

你这里也可直接使用 `tuyaos_demo_examples/src/examples/driver_speaker/fs` 下的 `tuya.bin` 。`tuya.bin` 中的文件内容如下：

```
/
└── media
    └── hello_tuya.mp3
```

然后即可对 tuya.bin 进行烧录，注意 T5 模组起始地址为  `0x600000` 。

## 使用方法

### 1. 使用代码中的 MP3 文件

将 `MP3_FILE_SOURCE` 宏修改如下：

```
#define MP3_FILE_SOURCE     USE_C_ARRAY
```

### 2. 使用内部文件系统中的 MP3 文件

将 `MP3_FILE_SOURCE` 宏修改如下：

```
#define MP3_FILE_SOURCE     USE_INTERNAL_FLASH
```

> 注意：需要提前将音频文件烧录到 flash 中。镜像制作和烧录方法参考上面的制作文件系统。

### 3. 声音调节

声音调节函数为：

```c
OPERATE_RET tkl_ao_set_vol(INT32_T card, TKL_AO_CHN_E chn, VOID *handle, INT32_T vol);
```

通过设置 `vol` 进行参数修改。如：30% 音量配置如下：

```
tkl_ao_set_vol(TKL_AUDIO_TYPE_BOARD, 0, NULL, 30);
```

