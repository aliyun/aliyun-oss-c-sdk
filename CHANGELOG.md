# ChangeLog - Aliyun OSS SDK for C
## 版本号：3.7.1 日期：2019-04-05
### 变更内容
- 修复：断点续传下载进度条更新不正确的问题

## 版本号：3.7.0 日期：2019-02-23
### 变更内容
- 添加：支持oss_get_object_meta
- 添加：支持oss_put_object_acl，oss_get_object_acl
- 添加：支持windows x64 平台
- 优化: 完善测试代码 
- 添加：支持windows 下 使用cmake 方式构建
- 添加：添加curl debug 日志信息
- 添加：支持select object

## 版本号：3.6.0 日期：2018-04-19
### 变更内容
- 添加：create_bucket支持指定存储类型，支持oss_restore_object
- 添加：支持symlink，oss_put_symlink和oss_get_symlink
- 添加：存储空间设置logging，lifecycle，website，referer，cors
- 添加：支持oss_list_bucket，oss_get_bucket_location
- 添加：支持oss_get_bucket_location，oss_get_bucket_info

## 版本号：3.5.2 日期：2017-11-14
### 变更内容
- 修复：oss_resumable_upload_file、oss_resumable_download_file不支持STS鉴权方式的问题

## 版本号：3.5.1 日期：2017-08-11
### 变更内容
- 修复：`apr_file_info_get`在特定文件系统下报`70008 APR_INCOMPLETE`错误的问题
- 修复：`oss_delete_objects_by_prefix`在中的`params->next_marker`使用释放后的内存的问题
- 修复：Windows的`minixml`库升级到2.9

## 版本号：3.5.0 日期：2017-08-01
### 变更内容
- 添加：支持并发断点续传下载`oss_resumable_download_file`
- 修复：`aos_should_retry`重试判读错误的问题

## 版本号：3.4.3 日期：2017-04-26
### 变更内容
- 修复：添加宏`ULLONG_MAX`的定义
- 修复：示例工程的CMakeLists中加入`oss_resumable_sample.c`
- 修复：`oss_open_checkpoint_file`错误打印日志的问题

## 版本号：3.4.2 日期：2017-04-23
### 变更内容
- 修复：解决分片上传ContentType被覆盖的问题

## 版本号：3.4.1 日期：2017-04-07
### 变更内容
- 添加：list_object使用示例`oss_list_object_sample.c`
- 修复：CMakeLists中加入`oss_resumable.h`

## 版本号：3.4.0 日期：2017-02-22
### 变更内容
- 添加：支持并发断点续传上传`oss_resumable_upload_file`
- 修复：`oss_gen_signed_url`支持临时用户签名
- 修复：初始化默认不打开`fd 2`，退出时不关闭`fd 2`
- 修复：修复key为`xxx/./yyy/`，`./async_test/test`报`SignatureDoesNotMatch`的问题

## 版本号：3.3.0 日期：2016-12-28
### 变更内容
 - 添加：支持代理Proxy
 - 修复：oss_get_object_to_file先下载到本地临时文件，成功后修改文件名称
 - 修复：去除Visual Studio编译警告aos_util.c(512) C4146
 - 修复：URL上传下载添加CRC校验

## 版本号：3.2.1 日期：2016-11-21
### 变更内容
- 解决oss_copy_object源文件名没有url编码的问题

## 版本号：3.2.0 日期：2016-11-14
### 变更内容
 - 支持上传、下载[CRC](https://github.com/aliyun/aliyun-oss-c-sdk/blob/master/oss_c_sdk_test/test_oss_crc.c)检验
 - 支持[上传回调](https://github.com/aliyun/aliyun-oss-c-sdk/blob/master/oss_c_sdk_test/test_oss_callback.c)功能
 - 支持[进度条](https://github.com/aliyun/aliyun-oss-c-sdk/blob/master/oss_c_sdk_test/test_oss_progress.c)功能

## 版本号：3.1.0 日期：2016-08-10
### 变更内容
 - 支持[RTMP](https://github.com/aliyun/aliyun-oss-c-sdk/blob/master/oss_c_sdk_test/test_oss_live.c)功能
 - 支持[图片服务](https://github.com/aliyun/aliyun-oss-c-sdk/blob/master/oss_c_sdk_test/test_oss_image.c)功能

## 版本号：3.0.0 日期：2016-05-24
### 变更内容
 - Windows和Linux版本合并
 
## 版本号：2.1.0 日期：2016-03-28
### 变更内容
 - 完善示例程序
 - header长度由限制为1K升级为最长8K
 - 解决部分单词拼写错误

## 版本号：2.0.0 日期：2016-03-08
### 变更内容
 - complete multipart接口支持覆盖原有head
 - 重构示例程序和组织方式
 - 开放params参数，允许用户自定义设置
 - 允许params和headers参数为空，简化使用
 - 支持https
 - 支持ip
 - 新增部分测试
 - 新增oss_put_bucket_acl接口
 - 新增目录相关示例
 - 新增signed url相关示例
 - 完善接口注释
 - 删除无用的port配置参数
 - 调整oss_init_multipart_upload接口参数顺序
 - 优化配置参数名称，使其与官方网站保持一致
 - 解决endpoint不能含有http等前缀的问题
 - 解决用户无法设置content-type的问题
 - 解决无法自动根据file name和key设置content-type的问题
 - 解决list upload parts为空时coredump的问题
 - 解决oss_upload_file接口在断点续传时可能会coredump的问题
 - 解决部分单词拼写错误
 - 解决所有警告
 - 解决部分头文件宏保护无效的问题
 - 解决oss_head_object_by_url接口不生效的问题

## 版本号：1.0.0 日期：2015-12-16
### 变更内容
 - 调整OSS C SDK依赖的XML第三方库，使用minixml替换libxml减小OSS C SDK的大小
 - 修改编译方式为CMAKE，同时提供嵌入式环境的Makefile.embeded，减少automake重复编译的问题
 - 新增oss_upload_file接口，封装multipart upload相关的接口，使用multipart方式上传文件
 - 新增oss_delete_objects_by_prefix接口，删除指定prefix的object
 - 新增OSS C SDK根据object name或者filename自动添加content_type
 - 完善OSS C SDK demo的调用示例，方便用户快速入门

## 版本号：0.0.7 日期：2015-11-11
### 变更内容
 - OSS C SDK修复sts_token超过http header最大限制的问题

## 版本号：0.0.6 日期：2015-10-29
### 变更内容
 - OSS C SDK签名时请求头支持x-oss-date，允许用户指定签名时间，解决系统时间偏差导致签名出错的问题
 - OSS C SDK支持CNAME方式访问OSS，CNAME方式请求时指定is_oss_domain值为0
 - 新增OSS C SDK demo,提供简单的接口调用示例，方便用户快速入门
 - OSS C SDK sample示例中去除对utf8第三方库的依赖

## 版本号：0.0.5 日期：2015-09-10
### 变更内容
 - 调整OSS C SDK获取GMT时间的方式，解决LOCALE变化可能导致签名出错的问题
 - aos_status_t结构体增加req_id字段，方便定位请求出错问题

## 版本号：0.0.4 日期：2015-07-27
### 变更内容
 - 增加生命周期相关的接口oss_put_bucket_lifecycle、oss_get_bucket_lifecycle以及oss_delete_bucket_lifecycle
 - OSS C SDK支持长连接，默认使用连接池支持keep alive功能
 - oss_list_object增加子目录的输出

## 版本号：0.0.3 日期：2015-07-08
### 变更内容
 - 增加oss_append_object_from_buffer接口，支持追加上传buffer中的内容到object
 - 增加oss_append_object_from_file接口，支持追加上传文件中的内容到object

## 版本号：0.0.2 日期：2015-06-10
### 变更内容
 - 增加oss_upload_part_copy，支持Upload Part Copy方式拷贝
 - 增加sts服务临时授权方式访问OSS

## 版本号：0.0.1 日期：2015-05-28
### 变更内容
 - 基于OSS API文档，提供OSS bucket、object以及multipart相关的常见操作API
 - 提供基于CuTest的sample




