# 设备天气接口（云端）文档



## 接口介绍

| 字段        | 说明                                                         |
| ----------- | ------------------------------------------------------------ |
| Api         | thing.weather.get                                            |
| Api Version | 1.0                                                          |
| 接口名称    | 设备端获取天气接口                                           |
| 详细描述    | 通过经纬度获取天气数据，经纬度取值优先级依次是：家庭位置->设备配网位置->设备配网ip |



## 接口示例

### 实时天气

- **入参**

  ```javascript
  {
   "codes":["w.currdate","w.humidity","w.conditionNum","w.pressure","w.uvi","w.windDir","w.windSpeed","w.sunrise","w.sunset","w.temp","c.city","c.area","t.local"]
  }
  ```

- **出参**

  ```javascript
  {
    "result": {
      "data": {
        "w.humidity": 89,
        "w.conditionNum": "132",
        "w.pressure": 998,
        "w.uvi": 6,
        "w.windDir": "NE",
        "w.windSpeed": "1.8",
        "w.sunrise": "2024-04-07 05:41",
        "w.sunset": "2024-04-07 18:22",
        "w.temp": 13,
        "c.area": "西湖区",
        "c.city": "杭州"
      },
      "expiration": 25
    },
    "t": 1712454111,
    "e": false,
    "success": true
  }
  ```

### 预报天气

最高支持获取 7 天的预报

- **入参**

  ```javascript
  {
      "codes": ["w.date.2","w.conditionNum","w.pressure","w.uvi","w.windDir","w.windSpeed","w.sunrise","w.sunset","c.city","c.area","t.local"]
  }
  ```

- **出参**

  ```javascript
  {
    "result": {
      "data": {
        "w.conditionNum.0": "122",
        "w.pressure.0": 998,
        "w.uvi.0": 6,
        "w.windDir.0": "NE",
        "w.windSpeed.0": "2.4",
        "w.sunRise.0": "2024-04-07 05:41",
        "w.sunSet.0": "2024-04-07 18:22",
        "w.conditionNum.1": "122",
        "w.uvi.1": 1,
        "w.windDir.1": "NE",
        "w.windSpeed.1": "2.4",
        "w.sunRise.1": "2024-04-08 05:40",
        "w.sunSet.1": "2024-04-08 18:22",
        "c.area": "西湖区",
        "c.city": "杭州"
      },
      "expiration": 24
    },
    "t": 1712454165,
    "e": false,
    "success": true
  }
  ```



## 入参说明

### w.x 支持的天气查询字段

| 标识         | 描述                                   | 类型   | 举例               | 国内实时 | 国内预报 | 国外实时 | 国外预报 |
| ------------ | -------------------------------------- | ------ | ------------------ | -------- | -------- | -------- | -------- |
| temp         | 温度：(单位：℃）                       | 整数   | 11                 | ✅        | ❌        | ✅        | ✅        |
| humidity     | 湿度                                   | 整数   | 32                 | ✅        | ✅        | ✅        | ✅        |
| conditionNum | 天气概况数字编码                       | 字符串 | "104"              | ✅        | ✅        | ✅        | ✅        |
| pressure     | 气压：(单位：mbar）                    | 整数   |                    | ✅        | ❌        | ✅        | ✅        |
| uvi          | 紫外线指数                             | 整数   |                    | ✅        | ✅        | ✅        | ✅        |
| windDir      | 风向                                   | 字符串 | "N"                | ✅        | ✅        | ✅        | ✅        |
| windSpeed    | 风速：(单位：m/s）                     | 字符串 | "1.0"              | ✅        | ✅        | ✅        | ✅        |
| sunrise      | 日出时间 (默认当地时区)                | 字符串 | "2022-10-20 05:24" | ✅        | ✅        | ✅        | ✅        |
| sunset       | 日落时间 (默认当地时区)                | 字符串 | "2022-10-20 18:32" | ✅        | ✅        | ✅        | ✅        |
| realFeel     | 温度实感 (单位：℃）                    | 整数   |                    | ✅        | ❌        | ✅        | ❌        |
| windLevel    | 风等级                                 | 整数   |                    | ✅        | ❌        | ❌        | ❌        |
| aqi          | 空气质量指数 (美国 EPA 标准：0 ~ 500） | 整数   |                    | ✅        | ❌        | ✅        | ✅        |
| pm25         | pm2.5 浓度 (单位：µg/m³)               | 整数   |                    | ✅        | ❌        | ✅        | ✅        |
| so2          | 二氧化硫浓度 (单位：µg/m³)             | 整数   |                    | ✅        | ❌        | ✅        | ✅        |
| pm10         | pm10 浓度 (单位：µg/m³)                | 整数   |                    | ✅        | ❌        | ✅        | ✅        |
| o3           | o3 浓度 (单位：µg/m³)                  | 整数   |                    | ✅        | ❌        | ✅        | ✅        |
| no2          | no2 浓度 (单位：µg/m³)                 | 整数   |                    | ✅        | ❌        | ✅        | ✅        |
| co           | co 浓度 (单位：µg/m³)                  | 整数   |                    | ✅        | ❌        | ✅        | ✅        |
| thigh        | 最高温度 (单位：℃）                    | 整数   |                    | ❌        | ✅        | ❌        | ✅        |
| tlow         | 最低温度 (单位：℃）                    | 整数   |                    | ❌        | ✅        | ❌        | ✅        |

**说明：**

1. 不传 w.date.*、 w.currdate时, 默认只返回实时天气。
2. 传w.date.*, 返回预报天气。
3. 传w.currdate, 返回实时天气。
4. 同时传w.date.*、w.currdate, 返回预报+实时,  不推荐，返回结果顺序会被打乱乱序，请分两次请求。

### t.x 时区选择器，默认 UTC 时区

| local | 使用用户本地时区展示时间 | 需要配合日出日落查询条件使用 | 不区分实时和预报 |
| ----- | ------------------------ | ---------------------------- | ---------------- |

### c.x 城市，支持查询的城市属性

| name   | 如果有城市，优先使用城市，没有城市使用区县，没有区县使用省份 |
| ------ | ------------------------------------------------------------ |
| city   | 城市名称                                                     |
| area   | 区县名称                                                     |
| zoneId | 时区                                                         |



## 出参说明

### conditionNum 枚举

| conditonNum | desc           | conditon （设备和面板） | mark （App 客户端） |
| ----------- | -------------- | ----------------------- | ------------------- |
| 101         | 大雨           | HEAVY_RAIN              | Rainy               |
| 102         | 雷暴           | THUNDERSTORM            | Rainy               |
| 103         | 沙尘暴         | SANDSTORM               | Sunny               |
| 104         | 小雪           | LIGHT_SNOW              | Snow                |
| 105         | 雪             | SNOW                    | Snow                |
| 106         | 冻雾           | FROZEN_FOG              | Snow                |
| 107         | 暴雨           | RAINSTORM               | Rainy               |
| 108         | 局部阵雨       | SCATTERED_SHOWERS       | Rainy               |
| 109         | 浮尘           | DUST                    | Sunny               |
| 110         | 雷电           | LIGHTNING               | Rainy               |
| 111         | 小阵雨         | LIGHT_SHOWERS           | Rainy               |
| 112         | 雨             | RAIN                    | Rainy               |
| 113         | 雨夹雪         | SLEET                   | Snow                |
| 114         | 尘卷风         | DUST_STORM              | Sunny               |
| 115         | 冰粒           | ICY                     | Snow                |
| 116         | 强沙尘暴       | HEAVY_SANDSTORM         | Rainy               |
| 117         | 扬沙           | SAND                    | Sunny               |
| 118         | 小到中雨       | SMALL_TO_MEDIUM_RAIN    | Rainy               |
| 119         | 大部晴朗       | MOSTLY_SUNNY            | Cloud               |
| 120         | 晴             | SUNNY                   | Sunny               |
| 121         | 雾             | FOG                     | Fog                 |
| 122         | 阵雨           | SHOWERS                 | Rainy               |
| 123         | 强阵雨         | HEAVY_SHOWERS           | Rainy               |
| 124         | 大雪           | HEAVY_SNOW              | Snow                |
| 125         | 特大暴雨       | EXTREME_RAINSTORM       | Rainy               |
| 126         | 暴雪           | BLIZZARD                | Snow                |
| 127         | 冰雹           | HAIL                    | Snow                |
| 128         | 小到中雪       | SMALL_TO_MEDIUM_SNOW    | Snow                |
| 129         | 少云           | PARTLY_CLOUDY           | Cloud               |
| 130         | 小阵雪         | LIGHT_SNOW_SHOWERS      | Snow                |
| 131         | 中雪           | MEDIUM_SNOW             | Snow                |
| 132         | 阴             | OVERCAST                | Cloud               |
| 133         | 冰针           | NEEDLE_ICE              | Snow                |
| 134         | 大暴雨         | HEAVY_RAINSTORM         | Rainy               |
| 136         | 雷阵雨伴有冰雹 | THUNDERSHOWER_WITH_HAIL | Snow                |
| 137         | 冻雨           | FREEZING_RAIN           | Rainy               |
| 138         | 阵雪           | SNOW_SHOWERS            | Rainy               |
| 139         | 小雨           | LIGHT_RAIN              | Rainy               |
| 140         | 霾             | HAZE                    | Cloud               |
| 141         | 中雨           | MEDIUM_RAIM             | Rainy               |
| 142         | 多云           | CLOUDY                  | Cloud               |
| 143         | 雷阵雨         | THUNDERSHOWER           | Rainy               |
| 144         | 中到大雨       | MEDIUM_TO_BIG_RAIN      | Rainy               |
| 145         | 大到暴雨       | BIG_TO_HEAVY_RAIN       | Rainy               |
| 146         | 晴朗           | CLEAR                   | Clear               |

### expiration 过期时间

 下一次发起请求的间隔时间（单位：分钟），在此期间内请求的数据结果不变。如果在此间隔期内重复访问，会请求失败，返回错误码 "AJAX_BROWSE_UUID_REPEAT"。

```javascript
{
  "t": 1712454165,
  "errorCode": "AJAX_BROWSE_UUID_REPEAT",
  "success": false
}
```



### 更多参数

请参照 [参数说明](https://developer.tuya.com/cn/docs/iot/weather-function-description?id=Ka6dcs2cw4avp&from_wecom=1#title-12-附录一：天气数据参数 ASCII 码)。