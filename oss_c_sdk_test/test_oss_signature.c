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

static char image_file[1024];


void test_signature_setup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;

    /*build test file path*/
    sprintf(image_file, "%sexample.jpg", get_test_file_path());
    
    /* create test bucket */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

}

void test_signature_cleanup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    oss_request_options_t *options = NULL;

    aos_table_t *resp_headers = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    /* delete test object */
    delete_test_object_by_prefix(options, TEST_BUCKET_NAME, "test_signature");

    /* delete test bucket */
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    oss_delete_bucket(options, &bucket, &resp_headers);
    apr_sleep(apr_time_from_sec(3));

    aos_pool_destroy(p);
}

void test_signature_put_object(CuTest *tc)
{
    aos_pool_t *p = NULL;
    const char *object_name = __FUNCTION__;
    const char *filename = __FILE__;
    aos_status_t *s = NULL;
    int is_cname = 0;
    int signatrue_version = 2;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t file;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;

    aos_pool_create(&p, NULL);

    /* test put object without parameter */
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    options->config->signature_version = signatrue_version;
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    aos_str_set(&bucket, TEST_BUCKET_NAME); 
    aos_str_set(&object, object_name);
    aos_str_set(&file, filename);
    s = oss_put_object_from_file(options, &bucket, &object, &file, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /* test put object with parameter */
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    options->config->signature_version = signatrue_version;
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    params = aos_table_make(p, 1);
    apr_table_set(params, "x-param-test", "test");
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    resp_headers = NULL;
    s = oss_do_put_object_from_file(options, &bucket, &object, &file,
        headers, params, NULL, &resp_headers, NULL);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_signature_get_object(CuTest *tc)
{
    aos_pool_t *p = NULL;
    const char *object_name = __FUNCTION__;
    const char *object_image;
    const char *filename = __FILE__;
    aos_status_t *s = NULL;
    int is_cname = 0;
    int signatrue_version = 2;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t file;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_list_t buffer;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;

    aos_pool_create(&p, NULL);

    /*put test object*/
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    options->config->signature_version = signatrue_version;
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&file, filename);
    s = oss_put_object_from_file(options, &bucket, &object, &file, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /* test get object without paramters*/
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    options->config->signature_version = signatrue_version;
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&buffer);
    resp_headers = NULL;
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, NULL, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /* put image */
    object_image = apr_psprintf(p, "%s_image.jpg", __FUNCTION__);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    options->config->signature_version = signatrue_version;
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_image);
    aos_str_set(&file, image_file);
    resp_headers = NULL;
    s = oss_put_object_from_file(options, &bucket, &object, &file, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /* test get object with paramters*/
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    options->config->signature_version = signatrue_version;
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    params = aos_table_make(p, 1);
    apr_table_set(params, OSS_PROCESS, "image/resize,m_fixed,w_100,h_100");
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_image);
    aos_list_init(&buffer);
    resp_headers = NULL;
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, NULL, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_signature_put_get_object_by_url(CuTest *tc)
{
    aos_pool_t *p = NULL;
    const char *object_name = __FUNCTION__;
    const char *object_image;
    const char *filename = __FILE__;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_http_request_t *req = NULL;
    aos_status_t *s = NULL;
    int two_minute = 120;
    int is_cname = 0;
    int signatrue_version = 2;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t file;
    aos_string_t url;
    int64_t effective_time;
    char *signed_url = NULL;

    aos_pool_create(&p, NULL);

    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    options->config->signature_version = signatrue_version;
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&file, filename);

    /*signed url without parameters*/

    /*put_object_from_file */
    effective_time = apr_time_now() / 1000000 + two_minute;
    req = aos_http_request_create(p);
    req->method = HTTP_PUT;
    signed_url = oss_gen_signed_url(options, &bucket, &object, effective_time, req);
    aos_str_set(&url, signed_url);
    resp_headers = NULL;
    s = oss_put_object_from_file_by_url(options, &url, &file, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /*get_object_to_buffer*/
    effective_time = apr_time_now() / 1000000 + two_minute;
    req = aos_http_request_create(p);
    req->method = HTTP_GET;
    signed_url = oss_gen_signed_url(options, &bucket, &object, effective_time, req);
    aos_str_set(&url, signed_url);
    resp_headers = NULL;
    aos_list_init(&buffer);
    s = oss_get_object_to_buffer_by_url(options, &url,headers, NULL, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /*gen signed url with parameters*/

    /*put_object_from_file */
    effective_time = apr_time_now() / 1000000 + two_minute;
    req = aos_http_request_create(p);
    req->method = HTTP_PUT;
    req->query_params = aos_table_make(p, 1);
    apr_table_set(req->query_params, "x-param-test", "test");
    signed_url = oss_gen_signed_url(options, &bucket, &object, effective_time, req);
    aos_str_set(&url, signed_url);
    resp_headers = NULL;
    s = oss_put_object_from_file_by_url(options, &url, &file, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /*get_object_to_bufer */
    effective_time = apr_time_now() / 1000000 + two_minute;
    req = aos_http_request_create(p);
    req->method = HTTP_GET;
    req->query_params = aos_table_make(p, 1);
    apr_table_set(req->query_params, "x-param-test", "test");
    signed_url = oss_gen_signed_url(options, &bucket, &object, effective_time, req);
    aos_str_set(&url, signed_url);
    resp_headers = NULL;
    aos_list_init(&buffer);
    s = oss_get_object_to_buffer_by_url(options, &url, headers, NULL, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /*put image from file*/
    object_image = apr_psprintf(p, "%s_image.jpg", __FUNCTION__);
    effective_time = apr_time_now() / 1000000 + two_minute;
    req = aos_http_request_create(p);
    req->method = HTTP_PUT;
    aos_str_set(&object, object_image);
    signed_url = oss_gen_signed_url(options, &bucket, &object, effective_time, req);
    aos_str_set(&url, signed_url);
    resp_headers = NULL;
    aos_str_set(&file, image_file);
    s = oss_put_object_from_file_by_url(options, &url, &file, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /*get image to buffer with processs & parameter*/
    effective_time = apr_time_now() / 1000000 + two_minute;
    req = aos_http_request_create(p);
    req->method = HTTP_GET;
    req->query_params = aos_table_make(p, 2);
    aos_str_set(&object, object_image);
    apr_table_set(req->query_params, "x-param-test", "test");
    apr_table_set(req->query_params, OSS_PROCESS, "image/resize,m_fixed,w_100,h_100");
    signed_url = oss_gen_signed_url(options, &bucket, &object, effective_time, req);
    aos_str_set(&url, signed_url);
    resp_headers = NULL;
    aos_list_init(&buffer);
    s = oss_get_object_to_buffer_by_url(options, &url, NULL, NULL, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}


void test_signature_negative(CuTest *tc)
{
    aos_pool_t *p = NULL;
    const char *object_name = __FUNCTION__;
    const char *object_image;
    const char *filename = __FILE__;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_http_request_t *req = NULL;
    aos_status_t *s = NULL;
    int two_minute = 120;
    int is_cname = 0;
    int signatrue_version = 2;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t file;
    aos_string_t url;
    int64_t effective_time;
    char *signed_url = NULL;

    aos_pool_create(&p, NULL);

    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    options->config->signature_version = signatrue_version;
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&file, filename);

    /*put image from file*/
    object_image = apr_psprintf(p, "%s_image.jpg", __FUNCTION__);
    effective_time = apr_time_now() / 1000000 + two_minute;
    req = aos_http_request_create(p);
    req->method = HTTP_PUT;
    aos_str_set(&object, object_image);
    signed_url = oss_gen_signed_url(options, &bucket, &object, effective_time, req);
    aos_str_set(&url, signed_url);
    resp_headers = NULL;
    aos_str_set(&file, image_file);
    s = oss_put_object_from_file_by_url(options, &url, &file, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /*get image to buffer with processs & parameter*/
    effective_time = apr_time_now() / 1000000 + two_minute;
    req = aos_http_request_create(p);
    req->method = HTTP_GET;
    req->query_params = aos_table_make(p, 2);
    aos_str_set(&object, object_image);
    apr_table_set(req->query_params, "x-param-test", "test");
    apr_table_set(req->query_params, OSS_PROCESS, "image/resize,m_fixed,w_100,h_100");
    signed_url = oss_gen_signed_url(options, &bucket, &object, effective_time, req);
    aos_str_set(&url, signed_url);
    resp_headers = NULL;
    aos_list_init(&buffer);
    s = oss_get_object_to_buffer_by_url(options, &url, NULL, NULL, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    signed_url = apr_psprintf(p, "%s&x-param-test2=test", signed_url);
    aos_str_set(&url, signed_url);
    resp_headers = NULL;
    aos_list_init(&buffer);
    s = oss_get_object_to_buffer_by_url(options, &url, NULL, NULL, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 403, s->code);
    CuAssertStrEquals(tc, "SignatureDoesNotMatch", s->error_code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

CuSuite *test_oss_signature()
{
    CuSuite* suite = CuSuiteNew();   

    SUITE_ADD_TEST(suite, test_signature_setup);
    SUITE_ADD_TEST(suite, test_signature_put_object);
    SUITE_ADD_TEST(suite, test_signature_get_object);
    SUITE_ADD_TEST(suite, test_signature_put_get_object_by_url);
    SUITE_ADD_TEST(suite, test_signature_negative);
    SUITE_ADD_TEST(suite, test_signature_cleanup); 
    
    return suite;
}
