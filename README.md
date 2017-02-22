# Aliyun OSS SDK for C

[![GitHub version](https://badge.fury.io/gh/aliyun%2Faliyun-oss-c-sdk.svg)](https://badge.fury.io/gh/aliyun%2Faliyun-oss-c-sdk)
[![Build Status](https://travis-ci.org/aliyun/aliyun-oss-c-sdk.svg?branch=master)](https://travis-ci.org/aliyun/aliyun-oss-c-sdk)
[![Coverage Status](https://coveralls.io/repos/github/aliyun/aliyun-oss-c-sdk/badge.svg?branch=master)](https://coveralls.io/github/aliyun/aliyun-oss-c-sdk?branch=master)
[![Software License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](LICENSE)

## 关于
阿里云对象存储（Object Storage Service，简称OSS），是阿里云对外提供的海量、安全、低成本、高可靠的云存储服务。用户可以通过调用API，在任何应用、任何时间、任何地点上传和下载数据，也可以通过用户Web控制台对数据进行简单的管理。OSS适合存放任意文件类型，适合各种网站、开发企业及开发者使用。OSS C SDK提供了一系列接口方便用户使用OSS。

## 版本
 - 当前版本：3.4.0

## 安装方法
### 环境依赖
OSS C SDK使用curl进行网络操作，无论是作为客户端还是服务器端，都需要依赖curl。
另外，OSS C SDK使用apr/apr-util库解决内存管理以及跨平台问题，使用minixml库解析请求返回的xml，
OSS C SDK并没有带上这几个外部库，您需要确认这些库已经安装，并且将它们的头文件目录和库文件目录都加入到了项目中。

#### 第三方库下载以及安装

##### libcurl （建议 7.32.0 及以上版本）

  请从[这里](http://curl.haxx.se/download.html)下载，并参考[libcurl 安装指南](http://curl.haxx.se/docs/install.html)安装。典型的安装方式如下：
```shell
    ./configure
    make
    make install
```

注意：
 - 执行./configure时默认是配置安装目录为/usr/local/，如果需要指定安装目录，请使用 ./configure --prefix=/your/install/path/

##### apr （建议 1.5.2 及以上版本）

  请从[这里](https://apr.apache.org/download.cgi)下载，典型的安装方式如下：
 ```shell
    ./configure
    make
    make install
```

注意：
 - 执行./configure时默认是配置安装目录为/usr/local/，如果需要指定安装目录，请使用 ./configure --prefix=/your/install/path/

##### apr-util （建议 1.5.4 及以上版本）

  请从[这里](https://apr.apache.org/download.cgi)下载，安装时需要注意指定--with-apr选项，典型的安装方式如下：
```shell
    ./configure --with-apr=/your/apr/install/path
    make
    make install
```

注意：
 - 执行./configure时默认是配置安装目录为/usr/local/，如果需要指定安装目录，请使用 ./configure --prefix=/your/install/path/
 - 需要通过--with-apr指定apr安装目录，如果apr安装到系统目录下需要指定--with-apr=/usr/local/apr/

##### minixml （建议 2.8 及以上版本）

  请从[这里](http://www.msweet.org/downloads.php?L+Z3)下载，典型的安装方式如下:
```shell
    ./configure
    make
    make install
```


注意：
 - 执行./configure时默认是配置安装目录为/usr/local/，如果需要指定安装目录，请使用 ./configure --prefix=/your/install/path/

##### CMake (建议2.6.0及以上版本)

  请从[这里](https://cmake.org/download)下载，典型的安装方式如下：
```shell
    ./configure
    make
    make install
```

注意：
 - 执行./configure时默认是配置安装目录为/usr/local/，如果需要指定安装目录，请使用 ./configure --prefix=/your/install/path/

#### OSS C SDK的安装

  安装时请在cmake命令中指定第三方库头文件以及库文件的路径，典型的编译命令如下：
```shell
    cmake .
    make
    make install
```

注意：
 - 执行cmake . 时默认会到/usr/local/下面去寻找curl，apr，apr-util，mxml的头文件和库文件。
 - 默认编译是Debug类型，可以指定以下几种编译类型： Debug, Release, RelWithDebInfo和MinSizeRel，如果要使用release类型编译，则执行cmake . -DCMAKE_BUILD_TYPE=Release
 - 如果您在安装curl，apr，apr-util，mxml时指定了安装目录，则需要在执行cmake时指定这些库的路径，比如：
```shell
   cmake . -DCURL_INCLUDE_DIR=/usr/local/include/curl/ -DCURL_LIBRARY=/usr/local/lib/libcurl.a -DAPR_INCLUDE_DIR=/usr/local/include/apr-1/ -DAPR_LIBRARY=/usr/local/lib/libapr-1.a -DAPR_UTIL_INCLUDE_DIR=/usr/local/apr/include/apr-1 -DAPR_UTIL_LIBRARY=/usr/local/apr/lib/libaprutil-1.a -DMINIXML_INCLUDE_DIR=/usr/local/include -DMINIXML_LIBRARY=/usr/local/lib/libmxml.a
```
 - 如果要指定安装目录，则需要在cmake时增加： -DCMAKE_INSTALL_PREFIX=/your/install/path/usr/local/

## License
- MIT
 
## 联系我们
- [阿里云OSS官方网站](http://oss.aliyun.com)
- [阿里云OSS官方论坛](http://bbs.aliyun.com)
- [阿里云OSS官方文档中心](http://www.aliyun.com/product/oss#Docs)
- 阿里云官方技术支持：[提交工单](https://workorder.console.aliyun.com/#/ticket/createIndex)
