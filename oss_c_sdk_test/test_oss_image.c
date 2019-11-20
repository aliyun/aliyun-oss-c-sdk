#include "CuTest.h"
#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "aos_transport.h"
#include "aos_http_io.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_xml.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_test_util.h"
#include "apr_time.h"

static char test_file[1024];
static char *original_image  = "example.jpg";
static char *processed_image = "processed_example.jpg";

static void put_example_image(CuTest *tc);

void test_image_setup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;

    TEST_BUCKET_NAME = get_test_bucket_name(aos_global_pool, "image");

    // create test bucket
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);
    CuAssertIntEquals(tc, 200, s->code);

    sprintf(test_file, "%sexample.jpg", get_test_file_path());

    aos_pool_destroy(p);
}

void test_image_cleanup(CuTest *tc)
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
    delete_test_object(options, TEST_BUCKET_NAME, original_image);
    delete_test_object(options, TEST_BUCKET_NAME, processed_image);

    /* delete test bucket */
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    oss_delete_bucket(options, &bucket, &resp_headers);
    apr_sleep(apr_time_from_sec(3));

    aos_pool_destroy(p);
}

void test_resize_image(CuTest *tc) {
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
    image_info_t image_info;

    /* put original image */
    put_example_image(tc);

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, original_image);
    aos_list_init(&buffer);

    params = aos_table_make(p, 1);
    apr_table_set(params, OSS_PROCESS, "image/resize,m_fixed,w_100,h_100");

    /* get processed image to buffer */
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, 
                                 params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    aos_str_set(&object, processed_image);

    /* put processed image */
    s= oss_put_object_from_buffer(options, &bucket, &object, &buffer,
                                   headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    /* check processed image */
    s = get_image_info(options, TEST_BUCKET_NAME, processed_image, &image_info);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 100, image_info.height);
    CuAssertIntEquals(tc, 100, image_info.width);
    //CuAssertIntEquals(tc, 3347, image_info.size);
    CuAssertStrEquals(tc, "jpg", image_info.format);

    aos_pool_destroy(p);

    printf("test_resize_image ok\n");
}

void test_crop_image(CuTest *tc) {
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
    image_info_t image_info;

    /* put original image */
    put_example_image(tc);

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, original_image);
    aos_list_init(&buffer);

    params = aos_table_make(p, 1);
    apr_table_set(params, OSS_PROCESS, "image/crop,w_100,h_100,x_100,y_100,r_1");

    /* get processed image to buffer */
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, 
                                 params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    aos_str_set(&object, processed_image);

    /* put processed image */
    s= oss_put_object_from_buffer(options, &bucket, &object, &buffer,
                                   headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    /* check processed image */
    s = get_image_info(options, TEST_BUCKET_NAME, processed_image, &image_info);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 100, image_info.height);
    CuAssertIntEquals(tc, 100, image_info.width);
    CuAssertIntEquals(tc, 1969, image_info.size);
    CuAssertStrEquals(tc, "jpg", image_info.format);

    aos_pool_destroy(p);
    
    printf("test_crop_image ok\n");
}

void test_rotate_image(CuTest *tc) {
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
    image_info_t image_info;

    /* put original image */
    put_example_image(tc);

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, original_image);
    aos_list_init(&buffer);

    params = aos_table_make(p, 1);
    apr_table_set(params, OSS_PROCESS, "image/rotate,90");

    /* get processed image to buffer */
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, 
                                 params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    aos_str_set(&object, processed_image);

    /* put processed image */
    s= oss_put_object_from_buffer(options, &bucket, &object, &buffer,
                                   headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    /* check processed image */
    s = get_image_info(options, TEST_BUCKET_NAME, processed_image, &image_info);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 400, image_info.height);
    CuAssertIntEquals(tc, 267, image_info.width);
    CuAssertIntEquals(tc, 20998, image_info.size);
    CuAssertStrEquals(tc, "jpg", image_info.format);

    aos_pool_destroy(p);

    printf("test_rotate_image ok\n");
}

void test_sharpen_image(CuTest *tc) {
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
    image_info_t image_info;

    /* put original image */
    put_example_image(tc);

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, original_image);
    aos_list_init(&buffer);

    params = aos_table_make(p, 1);
    apr_table_set(params, OSS_PROCESS, "image/sharpen,100");

    /* get processed image to buffer */
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, 
                                 params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    aos_str_set(&object, processed_image);

    /* put processed image */
    s= oss_put_object_from_buffer(options, &bucket, &object, &buffer,
                                   headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    /* check processed image */
    s = get_image_info(options, TEST_BUCKET_NAME, processed_image, &image_info);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 267, image_info.height);
    CuAssertIntEquals(tc, 400, image_info.width);
    CuAssertIntEquals(tc, 23015, image_info.size);
    CuAssertStrEquals(tc, "jpg", image_info.format);

    aos_pool_destroy(p);

    printf("test_sharpen_image ok\n");
}

void test_watermark_image(CuTest *tc) {
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
    image_info_t image_info;

    /* put original image */
    put_example_image(tc);

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, original_image);
    aos_list_init(&buffer);

    params = aos_table_make(p, 1);
    apr_table_set(params, OSS_PROCESS, "image/watermark,text_SGVsbG8g5Zu-54mH5pyN5YqhIQ");

    /* get processed image to buffer */
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, 
                                 params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    aos_str_set(&object, processed_image);

    /* put processed image */
    s= oss_put_object_from_buffer(options, &bucket, &object, &buffer,
                                   headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    /* check processed image */
    s = get_image_info(options, TEST_BUCKET_NAME, processed_image, &image_info);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 267, image_info.height);
    CuAssertIntEquals(tc, 400, image_info.width);
    CuAssertStrEquals(tc, "jpg", image_info.format);

    aos_pool_destroy(p);

    printf("test_watermark_image ok\n");
}

void test_format_image(CuTest *tc) {
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
    image_info_t image_info;

    /* put original image */
    put_example_image(tc);

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, original_image);
    aos_list_init(&buffer);

    params = aos_table_make(p, 1);
    apr_table_set(params, OSS_PROCESS, "image/format,png");

    /* get processed image to buffer */
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, 
                                 params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    aos_str_set(&object, processed_image);

    /* put processed image */
    s= oss_put_object_from_buffer(options, &bucket, &object, &buffer,
                                   headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    /* check processed image */
    s = get_image_info(options, TEST_BUCKET_NAME, processed_image, &image_info);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 267, image_info.height);
    CuAssertIntEquals(tc, 400, image_info.width);
    CuAssertIntEquals(tc, 160733, image_info.size);
    CuAssertStrEquals(tc, "png", image_info.format);

    aos_pool_destroy(p);

    printf("test_format_image ok\n");
}

void put_example_image(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    char *content_type = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, original_image);
    aos_str_set(&filename, test_file);

    s = oss_put_object_from_file(options, &bucket, &object, &filename, 
        headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /* head object */
    s = oss_head_object(options, &bucket, &object, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    
    content_type = (char*)(apr_table_get(resp_headers, OSS_CONTENT_TYPE));
    CuAssertStrEquals(tc, "image/jpeg", content_type);

    aos_pool_destroy(p);
}

CuSuite *test_oss_image()
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_image_setup);
    SUITE_ADD_TEST(suite, test_resize_image);
    SUITE_ADD_TEST(suite, test_crop_image);
    SUITE_ADD_TEST(suite, test_rotate_image);
    SUITE_ADD_TEST(suite, test_sharpen_image);
    SUITE_ADD_TEST(suite, test_watermark_image);
    SUITE_ADD_TEST(suite, test_format_image);
    SUITE_ADD_TEST(suite, test_image_cleanup);

    return suite;
}
