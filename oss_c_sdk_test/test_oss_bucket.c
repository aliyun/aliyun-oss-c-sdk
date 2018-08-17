#include "CuTest.h"
#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_xml.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_test_util.h"

void test_bucket_setup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;
    char *object_name1 = "oss_test_object1";
    char *object_name2 = "oss_test_object2";
    char *object_name3 = "oss_tmp1/";
    char *object_name4 = "oss_tmp2/";
    char *object_name5 = "oss_tmp3/";
    char *object_name6 = "oss_tmp3/1";
    char *str = "test c oss sdk";
    aos_table_t *headers1 = NULL;
    aos_table_t *headers2 = NULL;
    aos_table_t *headers3 = NULL;
    aos_table_t *headers4 = NULL;
    aos_table_t *headers5 = NULL;
    int i = 0;

    //set log level, default AOS_LOG_WARN
    aos_log_set_level(AOS_LOG_WARN);

    //set log output, default stderr
    aos_log_set_output(NULL);

    //create test bucket
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);

    CuAssertIntEquals(tc, 200, s->code);
    CuAssertStrEquals(tc, NULL, s->error_code);
    TEST_CASE_LOG("create test bucket success!\n");

    //create test object
    headers1 = aos_table_make(p, 0);
    headers2 = aos_table_make(p, 0);
    headers3 = aos_table_make(p, 0);
    headers4 = aos_table_make(p, 0);
    headers5 = aos_table_make(p, 0);
    create_test_object(options, TEST_BUCKET_NAME, object_name1, str, headers1);
    create_test_object(options, TEST_BUCKET_NAME, object_name2, str, headers2);
    create_test_object(options, TEST_BUCKET_NAME, object_name3, str, headers3);
    create_test_object(options, TEST_BUCKET_NAME, object_name4, str, headers4);
    create_test_object(options, TEST_BUCKET_NAME, object_name5, str, headers5);
    for (i = 0; i < 1100; i++) {
        char *obj_name = apr_psprintf(p, "%s/%d.txt", object_name6, i);
        create_test_object(options, TEST_BUCKET_NAME, obj_name, str, NULL);
    }

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

void test_bucket_cleanup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t prefix;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);

    // delete objects
    aos_str_set(&prefix, "oss_test_object");
    oss_delete_objects_by_prefix(options, &bucket, &prefix);
    
    aos_str_set(&prefix, "oss_tmp1");
    oss_delete_objects_by_prefix(options, &bucket, &prefix);

    aos_str_set(&prefix, "oss_tmp2");
    oss_delete_objects_by_prefix(options, &bucket, &prefix);

    aos_str_set(&prefix, "oss_tmp3");
    oss_delete_objects_by_prefix(options, &bucket, &prefix);

    aos_pool_destroy(p);
}

void test_create_bucket(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    oss_acl_e oss_acl;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    oss_acl = OSS_ACL_PRIVATE;

    //create the same bucket twice with same bucket acl
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertStrEquals(tc, NULL, s->error_code);

    //create the same bucket with different bucket acl
    oss_acl = OSS_ACL_PUBLIC_READ;
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertStrEquals(tc, NULL, s->error_code);
    aos_pool_destroy(p);

    printf("test_create_bucket ok\n");
}

void test_create_bucket_with_storage_class(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    aos_string_t bucket;
    aos_table_t *resp_headers = NULL;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;
    oss_storage_class_type_e storage_class_tp = OSS_STORAGE_CLASS_IA;
    char IA_BUCKET_NAME[128] = {0};
    apr_snprintf(IA_BUCKET_NAME, 127, "%s-ia", TEST_BUCKET_NAME);

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    aos_str_set(&bucket, IA_BUCKET_NAME);
    oss_delete_bucket(options, &bucket, &resp_headers);
    apr_sleep(apr_time_from_sec(5));

    //create the bucket with storage class
    s = create_test_bucket_with_storage_class(options, IA_BUCKET_NAME, oss_acl, storage_class_tp);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertStrEquals(tc, NULL, s->error_code);

     //create the same bucket again with different storage class
    s = create_test_bucket_with_storage_class(options, IA_BUCKET_NAME, 
                                            oss_acl, OSS_STORAGE_CLASS_ARCHIVE);
    // 409: BucketAlreadyExists Cannot modify existing bucket's storage class
    CuAssertIntEquals(tc, 409, s->code);

     //create the same bucket again with different storage class
    s = create_test_bucket_with_storage_class(options, IA_BUCKET_NAME, 
                                            oss_acl, OSS_STORAGE_CLASS_STANDARD);
    // 409: BucketAlreadyExists Cannot modify existing bucket's storage class
    CuAssertIntEquals(tc, 409, s->code);

    //delete bucket 
    aos_str_set(&bucket, IA_BUCKET_NAME);
    s = oss_delete_bucket(options, &bucket, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

void test_delete_bucket(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    oss_acl_e oss_acl;
    int is_cname = 0;
    oss_request_options_t *options;
    aos_table_t *resp_headers = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    oss_acl = OSS_ACL_PUBLIC_READ;
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);

    //delete bucket not empty
    s = oss_delete_bucket(options, &bucket, &resp_headers);
    CuAssertIntEquals(tc, 409, s->code);
    CuAssertStrEquals(tc, "BucketNotEmpty", s->error_code);
    CuAssertTrue(tc, s->req_id != NULL);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("test_delete_bucket ok\n");
}

void test_put_bucket_acl(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_acl_e oss_acl;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    oss_acl = OSS_ACL_PUBLIC_READ_WRITE;
    s = oss_put_bucket_acl(options, &bucket, oss_acl, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    aos_pool_destroy(p);

    printf("test_put_bucket_acl ok\n");
}

/* only object has OSS_ACL_DEFAULT acl, it will return 400 while put bucket acl to OSS_ACL_DEFAULT */
void test_put_bucket_acl_invalid_acl(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_acl_e oss_acl;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    oss_acl = OSS_ACL_DEFAULT;
    s = oss_put_bucket_acl(options, &bucket, oss_acl, &resp_headers);
    CuAssertIntEquals(tc, 400, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    CuAssertStrEquals(tc, "InvalidArgument", s->error_code); 
    aos_pool_destroy(p);

    printf("test_put_bucket_acl_invalid_acl ok\n");
}

void test_get_bucket_acl(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t oss_acl;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_get_bucket_acl(options, &bucket, &oss_acl, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertStrEquals(tc, "public-read-write", oss_acl.data);
    CuAssertPtrNotNull(tc, resp_headers);
    aos_pool_destroy(p);

    printf("test_get_bucket_acl ok\n");
}

void test_get_bucket_location(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t oss_location;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_get_bucket_location(options, &bucket, &oss_location, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    TEST_CASE_LOG("endpoint: %s, location: %s\n", TEST_OSS_ENDPOINT, oss_location.data);
    CuAssertIntEquals(tc, 1, oss_location.len != 0);
    CuAssertPtrNotNull(tc, resp_headers);
    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_get_bucket_info(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_bucket_info_t bucket_info;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_get_bucket_info(options, &bucket, &bucket_info, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    TEST_CASE_LOG("endpoint: %s, location: %s\n", TEST_OSS_ENDPOINT, bucket_info.location.data);
    TEST_CASE_LOG("user id %s, name %s, \n", bucket_info.owner_id.data, bucket_info.owner_name.data);
    CuAssertTrue(tc, bucket_info.location.len != 0);
    CuAssertTrue(tc, bucket_info.acl.len != 0);
    CuAssertTrue(tc, bucket_info.created_date.len != 0);
    CuAssertTrue(tc, bucket_info.extranet_endpoint.len != 0);
    CuAssertTrue(tc, bucket_info.intranet_endpoint.len != 0);
    CuAssertTrue(tc, bucket_info.owner_id.len != 0);
    CuAssertTrue(tc, bucket_info.owner_name.len != 0);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_str_set(&bucket, "impossibleexistbucket");
    s = oss_get_bucket_info(options, &bucket, &bucket_info, &resp_headers);
    CuAssertIntEquals(tc, 404, s->code);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_get_bucket_stat(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_bucket_stat_t bucket_stat;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_get_bucket_stat(options, &bucket, &bucket_stat, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    TEST_CASE_LOG("storage %d, object count %d, multipart upload count %d\n", \
            (int)bucket_stat.storage_in_bytes, (int)bucket_stat.object_count, (int)bucket_stat.multipart_upload_count);
    CuAssertTrue(tc, bucket_stat.object_count > 0);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_head_bucket(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    char *oss_location = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_head_bucket(options, &bucket, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    oss_location = (char*)(apr_table_get(resp_headers, OSS_CANNONICALIZED_HEADER_REGION));
    TEST_CASE_LOG("region is %s\n", oss_location);
    CuAssertPtrNotNull(tc, oss_location);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_get_bucket_storage_capacity(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    long oss_storage_capacity = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_get_bucket_storage_capacity(options, &bucket, &oss_storage_capacity, &resp_headers);
    TEST_CASE_LOG("get storage capacity %ld\n", oss_storage_capacity);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_put_bucket_storage_capacity(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    long oss_storage_capacity = 0;
    long capacity = 100;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_put_bucket_storage_capacity(options, &bucket, capacity, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    aos_pool_destroy(p);
    TEST_CASE_LOG("set capacity %ld success\n", capacity);

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_get_bucket_storage_capacity(options, &bucket, &oss_storage_capacity, &resp_headers);
    TEST_CASE_LOG("get storage capacity %ld\n", oss_storage_capacity);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, capacity, oss_storage_capacity);
    CuAssertPtrNotNull(tc, resp_headers);
    aos_pool_destroy(p);

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_put_bucket_storage_capacity(options, &bucket, -1, &resp_headers);
    if (s->error_msg) {
        printf("%s %s\n", s->error_msg, s->error_code);
    }
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    aos_pool_destroy(p);

    printf("test_put_bucket_storage_capacity ok\n");
}

void test_put_bucket_logging(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_logging_config_content_t *content;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    content = oss_create_logging_rule_content(p);

    aos_str_set(&content->target_bucket, TEST_BUCKET_NAME);
    aos_str_set(&content->prefix, "my-log-");
    s = oss_put_bucket_logging(options, &bucket, content, &resp_headers);
    if (s->error_msg) {
        printf("%s %s\n", s->error_msg, s->error_code);
    }
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_get_bucket_logging(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_logging_config_content_t *content = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    content = oss_create_logging_rule_content(p);

    s = oss_get_bucket_logging(options, &bucket, content, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    CuAssertStrEquals(tc, TEST_BUCKET_NAME, content->target_bucket.data);
    CuAssertStrEquals(tc, "my-log-", content->prefix.data);

    TEST_CASE_LOG("%s: bucket:%s, prefix:%s\n", __FUNCTION__, 
                content->target_bucket.data, content->prefix.data);
    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_delete_bucket_logging(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_logging_config_content_t *content = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);

    s = oss_delete_bucket_logging(options, &bucket, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    content = oss_create_logging_rule_content(p);

    s = oss_get_bucket_logging(options, &bucket, content, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    CuAssertStrEquals(tc, "", content->target_bucket.data);
    CuAssertStrEquals(tc, "", content->prefix.data);

    TEST_CASE_LOG("%s: bucket:%s, prefix:%s\n", __FUNCTION__, 
                content->target_bucket.data, content->prefix.data);
 
    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_list_object(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_list_object_params_t *params = NULL;
    oss_list_object_content_t *content = NULL;
    int size = 0;
    char *key = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    params = oss_create_list_object_params(p);
    params->max_ret = 1;
    params->truncated = 0;
    aos_str_set(&params->prefix, "oss_test_object");
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_list_object(options, &bucket, params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 1, params->truncated);
    CuAssertStrEquals(tc, "oss_test_object1", params->next_marker.data);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_list_for_each_entry(oss_list_object_content_t, content, &params->object_list, node) {
        ++size;
        key = apr_psprintf(p, "%.*s", content->key.len, content->key.data);
    }
    CuAssertIntEquals(tc, 1 ,size);
    CuAssertStrEquals(tc, "oss_test_object1", key);
    
    size = 0;
    resp_headers = NULL;
    aos_list_init(&params->object_list);
    aos_str_set(&params->marker, params->next_marker.data);
    s = oss_list_object(options, &bucket, params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 0, params->truncated);
    aos_list_for_each_entry(oss_list_object_content_t, content, &params->object_list, node) {
        ++size;
        key = apr_psprintf(p, "%.*s", content->key.len, content->key.data);
    }
    CuAssertIntEquals(tc, 1 ,size);
    CuAssertStrEquals(tc, "oss_test_object2", key);
    CuAssertPtrNotNull(tc, resp_headers);
    aos_pool_destroy(p);

    printf("test_list_object ok\n");
}

void test_list_buckets(CuTest *tc)
{
    aos_pool_t *p = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_list_buckets_params_t *params = NULL;
    oss_list_bucket_content_t *content = NULL;
    int size = 0;

    TEST_CASE_LOG("begin test\n");
    /* list all buckets */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    params = oss_create_list_buckets_params(p);
    params->max_keys = 100;
    s = oss_list_bucket(options, params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_list_for_each_entry(oss_list_bucket_content_t, content, &params->bucket_list, node) {
        ++size;
    }
    TEST_CASE_LOG("Get %d buckets total\n", size);
    CuAssertIntEquals(tc, 1 , size > 0);
    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_list_buckets_with_invalid_prefix(CuTest *tc)
{
    aos_pool_t *p = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_list_buckets_params_t *params = NULL;

    /* list bucket one by one */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    params = oss_create_list_buckets_params(p);
    params->max_keys = 1;
    aos_str_set(&params->prefix, "impossibleMatch");

    s = oss_list_bucket(options, params, &resp_headers);
    CuAssertIntEquals(tc, 400, s->code);
    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_list_buckets_with_iterator(CuTest *tc)
{
    aos_pool_t *p = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_list_buckets_params_t *params = NULL;
    oss_list_bucket_content_t *content = NULL;
    oss_acl_e oss_acl;
    int match_num = 0;
    int size = 0;
    aos_string_t bucket;
    char BUCKET_NAME2[128] = {0};
    apr_snprintf(BUCKET_NAME2, 127, "%s-test-itor", TEST_BUCKET_NAME);

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    oss_acl = OSS_ACL_PRIVATE;
    //create the second bucket to iterate
    s = create_test_bucket(options, BUCKET_NAME2, oss_acl);
    if (s->error_code) {
        TEST_CASE_LOG("bucket name %s, %s %s\n", BUCKET_NAME2, s->error_code, s->error_msg);
    }
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertStrEquals(tc, NULL, s->error_code);

    /* list bucket one by one */
    params = oss_create_list_buckets_params(p);
    params->max_keys = 1;

    s = oss_list_bucket(options, params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    aos_list_for_each_entry(oss_list_bucket_content_t, content, &params->bucket_list, node) {
        TEST_CASE_LOG("Get bucket %s\n", content->name.data);
        if (!strcmp(content->name.data, TEST_BUCKET_NAME)
                || !strcmp(content->name.data, BUCKET_NAME2) ) {
            match_num++;
        }
        size++;
    }
    CuAssertIntEquals(tc, 1, size);

    while (params->truncated) {
        aos_list_init(&params->bucket_list);
        aos_str_set(&params->marker, params->next_marker.data);
        params->max_keys = 10;
        TEST_CASE_LOG("marker:%s, next marker: %s\n", params->marker.data, params->next_marker.data);
        s = oss_list_bucket(options, params, &resp_headers);
        CuAssertIntEquals(tc, 200, s->code);
        aos_list_for_each_entry(oss_list_bucket_content_t, content, &params->bucket_list, node) {
            TEST_CASE_LOG("Get bucket %s\n", content->name.data);
            if (!strcmp(content->name.data, TEST_BUCKET_NAME)
                    || !strcmp(content->name.data, BUCKET_NAME2) ) {
                match_num++;
            }
            size++;
        }
        CuAssertIntEquals(tc, 1, size > 1);
        if (size > 100) {
            if (match_num != 2) {
                TEST_CASE_LOG("not found expect bucket within %d buckets", size);
                match_num = 2;
            }
            break;
        }
    }

    CuAssertIntEquals(tc, 2, match_num);

    //delete bucket 
    aos_str_set(&bucket, BUCKET_NAME2);
    s = oss_delete_bucket(options, &bucket, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_list_object_with_delimiter(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_list_object_params_t *params = NULL;
    oss_list_object_common_prefix_t *common_prefix = NULL;
    int size = 0;
    char *prefix = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    params = oss_create_list_object_params(p);
    params->max_ret = 5;
    params->truncated = 0;
    aos_str_set(&params->prefix, "oss_tmp");
    aos_str_set(&params->delimiter, "/");
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_list_object(options, &bucket, params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 0, params->truncated);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_list_for_each_entry(oss_list_object_common_prefix_t, common_prefix, &params->common_prefix_list, node) {
        ++size;
        prefix = apr_psprintf(p, "%.*s", common_prefix->prefix.len, 
                              common_prefix->prefix.data);
        if (size == 1) {
            CuAssertStrEquals(tc, "oss_tmp1/", prefix);
        } else if (size == 2) {
            CuAssertStrEquals(tc, "oss_tmp2/", prefix);
        }
    }
    CuAssertIntEquals(tc, 3, size);
    aos_pool_destroy(p);

    printf("test_list_object_with_delimiter ok\n");
}

void test_lifecycle(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t lifecycle_rule_list;
    oss_lifecycle_rule_content_t *invalid_rule_content = NULL;
    oss_lifecycle_rule_content_t *rule_content = NULL;
    oss_lifecycle_rule_content_t *rule_content1 = NULL;
    oss_lifecycle_rule_content_t *rule_content2 = NULL;
    oss_lifecycle_rule_content_t *rule_content3 = NULL;
    oss_lifecycle_rule_content_t *rule_content4 = NULL;
    int size = 0;
    char *rule_id = NULL;
    char *prefix = NULL;
    char *status = NULL;
    int days = INT_MAX;
    char* date = NULL;
    char* created_before_date = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);

    //put invalid lifecycle rule
    aos_list_init(&lifecycle_rule_list);
    invalid_rule_content = oss_create_lifecycle_rule_content(p);
    aos_str_set(&invalid_rule_content->id, "");
    aos_str_set(&invalid_rule_content->prefix, "pre");
    aos_list_add_tail(&invalid_rule_content->node, &lifecycle_rule_list);
    s = oss_put_bucket_lifecycle(options, &bucket, &lifecycle_rule_list, 
                                 &resp_headers);
    CuAssertIntEquals(tc, 400, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    //put lifecycle
    resp_headers = NULL;
    aos_list_init(&lifecycle_rule_list);
    rule_content1 = oss_create_lifecycle_rule_content(p);
    aos_str_set(&rule_content1->id, "1");
    aos_str_set(&rule_content1->prefix, "pre1");
    aos_str_set(&rule_content1->status, "Enabled");
    rule_content1->days = 1;

    rule_content2 = oss_create_lifecycle_rule_content(p);
    aos_str_set(&rule_content2->id, "2");
    aos_str_set(&rule_content2->prefix, "pre2");
    aos_str_set(&rule_content2->status, "Enabled");
    aos_str_set(&rule_content2->date, "2022-10-11T00:00:00.000Z");

    rule_content3 = oss_create_lifecycle_rule_content(p);
    aos_str_set(&rule_content3->id, "3");
    aos_str_set(&rule_content3->prefix, "pre3");
    aos_str_set(&rule_content3->status, "Enabled");
    aos_str_set(&rule_content3->created_before_date, "2017-10-11T00:00:00.000Z");
    rule_content3->abort_multipart_upload_dt.days=1;

    rule_content4 = oss_create_lifecycle_rule_content(p);
    aos_str_set(&rule_content4->id, "4");
    aos_str_set(&rule_content4->prefix, "pre4");
    aos_str_set(&rule_content4->status, "Enabled");
    aos_str_set(&rule_content4->created_before_date, "2017-10-11T00:00:00.000Z");
    aos_str_set(&rule_content4->abort_multipart_upload_dt.created_before_date, "2012-10-11T00:00:00.000Z");
 
    aos_list_add_tail(&rule_content1->node, &lifecycle_rule_list);
    aos_list_add_tail(&rule_content2->node, &lifecycle_rule_list);
    aos_list_add_tail(&rule_content3->node, &lifecycle_rule_list);
    aos_list_add_tail(&rule_content4->node, &lifecycle_rule_list);

    s = oss_put_bucket_lifecycle(options, &bucket, &lifecycle_rule_list, 
                                 &resp_headers);
    if (s->error_msg) {
        TEST_CASE_LOG("%s %s\n", s->error_msg, s->error_code);
    }
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    //get lifecycle
    resp_headers = NULL;
    aos_list_init(&lifecycle_rule_list);
    s = oss_get_bucket_lifecycle(options, &bucket, &lifecycle_rule_list, 
                                 &resp_headers);
    if (s->error_msg) {
        printf("%s %s", s->error_msg, s->error_code);
    }
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_list_for_each_entry(oss_lifecycle_rule_content_t, rule_content, &lifecycle_rule_list, node) {
        if (size == 0) {
            rule_id = apr_psprintf(p, "%.*s", rule_content->id.len, 
                    rule_content->id.data);
            CuAssertStrEquals(tc, "1", rule_id);
            prefix = apr_psprintf(p, "%.*s", rule_content->prefix.len, 
                    rule_content->prefix.data);
            CuAssertStrEquals(tc, "pre1", prefix);
            date = apr_psprintf(p, "%.*s", rule_content->date.len, 
                    rule_content->date.data);
            CuAssertStrEquals(tc, "", date);
            status = apr_psprintf(p, "%.*s", rule_content->status.len, 
                    rule_content->status.data);
            CuAssertStrEquals(tc, "Enabled", status);
            days = rule_content->days;
            CuAssertIntEquals(tc, 1, days);
        }
        else if (size == 1){
            rule_id = apr_psprintf(p, "%.*s", rule_content->id.len, 
                    rule_content->id.data);
            CuAssertStrEquals(tc, "2", rule_id);
            prefix = apr_psprintf(p, "%.*s", rule_content->prefix.len, 
                    rule_content->prefix.data);
            CuAssertStrEquals(tc, "pre2", prefix);
            date = apr_psprintf(p, "%.*s", rule_content->date.len, 
                    rule_content->date.data);
            CuAssertStrEquals(tc, "2022-10-11T00:00:00.000Z", date);
            status = apr_psprintf(p, "%.*s", rule_content->status.len, 
                    rule_content->status.data);
            CuAssertStrEquals(tc, "Enabled", status);
            days = rule_content->days;
            CuAssertIntEquals(tc, INT_MAX, days);
        } else if (size == 2) {
            rule_id = apr_psprintf(p, "%.*s", rule_content->id.len, 
                    rule_content->id.data);
            CuAssertStrEquals(tc, "3", rule_id);

            prefix = apr_psprintf(p, "%.*s", rule_content->prefix.len, 
                    rule_content->prefix.data);
            CuAssertStrEquals(tc, "pre3", prefix);

            date = apr_psprintf(p, "%.*s", rule_content->date.len, 
                    rule_content->date.data);
            CuAssertStrEquals(tc, "", date);

            created_before_date = apr_psprintf(p, "%.*s", rule_content->created_before_date.len, 
                    rule_content->created_before_date.data);
            CuAssertStrEquals(tc, "2017-10-11T00:00:00.000Z", created_before_date);

            days = rule_content->abort_multipart_upload_dt.days;
            CuAssertIntEquals(tc, 1, days);

            status = apr_psprintf(p, "%.*s", rule_content->status.len, 
                    rule_content->status.data);
            CuAssertStrEquals(tc, "Enabled", status);
            days = rule_content->days;
            CuAssertIntEquals(tc, INT_MAX, days);
        } else if (size == 3) {
            rule_id = apr_psprintf(p, "%.*s", rule_content->id.len, 
                    rule_content->id.data);
            CuAssertStrEquals(tc, "4", rule_id);

            prefix = apr_psprintf(p, "%.*s", rule_content->prefix.len, 
                    rule_content->prefix.data);
            CuAssertStrEquals(tc, "pre4", prefix);

            created_before_date = apr_psprintf(p, "%.*s", rule_content->created_before_date.len, 
                    rule_content->created_before_date.data);
            CuAssertStrEquals(tc, "2017-10-11T00:00:00.000Z", created_before_date);

            created_before_date = apr_psprintf(p, "%.*s", 
                    rule_content->abort_multipart_upload_dt.created_before_date.len, 
                    rule_content->abort_multipart_upload_dt.created_before_date.data);
            CuAssertStrEquals(tc, "2012-10-11T00:00:00.000Z", created_before_date);

            days = rule_content->abort_multipart_upload_dt.days;
            CuAssertIntEquals(tc, INT_MAX, days);

            status = apr_psprintf(p, "%.*s", rule_content->status.len, 
                    rule_content->status.data);
            CuAssertStrEquals(tc, "Enabled", status);
            days = rule_content->days;
            CuAssertIntEquals(tc, INT_MAX, days);
        }

        ++size;
    }
    CuAssertIntEquals(tc, 4 ,size);

    //delete lifecycle
    resp_headers = NULL;
    s = oss_delete_bucket_lifecycle(options, &bucket, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    aos_pool_destroy(p);

    printf("test_lifecycle ok\n");
}

void test_put_bucket_website(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_website_config_t website_config;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);

    aos_str_set(&website_config.suffix_str, "index.html");
    aos_str_set(&website_config.key_str, "");

    s = oss_put_bucket_website(options, &bucket, &website_config, 
                               &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_str_set(&website_config.key_str, "errorDocument.html");
    s = oss_put_bucket_website(options, &bucket, &website_config, 
                               &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_get_bucket_website(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_website_config_t website_config;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_get_bucket_website(options, &bucket, &website_config, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertStrEquals(tc, "index.html", website_config.suffix_str.data);
    CuAssertStrEquals(tc, "errorDocument.html", website_config.key_str.data);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_delete_bucket_website(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_delete_bucket_website(options, &bucket, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_put_bucket_referer(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_referer_config_t referer_config;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);

    aos_list_init(&referer_config.referer_list);
    oss_create_and_add_refer(p, &referer_config, "http://www.aliyun.com");
    oss_create_and_add_refer(p, &referer_config, "https://www.aliyun.com");
    referer_config.allow_empty_referer = 0;

    s = oss_put_bucket_referer(options, &bucket, &referer_config, 
                               &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    referer_config.allow_empty_referer = 1;
    s = oss_put_bucket_referer(options, &bucket, &referer_config, 
                               &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_get_bucket_referer(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_referer_config_t referer_config;
    oss_referer_t *referer;
    int match_num = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_list_init(&referer_config.referer_list);
    s = oss_get_bucket_referer(options, &bucket, &referer_config, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 1, referer_config.allow_empty_referer);
    aos_list_for_each_entry(oss_referer_t, referer, &referer_config.referer_list, node) {
        TEST_CASE_LOG("get referer %s\n", referer->referer.data);
        if (!strcmp(referer->referer.data, "http://www.aliyun.com") 
            || !strcmp(referer->referer.data, "https://www.aliyun.com")) {
            match_num++;
        }
    }

    CuAssertIntEquals(tc, 2, match_num);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_put_bucket_cors(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t cors_rule_list;
    oss_cors_rule_t *cors_rule1 = NULL, *cors_rule2 = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);

    aos_list_init(&cors_rule_list);

    cors_rule1 = oss_create_cors_rule(p);
    aos_list_add_tail(&cors_rule1->node, &cors_rule_list);

    oss_create_sub_cors_rule(p, &cors_rule1->allowed_origin_list, "allowed_origin_1_1");
    oss_create_sub_cors_rule(p, &cors_rule1->allowed_origin_list, "allowed_origin_2_1");

    oss_create_sub_cors_rule(p, &cors_rule1->allowed_method_list, "PUT");
    oss_create_sub_cors_rule(p, &cors_rule1->allowed_method_list, "GET");

    oss_create_sub_cors_rule(p, &cors_rule1->allowed_head_list, "Authorization");

    oss_create_sub_cors_rule(p, &cors_rule1->expose_head_list, "expose_head_1_1");
    oss_create_sub_cors_rule(p, &cors_rule1->expose_head_list, "expose_head_2_1");

    cors_rule2 = oss_create_cors_rule(p);
    aos_list_add_tail(&cors_rule2->node, &cors_rule_list);

    oss_create_sub_cors_rule(p, &cors_rule2->allowed_origin_list, "allowed_origin_2_1");
    oss_create_sub_cors_rule(p, &cors_rule2->allowed_origin_list, "allowed_origin_2_2");

    oss_create_sub_cors_rule(p, &cors_rule2->allowed_method_list, "PUT");
    oss_create_sub_cors_rule(p, &cors_rule2->allowed_method_list, "GET");

    oss_create_sub_cors_rule(p, &cors_rule2->allowed_head_list, "Authorization");

    oss_create_sub_cors_rule(p, &cors_rule2->expose_head_list, "expose_head_2_1");
    oss_create_sub_cors_rule(p, &cors_rule2->expose_head_list, "expose_head_2_2");

    s = oss_put_bucket_cors(options, &bucket, &cors_rule_list, 
                                 &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    cors_rule1->max_age_seconds = 100000;
    s = oss_put_bucket_cors(options, &bucket, &cors_rule_list, 
                                 &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_get_bucket_cors(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t cors_rule_list;
    oss_cors_rule_t *cors_rule = NULL;
    oss_sub_cors_rule_t *sub_cors_rule = NULL;
    int size = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_list_init(&cors_rule_list);
    s = oss_get_bucket_cors(options, &bucket, &cors_rule_list, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    aos_list_for_each_entry(oss_cors_rule_t, cors_rule, &cors_rule_list, node) {
        TEST_CASE_LOG("%d\n", cors_rule->max_age_seconds);
        aos_list_for_each_entry(oss_sub_cors_rule_t, sub_cors_rule, &cors_rule->allowed_origin_list, node) {
            int is_match_put_before = !strncmp("allowed_origin", sub_cors_rule->rule.data, 14);
            CuAssertIntEquals(tc, 1, is_match_put_before);
            TEST_CASE_LOG("%s\n", sub_cors_rule->rule.data);
        }

        aos_list_for_each_entry(oss_sub_cors_rule_t, sub_cors_rule, &cors_rule->allowed_method_list, node) {
            int is_match_put_before = strcmp("PUT", sub_cors_rule->rule.data) 
                            || strcmp("GET", sub_cors_rule->rule.data);
            CuAssertIntEquals(tc, 1, is_match_put_before);
            TEST_CASE_LOG("%s\n", sub_cors_rule->rule.data);
        }

        aos_list_for_each_entry(oss_sub_cors_rule_t, sub_cors_rule, &cors_rule->allowed_head_list, node) {
            CuAssertStrEquals(tc, "authorization", sub_cors_rule->rule.data);
            TEST_CASE_LOG("%s\n", sub_cors_rule->rule.data);
        }

        aos_list_for_each_entry(oss_sub_cors_rule_t, sub_cors_rule, &cors_rule->expose_head_list, node) {
            int is_match_put_before = !strncmp("expose_head", sub_cors_rule->rule.data, 11);
            CuAssertIntEquals(tc, 1, is_match_put_before);
            TEST_CASE_LOG("%s\n", sub_cors_rule->rule.data);
        }
        size++;
    }

    CuAssertIntEquals(tc, 2, size);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_delete_bucket_cors(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_delete_bucket_cors(options, &bucket, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_delete_objects_quiet(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_status_t *s = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    char *object_name1 = "oss_test_object1";
    char *object_name2 = "oss_test_object2";
    oss_object_key_t *content1 = NULL;
    oss_object_key_t *content2 = NULL;
    aos_list_t object_list;
    aos_list_t deleted_object_list;
    int is_quiet = 1;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);

    aos_list_init(&object_list);
    aos_list_init(&deleted_object_list);
    content1 = oss_create_oss_object_key(p);
    aos_str_set(&content1->key, object_name1);
    aos_list_add_tail(&content1->node, &object_list);
    content2 = oss_create_oss_object_key(p);
    aos_str_set(&content2->key, object_name2);
    aos_list_add_tail(&content2->node, &object_list);

    s = oss_delete_objects(options, &bucket, &object_list, is_quiet,
        &resp_headers, &deleted_object_list);

    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    aos_pool_destroy(p);

    printf("test_delete_objects_quiet ok\n");
}

void test_delete_objects_not_quiet(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_status_t *s = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    char *object_name1 = "oss_tmp1/";
    char *object_name2 = "oss_tmp2/";
    oss_object_key_t *content = NULL;
    oss_object_key_t *content1 = NULL;
    oss_object_key_t *content2 = NULL;
    aos_list_t object_list;
    aos_list_t deleted_object_list;
    int is_quiet = 0;
    
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);

    aos_list_init(&object_list);
    aos_list_init(&deleted_object_list);
    content1 = oss_create_oss_object_key(p);
    aos_str_set(&content1->key, object_name1);
    aos_list_add_tail(&content1->node, &object_list);
    content2 = oss_create_oss_object_key(p);
    aos_str_set(&content2->key, object_name2);
    aos_list_add_tail(&content2->node, &object_list);
    
    s = oss_delete_objects(options, &bucket, &object_list, is_quiet, 
        &resp_headers, &deleted_object_list);

    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_list_for_each_entry(oss_object_key_t, content, &deleted_object_list, node) {
        printf("Deleted key:%.*s\n", content->key.len, content->key.data);
    }
    aos_pool_destroy(p);

    printf("test_delete_objects_not_quiet ok\n");
}

void test_delete_objects_by_prefix(CuTest *tc)
{
    aos_pool_t *p = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_status_t *s = NULL;
    aos_string_t prefix;
    
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);

    // delete none
    aos_str_set(&prefix, "oss_tmp3/2");

    s = oss_delete_objects_by_prefix(options, &bucket, &prefix);
    CuAssertIntEquals(tc, 200, s->code);

    // delete one object
    aos_str_set(&prefix, "oss_tmp3/1/0.txt");

    s = oss_delete_objects_by_prefix(options, &bucket, &prefix);
    CuAssertIntEquals(tc, 200, s->code);

    // delete multi-objects
    aos_str_set(&prefix, "oss_tmp3/1");

    s = oss_delete_objects_by_prefix(options, &bucket, &prefix);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

    printf("test_delete_object_by_prefix ok\n");
}

void test_bucket_invalid_parameter(CuTest *tc)
{
	aos_pool_t *p = NULL;
	oss_request_options_t *options = NULL;
	int is_cname = 0;
	aos_string_t bucket;
	aos_status_t *s = NULL;
	aos_table_t *resp_headers = NULL;

	aos_pool_create(&p, NULL);
	options = oss_request_options_create(p);
	init_test_request_options(options, is_cname);
	aos_str_set(&bucket, TEST_BUCKET_NAME);

	s = oss_create_bucket(NULL, NULL, OSS_ACL_DEFAULT, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_create_bucket(options, NULL, OSS_ACL_DEFAULT, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_create_bucket_with_storage_class(NULL, NULL, OSS_ACL_DEFAULT, OSS_STORAGE_CLASS_STANDARD, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_create_bucket_with_storage_class(options, NULL, OSS_ACL_DEFAULT, OSS_STORAGE_CLASS_STANDARD, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_bucket(NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_bucket(options, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_acl(NULL, NULL, OSS_ACL_DEFAULT, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_acl(options, NULL, OSS_ACL_DEFAULT, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_acl(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_acl(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_acl(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_head_bucket(NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_head_bucket(options, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_location(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_location(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_location(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_info(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_info(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_info(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_stat(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_stat(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_stat(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_referer(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_referer(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_referer(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_storage_capacity(NULL, NULL, 1024L, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_storage_capacity(options, NULL, 1024L, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_storage_capacity(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_storage_capacity(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_storage_capacity(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_list_object(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_list_object(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_list_object(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_list_bucket(NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_list_bucket(options, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_logging(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_logging(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_logging(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_logging(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_logging(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_logging(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_bucket_logging(NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_bucket_logging(options, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_lifecycle(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_lifecycle(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_lifecycle(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_cors(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_cors(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_cors(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_cors(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_cors(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_cors(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_bucket_cors(NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_bucket_cors(options, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_referer(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_referer(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_referer(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_website(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_website(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_put_bucket_website(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_website(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_website(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_website(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_bucket_website(NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_bucket_website(options, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_lifecycle(NULL, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_lifecycle(options, NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_get_bucket_lifecycle(options, &bucket, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_bucket_lifecycle(NULL, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_bucket_lifecycle(options, NULL, &resp_headers);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_objects(NULL, NULL, NULL, 0, &resp_headers, NULL);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_objects(options, NULL, NULL, 0, &resp_headers, NULL);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_objects(options, &bucket, NULL, 0, &resp_headers, NULL);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	aos_list_t object_list;
	aos_list_init(&object_list);
	s = oss_delete_objects(options, &bucket, &object_list, 0, &resp_headers, NULL);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_objects_by_prefix(NULL, NULL, NULL);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_objects_by_prefix(options, NULL, NULL);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	s = oss_delete_objects_by_prefix(options, &bucket, NULL);
	CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
	CuAssertStrEquals(tc, AOS_PARAMETER_NULLEMPTY_ERROR, s->error_code);

	aos_pool_destroy(p);

	printf("test_bucket_invalid_parameter ok\n");
}

CuSuite *test_oss_bucket()
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_bucket_setup);
    SUITE_ADD_TEST(suite, test_create_bucket);
    SUITE_ADD_TEST(suite, test_get_bucket_info);
    SUITE_ADD_TEST(suite, test_get_bucket_stat);
    SUITE_ADD_TEST(suite, test_put_bucket_website);
    SUITE_ADD_TEST(suite, test_get_bucket_website);
    SUITE_ADD_TEST(suite, test_delete_bucket_website);
    SUITE_ADD_TEST(suite, test_put_bucket_referer);
    SUITE_ADD_TEST(suite, test_get_bucket_referer);
    SUITE_ADD_TEST(suite, test_put_bucket_logging);
    SUITE_ADD_TEST(suite, test_get_bucket_logging);
    SUITE_ADD_TEST(suite, test_delete_bucket_logging);
    SUITE_ADD_TEST(suite, test_put_bucket_cors);
    SUITE_ADD_TEST(suite, test_get_bucket_cors);
    SUITE_ADD_TEST(suite, test_delete_bucket_cors);
    SUITE_ADD_TEST(suite, test_get_bucket_location);
    SUITE_ADD_TEST(suite, test_put_bucket_storage_capacity);
    SUITE_ADD_TEST(suite, test_get_bucket_storage_capacity);
    SUITE_ADD_TEST(suite, test_list_buckets);
    SUITE_ADD_TEST(suite, test_list_buckets_with_invalid_prefix);
    SUITE_ADD_TEST(suite, test_list_buckets_with_iterator);
    SUITE_ADD_TEST(suite, test_put_bucket_acl);
    SUITE_ADD_TEST(suite, test_put_bucket_acl_invalid_acl);
    SUITE_ADD_TEST(suite, test_get_bucket_acl);
    SUITE_ADD_TEST(suite, test_delete_objects_by_prefix);
    SUITE_ADD_TEST(suite, test_list_object);
    SUITE_ADD_TEST(suite, test_list_object_with_delimiter);
    SUITE_ADD_TEST(suite, test_delete_bucket);
    SUITE_ADD_TEST(suite, test_lifecycle);
    SUITE_ADD_TEST(suite, test_delete_objects_quiet);
    SUITE_ADD_TEST(suite, test_delete_objects_not_quiet);
    SUITE_ADD_TEST(suite, test_create_bucket_with_storage_class);
	SUITE_ADD_TEST(suite, test_bucket_invalid_parameter);
    SUITE_ADD_TEST(suite, test_bucket_cleanup);

    return suite;
}
