## 智能家居控制

项目使用了platform io进行编译。

每个项目里data目录下的config.json是连接wifi，mqtt等需要的配置。需要完善


如果要用Arduino IDE，只需进行少量修改即可兼容。

1. 将各个项目中 src的内容拷贝到同 ino结尾文件名相同的文件夹中。

	如：`esp8266-mp3/src`文件夹下面有 esp8266_mp3.ino, 则新建文件夹 esp8266_mp3, 并将src内所有的文件拷贝到esp8266_mp3中

2. 拷贝 data文件夹到项目文件夹下

	如： `esp8266_mp3/data`


