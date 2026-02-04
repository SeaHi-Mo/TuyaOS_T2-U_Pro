# 修改记录

| 版本 | 时间       | 修改内容 |
| ---- | ---------- | -------- |
| v0.1.0 | 2022.04.20 | 初版     |
| v0.2.0 | 2023.05.29 | 计量驱动新框架     |

# 基本功能

1. 支持 hlw8012, bl0937, hlw8032 和 bl0942 计量芯片
1. 支持hlw8012, bl0937 计量芯片校准


# 组件依赖

无


# 资源依赖

+ hlw8012, bl0937 : GPIO 中断，GPIO 输出，硬件定时器

+ hlw8032, bl0942 : uart

# 使用说明

## bl0937 和 hlw8012

对应 bl0937 和 hlw8012 的采样，可以通过修改 `tdd_energy_monitor_bl0937_hlw8012.c` 文件中宏进行调整。

+ 电流采样：可以修改 `CURRENT_SAMPLE_US_DEF` 和 `CURRENT_SAMPLE_US_MAX` 两个宏进行调整电流采样时间。当电流较小（220v下约 120mA 以下）时会使用 `CURRENT_SAMPLE_US_MAX` 配置的时间。如果你需要较快的转换数据可以将 `CURRENT_SAMPLE_US_DEF` 和 `CURRENT_SAMPLE_US_MAX`  适当的调小，但是其精度就会变低。如果你需要较高的精度可以将 `CURRENT_SAMPLE_US_DEF` 和 `CURRENT_SAMPLE_US_MAX`  调高。
+ 电压采样：默认为 1s，通过 `VOLTAGE_SAMPLE_US` 进行配置。需要注意的是，电流和电压都是 CF1 脚输出的所以电压和电流的更新时间为电流采样时间+电压采样时间。
+ 功率采样：可以通过修改 `POWER_SAMPLE_US` 和 `POWER_FAST_SAMPLE_US`  两个宏控制功率采样时间。当功率较小（bl0937 约为 240w，hlw8012约为1660w以下认为是功率较小）时采样时间最长为 `POWER_SAMPLE_US`。

