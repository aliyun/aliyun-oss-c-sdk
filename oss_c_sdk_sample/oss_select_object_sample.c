#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void select_object_to_buffer()
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
    oss_select_object_params_t *select_params = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, "oss-select");
    aos_str_set(&object, "city_sample_data.csv");
    aos_list_init(&buffer);

    aos_string_t select_sql;
    aos_str_set(&select_sql, "select * from ossobject limit 10");

    select_params = oss_create_select_object_params(p);

    s = oss_select_object_to_buffer(options, &bucket, &object,
        &select_sql, select_params, &buffer, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("get object to buffer succeeded\n");
    }
    else 
    {
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

void select_object_to_file()
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
    char *buf = NULL;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;
    oss_select_object_params_t *select_params = NULL;
    aos_string_t filename;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, "oss-select");
    aos_str_set(&object, "city_sample_data.csv");
    aos_str_set(&filename, "city_sample_data_part.csv");

    aos_string_t select_sql;
    aos_str_set(&select_sql, "select * from ossobject limit 10");

    select_params = oss_create_select_object_params(p);

    s = oss_select_object_to_file(options, &bucket, &object,
        &select_sql, select_params, &filename, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("get object to buffer succeeded\n");
    }
    else
    {
        printf("get object to buffer failed\n");
    }

    aos_pool_destroy(p);
}

void create_select_object_meta()
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
    oss_select_object_meta_params_t *meta_params = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, "oss-select");
    aos_str_set(&object, "city_sample_data.csv");

    meta_params = oss_create_select_object_meta_params(p);

    s = oss_create_select_object_meta(options, &bucket, &object,
        meta_params, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("get object to buffer succeeded\n");
    }
    else
    {
        printf("get object to buffer failed\n");
    }

    aos_pool_destroy(p);
}



void select_object_sample()
{
    select_object_to_buffer();
    select_object_to_file();
    create_select_object_meta();
}
