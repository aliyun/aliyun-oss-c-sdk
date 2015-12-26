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
    aos_pool_t *p;
    aos_string_t bucket;
    aos_string_t object;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_status_t *s;
    aos_list_t buffer;
    aos_buf_t *content;
    char *buf;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_oss_domain);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    headers = aos_table_make(p, 0);
    aos_list_init(&buffer);

    s = oss_get_object_to_buffer(options, &bucket, &object, 
                                 headers, &buffer, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("get object to buffer succeeded\n");
    }
    else {
        printf("get object to buffer failed\n");  
    }

    //get buffer len
    aos_list_for_each_entry(content, &buffer, node) {
        len += aos_buf_size(content);
    }

    buf = aos_pcalloc(p, len + 1);
    buf[len] = '\0';

    //copy buffer content to memory
    aos_list_for_each_entry(content, &buffer, node) {
        size = aos_buf_size(content);
        memcpy(buf + pos, content->pos, size);
        pos += size;
    }

    aos_pool_destroy(p);
}

void get_object_to_local_file()
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *download_filename = "get_object_to_local_file.txt";
    aos_string_t object;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_status_t *s;
    aos_string_t file;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_oss_domain);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    headers = aos_table_make(p, 0);
    aos_str_set(&file, download_filename);

    s = oss_get_object_to_file(options, &bucket, &object, headers, 
                               &file, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("get object to local file succeeded\n");
    } else {
        printf("get object to local file failed\n");
    }

    aos_pool_destroy(p);
}

void get_object_by_signed_url()
{
    aos_pool_t *p;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t url;
    int is_oss_domain = 1;
    aos_http_request_t *request = NULL;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    oss_request_options_t *options;
    aos_list_t buffer;
    aos_status_t *s;
    aos_string_t file;
    char *signed_url = NULL;
    int64_t expires_time;

    aos_pool_create(&p, NULL);

    options = oss_request_options_create(p);
    init_sample_request_options(options, is_oss_domain);

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
    s = oss_get_object_to_buffer_by_url(options, &url, headers, 
            &buffer, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("get object by signed url succeeded\n");
    } else {
	printf("get object by signed url failed\n");
    }

    aos_pool_destroy(p);
}

int main(int argc, char *argv[])
{
    //aos_http_io_initialize first 
    if (aos_http_io_initialize("oss_sample", 0) != AOSE_OK) {
        exit(1);
    }
    
    get_object_to_buffer();
    get_object_to_local_file();
    get_object_by_signed_url();

    //aos_http_io_deinitialize last
    aos_http_io_deinitialize();

    return 0;
}
