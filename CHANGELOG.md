# ChangeLog - Aliyun OSS SDK for C

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
 - 调整oss_get_object_to_buffer_by_url和oss_init_multipart_upload接口参数顺序
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




