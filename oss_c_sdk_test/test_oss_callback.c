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

void test_callback_setup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;

    TEST_BUCKET_NAME = get_test_bucket_name(aos_global_pool, "test-c-sdk-callback");

    /* create test bucket */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);

    CuAssertIntEquals(tc, 200, s->code);
    aos_pool_destroy(p);
}

void test_callback_cleanup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    oss_request_options_t *options = NULL;
    char *object_name1 = "oss_test_cb_put_object.ts";
    char *object_name2 = "oss_test_cb_multipart_object.ts";

    //aos_table_t *resp_headers = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    /* delete test object */
    delete_test_object(options, TEST_BUCKET_NAME, object_name1);
    delete_test_object(options, TEST_BUCKET_NAME, object_name2);

    /* delete test bucket */
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    oss_delete_bucket(options, &bucket, NULL);
    apr_sleep(apr_time_from_sec(3));

    aos_pool_destroy(p);
}

void test_callback_put_object_from_buffer(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_cb_put_object.ts";
    char *str = "test oss c sdk";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *headers = NULL;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t resp_body;
    aos_list_t buffer;
    aos_buf_t *content;
    char *buf = NULL;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;
    char b64_buf[1024];
    int b64_len;
    
    /* JSON format */
    char *callback =  "{"
        "\"callbackUrl\":\"http://oss-demo.aliyuncs.com:23450\","
        "\"callbackHost\":\"oss-cn-hangzhou.aliyuncs.com\","
        "\"callbackBody\":\"bucket=${bucket}&object=${object}&size=${size}&mimeType=${mimeType}\","
        "\"callbackBodyType\":\"application/x-www-form-urlencoded\""
        "}";

    aos_log_level_e oldLogLevel;
    oldLogLevel = aos_log_level;
    aos_log_set_level(AOS_LOG_DEBUG);

    /* init test */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
   
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_list_init(&buffer);

    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    /* put call into header */
    b64_len = aos_base64_encode((unsigned char*)callback, strlen(callback), b64_buf);
    b64_buf[b64_len] = '\0';

    headers = aos_table_make(p, 1);
    apr_table_set(headers, OSS_CALLBACK, b64_buf);
    
    /* test put object */
    s = oss_do_put_object_from_buffer(options, &bucket, &object, &buffer, 
        headers, NULL, NULL, &resp_headers, &resp_body);
    CuAssertIntEquals(tc, 200, s->code);

    /* get buffer len */
    len = aos_buf_list_len(&resp_body);
    buf = (char *)aos_pcalloc(p, (apr_size_t)(len + 1));
    buf[len] = '\0';

    /* copy buffer content to memory */
    aos_list_for_each_entry(aos_buf_t, content, &resp_body, node) {
        size = aos_buf_size(content);
        memcpy(buf + pos, content->pos, (size_t)size);
        pos += size;
    }
    CuAssertStrEquals(tc, buf, "{\"Status\":\"OK\"}");

    aos_pool_destroy(p);

    /* head object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    s = oss_head_object(options, &bucket, &object, NULL, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

    aos_log_set_level(oldLogLevel);

    printf("test_callback_put_object_from_buffer ok\n");
}

void test_callback_multipart_from_buffer(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "oss_test_cb_multipart_object.ts";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;
    oss_list_upload_part_params_t *params = NULL;
    aos_string_t upload_id;
    aos_list_t complete_part_list;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_list_part_content_t *part_content1 = NULL;
    oss_complete_part_content_t *complete_content1 = NULL;
    int part_num = 1;
    aos_list_t resp_body;
    aos_buf_t *content;
    char *buf = NULL;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;
    char b64_buf[1024];
    int b64_len;

    /* JSON format */
    char *callback =  "{"
        "\"callbackUrl\":\"http://oss-demo.aliyuncs.com:23450\","
        "\"callbackHost\":\"oss-cn-hangzhou.aliyuncs.com\","
        "\"callbackBody\":\"bucket=${bucket}&object=${object}&size=${size}&mimeType=${mimeType}\","
        "\"callbackBodyType\":\"application/x-www-form-urlencoded\""
        "}";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    /* put call into header */
    b64_len = aos_base64_encode((unsigned char*)callback, strlen(callback), b64_buf);
    b64_buf[b64_len] = '\0';

    headers = aos_table_make(p, 1);
    apr_table_set(headers, OSS_CALLBACK, b64_buf);

    //init mulitipart
    s = init_test_multipart_upload(options, TEST_BUCKET_NAME, object_name, &upload_id);
    CuAssertIntEquals(tc, 200, s->code);

    //upload part
    aos_list_init(&buffer);
    make_random_body(p, 2, &buffer);

    s = oss_upload_part_from_buffer(options, &bucket, &object, &upload_id,
        part_num++, &buffer, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    aos_list_init(&buffer);
    make_random_body(p, 200, &buffer);
    s = oss_upload_part_from_buffer(options, &bucket, &object, &upload_id,
        part_num++, &buffer, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    //list part
    params = oss_create_list_upload_part_params(p);
    params->max_ret = 1;
    aos_list_init(&complete_part_list);

    s = oss_list_upload_part(options, &bucket, &object, &upload_id, 
                             params, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    aos_list_for_each_entry(oss_list_part_content_t, part_content1, &params->part_list, node) {
        complete_content1 = oss_create_complete_part_content(p);
        aos_str_set(&complete_content1->part_number, part_content1->part_number.data);
        aos_str_set(&complete_content1->etag, part_content1->etag.data);
        aos_list_add_tail(&complete_content1->node, &complete_part_list);
    }

    //complete multipart
    s = oss_do_complete_multipart_upload(options, &bucket, &object, &upload_id,
            &complete_part_list, headers, NULL, &resp_headers, &resp_body);
    CuAssertIntEquals(tc, 200, s->code);

    /* get buffer len */
    len = aos_buf_list_len(&resp_body);
    buf = (char *)aos_pcalloc(p, (apr_size_t)(len + 1));
    buf[len] = '\0';

    /* copy buffer content to memory */
    aos_list_for_each_entry(aos_buf_t, content, &resp_body, node) {
        size = aos_buf_size(content);
        memcpy(buf + pos, content->pos, (size_t)size);
        pos += size;
    }
    CuAssertStrEquals(tc, buf, "{\"Status\":\"OK\"}");

    aos_pool_destroy(p);

    printf("test_callback_multipart_from_buffer ok\n");
}

CuSuite *test_oss_callback()
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_callback_setup);
    SUITE_ADD_TEST(suite, test_callback_put_object_from_buffer);
    SUITE_ADD_TEST(suite, test_callback_multipart_from_buffer); 
    SUITE_ADD_TEST(suite, test_callback_cleanup);

    return suite;
}
