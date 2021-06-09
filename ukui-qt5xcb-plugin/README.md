## ukui-qt5xcb-plugin

ukui-qt5xcb-plugin 是UKUI桌面环境集成的Qt平台插件, 主要用于Qt应用的分数缩放

### 项目安装依赖

1. 执行：`git clone http://gitlab2.com/kylin-desktop/ukui-qt5xcb-plugin.git` 下载代码
2. 执行: `sudo mk-build-deps -ir debian/control` 会安装项目所有依赖

### 项目操作

1. 项目打包

- 根目录下执行如下命令，在上层目录生成 `deb` 文件

```shell
debuild
```

2. 项目安装

上层目录生成 `deb` 二进制文件后，进入上层目录，执行 `sudo dpkg -i ./ukui-qt5xcb-plugin_xxxx.deb`

安装完成后，Qt5.12.8环境就可以实现分数缩放了


