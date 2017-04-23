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
    aos_table_t *headers6 = NULL;

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

    //create test object
    headers1 = aos_table_make(p, 0);
    headers2 = aos_table_make(p, 0);
    headers3 = aos_table_make(p, 0);
    headers4 = aos_table_make(p, 0);
    headers5 = aos_table_make(p, 0);
    headers6 = aos_table_make(p, 0);
    create_test_object(options, TEST_BUCKET_NAME, object_name1, str, headers1);
    create_test_object(options, TEST_BUCKET_NAME, object_name2, str, headers2);
    create_test_object(options, TEST_BUCKET_NAME, object_name3, str, headers3);
    create_test_object(options, TEST_BUCKET_NAME, object_name4, str, headers4);
    create_test_object(options, TEST_BUCKET_NAME, object_name5, str, headers5);
    create_test_object(options, TEST_BUCKET_NAME, object_name6, str, headers6);

    aos_pool_destroy(p);
}

void test_bucket_cleanup(CuTest *tc)
{
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
        } else if(size == 2) {
            CuAssertStrEquals(tc, "oss_tmp2/", prefix);
        }
    }
    CuAssertIntEquals(tc, 2, size);
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
    int size = 0;
    char *rule_id = NULL;
    char *prefix = NULL;
    char *status = NULL;
    int days = INT_MAX;
    char* date = NULL;

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
    aos_list_add_tail(&rule_content1->node, &lifecycle_rule_list);
    aos_list_add_tail(&rule_content2->node, &lifecycle_rule_list);

    s = oss_put_bucket_lifecycle(options, &bucket, &lifecycle_rule_list, 
                                 &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    //get lifecycle
    resp_headers = NULL;
    aos_list_init(&lifecycle_rule_list);
    s = oss_get_bucket_lifecycle(options, &bucket, &lifecycle_rule_list, 
                                 &resp_headers);
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
        }
        ++size;
    }
    CuAssertIntEquals(tc, 2 ,size);

    //delete lifecycle
    resp_headers = NULL;
    s = oss_delete_bucket_lifecycle(options, &bucket, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    aos_pool_destroy(p);

    printf("test_lifecycle ok\n");
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
    char *prefix_str = "oss_tmp3";
    
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&prefix, prefix_str);

    s = oss_delete_objects_by_prefix(options, &bucket, &prefix);
    CuAssertIntEquals(tc, 200, s->code);
    aos_pool_destroy(p);

    printf("test_delete_object_by_prefix ok\n");
}

CuSuite *test_oss_bucket()
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_bucket_setup);
    SUITE_ADD_TEST(suite, test_create_bucket);
    SUITE_ADD_TEST(suite, test_put_bucket_acl);
    SUITE_ADD_TEST(suite, test_get_bucket_acl);
    SUITE_ADD_TEST(suite, test_delete_objects_by_prefix);
    SUITE_ADD_TEST(suite, test_list_object);
    SUITE_ADD_TEST(suite, test_list_object_with_delimiter);
    SUITE_ADD_TEST(suite, test_delete_bucket);
    SUITE_ADD_TEST(suite, test_lifecycle);
    SUITE_ADD_TEST(suite, test_delete_objects_quiet);
    SUITE_ADD_TEST(suite, test_delete_objects_not_quiet);
    SUITE_ADD_TEST(suite, test_bucket_cleanup);

    return suite;
}
