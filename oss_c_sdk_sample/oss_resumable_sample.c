#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void resumable_upload_with_multi_threads()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t resp_body;
    oss_request_options_t *options = NULL;
    oss_resumable_clt_params_t *clt_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, "my_key_1.zip");
    aos_str_set(&filename, "local_big_file.zip");
    aos_list_init(&resp_body);

    // upload
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_FALSE, NULL);
    s = oss_resumable_upload_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers, &resp_body);

    if (aos_status_is_ok(s)) {
        printf("upload succeeded\n");
    } else {
        printf("upload failed\n");
    }

    aos_pool_destroy(p);
}

void resumable_upload_with_resumable()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t resp_body;
    oss_request_options_t *options = NULL;
    oss_resumable_clt_params_t *clt_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, "my_key_2.zip");
    aos_str_set(&filename, "local_big_file.zip");
    aos_list_init(&resp_body);

    // upload
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_TRUE, NULL);
    s = oss_resumable_upload_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers, &resp_body);

    if (aos_status_is_ok(s)) {
        printf("upload succeeded\n");
    } else {
        printf("upload failed\n");
    }

    aos_pool_destroy(p);
}

void resumable_upload_sample()
{
    resumable_upload_with_multi_threads();
    resumable_upload_with_resumable();
}

void resumable_download_with_multi_threads()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    oss_resumable_clt_params_t *clt_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, "my_key_1.zip");
    aos_str_set(&filename, "local_big_file_1.zip");

    // download
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_FALSE, NULL);
    s = oss_resumable_download_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("download succeeded\n");
    } else {
        printf("download failed\n");
    }

    aos_pool_destroy(p);
}

void resumable_download_with_resumable()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    oss_resumable_clt_params_t *clt_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, "my_key_2.zip");
    aos_str_set(&filename, "local_big_file_2.zip");

    // download
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_TRUE, NULL);
    s = oss_resumable_download_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("download succeeded\n");
    } else {
        printf("download failed\n");
    }

    aos_pool_destroy(p);
}


void resumable_download_sample()
{
    resumable_download_with_multi_threads();
    resumable_download_with_resumable();
}
