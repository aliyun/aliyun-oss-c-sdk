#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void append_object_from_buffer_with_crc()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *str = "Time is a bird for ever on the wing.";
    aos_status_t *s = NULL;
    int is_cname = 0;
    int64_t position = 0;
    uint64_t initcrc = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t resp_body;
    oss_request_options_t *options = NULL;
    aos_list_t buffer;
    aos_buf_t *content = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    aos_list_init(&resp_body);

    /* append object */
    aos_list_init(&buffer);
    content = aos_buf_pack(p, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    oss_delete_object(options, &bucket, &object, NULL);

    s = oss_do_append_object_from_buffer(options, &bucket, &object, position, 
        initcrc, &buffer, headers, NULL, NULL, &resp_headers, &resp_body);
    if (aos_status_is_ok(s)) {
        printf("crc: append object from buffer succeeded\n");
    } else {
        printf("crc: append object from buffer failed\n");      
    } 

    position = aos_atoi64((char*)(apr_table_get(resp_headers, OSS_NEXT_APPEND_POSITION)));
    initcrc = aos_atoui64((char*)(apr_table_get(resp_headers, OSS_HASH_CRC64_ECMA)));

    /* append object */
    s = oss_do_append_object_from_buffer(options, &bucket, &object, position, 
        initcrc, &buffer, NULL, NULL, NULL, NULL, NULL);

    /* delete object */
    s= oss_delete_object(options, &bucket, &object, NULL);

    aos_pool_destroy(p);
}

void append_object_from_file_with_crc()
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_append_object.txt";
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    int64_t position = 0;
    uint64_t initcrc = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t resp_body;
    oss_request_options_t *options = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);

    aos_str_set(&filename, __FILE__);

    /* append object */
    s = oss_do_append_object_from_file(options, &bucket, &object, position, 
        initcrc, &filename, headers, NULL, NULL, &resp_headers, &resp_body);
    if (aos_status_is_ok(s)) {
        printf("crc: append object from file succeeded\n");
    } else {
        printf("crc: append object from file failed\n");      
    } 

    position = aos_atoi64((char*)(apr_table_get(resp_headers, OSS_NEXT_APPEND_POSITION)));
    initcrc = aos_atoui64((char*)(apr_table_get(resp_headers, OSS_HASH_CRC64_ECMA)));

    /* append object */
    s = oss_do_append_object_from_file(options, &bucket, &object, position, 
        initcrc, &filename, NULL, NULL, NULL, NULL, NULL);
    if (aos_status_is_ok(s)) {
        printf("crc: append object from file succeeded\n");
    } else {
        printf("crc: append object from file failed\n");      
    } 

    /* delete object */
    s= oss_delete_object(options, &bucket, &object, NULL);

    aos_pool_destroy(p);
}

void disable_crc() 
{
    aos_pool_t *p = NULL;
    char *str = "Sow nothing, reap nothing.";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    oss_request_options_t *options = NULL;
    aos_list_t resp_body;
    aos_list_t buffer;
    aos_buf_t *content;

    /* init test*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
   
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    aos_list_init(&resp_body);

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    options->ctl->options->enable_crc = AOS_FALSE;
    
    /* put object */
    s = oss_put_object_from_buffer(options, &bucket, &object, &buffer, NULL, NULL);
    if (aos_status_is_ok(s)) {
        printf("put object from buffer succeeded\n");
    } else {
        printf("put object from buffer failed\n");      
    } 

    aos_pool_destroy(p);

    /* get object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);

    options->ctl->options->enable_crc = AOS_FALSE;

    s = oss_get_object_to_buffer(options, &bucket, &object, NULL, NULL, &buffer, NULL);
    if (aos_status_is_ok(s)) {
        printf("get object to buffer succeeded\n");
    } else {
        printf("get object to buffer failed\n");      
    } 

    aos_pool_destroy(p);
}

void crc_sample()
{
    /* 说明：CRC校验功能默认打开，上传、下载默认启动CRC校验；append_object需要指定上次append返回的CRC */
    append_object_from_buffer_with_crc();
    append_object_from_file_with_crc();

    disable_crc();
}
