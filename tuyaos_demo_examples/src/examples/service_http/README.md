# HTTP  简介
- example_http 例程演示了如何使用原生http接口。
- example_iot_http.c 演示如何从涂鸦云获取天气服务数据。以此来向开发者展示如何使用涂鸦 API 获取涂鸦云的数据。
  

**重要：在执行示例之前需要先调用 `example_soc_init` 进入配网模式，并使用涂鸦智能 APP 对设备完成配网后，方可调用。**

## http

### 运行结果
```c
[01-01 14:58:54 ty D][lr:0xadbd3] Connect: httpbin.org Port: 80  -->>
[01-01 14:58:54 ty D][lr:0x7a689] unw_gethostbyname httpbin.org, prio 1
[01-01 14:58:54 ty D][lr:0x7a7b5] use dynamic dns ip:44.194.147.17 for domain:httpbin.org
[01-01 14:58:54 ty D][lr:0xbc05d] bind ip:c0a81cc2 port:0 ok
[01-01 14:58:54 ty D][lr:0xadbed] Connect: httpbin.org Port: 80  --<< ,r:0
[01-01 14:58:54 ty I][example_http.c:126] rsp:{
  "args": {}, 
  "headers": {
    "Host": "httpbin.org", 
    "User-Agent": "TUYA_IOT_SDK", 
    "X-Amzn-Trace-Id": "Root=1-65308a26-2d1b68cd71ac3ac235a5020a"
  }, 
  "origin": "124.90.34.126", 
  "url": "h[01-01 14:58:54 ty D][lr:0xbc0fd] tcp transporter close socket fd:4
```
返回内容较多，日志只能显示前1K数据。

## iot http

**天气服务的云端 Api 说明请查看文档 weather_api.md**

### 运行结果

```c
[04-18 16:22:14 ty D][lr:0x7b2ff] Post Data: {"codes": ["w.currdate","w.humidity","w.conditionNum","w.pressure","w.uvi","w.windDir","w.windSpeed","w.sunrise","w.sunset","w.temp","c.city","c.area","t.local"],"t":1713428534}
[04-18 16:22:14 ty D][lr:0x7b33f] Post URL: https://a3.tuyacn.com/d.json?a=thing.weather.get&devId=6c43647ca9d249e0ee2n72&et=3&t=1713428534&v=1.0&sign=379b8600f18914ebfffe16cd45f12462
[04-18 16:22:14 ty D][lr:0xb2943] Connect: a3.tuyacn.com Port: 443  -->>
[04-18 16:22:14 ty D][lr:0x7ebb9] unw_gethostbyname a3.tuyacn.com, prio 1
[04-18 16:22:14 ty D][lr:0x7ece5] use dynamic dns ip:81.69.183.170 for domain:a3.tuyacn.com
[04-18 16:22:14 ty D][lr:0xbd4d5] bind ip:c0a81ce0 port:0 ok
[04-18 16:22:14 ty D][lr:0x7f351] MAX SECURITY_LEVEL:0, TUYA SECURITY_LEVEL:0, mode:0
[04-18 16:22:14 ty D][lr:0x7f36b] TUYA_TLS Begin Connect a3.tuyacn.com:443
[04-18 16:22:14 ty D][lr:0x7f3db] TUYA_TLS PSK Mode
[04-18 16:22:14 ty D][lr:0x7f461] socket fd is set. set to inner send/recv to handshake
[04-18 16:22:14 ty D][lr:0x7f4ab] handshake finish for a3.tuyacn.com. set send/recv to user set
[04-18 16:22:14 ty D][lr:0x7f4dd] TUYA_TLS Success Connect a3.tuyacn.com:443 Suit:TLS-PSK-WITH-AES-128-CBC-SHA256
[04-18 16:22:14 ty D][lr:0xb295d] Connect: a3.tuyacn.com Port: 443  --<< ,r:0
[04-18 16:22:14 ty D][lr:0x7babf] Decode Rev:{"result":{"data":{"w.humidity":50,"w.conditionNum":"120","w.pressure":995,"w.uvi":6,"w.windDir":"E","w.windSpeed":"2.5","w.sunrise":"2024-04-18 05:29","w.sunset":"2024-04-18 18:29","w.temp":24,"c.area":"西4-18 16:22:14 ty D][lr:0x83819] tls transporter close socket fd:9
[04-18 16:22:14 ty D][lr:0xbd575] tcp transporter close socket fd:9
[04-18 16:22:14 ty D][lr:0x83833] tls transporter close tls handler:0x417838
[04-18 16:22:14 ty D][lr:0x7f657] TUYA_TLS Disconnect ENTER
[04-18 16:22:14 ty D][lr:0x7f6a7] TUYA_TLS Disconnect Success
[04-18 16:22:15 ty D][lr:0x7f2e3] tuya_tls_connect_destroy.
[04-18 16:22:15 ty N][example_iot_http.c:58] {"data":{"w.humidity":50,"w.conditionNum":"120","w.pressure":995,"w.uvi":6,"w.windDir":"E","w.windSpeed":"2.5","w.sunrise":"2024-04-18 05:29","w.sunset":"2024-04-18 18:29","w.temp":24,"c.area":"西湖区","c.c
```

注意这里的天气服务返回的内容（内容为 json 格式）太多了，所以在日志中显示可能会不全。

## 技术支持

您可以通过以下方法获得涂鸦的支持:

- TuyaOS 论坛： https://www.tuyaos.com

- 开发者中心： https://developer.tuya.com

- 帮助中心： https://support.tuya.com/help

- 技术支持工单中心： https://service.console.tuya.com
