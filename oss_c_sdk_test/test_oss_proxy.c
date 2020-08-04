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

static char test_file[1024];

void test_proxy_setup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;

    TEST_BUCKET_NAME = get_test_bucket_name(aos_global_pool, "proxy");

    /* create test bucket */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);

    sprintf(test_file, "%sBingWallpaper-2017-01-19.jpg", get_test_file_path());

    CuAssertIntEquals(tc, 200, s->code);
    aos_pool_destroy(p);
}

void test_proxy_cleanup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    oss_request_options_t *options = NULL;
    char *object_name1 = "oss_test_proxy_put_object.txt";

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

void init_test_proxy_request_options(oss_request_options_t *options, int is_cname)
{
    options->config = oss_config_create(options->pool);
    init_test_config(options->config, is_cname);
    aos_str_set(&options->config->proxy_host, decrypt("^]DRRDR^D^Z", options->pool));
    aos_str_set(&options->config->proxy_user, decrypt("\x1e\xf\x19\x1e\xf\x18", options->pool));
    aos_str_set(&options->config->proxy_passwd, decrypt("\"\xf\x6\x6\x5[XY^_", options->pool));
    options->config->proxy_port = 3128;

    options->ctl = aos_http_controller_create(options->pool, 0);
    options->ctl->options = aos_http_request_options_create(options->pool);
    oss_config_resolve(options->pool, options->config, options->ctl);
    options->ctl->options->verify_ssl = AOS_FALSE;
}

void test_proxy_put_object_from_buffer(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_proxy_put_object.txt";
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
    init_test_proxy_request_options(options, is_cname);

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
    init_test_proxy_request_options(options, is_cname);

    s = oss_get_object_to_buffer(options, &bucket, &object, NULL, NULL, &buffer, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

    printf("test_proxy_put_object_from_buffer ok\n");
}

void test_proxy_list_object(CuTest *tc)
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
    init_test_proxy_request_options(options, is_cname);
    params = oss_create_list_object_params(p);
    params->max_ret = 1;
    params->truncated = 0;
    aos_str_set(&params->prefix, "oss_test_proxy_");
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_list_object(options, &bucket, params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    aos_list_for_each_entry(oss_list_object_content_t, content, &params->object_list, node) {
        ++size;
    }
    CuAssertIntEquals(tc, 1 ,size);

    printf("test_proxy_list_object ok\n");
}

void test_proxy_resumable_upload(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_proxy_resumable_upload.jpg";
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
    int64_t content_length = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_proxy_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, test_file);

    // upload object
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_FALSE, NULL);
    s = oss_resumable_upload_file(options, &bucket, &object, &filename, headers, NULL,
        clt_params, NULL, &resp_headers, &resp_body);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

    // head object
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = oss_head_object(options, &bucket, &object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    content_length = atol((char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH));
    CuAssertTrue(tc, content_length == get_file_size(test_file));

    aos_pool_destroy(p);

    printf("test_proxy_resumable_upload ok\n");
}

CuSuite *test_oss_proxy()
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_proxy_setup);
    SUITE_ADD_TEST(suite, test_proxy_put_object_from_buffer);
    SUITE_ADD_TEST(suite, test_proxy_list_object);
    SUITE_ADD_TEST(suite, test_proxy_resumable_upload);
    SUITE_ADD_TEST(suite, test_proxy_cleanup);

    return suite;
}
