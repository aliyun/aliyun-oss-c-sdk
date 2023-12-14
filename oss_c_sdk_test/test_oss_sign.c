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

void test_sign_setup(CuTest *tc)
{
}

void test_sign_cleanup(CuTest *tc)
{
}

static void test_sign_v4_object_full(CuTest* tc)
{
    aos_pool_t* p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    oss_request_options_t* options = NULL;
    aos_table_t* headers = NULL;
    aos_table_t* querys = NULL;
    aos_table_t* resp_headers = NULL;
    aos_status_t* s = NULL;
    aos_list_t buffer;
    aos_buf_t* content = NULL;
    const char* str = "";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    options->config = oss_config_create(options->pool);
    aos_str_set(&options->config->endpoint, "oss-cn-hangzhou.aliyuncs.com");
    aos_str_set(&options->config->access_key_id, "ak");
    aos_str_set(&options->config->access_key_secret, "sk");
    aos_str_set(&options->config->region, "cn-hangzhou");
    options->config->signature_version = 4;
    options->ctl = aos_http_controller_create(options->pool, 0);

    aos_str_set(&bucket, "oss-bucket-test");
    aos_str_set(&object, "test-sign-V4/-!@#$%^&*()/abcdefjhijklmnoqprstuvwxyz/123456789");


    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-head1", "value");
    apr_table_set(headers, "abc", "value");
    apr_table_set(headers, "ZAbc", "value");
    apr_table_set(headers, "XYZ", "value");

    //force x-oss-date to the speical value for test
    apr_table_set(headers, "x-oss-date", "20221016T040719Z");

    querys = aos_table_make(p, 1);
    apr_table_set(querys, "param1", "value1");
    apr_table_set(querys, "|param1", "value2");
    apr_table_set(querys, "+param1", "value3");
    apr_table_set(querys, "|param1", "value4");
    apr_table_set(querys, "+param2", "");
    apr_table_set(querys, "|param2", "");
    apr_table_set(querys, "param2", "");

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    s = oss_do_put_object_from_buffer(options, &bucket, &object,
        &buffer, headers, querys, NULL, &resp_headers, NULL);

    CuAssertIntEquals(tc, 403, s->code);

    str = apr_table_get(headers, OSS_AUTHORIZATION);
    CuAssertStrEquals(tc, "OSS4-HMAC-SHA256 Credential=ak/20221016/cn-hangzhou/oss/aliyun_v4_request,Signature=16e58322c5aafb55edd82813b02d6521caab8194882c786bf0e77b7987a63979", str);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);

}

static void test_sign_v4_object_without_query(CuTest* tc)
{
    aos_pool_t* p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    oss_request_options_t* options = NULL;
    aos_table_t* headers = NULL;
    aos_table_t* resp_headers = NULL;
    aos_status_t* s = NULL;
    aos_list_t buffer;
    aos_buf_t* content = NULL;
    const char* str = "";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    options->config = oss_config_create(options->pool);
    aos_str_set(&options->config->endpoint, "oss-cn-hangzhou.aliyuncs.com");
    aos_str_set(&options->config->access_key_id, "ak");
    aos_str_set(&options->config->access_key_secret, "sk");
    aos_str_set(&options->config->region, "cn-hangzhou");
    options->config->signature_version = 4;
    options->ctl = aos_http_controller_create(options->pool, 0);

    aos_str_set(&bucket, "oss-bucket-test");
    aos_str_set(&object, "test-sign-V4/-!@#$%^&*()/abcdefjhijklmnoqprstuvwxyz/123456789");


    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-head1", "value");

    //force x-oss-date to the speical value for test
    apr_table_set(headers, "x-oss-date", "20221016T091431Z");

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    s = oss_do_put_object_from_buffer(options, &bucket, &object,
        &buffer, headers, NULL, NULL, &resp_headers, NULL);

    CuAssertIntEquals(tc, 403, s->code);

    str = apr_table_get(headers, OSS_AUTHORIZATION);
    CuAssertStrEquals(tc, "OSS4-HMAC-SHA256 Credential=ak/20221016/cn-hangzhou/oss/aliyun_v4_request,Signature=5028172a0a3806285979896982bee6e36e915cbf428ee7100a809e1f0ff43b64", str);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_sign_v4_object_cloudbox_id_full(CuTest* tc)
{
    aos_pool_t* p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    oss_request_options_t* options = NULL;
    aos_table_t* headers = NULL;
    aos_table_t* querys = NULL;
    aos_table_t* resp_headers = NULL;
    aos_status_t* s = NULL;
    aos_list_t buffer;
    aos_buf_t* content = NULL;
    const char* str = "";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    options->config = oss_config_create(options->pool);
    aos_str_set(&options->config->endpoint, "oss-cn-hangzhou.aliyuncs.com");
    aos_str_set(&options->config->access_key_id, "ak");
    aos_str_set(&options->config->access_key_secret, "sk");
    aos_str_set(&options->config->region, "cn-hangzhou");
    aos_str_set(&options->config->cloudbox_id, "cloudbox-id");
    options->config->signature_version = 4;
    options->ctl = aos_http_controller_create(options->pool, 0);

    aos_str_set(&bucket, "oss-bucket-test");
    aos_str_set(&object, "test-sign-V4/-!@#$%^&*()/abcdefjhijklmnoqprstuvwxyz/123456789");


    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-head1", "value");
    apr_table_set(headers, "abc", "value");
    apr_table_set(headers, "ZAbc", "value");
    apr_table_set(headers, "XYZ", "value");

    //force x-oss-date to the speical value for test
    apr_table_set(headers, "x-oss-date", "20221016T091831Z");

    querys = aos_table_make(p, 1);
    apr_table_set(querys, "param1", "value1");
    apr_table_set(querys, "|param1", "value2");
    apr_table_set(querys, "+param1", "value3");
    apr_table_set(querys, "|param1", "value4");
    apr_table_set(querys, "+param2", "");
    apr_table_set(querys, "|param2", "");
    apr_table_set(querys, "param2", "");

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    s = oss_do_put_object_from_buffer(options, &bucket, &object,
        &buffer, headers, querys, NULL, &resp_headers, NULL);

    CuAssertIntEquals(tc, 403, s->code);

    str = apr_table_get(headers, OSS_AUTHORIZATION);
    CuAssertStrEquals(tc, "OSS4-HMAC-SHA256 Credential=ak/20221016/cloudbox-id/oss-cloudbox/aliyun_v4_request,Signature=939eae7d5012d444c8d1dd8c68811275928f780a859e41ddbe52cf10a100adb4", str);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_sign_v4_with_gmt_datefomat(CuTest* tc)
{
    aos_pool_t* p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    oss_request_options_t* options = NULL;
    aos_table_t* headers = NULL;
    aos_table_t* querys = NULL;
    aos_table_t* resp_headers = NULL;
    aos_status_t* s = NULL;
    aos_list_t buffer;
    aos_buf_t* content = NULL;
    const char* str = "";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    options->config = oss_config_create(options->pool);
    aos_str_set(&options->config->endpoint, "oss-cn-hangzhou.aliyuncs.com");
    aos_str_set(&options->config->access_key_id, "ak");
    aos_str_set(&options->config->access_key_secret, "sk");
    aos_str_set(&options->config->region, "cn-hangzhou");
    aos_str_set(&options->config->cloudbox_id, "cloudbox-id");
    options->config->signature_version = 4;
    options->ctl = aos_http_controller_create(options->pool, 0);

    aos_str_set(&bucket, "oss-bucket-test");
    aos_str_set(&object, "test-sign-V4/-!@#$%^&*()/abcdefjhijklmnoqprstuvwxyz/123456789");


    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-head1", "value");
    apr_table_set(headers, "abc", "value");
    apr_table_set(headers, "ZAbc", "value");
    apr_table_set(headers, "XYZ", "value");

    //force x-oss-date to the speical value for test
    apr_table_set(headers, "x-oss-date", "Thu, 19 Mar 2015 18:00:00 GMT");

    querys = aos_table_make(p, 1);
    apr_table_set(querys, "param1", "value1");
    apr_table_set(querys, "|param1", "value2");
    apr_table_set(querys, "+param1", "value3");
    apr_table_set(querys, "|param1", "value4");
    apr_table_set(querys, "+param2", "");
    apr_table_set(querys, "|param2", "");
    apr_table_set(querys, "param2", "");

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    s = oss_do_put_object_from_buffer(options, &bucket, &object,
        &buffer, headers, querys, NULL, &resp_headers, NULL);

    CuAssertIntEquals(tc, 403, s->code);
    str = apr_table_get(headers, OSS_AUTHORIZATION);
    CuAssertPtrNotNull(tc, strstr(str, "OSS4-HMAC-SHA256 Credential=ak/20150319/cloudbox-id/oss-cloudbox/aliyun_v4_request"));

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_sign_v4_presign(CuTest* tc)
{
    printf("%s ok\n", __FUNCTION__);
}

CuSuite *test_oss_sign()
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_sign_setup);
    SUITE_ADD_TEST(suite, test_sign_v4_object_full);
    SUITE_ADD_TEST(suite, test_sign_v4_object_without_query);
    SUITE_ADD_TEST(suite, test_sign_v4_object_cloudbox_id_full);
    SUITE_ADD_TEST(suite, test_sign_v4_with_gmt_datefomat);
    SUITE_ADD_TEST(suite, test_sign_v4_presign);
    SUITE_ADD_TEST(suite, test_sign_cleanup);

    return suite;
}
