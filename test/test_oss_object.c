#include "CuTest.h"
#include "test.h"
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

void test_object_setup(CuTest *tc)
{
    aos_pool_t *p;
    int is_oss_domain = 1;
    aos_status_t *s;
    oss_request_options_t *options;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;

    //create test bucket
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);

    CuAssertIntEquals(tc, 200, s->code);
    aos_pool_destroy(p);
}

void test_object_cleanup(CuTest *tc)
{
    aos_pool_t *p;
    int is_oss_domain = 1;
    aos_string_t bucket;
    aos_status_t *s;
    oss_request_options_t *options;
    char *object_name1 = "oss_test_put_object.txt";
    char *object_name2 = "oss_test_put_object_from_file.txt";
    char *object_name3 = "oss_test_object_by_url";
    char *object_name4 = "oss_test_append_object";
    char *object_name5 = "oss_test_append_object_from_file";
    char *object_name6 = "oss_test_copy_object";
    aos_table_t *resp_headers;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);

    //delete test object
    delete_test_object(options, TEST_BUCKET_NAME, object_name1);
    delete_test_object(options, TEST_BUCKET_NAME, object_name2);
    delete_test_object(options, TEST_BUCKET_NAME, object_name3);
    delete_test_object(options, TEST_BUCKET_NAME, object_name4);
    delete_test_object(options, TEST_BUCKET_NAME, object_name5);
    delete_test_object(options, TEST_BUCKET_NAME, object_name6);

    //delete test bucket
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s= oss_delete_bucket(options, &bucket, &resp_headers);

    aos_pool_destroy(p);
}

void test_put_object_from_buffer(CuTest *tc)
{
    aos_pool_t *p;
    char *object_name = "oss_test_put_object.txt";
    char *str = "test oss c sdk";
    aos_status_t *s;
    int is_oss_domain = 1;
    aos_table_t *headers;
    oss_request_options_t *options;

    //test put object
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    s = create_test_object(options, TEST_BUCKET_NAME, object_name, str, headers);
    CuAssertIntEquals(tc, 200, s->code);
    printf("test_put_object_from_buffer ok\n");

    aos_pool_destroy(p);
}

void test_put_object_from_file(CuTest *tc)
{
    aos_pool_t *p;
    char *object_name = "oss_test_put_object_from_file.txt";
    char *filename = __FILE__;
    aos_status_t *s;
    oss_request_options_t *options;
    int is_oss_domain = 1;
    aos_table_t *headers;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    headers = aos_table_make(p, 5);
    s = create_test_object_from_file(options, TEST_BUCKET_NAME, object_name, filename, headers);
    CuAssertIntEquals(tc, 200, s->code);
    printf("test_put_object_from_file ok\n");

    aos_pool_destroy(p);
}

void test_get_object_to_buffer(CuTest *tc)
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *object_name = "oss_test_put_object.txt";
    aos_string_t object;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_status_t *s;
    aos_list_t buffer;
    aos_buf_t *content;
    char *expect_content = "test oss c sdk";
    char *buf;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;
    char *content_type = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    headers = aos_table_make(p, 0);
    aos_list_init(&buffer);

    //test get object to buffer
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    //get buffer len
    len = aos_buf_list_len(&buffer);

    buf = aos_pcalloc(p, len + 1);
    buf[len] = '\0';

    //copy buffer content to memory
    aos_list_for_each_entry(content, &buffer, node) {
        size = aos_buf_size(content);
        memcpy(buf + pos, content->pos, size);
        pos += size;
    }

    CuAssertStrEquals(tc, expect_content, buf);
    content_type = (char*)(apr_table_get(resp_headers, OSS_CONTENT_TYPE));
    CuAssertStrEquals(tc, "text/plain", content_type);

    printf("test_get_object_to_buffer ok\n");
    aos_pool_destroy(p);
}

void test_get_object_to_buffer_with_range(CuTest *tc)
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *object_name = "oss_test_put_object.txt";
    aos_string_t object;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_status_t *s;
    aos_list_t buffer;
    aos_buf_t *content;
    char *expect_content = "oss c sdk";
    char *buf;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "Range", " bytes=5-13");
    aos_list_init(&buffer);

    //test get object to buffer
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 206, s->code);

    //get buffer len
    len = aos_buf_list_len(&buffer);

    buf = aos_pcalloc(p, len + 1);
    buf[len] = '\0';

    //copy buffer content to memory
    aos_list_for_each_entry(content, &buffer, node) {
        size = aos_buf_size(content);
        memcpy(buf + pos, content->pos, size);
        pos += size;
    }

    CuAssertStrEquals(tc, expect_content, buf);
    printf("test_get_object_to_buffer_with_range ok\n");

    aos_pool_destroy(p);
}

void test_get_object_to_file(CuTest *tc)
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *object_name = "oss_test_put_object_from_file.txt";
    aos_string_t object;
    char *filename = TEST_DIR"/data/oss_test_get_object_to_file";
    char *source_filename = __FILE__;
    aos_string_t file;
    oss_request_options_t *options; 
    int is_oss_domain = 1;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_status_t *s;
    char *content_type = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&file, filename);
    headers = aos_table_make(p, 5);

    //test get object to file
    s = oss_get_object_to_file(options, &bucket, &object, headers, &file, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, get_file_size(source_filename), get_file_size(filename));
    content_type = (char*)(apr_table_get(resp_headers, OSS_CONTENT_TYPE));
    CuAssertStrEquals(tc, "text/plain", content_type);

    printf("test_get_object_to_file ok\n");

    remove(filename);
    aos_pool_destroy(p);
}

void test_head_object(CuTest *tc)
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *object_name = "oss_test_put_object.txt";
    aos_string_t object;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_status_t *s;
    char *user_meta;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    headers = aos_table_make(p, 0);

    //test head object
    s = oss_head_object(options, &bucket, &object, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    user_meta = (char*)(apr_table_get(resp_headers, "x-oss-meta-author"));
    CuAssertStrEquals(tc,"oss", user_meta);
    printf("test_head_object ok\n");

    aos_pool_destroy(p);
}

void test_delete_object(CuTest *tc)
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *object_name = "oss_test_put_object";
    aos_string_t object;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_table_t *resp_headers;
    aos_status_t *s;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
 
    //test delete object
    s = oss_delete_object(options, &bucket, &object, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code);
    printf("test_delete_object ok\n");

    aos_pool_destroy(p);
}

void test_copy_object(CuTest *tc)
{
    aos_pool_t *p;
    aos_string_t source_bucket;
    char *source_object_name = "oss_test_put_object.txt";
    aos_string_t source_object;
    aos_string_t dest_bucket;
    char *dest_object_name = "oss_test_copy_object";
    aos_string_t dest_object;
    oss_request_options_t *options;
    int is_oss_domain = 1;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_status_t *s;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&source_bucket, TEST_BUCKET_NAME);
    aos_str_set(&source_object, source_object_name);
    aos_str_set(&dest_bucket, TEST_BUCKET_NAME);
    aos_str_set(&dest_object, dest_object_name);
    headers = aos_table_make(p, 5);

    //test copy object
    s = oss_copy_object(options, &source_bucket, &source_object, 
        &dest_bucket, &dest_object, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    printf("test_copy_object ok\n");

    aos_pool_destroy(p);
}

void test_object_by_url(CuTest *tc)
{
    aos_pool_t *p;
    oss_request_options_t *options;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_http_request_t *req;
    aos_list_t buffer;
    aos_status_t *s;
    aos_string_t url;
    apr_time_t now;
    int two_minute = 120;
    int is_oss_domain = 1;
    char *object_name = "oss_test_object_by_url";
    aos_string_t bucket;
    aos_string_t object;
    char *str = "test oss c sdk for object url api";
    char *filename = __FILE__;
    char *filename_download = TEST_DIR"/data/oss_test_object_by_url";
    aos_string_t file;
    int64_t effective_time;
    int64_t expired_time;
    char *url_str;
    aos_buf_t *content;
   
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    req = aos_http_request_create(p);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&file, filename);
    aos_list_init(&buffer);

    now = apr_time_now();
    effective_time = now / 1000000 + two_minute;
    expired_time = now / 1000000 - two_minute;

    //test effective url for put_object_from_buffer
    req->method = HTTP_PUT;
    url_str = gen_test_signed_url(options, TEST_BUCKET_NAME, object_name, effective_time, req);
    aos_str_set(&url, url_str);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);
    s = oss_put_object_from_buffer_by_url(options, &url, &buffer, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    //test effective url for put_object_from_file
    s = oss_put_object_from_file_by_url(options, &url, &file, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    //test effective url for get_object_to_buffer
    req->method = HTTP_GET;
    url_str = gen_test_signed_url(options, TEST_BUCKET_NAME, object_name, effective_time, req);
    aos_str_set(&url, url_str);
    s = oss_get_object_to_buffer_by_url(options, &url, headers, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    //test effective url for get_object_to_file
    aos_str_set(&file, filename_download);
    s = oss_get_object_to_file_by_url(options, &url, headers, &file, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, get_file_size(filename), get_file_size(filename_download));

    //test effective url for head_object
    req->method = HTTP_HEAD;
    url_str = gen_test_signed_url(options, TEST_BUCKET_NAME, object_name, effective_time, req);
    aos_str_set(&url, url_str);
    s = oss_head_object_by_url(options, &url, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    printf("test_object_by_url ok\n");

    remove(filename_download);
    aos_pool_destroy(p);
}

void test_append_object_from_buffer(CuTest *tc)
{
    aos_pool_t *p;
    char *object_name = "oss_test_append_object";
    aos_string_t bucket;
    aos_string_t object;
    char *str = "test oss c sdk";
    aos_status_t *s;
    int is_oss_domain = 1;
    int64_t position = 0;
    aos_table_t *headers;
    aos_table_t *headers1;
    aos_table_t *resp_headers;
    oss_request_options_t *options;
    aos_list_t buffer;
    aos_buf_t *content;
    char *next_append_position;

    //test append object 
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    s = oss_head_object(options, &bucket, &object, headers, &resp_headers);
    if(s->code == 200) {
        next_append_position = (char*)(apr_table_get(resp_headers, "x-oss-next-append-position"));
        position = atoi(next_append_position);
    }

    //append object
    headers1 = aos_table_make(p, 0);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);
    s = oss_append_object_from_buffer(options, &bucket, &object, position, &buffer, headers1, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    printf("test_append_object_from_buffer ok\n");

    aos_pool_destroy(p);
}

void test_append_object_from_file(CuTest *tc)
{
    aos_pool_t *p;
    char *object_name = "oss_test_append_object_from_file";
    aos_string_t bucket;
    aos_string_t object;
    char *filename = __FILE__;
    aos_string_t append_file;
    aos_status_t *s;
    int is_oss_domain = 1;
    int64_t position = 0;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    oss_request_options_t *options;

    //test append object 
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&append_file, filename);

    s = oss_append_object_from_file(options, &bucket, &object, position, &append_file, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    printf("test_append_object_from_file ok\n");

    aos_pool_destroy(p);
}

CuSuite *test_oss_object()
{
    CuSuite* suite = CuSuiteNew();   

    SUITE_ADD_TEST(suite, test_object_setup);
    SUITE_ADD_TEST(suite, test_put_object_from_buffer);
    SUITE_ADD_TEST(suite, test_put_object_from_file);
    SUITE_ADD_TEST(suite, test_get_object_to_buffer);
    SUITE_ADD_TEST(suite, test_get_object_to_buffer_with_range);
    SUITE_ADD_TEST(suite, test_get_object_to_file);
    SUITE_ADD_TEST(suite, test_head_object);
    //SUITE_ADD_TEST(suite, test_copy_object);
    SUITE_ADD_TEST(suite, test_object_by_url);
    SUITE_ADD_TEST(suite, test_delete_object);
    SUITE_ADD_TEST(suite, test_put_object_from_file);
    SUITE_ADD_TEST(suite, test_append_object_from_buffer);
    SUITE_ADD_TEST(suite, test_append_object_from_file);
    SUITE_ADD_TEST(suite, test_object_cleanup); 
    
    return suite;
}
