# ONVIF Camera Server

这个工程基于开源项目[onvif_srvd](https://github.com/KoynovStas/onvif_srvd)开发, 用于在MaixCAM2上运行支持ONVIF服务的IPC摄像头

This project is based on the open source project [onvif_srvd](https://github.com/KoynovStas/onvif_srvd), which is used to run an ONVIF service IPC camera on MaixCAM2


运行命令参考:
```shell
./onvif_server  --ifs wlan0 \
--scope onvif://www.onvif.org/name/MaixCAM2 \
--scope onvif://www.onvif.org/Profile/M \
--manufacturer sipeed \
--firmware_ver v4.12.5 \
--model MaixCAM2 \
--serial_num 12345678 \
--hardware_id 11:22:33:44:55 \
--name RTSP \
--width 800 \
--height 600 \
--url rtsp://192.168.10.79:8554/live \
--type JPEG \
--log_file out.log \
--no_fork \
--port 8080
```

关于generated文件, 该文件是基于onvif_srvd生成的, 请勿修改