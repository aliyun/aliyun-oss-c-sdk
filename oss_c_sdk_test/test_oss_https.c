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
#include "aos_crc64.h"

static char ca_file[1024];
static char test_endpoint[1024];

void test_https_setup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;

    TEST_BUCKET_NAME = get_test_bucket_name(aos_global_pool, "https");

    /* create test bucket */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);

    sprintf(ca_file, "%sca-certificates.crt", get_test_file_path());

    if (!strncasecmp("http://", TEST_OSS_ENDPOINT, 7)) {
        sprintf(test_endpoint, "https://%s", TEST_OSS_ENDPOINT + 7);
    }

    CuAssertIntEquals(tc, 200, s->code);
    aos_pool_destroy(p);
}

void test_https_cleanup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    oss_request_options_t *options = NULL;
    char *object_name1 = "oss_test_https_put_object.txt";

    aos_table_t *resp_headers = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    /* delete test object */
    delete_test_object(options, TEST_BUCKET_NAME, object_name1);

    /* delete test bucket */
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    oss_delete_bucket(options, &bucket, &resp_headers);
    apr_sleep(apr_time_from_sec(3));

    aos_pool_destroy(p);
}

void init_test_https_request_options(oss_request_options_t *options, int is_cname)
{
    options->config = oss_config_create(options->pool);

    aos_str_set(&options->config->endpoint, test_endpoint);
    aos_str_set(&options->config->access_key_id, TEST_ACCESS_KEY_ID);
    aos_str_set(&options->config->access_key_secret, TEST_ACCESS_KEY_SECRET);
    options->config->is_cname = is_cname;

    options->ctl = aos_http_controller_create(options->pool, 0);
    options->ctl->options = aos_http_request_options_create(options->pool);
    options->ctl->options->verify_ssl = AOS_TRUE;
    options->ctl->options->ca_file = ca_file;
    options->ctl->options->ca_path = get_test_file_path();
}

void test_https_put_object_from_buffer(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_https_put_object.txt";
    char *str = "Sow nothing, reap nothing.";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_list_t buffer;
    aos_buf_t *content;

    /* init test*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_https_request_options(options, is_cname);

    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    headers = aos_table_make(p, 2);
    apr_table_set(headers, "Expect", "");
    apr_table_set(headers, "Transfer-Encoding", "");

    /* test put object */
    s = oss_put_object_from_buffer(options, &bucket, &object, &buffer, headers, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

    /* test get object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_https_request_options(options, is_cname);

    s = oss_get_object_to_buffer(options, &bucket, &object, NULL, NULL, &buffer, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    CuAssertIntEquals(tc, AOS_TRUE, options->ctl->options->verify_ssl);
    CuAssertStrEquals(tc, ca_file, options->ctl->options->ca_file);

    aos_pool_destroy(p);

    printf("test_https_put_object_from_buffer ok\n");
}

void test_https_list_object(CuTest *tc)
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

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_https_request_options(options, is_cname);
    params = oss_create_list_object_params(p);
    params->max_ret = 1;
    params->truncated = 0;
    aos_str_set(&params->prefix, "oss_test_https_");
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_list_object(options, &bucket, params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, AOS_TRUE, options->ctl->options->verify_ssl);
    CuAssertStrEquals(tc, ca_file, options->ctl->options->ca_file);

    aos_list_for_each_entry(oss_list_object_content_t, content, &params->object_list, node) {
        ++size;
    }
    CuAssertIntEquals(tc, 1 ,size);

    printf("test_https_list_object ok\n");
}

void test_https_list_object_negative(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_list_object_params_t *params = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_https_request_options(options, is_cname);
    options->ctl->options->ca_file = "";
    options->ctl->options->ca_path = "";
    params = oss_create_list_object_params(p);
    params->max_ret = 1;
    params->truncated = 0;
    aos_str_set(&params->prefix, "oss_test_https_");
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_list_object(options, &bucket, params, &resp_headers);
    CuAssertIntEquals(tc, -996, s->code);
    CuAssertStrEquals(tc, "Problem with the SSL CA cert (path? access rights?)", s->error_msg);
    CuAssertIntEquals(tc, AOS_TRUE, options->ctl->options->verify_ssl);

    printf("test_https_list_object_negative ok\n");
}


CuSuite *test_oss_https()
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_https_setup);
    SUITE_ADD_TEST(suite, test_https_put_object_from_buffer);
    SUITE_ADD_TEST(suite, test_https_list_object);
    SUITE_ADD_TEST(suite, test_https_list_object_negative);
    SUITE_ADD_TEST(suite, test_https_cleanup);

    return suite;
}
