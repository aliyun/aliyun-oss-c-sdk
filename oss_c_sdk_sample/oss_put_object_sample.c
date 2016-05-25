#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void put_object_from_buffer()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    aos_list_t buffer;
    aos_buf_t *content = NULL;
    char *str = "test oss c sdk";
    aos_status_t *s = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    s = oss_put_object_from_buffer(options, &bucket, &object, 
                   &buffer, headers, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("put object from buffer succeeded\n");
    } else {
        printf("put object from buffer failed\n");      
    }    

    aos_pool_destroy(p);
}

void put_object_from_buffer_with_md5()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    aos_list_t buffer;
    unsigned char *md5 = NULL;
    char *buf = NULL;
    int64_t buf_len;
    char *b64_value = NULL;
    int b64_len;
    aos_buf_t *content = NULL;
    char *str = "test oss c sdk";
    aos_status_t *s = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers = aos_table_make(p, 2);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    //add Content-MD5
    buf_len = aos_buf_list_len(&buffer);
    buf = aos_buf_list_content(options->pool, &buffer);
    md5 = aos_md5(options->pool, buf, (apr_size_t)buf_len);
    b64_value = aos_pcalloc(options->pool, 50);
    b64_len = aos_base64_encode(md5, 20, b64_value);
    b64_value[b64_len] = '\0';
    apr_table_set(headers, OSS_CONTENT_MD5, b64_value);

    s = oss_put_object_from_buffer(options, &bucket, &object, 
                   &buffer, headers, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("put object from buffer with md5 succeeded\n");
    } else {
    printf("put object from buffer with md5 failed\n");
    }

    aos_pool_destroy(p);
}

void put_object_from_file()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    char *filename = __FILE__;
    aos_status_t *s = NULL;
    aos_string_t file;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers = aos_table_make(options->pool, 1);
    apr_table_set(headers, OSS_CONTENT_TYPE, "image/jpeg");
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    aos_str_set(&file, filename);

    s = oss_put_object_from_file(options, &bucket, &object, &file, 
                                 headers, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("put object from file succeeded\n");
    } else {
        printf("put object from file failed, code:%d, error_code:%s, error_msg:%s, request_id:%s\n",
            s->code, s->error_code, s->error_msg, s->req_id);
    }

    aos_pool_destroy(p);
}

void put_object_by_signed_url()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t url;
    int is_cname = 0;
    aos_http_request_t *request = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    char *filename = __FILE__;
    aos_status_t *s = NULL;
    aos_string_t file;
    char *signed_url = NULL;
    int64_t expires_time;

    aos_pool_create(&p, NULL);

    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);

    // create request
    request = aos_http_request_create(p);
    request->method = HTTP_PUT;

    // create headers
    headers = aos_table_make(options->pool, 0);

    // set value
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    aos_str_set(&file, filename);

    // expires time
    expires_time = apr_time_now() / 1000000 + 120;    

    // generate signed url for put 
    signed_url = oss_gen_signed_url(options, &bucket, &object, 
                                    expires_time, request);
    aos_str_set(&url, signed_url);
    
    printf("signed put url : %s\n", signed_url);

    // put object by signed url
    s = oss_put_object_from_file_by_url(options, &url, &file, 
                                        headers, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("put object by signed url succeeded\n");
    } else {
        printf("put object by signed url failed\n");
    }

    aos_pool_destroy(p);
}

void create_dir()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers = aos_table_make(options->pool, 0);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, DIR_NAME);
    aos_list_init(&buffer);

    s = oss_put_object_from_buffer(options, &bucket, &object, &buffer,
                                   headers, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("create dir succeeded\n");
    } else {
        printf("create dir failed\n");
    }

    aos_pool_destroy(p);
}

void put_object_to_dir()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    char *filename = __FILE__;
    char *key = NULL;
    aos_status_t *s = NULL;
    aos_string_t file;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers = aos_table_make(options->pool, 0);
    aos_str_set(&bucket, BUCKET_NAME);

    key = (char*)malloc(strlen(DIR_NAME) + strlen(OBJECT_NAME) + 1);
    strcpy(key, DIR_NAME);
    strcat(key, OBJECT_NAME);
    aos_str_set(&object, key);
    aos_str_set(&file, filename);

    s = oss_put_object_from_file(options, &bucket, &object, &file, 
                                 headers, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("put object to dir succeeded\n");
    } else {
        printf("put object to dir failed\n");
    }

    free(key);
    aos_pool_destroy(p);
}

void put_object_sample()
{
    put_object_from_buffer();
    put_object_from_file();
    put_object_by_signed_url();    

    create_dir();
    put_object_to_dir();
}
