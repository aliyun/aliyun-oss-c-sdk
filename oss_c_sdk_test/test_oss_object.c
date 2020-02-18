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

static char *TEST_BUCKET_NAME_2;

void test_object_setup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;

    TEST_BUCKET_NAME   = get_test_bucket_name(aos_global_pool, "object");
    TEST_BUCKET_NAME_2 = get_test_bucket_name(aos_global_pool, "object2");
    
    /* create test bucket */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);
    CuAssertIntEquals(tc, 200, s->code);

    s = create_test_bucket(options, TEST_BUCKET_NAME_2, oss_acl);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

}

void test_object_cleanup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    oss_request_options_t *options = NULL;
    char *object_name1 = "oss_test_put_object.ts";
    char *object_name2 = "oss_test_put_object_from_file.jpg";
    char *object_name3 = "oss_test_object_by_url";
    char *object_name4 = "oss_test_append_object";
    char *object_name5 = "oss_test_append_object_from_file";
    char *object_name6 = "oss_test_copy_object";
    char *object_name7 = "video_1.ts";
    char *object_name8 = "video_2.ts";
    char *object_name9 = "oss_test_put_object_from_file2.txt";
    char *object_name10 = "put_object_from_buffer_with_default_content_type";

    aos_table_t *resp_headers = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    /* delete test object */
    delete_test_object(options, TEST_BUCKET_NAME, object_name1);
    delete_test_object(options, TEST_BUCKET_NAME, object_name2);
    delete_test_object(options, TEST_BUCKET_NAME, object_name3);
    delete_test_object(options, TEST_BUCKET_NAME, object_name4);
    delete_test_object(options, TEST_BUCKET_NAME, object_name5);
    delete_test_object(options, TEST_BUCKET_NAME, object_name6);
    delete_test_object(options, TEST_BUCKET_NAME, object_name7);
    delete_test_object(options, TEST_BUCKET_NAME, object_name8);
    delete_test_object(options, TEST_BUCKET_NAME, object_name9);
    delete_test_object(options, TEST_BUCKET_NAME, object_name10);

    /* delete test bucket */
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    oss_delete_bucket(options, &bucket, &resp_headers);
    apr_sleep(apr_time_from_sec(3));

    aos_str_set(&bucket, TEST_BUCKET_NAME_2);
    oss_delete_bucket(options, &bucket, &resp_headers);
    apr_sleep(apr_time_from_sec(3));

    aos_pool_destroy(p);
}

void test_put_object_from_buffer(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_put_object.ts";
    char *str = "test oss c sdk";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *headers = NULL;
    aos_table_t *head_headers = NULL;
    aos_table_t *head_resp_headers = NULL;
    char *content_type = NULL;
    oss_request_options_t *options = NULL;

    /* test put object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    s = create_test_object(options, TEST_BUCKET_NAME, object_name, str, headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, headers);

    aos_pool_destroy(p);

    /* head object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    s = oss_head_object(options, &bucket, &object, 
                        head_headers, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
    
    content_type = (char*)(apr_table_get(head_resp_headers, OSS_CONTENT_TYPE));
    CuAssertStrEquals(tc, "video/MP2T", content_type);

    printf("test_put_object_from_buffer ok\n");
}

void test_put_object_from_buffer_with_default_content_type(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "put_object_from_buffer_with_default_content_type";
    char *str = "test oss c sdk";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *headers = NULL;
    aos_table_t *head_headers = NULL;
    aos_table_t *head_resp_headers = NULL;
    char *content_type = NULL;
    oss_request_options_t *options = NULL;

    /* test put object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    s = create_test_object(options, TEST_BUCKET_NAME, object_name, str, headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, headers);

    aos_pool_destroy(p);

    /* head object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    s = oss_head_object(options, &bucket, &object, 
                        head_headers, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
    
    content_type = (char*)(apr_table_get(head_resp_headers, OSS_CONTENT_TYPE));
    CuAssertStrEquals(tc, "application/octet-stream", content_type);

    printf("test_put_object_from_buffer_with_default_content_type ok\n");
}

void test_put_object_from_buffer_with_specified(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "./xxx/./ddd/";
    char *str = "test oss c sdk";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *headers = NULL;
    aos_table_t *head_headers = NULL;
    aos_table_t *head_resp_headers = NULL;
    oss_request_options_t *options = NULL;

    /* test put object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    s = create_test_object(options, TEST_BUCKET_NAME, object_name, str, headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, headers);

    aos_pool_destroy(p);

    /* head object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    s = oss_head_object(options, &bucket, &object, 
        head_headers, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);

	delete_test_object(options, TEST_BUCKET_NAME, object_name);

    printf("test_put_object_from_buffer_with_specified ok\n");
}

void test_put_object_from_file(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "video_1.ts";
    char *filename = __FILE__;
    aos_string_t bucket;
    aos_string_t object;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *head_headers = NULL;
    aos_table_t *head_resp_headers = NULL;
    char *content_type = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 5);
    s = create_test_object_from_file(options, TEST_BUCKET_NAME, 
            object_name, filename, headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, headers);

    aos_pool_destroy(p);

    /* head object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    s = oss_head_object(options, &bucket, &object, 
                        head_headers, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
    
    content_type = (char*)(apr_table_get(head_resp_headers, OSS_CONTENT_TYPE));
    CuAssertStrEquals(tc, "application/octet-stream", content_type);

    aos_pool_destroy(p);

    //NG Test
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 5);

    s = create_test_object_from_file(options, "c-sdk-no-exist",
        object_name, filename, headers);
    CuAssertIntEquals(tc, 404, s->code);

    s = create_test_object_from_file(options, TEST_BUCKET_NAME,
        object_name, "", headers);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, s->code);

    aos_pool_destroy(p);

    printf("test_put_object_from_file ok\n");
}

void test_put_object_with_large_length_header(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "video_2.ts";
    char *filename = __FILE__;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    int i = 0;
    int header_length = 0;
    aos_table_t *headers = NULL;
    char *user_meta = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    header_length = 1024 * 5;
    user_meta = (char*)calloc(header_length, 1);
    for (; i < header_length - 1; i++) {
        user_meta[i] = 'a';
    }
    user_meta[header_length - 1] = '\0';
    headers = aos_table_make(p, 2);
    apr_table_set(headers, "x-oss-meta-user-meta", user_meta);
    s = create_test_object_from_file(options, TEST_BUCKET_NAME, 
            object_name, filename, headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, headers);

    free(user_meta);
    aos_pool_destroy(p);

    printf("test_put_object_with_large_length_header_back_bound ok\n");
}

void test_put_object_from_file_with_content_type(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_put_object_from_file2.txt";
    char *filename = __FILE__;
    aos_string_t bucket;
    aos_string_t object;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *head_headers = NULL;
    aos_table_t *head_resp_headers = NULL;
    char *content_type = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(options->pool, 1);
    apr_table_set(headers, OSS_CONTENT_TYPE, "image/jpeg");

    s = create_test_object_from_file(options, TEST_BUCKET_NAME, 
            object_name, filename, headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, headers);

    aos_pool_destroy(p);

    /* head object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    s = oss_head_object(options, &bucket, &object, 
                        head_headers, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
    
    content_type = (char*)(apr_table_get(head_resp_headers, OSS_CONTENT_TYPE));
    CuAssertStrEquals(tc, "image/jpeg", content_type);

    aos_pool_destroy(p);

    printf("test_put_object_from_file ok\n");
}

void test_put_symlink_for_obj(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss.jpg";
    char *link_object_name = "link-to-oss.jpg";
    char *filename = __FILE__;
    aos_string_t bucket;
    aos_string_t sym_object;
    aos_string_t target_object;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *head_resp_headers = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(options->pool, 1);
    apr_table_set(headers, OSS_CONTENT_TYPE, "image/jpeg");
    s = create_test_object_from_file(options, TEST_BUCKET_NAME, 
            object_name, filename, headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, headers);
    aos_pool_destroy(p);

    /* link object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&sym_object, link_object_name);
    aos_str_set(&target_object, object_name);
    init_test_request_options(options, is_cname);
    s = oss_put_symlink(options, &bucket, &sym_object, 
                        &target_object, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
    aos_pool_destroy(p);
    
    printf("test_put_object_link ok\n");
}

void test_get_symlink_for_obj(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *link_object_name = "link-to-oss.jpg";
    char *target_link_name = NULL;
    aos_string_t bucket;
    aos_string_t link_object;
    aos_string_t object;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *head_resp_headers = NULL;

    /*get target link object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&link_object, link_object_name);
    init_test_request_options(options, is_cname);
    s = oss_get_symlink(options, &bucket, &link_object, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
    
    target_link_name = (char*)(apr_table_get(head_resp_headers, OSS_CANNONICALIZED_HEADER_SYMLINK));
    CuAssertStrEquals(tc, "oss.jpg", target_link_name);
    TEST_CASE_LOG("link_obj_name %s\n", target_link_name);

    aos_pool_destroy(p);

    aos_pool_create(&p, NULL);
    target_link_name = NULL;
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&link_object, "testasfadf");
    init_test_request_options(options, is_cname);
    s = oss_get_symlink(options, &bucket, &link_object, &head_resp_headers);
    CuAssertIntEquals(tc, 404, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);

    /* delete object */
    aos_str_set(&object, "oss.jpg");
    s = oss_delete_object(options, &bucket, &object, &head_resp_headers);
    CuAssertIntEquals(tc, 204, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);

    /* delete link object */
    aos_str_set(&object, "link-to-oss.jpg");
    s = oss_delete_object(options, &bucket, &object, &head_resp_headers);
    CuAssertIntEquals(tc, 204, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_restore_obj(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;
    char *object_name1 = "oss_test_object1";
    char *str = "test c oss sdk";
    aos_table_t *headers1 = NULL;
    aos_list_t buffer;
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    char IA_BUCKET_NAME[128] = {0};
    apr_snprintf(IA_BUCKET_NAME, 127, "%s-ia", TEST_BUCKET_NAME);

    //setup: create archive bucket
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, IA_BUCKET_NAME);

    s = create_test_bucket_with_storage_class(options, bucket.data, 
                                            oss_acl, OSS_STORAGE_CLASS_ARCHIVE);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertStrEquals(tc, NULL, s->error_code);

    //create test object
    headers1 = aos_table_make(p, 0);
    s = create_test_object(options, bucket.data, object_name1, str, headers1);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertStrEquals(tc, NULL, s->error_code);

    aos_str_set(&object, object_name1);
    aos_list_init(&buffer);
    headers1 = NULL;
    /* test get object to buffer */
    s = oss_get_object_to_buffer(options, &bucket, &object, headers1,
            params, &buffer, &resp_headers);
    /* expect fail because it's archive bucket */
    TEST_CASE_LOG("errcode[%d] %s %s\n", s->code, s->error_msg, s->error_code);
    CuAssertIntEquals(tc, -978, s->code);

    TEST_CASE_LOG("restore object begin.\n");
    headers1 = aos_table_make(p, 0);
    s = oss_restore_object(options, &bucket, &object, headers1, &resp_headers);
    CuAssertIntEquals(tc, 202, s->code);
    CuAssertStrEquals(tc, NULL, s->error_code);

    do {
        headers1 = aos_table_make(p, 0);
        s = oss_restore_object(options, &bucket, &object, headers1, &resp_headers);
        if (s->code != 409) {
            break;
        } else {
            apr_sleep(5000);
        }
    } while (1);
    TEST_CASE_LOG("\nrestore object done.\n");

    CuAssertIntEquals(tc, 200,  s->code);
    CuAssertStrEquals(tc, NULL, s->error_code);

    // restore the object 
    aos_list_init(&buffer);
    headers1 = NULL;
    /* test get object to buffer */
    s = oss_get_object_to_buffer(options, &bucket, &object, headers1,
            params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200,  s->code);
    CuAssertStrEquals(tc, NULL, s->error_code);

    // restore the object again to verify the data no need thaw 
    aos_list_init(&buffer);
    headers1 = NULL;
    /* test get object to buffer */
    s = oss_get_object_to_buffer(options, &bucket, &object, headers1,
            params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200,  s->code);
    CuAssertStrEquals(tc, NULL, s->error_code);

    //cleanup: delete archive bucket 
    delete_test_object(options, bucket.data, object_name1);
    s = oss_delete_bucket(options, &bucket, &resp_headers);
    TEST_CASE_LOG("errcode[%d] %s %s\n", s->code, s->error_msg, s->error_code);
    CuAssertIntEquals(tc, 204, s->code);
    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_get_object_to_buffer(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "oss_test_put_object.ts";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;
    aos_buf_t *content = NULL;
    char *expect_content = "test oss c sdk";
    char *buf = NULL;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;
    char *content_type = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&buffer);

    /* test get object to buffer */
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, 
                                 params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /* get buffer len */
    len = aos_buf_list_len(&buffer);

    buf = aos_pcalloc(p, (apr_size_t)(len + 1));
    buf[len] = '\0';

    /* copy buffer content to memory */
    aos_list_for_each_entry(aos_buf_t, content, &buffer, node) {
        size = aos_buf_size(content);
        memcpy(buf + pos, content->pos, (size_t)size);
        pos += size;
    }

    CuAssertStrEquals(tc, expect_content, buf);
    content_type = (char*)(apr_table_get(resp_headers, OSS_CONTENT_TYPE));
    CuAssertStrEquals(tc, "video/MP2T", content_type);
    aos_pool_destroy(p);

    printf("test_get_object_to_buffer ok\n");
}

void test_get_object_to_buffer_with_range(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "oss_test_put_object.ts";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;
    aos_buf_t *content = NULL;
    char *expect_content = "oss c sdk";
    char *buf = NULL;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "Range", " bytes=5-13");
    aos_list_init(&buffer);

    /* test get object to buffer */
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, 
                                 params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 206, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /* get buffer len */
    len = aos_buf_list_len(&buffer);

    buf = aos_pcalloc(p, (apr_size_t)(len + 1));
    buf[len] = '\0';

    /* copy buffer content to memory */
    aos_list_for_each_entry(aos_buf_t, content, &buffer, node) {
        size = aos_buf_size(content);
        memcpy(buf + pos, content->pos, (size_t)size);
        pos += size;
    }

    CuAssertStrEquals(tc, expect_content, buf);
    aos_pool_destroy(p);

    printf("test_get_object_to_buffer_with_range ok\n");
}

void test_get_object_to_file(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "oss_test_put_object_from_file2.txt";
    aos_string_t object;
    char *filename = "oss_test_get_object_to_file";
    char *source_filename = __FILE__;
    aos_string_t file;
    oss_request_options_t *options = NULL; 
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    char *content_type = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&file, filename);

    /* test get object to file */
    s = oss_get_object_to_file(options, &bucket, &object, headers, 
                               params, &file, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, get_file_size(source_filename), get_file_size(filename));
    content_type = (char*)(apr_table_get(resp_headers, OSS_CONTENT_TYPE));
    CuAssertStrEquals(tc, "image/jpeg", content_type);
    CuAssertPtrNotNull(tc, resp_headers);

    //negative case
    aos_str_set(&bucket, "c-sdk-no-exist");
    s = oss_get_object_to_file(options, &bucket, &object, headers,
        params, &file, &resp_headers);
    CuAssertIntEquals(tc, 404, s->code);

    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&file, "g:/invalid-path");
    s = oss_get_object_to_file(options, &bucket, &object, headers,
        params, &file, &resp_headers);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, s->code);

    remove(filename);
    aos_pool_destroy(p);

    printf("test_get_object_to_file ok\n");
}

void test_head_object_with_not_exist(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "not_exist.object";
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    headers = aos_table_make(p, 0);

    /* test head object */
    s = oss_head_object(options, &bucket, &object, headers, &resp_headers);
    CuAssertIntEquals(tc, 404, s->code);
    CuAssertStrEquals(tc, "UnknownError", s->error_code);
    CuAssertTrue(tc, NULL == s->error_msg);
    CuAssertTrue(tc, 0 != strlen(s->req_id));
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("test_head_object ok\n");
}

void test_head_object(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_put_object.ts";
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    char *user_meta = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    headers = aos_table_make(p, 0);

    /* test head object */
    s = oss_head_object(options, &bucket, &object, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    
    user_meta = (char*)(apr_table_get(resp_headers, "x-oss-meta-author"));
    CuAssertStrEquals(tc, "oss", user_meta);

    aos_pool_destroy(p);

    printf("test_head_object ok\n");
}

void test_get_object_meta_not_exist(CuTest *tc){
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "not_exist.object";
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    s = oss_get_object_meta(options, &bucket, &object, &resp_headers);
    CuAssertIntEquals(tc, 404, s->code);
    CuAssertStrEquals(tc, "UnknownError", s->error_code);
    CuAssertTrue(tc, NULL == s->error_msg);
    CuAssertTrue(tc, 0 != strlen(s->req_id));
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("test_head_object ok\n");
}

void test_get_object_meta(CuTest *tc){
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_put_object.ts";
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    
    s = oss_get_object_meta(options, &bucket, &object, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("test_get_object_meta ok\n");
}

void test_get_object_acl_not_exist(CuTest *tc){
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "not_exist.object";
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t oss_acl_str;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    
    s = oss_get_object_acl(options, &bucket, &object, &oss_acl_str, &resp_headers);
    CuAssertIntEquals(tc, 404, s->code);
    CuAssertStrEquals(tc, "NoSuchKey", s->error_code);
    CuAssertPtrNotNull(tc, resp_headers);
    
    aos_pool_destroy(p);

    printf("test_get_object_acl_not_exist ok\n");
}

void test_get_object_acl_object_empty(CuTest *tc){
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "";
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t oss_acl_str;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    
    s = oss_get_object_acl(options, &bucket, &object, &oss_acl_str, &resp_headers);
    CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
    CuAssertStrEquals(tc, AOS_EMPTY_STRING_ERROR, s->error_code);
    
    aos_pool_destroy(p);

    printf("test_get_object_acl_object_empty ok\n");
}

void test_get_object_acl_object_null(CuTest *tc){
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t oss_acl_str;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    
    s = oss_get_object_acl(options, &bucket, NULL, &oss_acl_str, &resp_headers);
    CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
    CuAssertStrEquals(tc, AOS_EMPTY_STRING_ERROR, s->error_code);

    aos_pool_destroy(p);

    printf("test_get_object_acl_object_null ok\n");
}

void test_get_object_acl(CuTest *tc){
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_put_object.ts";
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t oss_acl_str;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    
    s = oss_get_object_acl(options, &bucket, &object, &oss_acl_str, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    
    aos_pool_destroy(p);

    printf("test_get_object_acl ok\n");
}

void test_put_object_acl_invalid_acl(CuTest *tc){
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_put_object.ts";
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_acl_e oss_acl_invalid = (oss_acl_e)(OSS_ACL_DEFAULT + 1);
    
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    s = oss_put_object_acl(options, &bucket, &object, oss_acl_invalid, &resp_headers);
    CuAssertIntEquals(tc, 400, s->code);
    CuAssertStrEquals(tc, "MissingArgument", s->error_code);
    CuAssertPtrNotNull(tc, resp_headers);
    
    aos_pool_destroy(p);
    
    printf("test_put_object_acl_invalid_acl ok\n");
}

void test_put_object_acl_object_empty(CuTest *tc){
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "";
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_acl_e oss_acl = OSS_ACL_DEFAULT;
    
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    s = oss_put_object_acl(options, &bucket, &object, oss_acl, &resp_headers);
    CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
    CuAssertStrEquals(tc, AOS_EMPTY_STRING_ERROR, s->error_code);
    
    aos_pool_destroy(p);
    
    printf("test_put_object_acl_object_empty ok\n");
}

void test_put_object_acl_object_null(CuTest *tc){
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_acl_e oss_acl = OSS_ACL_DEFAULT;
    
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);

    s = oss_put_object_acl(options, &bucket, NULL, oss_acl, &resp_headers);
    CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
    CuAssertStrEquals(tc, AOS_EMPTY_STRING_ERROR, s->error_code);
    
    aos_pool_destroy(p);
    
    printf("test_put_object_acl_object_null ok\n");
}

void test_put_object_acl(CuTest *tc){
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_put_object.ts";
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;
    
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    s = oss_put_object_acl(options, &bucket, &object, oss_acl, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    
    aos_pool_destroy(p);
    
    printf("test_put_object_acl ok\n");
}

void test_delete_object(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "oss_test_put_object";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
 
    /* test delete object */
    s = oss_delete_object(options, &bucket, &object, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("test_delete_object ok\n");
}

void test_copy_object(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t source_bucket;
    char *source_object_name = "oss_test_put_object.ts";
    aos_string_t source_object;
    aos_string_t dest_bucket;
    char *dest_object_name = "oss_test_copy_object";
    aos_string_t dest_object;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_table_t *head_headers = NULL;
    aos_table_t *head_resp_headers = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&source_bucket, TEST_BUCKET_NAME);
    aos_str_set(&source_object, source_object_name);
    aos_str_set(&dest_bucket, TEST_BUCKET_NAME);
    aos_str_set(&dest_object, dest_object_name);
    headers = aos_table_make(p, 5);

    /* test copy object */
    s = oss_copy_object(options, &source_bucket, &source_object, 
                        &dest_bucket, &dest_object, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);
    apr_sleep(apr_time_from_sec(1));

    /* head object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = oss_head_object(options, &dest_bucket, &dest_object,
                        head_headers, &head_resp_headers);

    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);

    aos_pool_destroy(p);

    printf("test_copy_object ok\n");
}

void test_copy_object_with_source_url_encode(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t source_bucket;
    char *source_object_name = "y9n/g/%E9%98%B4%E9%98%B3%E5%B8%88-%E9%A3%9F%E6%A2%A6%E8%B2%98.ts";
    char *filename = __FILE__;
    aos_string_t source_object;
    aos_string_t dest_bucket;
    char *dest_object_name = "oss_test_copy_object";
    aos_string_t dest_object;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_table_t *head_headers = NULL;
    aos_table_t *head_resp_headers = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&source_bucket, TEST_BUCKET_NAME);
    aos_str_set(&source_object, source_object_name);
    aos_str_set(&dest_bucket, TEST_BUCKET_NAME);
    aos_str_set(&dest_object, dest_object_name);

    /* put object */
    s = create_test_object_from_file(options, TEST_BUCKET_NAME, 
        source_object_name, filename, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

    /* test copy object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    s = oss_copy_object(options, &source_bucket, &source_object, 
        &dest_bucket, &dest_object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);
    apr_sleep(apr_time_from_sec(1));

    /* head object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = oss_head_object(options, &dest_bucket, &dest_object,
        head_headers, &head_resp_headers);

    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);

    /* delete object */
    s = oss_delete_object(options, &source_bucket, &source_object, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code);

    s = oss_delete_object(options, &dest_bucket, &dest_object, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code);

    aos_pool_destroy(p);

    printf("test_copy_object_with_source_url_encode ok\n");
}

void test_copy_object_negative(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t source_bucket;
    char *source_object_name = NULL;
    aos_string_t source_object;
    aos_string_t dest_bucket;
    char *dest_object_name = "oss_test_copy_object";
    aos_string_t dest_object;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;

    char buffer[AOS_MAX_QUERY_ARG_LEN+1];
    memset(buffer, 'A', AOS_MAX_QUERY_ARG_LEN);
    buffer[AOS_MAX_QUERY_ARG_LEN] = '\0';
    source_object_name = buffer;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&source_bucket, TEST_BUCKET_NAME);
    aos_str_set(&source_object, source_object_name);
    aos_str_set(&dest_bucket, TEST_BUCKET_NAME);
    aos_str_set(&dest_object, dest_object_name);

    /* test copy object */
    s = oss_copy_object(options, &source_bucket, &source_object, 
        &dest_bucket, &dest_object, NULL, NULL);
    CuAssertIntEquals(tc, 400, s->code);
    CuAssertStrEquals(tc, "InvalidObjectName", s->error_code);

    aos_pool_destroy(p);

    printf("test_copy_object_negative ok\n");
}

void test_object_by_url(CuTest *tc)
{
    aos_pool_t *p = NULL;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_http_request_t *req = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t url;
    apr_time_t now;
    int two_minute = 120;
    int is_cname = 0;
    char *object_name = "oss_test_object_by_url";
    aos_string_t bucket;
    aos_string_t object;
    char *str = "test oss c sdk for object url api";
    char *filename = __FILE__;
    char *filename_download = "oss_test_object_by_url";
    aos_string_t file;
    int64_t effective_time;
    char *url_str = NULL;
    aos_buf_t *content = NULL;
    char special_query[AOS_MAX_QUERY_ARG_LEN + 1];


    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    req = aos_http_request_create(p);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&file, filename);
    aos_list_init(&buffer);

    now = apr_time_now();
    effective_time = now / 1000000 + two_minute;

    /* test effective url for put_object_from_buffer */
    req->method = HTTP_PUT;
    url_str = gen_test_signed_url(options, TEST_BUCKET_NAME, 
                                  object_name, effective_time, req);
    aos_str_set(&url, url_str);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);
    s = oss_put_object_from_buffer_by_url(options, &url, 
            &buffer, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /* test effective url for put_object_from_file */
    resp_headers = NULL;
    s = oss_put_object_from_file_by_url(options, &url, &file, 
            headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /* test effective url for get_object_to_buffer */
    req->method = HTTP_GET;
    url_str = gen_test_signed_url(options, TEST_BUCKET_NAME, 
                                  object_name, effective_time, req);
    aos_str_set(&url, url_str);
    s = oss_get_object_to_buffer_by_url(options, &url, headers, params,
            &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    /* test effective url for get_object_to_file */
    resp_headers = NULL;
    aos_str_set(&file, filename_download);
    s = oss_get_object_to_file_by_url(options, &url, headers, 
            headers, &file, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, get_file_size(filename), get_file_size(filename_download));
    CuAssertPtrNotNull(tc, resp_headers);

    /* test effective url for head_object */
    resp_headers = NULL;
    req->method = HTTP_HEAD;
    url_str = gen_test_signed_url(options, TEST_BUCKET_NAME, 
                                  object_name, effective_time, req);
    aos_str_set(&url, url_str);
    s = oss_head_object_by_url(options, &url, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    /* test invalid-bucketname url for put_object_from_file */
    req->method = HTTP_PUT;
    url_str = gen_test_signed_url(options, "InvalidBucketName",
        object_name, effective_time, req);
    resp_headers = NULL;
    s = oss_put_object_from_file_by_url(options, &url, &file,
        headers, &resp_headers);
    CuAssertIntEquals(tc, 403, s->code);

    /* test invalid filepath url for put_object_from_file */
    req->method = HTTP_PUT;
    url_str = gen_test_signed_url(options, TEST_BUCKET_NAME,
        object_name, effective_time, req);
    resp_headers = NULL;
    aos_str_set(&file, "g:/invalid-path");
    s = oss_put_object_from_file_by_url(options, &url, &file,
        headers, &resp_headers);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, s->code);


    /* test invalid-bucketname url for get_object_from_file */
    req->method = HTTP_GET;
    url_str = gen_test_signed_url(options, "InvalidBucketName",
        object_name, effective_time, req);
    resp_headers = NULL;
    aos_str_set(&file, filename_download);
    s = oss_get_object_to_file_by_url(options, &url, headers,
        headers, &file, &resp_headers);
    CuAssertIntEquals(tc, 403, s->code);

    /* test invalid filepath url for get_object_from_file */
    req->method = HTTP_GET;
    url_str = gen_test_signed_url(options, TEST_BUCKET_NAME,
        object_name, effective_time, req);
    resp_headers = NULL;
    aos_str_set(&file, "g:/invalid-path");
    s = oss_get_object_to_file_by_url(options, &url, headers,
        headers, &file, &resp_headers);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, s->code);

    /* test long query url fail*/
    memset(special_query, 0x30, AOS_MAX_QUERY_ARG_LEN);
    special_query[AOS_MAX_QUERY_ARG_LEN] = '\0';
    req->method = HTTP_GET;
    apr_table_set(req->query_params, "x-oss-process", special_query);
    url_str = gen_test_signed_url(options, TEST_BUCKET_NAME,
        object_name, effective_time, req);
    CuAssertTrue(tc, url_str == NULL);

    remove(filename_download);
    aos_pool_destroy(p);

    printf("test_object_by_url ok\n");
}

void test_append_object_from_buffer(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_append_object";
    aos_string_t bucket;
    aos_string_t object;
    char *str = "test oss c sdk";
    aos_status_t *s = NULL;
    int is_cname = 0;
    int64_t position = 0;
    aos_table_t *headers = NULL;
    aos_table_t *headers1 = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    aos_list_t buffer;
    aos_buf_t *content = NULL;
    char *next_append_position = NULL;

    /* test append object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    s = oss_head_object(options, &bucket, &object, headers, &resp_headers);
    if (s->code == 200) {
        next_append_position = (char*)(apr_table_get(resp_headers, 
                        "x-oss-next-append-position"));
        position = atoi(next_append_position);
    }
    CuAssertPtrNotNull(tc, resp_headers);

    /* append object */
    resp_headers = NULL;
    headers1 = aos_table_make(p, 0);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    s = oss_append_object_from_buffer(options, &bucket, &object, 
            position, &buffer, headers1, &resp_headers);

    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);

    printf("test_append_object_from_buffer ok\n");
}

void test_append_object_from_file(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_append_object_from_file";
    aos_string_t bucket;
    aos_string_t object;
    char *filename = __FILE__;
    aos_string_t append_file;
    aos_status_t *s = NULL;
    int is_cname = 0;
    int64_t position = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;

    /* test append object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&append_file, filename);

    s = oss_append_object_from_file(options, &bucket, &object, position, 
                                    &append_file, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    //negative case
    aos_str_set(&bucket, "c-sdk-no-exist");
    s = oss_append_object_from_file(options, &bucket, &object, position,
        &append_file, headers, &resp_headers);
    CuAssertIntEquals(tc, 400, s->code);

    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&append_file, "");
    s = oss_append_object_from_file(options, &bucket, &object, position,
        &append_file, headers, &resp_headers);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, s->code);

    aos_pool_destroy(p);

    printf("test_append_object_from_file ok\n");
}

void test_do_append_object_from_file(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_do_append_object_from_file";
    aos_string_t bucket;
    aos_string_t object;
    char *filename = __FILE__;
    aos_string_t append_file;
    aos_status_t *s = NULL;
    int is_cname = 0;
    int64_t position = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    aos_list_t resp_body;

    /* test append object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&append_file, filename);

    s = oss_do_append_object_from_file(options, &bucket, &object, position,
        0, &append_file, headers, NULL, NULL, &resp_headers, &resp_body);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    //negative case
    aos_str_set(&bucket, "c-sdk-no-exist");
    s = oss_do_append_object_from_file(options, &bucket, &object, position,
        0, &append_file, headers, NULL, NULL, &resp_headers, &resp_body);
    CuAssertIntEquals(tc, 400, s->code);

    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&append_file, "");
    s = oss_do_append_object_from_file(options, &bucket, &object, position,
        0, &append_file, headers, NULL, NULL, &resp_headers, &resp_body);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, s->code);

    aos_pool_destroy(p);

    printf("test_do_append_object_from_file ok\n");
}

void test_get_not_exist_object_to_file(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "oss_test_put_object_from_file_not_exist_.txt";
    aos_string_t object;
    char *filename = "oss_test_get_object_to_file_not_exist";
    aos_string_t file;
    oss_request_options_t *options = NULL; 
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&file, filename);

    /* test get object to file */
    s = oss_get_object_to_file(options, &bucket, &object, headers, 
        params, &file, &resp_headers);
    CuAssertIntEquals(tc, 404, s->code);
    CuAssertIntEquals(tc, -1, get_file_size(filename));

    aos_pool_destroy(p);

    printf("test_get_not_exist_object_to_file ok\n");
}

void test_put_object_from_buffer_with_invalid_endpoint(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_put_object_invalid_endpoint.ts";
    char *str = "test oss c sdk";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    oss_request_options_t *options = NULL;

    /* test put object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&options->config->endpoint, "192.168.1.1");
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    s = create_test_object(options, TEST_BUCKET_NAME, object_name, str, headers);
    CuAssertIntEquals(tc, AOSE_CONNECTION_FAILED, s->code);

    aos_pool_destroy(p);

    printf("test_put_object_from_buffer_with_invalid_endpoint ok\n");
}

void test_object_invalid_parameter(CuTest *tc)
{
    aos_pool_t *p = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    int i;
    char *invalid_name_list[] =
    { "a", "1", "!", "aa", "12", "a1",
        "a!", "1!", "aAa", "1A1", "a!a", "FengChao@123", "-a123", "a_123", "a123-",
        "1234567890123456789012345678901234567890123456789012345678901234", ""
    };

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    for (i = 0; i < sizeof(invalid_name_list) / sizeof(invalid_name_list[0]); i++) {
        aos_string_t bucket;
        aos_status_t *s = NULL;
        aos_table_t *resp_headers = NULL;
        aos_table_t *headers = NULL;
        aos_table_t *params = NULL;
        aos_str_set(&bucket, invalid_name_list[i]);
        headers = aos_table_make(p, 1);

        s = oss_put_object_from_buffer(options, &bucket, NULL, NULL, headers, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_put_object_from_file(options, &bucket, NULL, NULL, headers, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_get_object_to_buffer(options, &bucket, NULL, headers, params, NULL, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_restore_object(options, &bucket, NULL, headers, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_get_object_to_file(options, &bucket, NULL, headers, params, NULL, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_head_object(options, &bucket, NULL, headers, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_get_object_meta(options, &bucket, NULL, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_put_object_acl(options, &bucket, NULL, OSS_ACL_DEFAULT, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_put_object_acl(options, &bucket, NULL, OSS_ACL_DEFAULT, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_put_symlink(options, &bucket, NULL, NULL, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_get_symlink(options, &bucket, NULL, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_delete_object(options, &bucket, NULL, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_copy_object(options, &bucket, NULL, NULL, NULL, headers, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_append_object_from_buffer(options, &bucket, NULL, 1024LL, NULL, headers, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_append_object_from_file(options, &bucket, NULL, 1024LL, NULL, headers, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);
#if 0
        s = oss_put_object_from_buffer_by_url(options, &bucket, NULL, headers, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_put_object_from_file_by_url(options, &bucket, NULL, headers, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_get_object_to_buffer_by_url(options, &bucket, headers, params, NULL, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_get_object_to_file_by_url(options, &bucket, headers, params, NULL, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_head_object_by_url(options, &bucket, headers, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);
#endif
    }
    aos_pool_destroy(p);

    printf("test_object_invalid_parameter ok\n");
}

void test_get_object_to_buffer_with_maxbuffersize(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "video_1.ts";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    options->ctl->options = aos_http_request_options_create(options->pool);
    options->ctl->options->max_memory_size = 4;
    options->ctl->options->enable_crc = AOS_FALSE;

    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&buffer);

    /* test get object to buffer */
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, 
                                 params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, AOSE_OVER_MEMORY, s->code);


    options->ctl->options->enable_crc = AOS_TRUE;
    /* test get object to buffer */
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, 
                                 params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, AOSE_CRC_INCONSISTENT_ERROR, s->code);

    aos_pool_destroy(p);

    printf("test_get_object_to_buffer_with_maxbuffersize ok\n");

}

void test_get_object_to_buffer_use_invalid_sts(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "video_1.ts";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&options->config->sts_token, "invalid-sts");

    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&buffer);

    /* test get object to buffer */
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, 
                                 params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 403, s->code);

    aos_pool_destroy(p);

    printf("test_get_object_to_buffer_use_invalid_sts ok\n");

}



CuSuite *test_oss_object()
{
    CuSuite* suite = CuSuiteNew();   

    SUITE_ADD_TEST(suite, test_object_setup);
    SUITE_ADD_TEST(suite, test_put_object_from_buffer);
    SUITE_ADD_TEST(suite, test_put_object_from_file);
    SUITE_ADD_TEST(suite, test_put_object_from_buffer_with_specified);
    SUITE_ADD_TEST(suite, test_get_object_to_buffer);
    SUITE_ADD_TEST(suite, test_get_object_to_buffer_with_range);
    SUITE_ADD_TEST(suite, test_put_object_from_file_with_content_type);
    SUITE_ADD_TEST(suite, test_put_symlink_for_obj);
    SUITE_ADD_TEST(suite, test_get_symlink_for_obj);
    SUITE_ADD_TEST(suite, test_restore_obj);
    SUITE_ADD_TEST(suite, test_put_object_from_buffer_with_default_content_type);
    SUITE_ADD_TEST(suite, test_put_object_with_large_length_header);
    SUITE_ADD_TEST(suite, test_get_object_to_file);
    SUITE_ADD_TEST(suite, test_head_object);
    SUITE_ADD_TEST(suite, test_head_object_with_not_exist);
    SUITE_ADD_TEST(suite, test_get_object_meta_not_exist);
    SUITE_ADD_TEST(suite, test_get_object_meta);
    SUITE_ADD_TEST(suite, test_get_object_acl_not_exist);
    SUITE_ADD_TEST(suite, test_get_object_acl_object_empty);
    SUITE_ADD_TEST(suite, test_get_object_acl_object_null);
    SUITE_ADD_TEST(suite, test_get_object_acl);
    SUITE_ADD_TEST(suite, test_put_object_acl_invalid_acl);
    SUITE_ADD_TEST(suite, test_put_object_acl_object_empty);
    SUITE_ADD_TEST(suite, test_put_object_acl_object_null);
    SUITE_ADD_TEST(suite, test_put_object_acl);
    SUITE_ADD_TEST(suite, test_copy_object);
    SUITE_ADD_TEST(suite, test_copy_object_with_source_url_encode);
    SUITE_ADD_TEST(suite, test_copy_object_negative);
    SUITE_ADD_TEST(suite, test_object_by_url);
    SUITE_ADD_TEST(suite, test_delete_object);
    SUITE_ADD_TEST(suite, test_append_object_from_buffer);
    SUITE_ADD_TEST(suite, test_append_object_from_file);
    SUITE_ADD_TEST(suite, test_do_append_object_from_file);
    SUITE_ADD_TEST(suite, test_get_not_exist_object_to_file);
    SUITE_ADD_TEST(suite, test_put_object_from_buffer_with_invalid_endpoint);
    SUITE_ADD_TEST(suite, test_object_invalid_parameter);
    SUITE_ADD_TEST(suite, test_get_object_to_buffer_with_maxbuffersize);
    SUITE_ADD_TEST(suite, test_get_object_to_buffer_use_invalid_sts);
    SUITE_ADD_TEST(suite, test_object_cleanup); 
    
    return suite;
}
