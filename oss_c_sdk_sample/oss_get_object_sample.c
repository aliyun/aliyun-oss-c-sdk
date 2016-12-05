#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void get_object_to_buffer()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;
    aos_buf_t *content = NULL;
    char *buf = NULL;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    aos_list_init(&buffer);

    s = oss_get_object_to_buffer(options, &bucket, &object, 
                                 headers, params, &buffer, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("get object to buffer succeeded\n");
    }
    else {
        printf("get object to buffer failed\n");  
    }

    //get buffer len
    aos_list_for_each_entry(aos_buf_t, content, &buffer, node) {
        len += aos_buf_size(content);
    }

    buf = aos_pcalloc(p, (apr_size_t)(len + 1));
    buf[len] = '\0';

    //copy buffer content to memory
    aos_list_for_each_entry(aos_buf_t, content, &buffer, node) {
        size = aos_buf_size(content);
        memcpy(buf + pos, content->pos, (size_t)size);
        pos += size;
    }

    aos_pool_destroy(p);
}

void get_object_to_local_file()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *download_filename = "get_object_to_local_file.txt";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t file;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    headers = aos_table_make(p, 0);
    aos_str_set(&file, download_filename);

    s = oss_get_object_to_file(options, &bucket, &object, headers, 
                               params, &file, &resp_headers);
    if (aos_status_is_ok(s)) {
        printf("get object to local file succeeded\n");
    } else {
        printf("get object to local file failed\n");
    }

    aos_pool_destroy(p);
}

void get_object_to_buffer_with_range()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;
    aos_buf_t *content = NULL;
    char *buf = NULL;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    aos_list_init(&buffer);
    headers = aos_table_make(p, 1);

    /* 设置Range，读取文件的指定范围，bytes=20-100包括第20和第100个字符 */
    apr_table_set(headers, "Range", "bytes=20-100");

    s = oss_get_object_to_buffer(options, &bucket, &object, 
                                 headers, params, &buffer, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("get object to buffer succeeded\n");
    }
    else {
        printf("get object to buffer failed\n");  
    }

    //get buffer len
    aos_list_for_each_entry(aos_buf_t, content, &buffer, node) {
        len += aos_buf_size(content);
    }

    buf = aos_pcalloc(p, (apr_size_t)(len + 1));
    buf[len] = '\0';

    //copy buffer content to memory
    aos_list_for_each_entry(aos_buf_t, content, &buffer, node) {
        size = aos_buf_size(content);
        memcpy(buf + pos, content->pos, (size_t)size);
        pos += size;
    }

    aos_pool_destroy(p);
}

void get_object_to_local_file_with_range()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *download_filename = "get_object_to_local_file.txt";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t file;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    aos_str_set(&file, download_filename);
    headers = aos_table_make(p, 1);

    /* 设置Range，读取文件的指定范围，bytes=20-100包括第20和第100个字符 */
    apr_table_set(headers, "Range", "bytes=20-100");

    s = oss_get_object_to_file(options, &bucket, &object, headers, 
                               params, &file, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("get object to local file succeeded\n");
    } else {
        printf("get object to local file failed\n");
    }

    aos_pool_destroy(p);
}

void get_object_by_signed_url()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t url;
    int is_cname = 0;
    aos_http_request_t *request = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;    
    char *signed_url = NULL;
    int64_t expires_time;

    aos_pool_create(&p, NULL);

    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);

    // create request
    request = aos_http_request_create(p);
    request->method = HTTP_GET;

    // create headers
    headers = aos_table_make(options->pool, 0);

    // set value
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    aos_list_init(&buffer);

    // expires time
    expires_time = apr_time_now() / 1000000 + 120;    

    // generate signed url for put 
    signed_url = oss_gen_signed_url(options, &bucket, &object, 
                                    expires_time, request);
    aos_str_set(&url, signed_url);
    
    printf("signed get url : %s\n", signed_url);

    // put object by signed url
    s = oss_get_object_to_buffer_by_url(options, &url, 
            headers, params, &buffer, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("get object by signed url succeeded\n");
    } else {
        printf("get object by signed url failed\n");
    }

    aos_pool_destroy(p);
}

void get_oss_dir_to_local_dir()
{
    aos_pool_t *parent_pool = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    oss_list_object_params_t *params = NULL;

    aos_pool_create(&parent_pool, NULL);
    options = oss_request_options_create(parent_pool);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    params = oss_create_list_object_params(parent_pool);
    aos_str_set(&params->prefix, DIR_NAME);
    params->truncated = 1;

    while (params->truncated) {
        aos_pool_t *list_object_pool = NULL;
        aos_table_t *list_object_resp_headers = NULL;
        oss_list_object_content_t *list_content = NULL;
        
        aos_pool_create(&list_object_pool, parent_pool);
        options->pool = list_object_pool;
        s = oss_list_object(options, &bucket, params, &list_object_resp_headers);
        if (!aos_status_is_ok(s)) {
            aos_error_log("list objects of dir[%s] fail\n", DIR_NAME);
            aos_status_dup(parent_pool, s);
            aos_pool_destroy(list_object_pool);
            options->pool = parent_pool;
            return;
        }        

        aos_list_for_each_entry(oss_list_object_content_t, list_content, &params->object_list, node) {
            if ('/' == list_content->key.data[strlen(list_content->key.data) - 1]) {
                apr_dir_make_recursive(list_content->key.data, 
                        APR_OS_DEFAULT, parent_pool);                
            } else {
                aos_string_t object;
                aos_pool_t *get_object_pool = NULL;
                aos_table_t *headers = NULL;
                aos_table_t *query_params = NULL;
                aos_table_t *get_object_resp_headers = NULL;

                aos_str_set(&object, list_content->key.data);

                aos_pool_create(&get_object_pool, parent_pool);
                options->pool = get_object_pool;

                s = oss_get_object_to_file(options, &bucket, &object, 
                        headers, query_params, &object, &get_object_resp_headers);
                if (!aos_status_is_ok(s)) {
                    aos_error_log("get object[%s] fail\n", object.data);
                }

                aos_pool_destroy(get_object_pool);
                options->pool = list_object_pool;
            }
        }

        aos_list_init(&params->object_list);
        if (params->next_marker.data) {
            aos_str_set(&params->marker, params->next_marker.data);
        }

        aos_pool_destroy(list_object_pool);
    }

    if (aos_status_is_ok(s)) {
        printf("get dir succeeded\n");
    } else {
        printf("get dir failed\n");
    }
    aos_pool_destroy(parent_pool);
}

void get_object_sample()
{
    get_object_to_buffer();
    get_object_to_local_file();

    get_object_to_buffer_with_range();
    get_object_to_local_file_with_range();

    get_object_by_signed_url();

    get_oss_dir_to_local_dir();
}
