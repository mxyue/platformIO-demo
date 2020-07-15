
## 本项目主要功能

1. 检查室内室外光照强度
2. 检查人是否躺在床上
3. 检查卧室内是否有人
4. 爬取家里路由，判断指定用户是否已经连接wifi，(通过判断用户手机的连接，可以判断用户是否在房屋内)

## 配置文件

### 完善配置 `data/config.json`

``` json
{
  "username": "iot-xxx",
  "password": "xxx",
  "houseId": "xxx",
  "deviceNo": "xxx",
  "wifi": [
    {"ssid": "xxx", "password": "xxx"}
  ],
  "ap": {
    "ssid": "xxx",
    "password": "xxx"
  }
}
```