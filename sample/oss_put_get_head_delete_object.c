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
    aos_pool_t *p;
    aos_string_t bucket;
    aos_string_t object;
    int is_oss_domain = 1;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    oss_request_options_t *options;
    aos_list_t buffer;
    aos_buf_t *content;
    char *object_name = "oss_put_and_get_object";
    char *str = "test oss c sdk";
    aos_status_t *s;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_oss_domain);
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, object_name);

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    s = oss_put_object_from_buffer(options, &bucket, &object, 
				   &buffer, headers, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("put object from buffer succeeded\n");
    }
    else {
	printf("put object from buffer failed\n");      
    }    

    aos_pool_destroy(p);
}

void get_object_to_buffer()
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *object_name = "oss_put_and_get_object";
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
    aos_str_set(&object, object_name);
    headers = aos_table_make(p, 0);
    aos_list_init(&buffer);

    s = oss_get_object_to_buffer(options, &bucket, &object, headers, &buffer, &resp_headers);

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

void head_object()
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *object_name = "oss_put_and_get_object";
    aos_string_t object;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_status_t *s;
    char *user_meta;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_oss_domain);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, object_name);
    headers = aos_table_make(p, 0);

    s = oss_head_object(options, &bucket, &object, headers, &resp_headers);
    user_meta = (char*)(apr_table_get(resp_headers, "x-oss-meta-author"));

    if (NULL != s && 2 == s->code / 100 && 0 == strcmp("oss", user_meta)) {
        printf("head object succeeded\n");
    }
    else {
        printf("head object failed\n");
    }

    aos_pool_destroy(p);
}

void delete_object()
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *object_name = "oss_put_and_get_object";
    aos_string_t object;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_table_t *resp_headers;
    aos_status_t *s;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_oss_domain);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, object_name);

    s = oss_delete_object(options, &bucket, &object, &resp_headers);
    if (NULL != s && 204 == s->code) {
        printf("delete object succeeded\n");
    }
    else {
        printf("delete object failed\n");
    }

    aos_pool_destroy(p);
}

int main(int argc, char *argv[])
{
    //aos_http_io_initialize first 
    if (aos_http_io_initialize("oss_sample", 0) != AOSE_OK) {
        exit(1);
    }

    put_object_from_buffer();
    get_object_to_buffer();
    head_object();
    delete_object();

    //aos_http_io_deinitialize last
    aos_http_io_deinitialize();

    return 0;
}
