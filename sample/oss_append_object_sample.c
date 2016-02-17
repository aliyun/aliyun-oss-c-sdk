#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void append_object_from_buffer()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *str = "test oss c sdk";
    aos_status_t *s = NULL;
    int is_cname = 0;
    int64_t position = 0;
    aos_table_t *headers1 = NULL;
    aos_table_t *headers2 = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    aos_list_t buffer;
    aos_buf_t *content = NULL;
    char *next_append_position = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers1 = aos_table_make(p, 0);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    s = oss_head_object(options, &bucket, &object, headers1, &resp_headers);
    if (NULL != s && s->code == 200) {
        next_append_position = (char*)(apr_table_get(resp_headers, 
                        "x-oss-next-append-position"));
        position = atoi(next_append_position);
    }

    headers2 = aos_table_make(p, 0);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);
    s = oss_append_object_from_buffer(options, &bucket, &object, 
            position, &buffer, headers2, &resp_headers);
    
    if (NULL != s && 2 == s->code / 100)
    {
        printf("append object from buffer succeeded\n");
    } else {
        printf("append object from buffer failed\n");
    }

    aos_pool_destroy(p);
}

void append_object_from_file()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    aos_table_t *headers1 = NULL;
    aos_table_t *headers2 = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    char *filename = __FILE__;
    aos_status_t *s = NULL;
    aos_string_t file;
    int64_t position = 0;
    char *next_append_position = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers1 = aos_table_make(options->pool, 0);
    headers2 = aos_table_make(options->pool, 0);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    aos_str_set(&file, filename);

    s = oss_head_object(options, &bucket, &object, headers1, &resp_headers);
    if(NULL != s && 2 == s->code / 100) {
        next_append_position = (char*)(apr_table_get(resp_headers, 
                        "x-oss-next-append-position"));
        position = atoi(next_append_position);
    }

    s = oss_append_object_from_file(options, &bucket, &object, 
                                    position, &file, headers2, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("append object from file succeeded\n");
    } else {
        printf("append object from file failed\n");
    }    

    aos_pool_destroy(p);
}

int main(int argc, char *argv[])
{   
    if (aos_http_io_initialize(0) != AOSE_OK) {
        exit(1);
    }

    append_object_from_buffer();
    append_object_from_file();

    aos_http_io_deinitialize();

    return 0;
}
