# espeasylib

### 🚀 ESP32 & ESP8266 Web Mgmt.
### ⚡ Only 3 lines of code.
### ✨ WiFi, MQTT, OTA, LittleFS. So easy!

```cpp
#include "espeasylib.h"
void setup(){ setup1(); }
void loop(){ loop1(); }
```

#### espeasylib, 集成常用功能, 简化到只需三行代码, 就可以通过web页面实现WiFi连接, OTA更新, 文件管理, MQTT服务, 大幅降低入门难度. 万事开头难, 迈出第一步就成功了一半, 该库就是为了解决使用esp系列模块开头难的问题, 帮你完成大部分基础工作，快速迈出第一步。

#### 克隆该项目到你的arduino项目文件夹%USERPROFILE%\Documents\Arduino\libraries就可以使用了. 

#### 上传程序，首次启动会以espid建立一个wifi信号, 手机连接该信号会自动弹出配置页面, 搜索WIFI，会返回附近WiFi列表, 找到自己的路由器输入密码连接, 连接成功后会自动返回首页, 给开发板重新设置SSID和PASS以确保安全性, 设置一个足够复杂的MQTT主题, 避免与他人设备冲突, 点save会保存配置信息到开发板上, 点删除或者格式化会删除配置信息。

![alt text](https://github.com/aiplayuser/espeasylib/blob/main/espeasylib.png)

#### 首次上传程序需要使用usb数据线通过com口上传, 以后只需要导出已编译的二进制文件, 直接拖拽到选择文件的地方, 开发板会自动上传文件更新系统. 

#### mqtt.html是一个简单的mqtt消息收发页面, 相当于一个web客户端, 已经预置多个常用按钮，可以自定义修改, 点search会搜索该主题下的所有设备, 把mqtt.html上传至手机或者电脑, 用浏览器打开,收藏或者添加到桌面, 方便下次使用. 

![alt text](https://github.com/aiplayuser/espeasylib/blob/main/mqtt.png)

#### GPIO(0)是板载的flash按钮, 这里用作复位, 长按3秒清除配置文件. 

#### 代码很简单, 请仔细查看, 避免重复构建, 常用的库已经导入, 常用对象已经建立, web服务已经启动, 直接使用即可. 

#### 该代码仅仅为了新手快速入门，优化规范问题请自行解决。

#### 开发板管理器版本为esp8266-3.1.1，esp32_2.0.12，不同版本间会有不兼容问题，请保持版本一致。
