#ifndef OSS_TEST_UTIL_H
#define OSS_TEST_UTIL_H

#include "CuTest.h"
#include "aos_http_io.h"
#include "aos_string.h"
#include "aos_transport.h"
#include "aos_status.h"
#include "oss_define.h"

OSS_CPP_START

#define test_object_base() do {                                         \
        aos_str_set(&bucket, bucket_name);                              \
        aos_str_set(&object, object_name);                              \
    } while(0)

void make_rand_string(aos_pool_t *p, int len, aos_string_t *data);

aos_buf_t *make_random_buf(aos_pool_t *p, int len);

void make_random_body(aos_pool_t *p, int count, aos_list_t *bc);

int make_random_file(aos_pool_t *p, const char *filename, int len);

void init_test_config(oss_config_t *config, int is_cname);

void init_test_request_options(oss_request_options_t *options, int is_cname);

aos_status_t * create_test_bucket(const oss_request_options_t *options,
    const char *bucket_name, oss_acl_e oss_acl);

aos_status_t *create_test_object(const oss_request_options_t *options, const char *bucket_name, 
    const char *object_name, const char *data, aos_table_t *headers);

aos_status_t *create_test_object_from_file(const oss_request_options_t *options, const char *bucket_name,
    const char *object_name, const char *filename, aos_table_t *headers);

aos_status_t *delete_test_object(const oss_request_options_t *options,
    const char *bucket_name, const char *object_name);

aos_status_t *init_test_multipart_upload(const oss_request_options_t *options, const char *bucket_name, 
    const char *object_name, aos_string_t *upload_id);

aos_status_t *abort_test_multipart_upload(const oss_request_options_t *options, const char *bucket_name,
    const char *object_name, aos_string_t *upload_id);

aos_status_t *create_test_live_channel(const oss_request_options_t *options, const char *bucket_name,
    const char *live_channel);

aos_status_t *delete_test_live_channel(const oss_request_options_t *options, const char *bucket_name,
    const char *live_channel);

char *gen_test_signed_url(const oss_request_options_t *options, const char *bucket_name,
    const char *object_name, int64_t expires, aos_http_request_t *req);

unsigned long get_file_size(const char *file_path);

char *decrypt(const char *encrypted_str, aos_pool_t *pool);

void percentage(int64_t consumed_bytes, int64_t total_bytes);
void progress_callback(int64_t consumed_bytes, int64_t total_bytes);

OSS_CPP_END

#endif
