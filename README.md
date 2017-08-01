# Alibaba Cloud OSS SDK for C

[![GitHub Version](https://badge.fury.io/gh/aliyun%2Faliyun-oss-c-sdk.svg)](https://badge.fury.io/gh/aliyun%2Faliyun-oss-c-sdk)
[![Build Status](https://travis-ci.org/aliyun/aliyun-oss-c-sdk.svg?branch=master)](https://travis-ci.org/aliyun/aliyun-oss-c-sdk)
[![Coverage Status](https://coveralls.io/repos/github/aliyun/aliyun-oss-c-sdk/badge.svg?branch=master)](https://coveralls.io/github/aliyun/aliyun-oss-c-sdk?branch=master)
[![Software License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](LICENSE)

## [README of Chinese](https://github.com/aliyun/aliyun-oss-c-sdk/blob/master/README-CN.md)

## About
Alibaba Cloud Object Storage Service (OSS) is a cloud storage service provided by Alibaba Cloud, featuring massive capacity, security, a low cost, and high reliability. You can upload and download data on any application anytime and anywhere by calling APIs, and perform simple management of data through the web console. The OSS can store any type of files and therefore applies to various websites, development enterprises and developers. The OSS C SDK provides a variety of interfaces for convenient use of the OSS. 

## Version
 - Current version: 3.5.0. 

## Install OSS C SDK
### Environment dependency
The OSS C SDK adopts cURL for network operations on both clients and servers.
OSS C SDK uses the APR and APR-Util libraries for memory management and cross-platform operations, and uses the Mini-XML library for parsing XML returned by a request.
These external libraries are not included in the OSS C SDK. You need to install these libraries and add their header file directories and the library file directories to the project.

#### Download and install third-party libraries

##### libcurl (Version 7.32.0 or above is recommended)

  Download from [here](http://curl.haxx.se/download.html) and install it by referring to [libcurl Installation Guide](http://curl.haxx.se/docs/install.html). A typical installation approach is as follows:
```shell
    ./configure
    make
    make install
```

Notes:
 - When you run the ./configure command, the default installation directory is /usr/local/. To specify another installation directory, use ./configure --prefix=/your/install/path/.

##### APR (Version 1.5.2 or above is recommended)

  Download from [here](https://apr.apache.org/download.cgi). A typical installation method is as follows:
 ```shell
    ./configure
    make
    make install
```

Notes:
 - When you run the ./configure command, the default installation directory is /usr/local/. To specify another installation directory, use ./configure --prefix=/your/install/path/.

##### APR-Util (Version 1.5.4 or above is recommended)

  Download from [here](https://apr.apache.org/download.cgi). The --with-apr option must be specified during installation. A typical installation method is as follows:
```shell
    ./configure --with-apr=/your/apr/install/path
    make
    make install
```

Notes:
 - When you run the ./configure command, the default installation directory is /usr/local/. To specify another installation directory, use ./configure --prefix=/your/install/path/.
 - You need to specify the APR installation directory through --with-apr. To install APR under a system directory, specify --with-apr=/usr/local/apr/.

##### Mini-XML (Version 2.8 or above is recommended)

  Download from [here](http://www.msweet.org/downloads.php?L+Z3). A typical installation method is as follows:
```shell
    ./configure
    make
    make install
```


Notes:
 - When you run the ./configure command, the default installation directory is /usr/local/. To specify another installation directory, use ./configure --prefix=/your/install/path/.

##### CMake (Version 2.6.0 or above is recommended)

  Download from [here](https://cmake.org/download). A typical installation method is as follows:
```shell
    ./configure
    make
    make install
```

Notes:
 - When you run the ./configure command, the default installation directory is /usr/local/. To specify another installation directory, use ./configure --prefix=/your/install/path/.

#### Install OSS C SDK

  Specify the third-party library header file and library file paths in the cmake command during installation. A typical compilation command is as follows: 
```shell
    cmake .
    make
    make install
```

Notes:
 - When you run the cmake command, the header files and library files of cURL, APR, APR-Util and Mini-XML will be searched in the directory /usr/local/ by default.
 - The default compilation is of the Debug type and you can specify the following types of compilation: Debug, Release, RelWithDebInfo and MinSizeRel. To use the Release compilation type, run the command cmake . -DCMAKE_BUILD_TYPE=Release.
 - If You have specified installation directories for cURL, APR, APR-Util and Mini-XML, you need to specify the paths of these libraries when running CMake. For example, 
```shell
   cmake . -DCURL_INCLUDE_DIR=/usr/local/include/curl/ -DCURL_LIBRARY=/usr/local/lib/libcurl.a -DAPR_INCLUDE_DIR=/usr/local/include/apr-1/ -DAPR_LIBRARY=/usr/local/lib/libapr-1.a -DAPR_UTIL_INCLUDE_DIR=/usr/local/apr/include/apr-1 -DAPR_UTIL_LIBRARY=/usr/local/apr/lib/libaprutil-1.a -DMINIXML_INCLUDE_DIR=/usr/local/include -DMINIXML_LIBRARY=/usr/local/lib/libmxml.a
```
 - To specify an installation directory, add the following when running CMake: -DCMAKE_INSTALL_PREFIX=/your/install/path/usr/local/。

## License
- MIT
 
## Contact us
- [Alibaba Cloud OSS official website](http://oss.aliyun.com).
- [Alibaba Cloud OSS official forum](http://bbs.aliyun.com).
- [Alibaba Cloud OSS official documentation center](http://www.aliyun.com/product/oss#Docs).
- Alibaba Cloud official technical support: [Submit a ticket](https://workorder.console.aliyun.com/#/ticket/createIndex).
