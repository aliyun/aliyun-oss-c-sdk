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

static char test_file[1024];

void test_object_tagging_setup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;

    TEST_BUCKET_NAME   = get_test_bucket_name(aos_global_pool, "tagging");
    
    /* create test bucket */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

    sprintf(test_file, "%sBingWallpaper-2017-01-19.jpg", get_test_file_path());

}

void test_object_tagging_cleanup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    oss_request_options_t *options = NULL;
    char *object_name1 = "test_object_tagging_basic";
    char *object_name2 = "test_object_tagging_tag_argument";
    char *object_name3 = "test_object_tagging_put_object";
    char *object_name4 = "test_object_tagging_append_object";
    char *object_name5 = "test_object_tagging_copy_object_source";
    char *object_name6 = "test_object_tagging_copy_object_dest";
    char *object_name7 = "test_object_tagging_oss_object";
    char *object_name8 = "test_object_tagging_link_to_oss_object";
    char *object_name9 = "test_object_tagging_multipart_upload";
    char *object_name10 = "test_object_tagging_resumable_upload";
    

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

    aos_pool_destroy(p);
}

void test_object_tagging_basic(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_object_tagging_basic";
    char *str = "test oss c sdk";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *headers = NULL;
    aos_table_t *head_resp_headers = NULL;
    oss_request_options_t *options = NULL;
    aos_list_t tag_list;
    oss_tag_content_t *tag_content;
    int index;

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

    /* get object tagging*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    aos_list_init(&tag_list);
    s = oss_get_object_tagging(options, &bucket, &object, 
        &tag_list, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
    index = 0;
    aos_list_for_each_entry(oss_tag_content_t, tag_content, &tag_list, node) {
        index++;
    }
    CuAssertIntEquals(tc, 0, index);
    aos_pool_destroy(p);


    /*set object tagging*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);

    aos_list_init(&tag_list);
    tag_content = oss_create_tag_content(p);
    aos_str_set(&tag_content->key, "key1");
    aos_str_set(&tag_content->value, "value1");
    aos_list_add_tail(&tag_content->node, &tag_list);
    tag_content = oss_create_tag_content(p);
    aos_str_set(&tag_content->key, "key2");
    aos_str_set(&tag_content->value, "value2");
    aos_list_add_tail(&tag_content->node, &tag_list);

    s = oss_put_object_tagging(options, &bucket, &object,
        &tag_list, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);

    aos_pool_destroy(p);


    /*get object tagging again*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    aos_list_init(&tag_list);
    s = oss_get_object_tagging(options, &bucket, &object,
        &tag_list, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
    index = 0;
    aos_list_for_each_entry(oss_tag_content_t, tag_content, &tag_list, node) {
        char *value = NULL;
        if (index == 0) {
            value = apr_psprintf(p, "%.*s", tag_content->key.len,
                tag_content->key.data);
            CuAssertStrEquals(tc, "key1", value);
            value = apr_psprintf(p, "%.*s", tag_content->value.len,
                tag_content->value.data);
            CuAssertStrEquals(tc, "value1", value);
        }
        else if (index == 1) {
            value = apr_psprintf(p, "%.*s", tag_content->key.len,
                tag_content->key.data);
            CuAssertStrEquals(tc, "key2", value);
            value = apr_psprintf(p, "%.*s", tag_content->value.len,
                tag_content->value.data);
            CuAssertStrEquals(tc, "value2", value);
        }
        index++;
    }
    CuAssertIntEquals(tc, 2, index);
    aos_pool_destroy(p);


    /*delete object tagging*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    s = oss_delete_object_tagging(options, &bucket, &object, &head_resp_headers);
    CuAssertIntEquals(tc, 204, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);

    aos_pool_destroy(p);

    /*get object tagging again*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    aos_list_init(&tag_list);
    s = oss_get_object_tagging(options, &bucket, &object,
        &tag_list, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
    index = 0;
    aos_list_for_each_entry(oss_tag_content_t, tag_content, &tag_list, node) {
        index++;
    }
    CuAssertIntEquals(tc, 0, index);
    aos_pool_destroy(p);

    //negative case
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, "c-sdk-no-exist");
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    aos_list_init(&tag_list);
    s = oss_get_object_tagging(options, &bucket, &object,
        &tag_list, &head_resp_headers);
    CuAssertIntEquals(tc, 404, s->code);
    aos_pool_destroy(p);

    printf("test_object_tagging_basic ok\n");
}

void test_object_tagging_tag_list(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_object_tagging_tag_argument";
    char *str = "test oss c sdk";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *headers = NULL;
    aos_table_t *head_resp_headers = NULL;
    oss_request_options_t *options = NULL;
    aos_list_t tag_list;
    oss_tag_content_t *tag_content;

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

    /*set object tagging with empty tag list*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    aos_list_init(&tag_list);
    s = oss_put_object_tagging(options, &bucket, &object,
        &tag_list, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);

    aos_pool_destroy(p);


    /*set object tagging with empty tag.key */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);

    aos_list_init(&tag_list);
    tag_content = oss_create_tag_content(p);
    aos_str_set(&tag_content->key, "key1");
    aos_list_add_tail(&tag_content->node, &tag_list);

    s = oss_put_object_tagging(options, &bucket, &object,
        &tag_list, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);

    aos_pool_destroy(p);


    /*set object tagging with empty tag.value */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);

    aos_list_init(&tag_list);
    tag_content = oss_create_tag_content(p);
    aos_str_set(&tag_content->value, "value1");
    aos_list_add_tail(&tag_content->node, &tag_list);

    s = oss_put_object_tagging(options, &bucket, &object,
        &tag_list, &head_resp_headers);
    CuAssertIntEquals(tc, 400, s->code);
    CuAssertIntEquals(tc, 400, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);

    aos_pool_destroy(p);

    printf("test_object_tagging_tag_list ok\n");
}

void test_object_tagging_put_object(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_object_tagging_put_object";
    char *str = "test oss c sdk";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *headers = NULL;
    aos_table_t *head_resp_headers = NULL;
    oss_request_options_t *options = NULL;
    aos_list_t tag_list;
    oss_tag_content_t *tag_content;
    int index;
    aos_list_t buffer;
    aos_buf_t *content;

    /* test put object with tagging */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-tagging", "key1=value1&key2=value2");

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    s = oss_put_object_from_buffer(options, &bucket, &object,
        &buffer, headers, &head_resp_headers);

    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, headers);

    aos_pool_destroy(p);

    /*get object tagging again*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    aos_list_init(&tag_list);
    s = oss_get_object_tagging(options, &bucket, &object,
        &tag_list, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
    index = 0;
    aos_list_for_each_entry(oss_tag_content_t, tag_content, &tag_list, node) {
        char *value = NULL;
        if (index == 0) {
            value = apr_psprintf(p, "%.*s", tag_content->key.len,
                tag_content->key.data);
            CuAssertStrEquals(tc, "key1", value);
            value = apr_psprintf(p, "%.*s", tag_content->value.len,
                tag_content->value.data);
            CuAssertStrEquals(tc, "value1", value);
        }
        else if (index == 1) {
            value = apr_psprintf(p, "%.*s", tag_content->key.len,
                tag_content->key.data);
            CuAssertStrEquals(tc, "key2", value);
            value = apr_psprintf(p, "%.*s", tag_content->value.len,
                tag_content->value.data);
            CuAssertStrEquals(tc, "value2", value);
        }
        index++;
    }
    CuAssertIntEquals(tc, 2, index);
    aos_pool_destroy(p);

    printf("test_object_tagging_put_object ok\n");
}

void test_object_tagging_append_object(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_object_tagging_append_object";
    char *str = "test oss c sdk";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *headers = NULL;
    aos_table_t *head_resp_headers = NULL;
    oss_request_options_t *options = NULL;
    aos_list_t tag_list;
    oss_tag_content_t *tag_content;
    int index;
    aos_list_t buffer;
    aos_buf_t *content;

    /* test put object with tagging */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-tagging", "key1=value1&key2=value2");

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    s = oss_append_object_from_buffer(options, &bucket, &object, 0,
        &buffer, headers, &head_resp_headers);

    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, headers);

    aos_pool_destroy(p);

    /*get object tagging*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    aos_list_init(&tag_list);
    s = oss_get_object_tagging(options, &bucket, &object,
        &tag_list, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
    index = 0;
    aos_list_for_each_entry(oss_tag_content_t, tag_content, &tag_list, node) {
        char *value = NULL;
        if (index == 0) {
            value = apr_psprintf(p, "%.*s", tag_content->key.len,
                tag_content->key.data);
            CuAssertStrEquals(tc, "key1", value);
            value = apr_psprintf(p, "%.*s", tag_content->value.len,
                tag_content->value.data);
            CuAssertStrEquals(tc, "value1", value);
        }
        else if (index == 1) {
            value = apr_psprintf(p, "%.*s", tag_content->key.len,
                tag_content->key.data);
            CuAssertStrEquals(tc, "key2", value);
            value = apr_psprintf(p, "%.*s", tag_content->value.len,
                tag_content->value.data);
            CuAssertStrEquals(tc, "value2", value);
        }
        index++;
    }
    CuAssertIntEquals(tc, 2, index);
    aos_pool_destroy(p);

    printf("test_object_tagging_append_object ok\n");
}

void test_object_tagging_put_symlink(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_object_tagging_oss_object";
    char *link_object_name = "test_object_tagging_link_to_oss_object";
    aos_string_t bucket;
    aos_string_t sym_object;
    aos_string_t target_object;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *head_resp_headers = NULL;
    aos_list_t tag_list;
    oss_tag_content_t *tag_content;
    int index;

    /*put object*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-tagging", "key3=value3");
    s = create_test_object(options, TEST_BUCKET_NAME, object_name, "hello world", headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, headers);
    aos_pool_destroy(p);

    /* link object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-tagging", "key1=value1");
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&sym_object, link_object_name);
    aos_str_set(&target_object, object_name);
    init_test_request_options(options, is_cname);
    s = oss_do_put_symlink(options, &bucket, &sym_object,
        &target_object, headers, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
    aos_pool_destroy(p);

    /*get object tagging*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_list_init(&tag_list);
    s = oss_get_object_tagging(options, &bucket, &sym_object,
        &tag_list, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
    index = 0;
    aos_list_for_each_entry(oss_tag_content_t, tag_content, &tag_list, node) {
        char *value = NULL;
        if (index == 0) {
            value = apr_psprintf(p, "%.*s", tag_content->key.len,
                tag_content->key.data);
            CuAssertStrEquals(tc, "key1", value);
            value = apr_psprintf(p, "%.*s", tag_content->value.len,
                tag_content->value.data);
            CuAssertStrEquals(tc, "value1", value);
        }
        index++;
    }
    CuAssertIntEquals(tc, 1, index);
    aos_pool_destroy(p);

    printf("test_object_tagging_put_symlink ok\n");
}

void test_object_tagging_copy_object(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t source_bucket;
    char *source_object_name = "test_object_tagging_copy_object_source";
    aos_string_t source_object;
    aos_string_t dest_bucket;
    char *dest_object_name = "test_object_tagging_copy_object_dest";
    aos_string_t dest_object;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t tag_list;
    oss_tag_content_t *tag_content;
    int index;

    /*put object*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-tagging", "key3=value3");
    s = create_test_object(options, TEST_BUCKET_NAME, source_object_name, "hello world", headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, headers);

    aos_pool_destroy(p);


    /* test copy object with directive copy*/
    aos_pool_create(&p, NULL);
    headers = aos_table_make(p, 1);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&source_bucket, TEST_BUCKET_NAME);
    aos_str_set(&source_object, source_object_name);
    aos_str_set(&dest_bucket, TEST_BUCKET_NAME);
    aos_str_set(&dest_object, dest_object_name);
    apr_table_set(headers, "x-oss-tagging-directive", "Copy");

    s = oss_copy_object(options, &source_bucket, &source_object,
        &dest_bucket, &dest_object, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);
    apr_sleep(apr_time_from_sec(1));

    /*get object tagging*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_list_init(&tag_list);
    s = oss_get_object_tagging(options, &dest_bucket, &dest_object,
        &tag_list, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    index = 0;
    aos_list_for_each_entry(oss_tag_content_t, tag_content, &tag_list, node) {
        char *value = NULL;
        if (index == 0) {
            value = apr_psprintf(p, "%.*s", tag_content->key.len,
                tag_content->key.data);
            CuAssertStrEquals(tc, "key3", value);
            value = apr_psprintf(p, "%.*s", tag_content->value.len,
                tag_content->value.data);
            CuAssertStrEquals(tc, "value3", value);
        }
        index++;
    }
    CuAssertIntEquals(tc, 1, index);
    aos_pool_destroy(p);

    /* test copy object with directive replace*/
    aos_pool_create(&p, NULL);
    headers = aos_table_make(p, 1);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&source_bucket, TEST_BUCKET_NAME);
    aos_str_set(&source_object, source_object_name);
    aos_str_set(&dest_bucket, TEST_BUCKET_NAME);
    aos_str_set(&dest_object, dest_object_name);
    apr_table_set(headers, "x-oss-tagging-directive", "Replace");
    apr_table_set(headers, "x-oss-tagging", "key1=value1&key2=value2");

    s = oss_copy_object(options, &source_bucket, &source_object,
        &dest_bucket, &dest_object, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);
    apr_sleep(apr_time_from_sec(1));

    /*get object tagging*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_list_init(&tag_list);
    s = oss_get_object_tagging(options, &dest_bucket, &dest_object,
        &tag_list, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    index = 0;
    aos_list_for_each_entry(oss_tag_content_t, tag_content, &tag_list, node) {
        char *value = NULL;
        if (index == 0) {
            value = apr_psprintf(p, "%.*s", tag_content->key.len,
                tag_content->key.data);
            CuAssertStrEquals(tc, "key1", value);
            value = apr_psprintf(p, "%.*s", tag_content->value.len,
                tag_content->value.data);
            CuAssertStrEquals(tc, "value1", value);
        }
        index++;
    }
    CuAssertIntEquals(tc, 2, index);
    aos_pool_destroy(p);

    printf("test_object_tagging_copy_object ok\n");
}

void test_object_tagging_multipart_upload(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "test_object_tagging_multipart_upload";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;
    aos_table_t *headers = NULL;
    aos_table_t *upload_part_resp_headers = NULL;
    oss_list_upload_part_params_t *params = NULL;
    aos_table_t *list_part_resp_headers = NULL;
    aos_string_t upload_id;
    aos_list_t complete_part_list;
    oss_list_part_content_t *part_content1 = NULL;
    oss_list_part_content_t *part_content2 = NULL;
    oss_complete_part_content_t *complete_content1 = NULL;
    oss_complete_part_content_t *complete_content2 = NULL;
    aos_table_t *complete_resp_headers = NULL;
    aos_table_t *head_resp_headers = NULL;
    int part_num = 1;
    int part_num1 = 2;
    char *expect_part_num_marker = "1";
    char *content_type_for_complete = "application/octet-stream";
    aos_list_t tag_list;
    oss_tag_content_t *tag_content;
    int index;
    aos_table_t *resp_headers;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    headers = aos_table_make(options->pool, 2);

    //init mulitipart
    apr_table_add(headers, "x-oss-tagging", "key1=value1&key2=value2");
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    s = oss_init_multipart_upload(options, &bucket, &object,
        &upload_id, headers, &resp_headers);

    CuAssertIntEquals(tc, 200, s->code);

    //upload part
    aos_list_init(&buffer);
    make_random_body(p, 200, &buffer);

    s = oss_upload_part_from_buffer(options, &bucket, &object, &upload_id,
        part_num, &buffer, &upload_part_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, upload_part_resp_headers);

    aos_list_init(&buffer);
    make_random_body(p, 200, &buffer);
    s = oss_upload_part_from_buffer(options, &bucket, &object, &upload_id,
        part_num1, &buffer, &upload_part_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, upload_part_resp_headers);

    //list part
    params = oss_create_list_upload_part_params(p);
    params->max_ret = 1;
    aos_list_init(&complete_part_list);

    s = oss_list_upload_part(options, &bucket, &object, &upload_id,
        params, &list_part_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 1, params->truncated);
    CuAssertStrEquals(tc, expect_part_num_marker,
        params->next_part_number_marker.data);
    CuAssertPtrNotNull(tc, list_part_resp_headers);

    aos_list_for_each_entry(oss_list_part_content_t, part_content1, &params->part_list, node) {
        complete_content1 = oss_create_complete_part_content(p);
        aos_str_set(&complete_content1->part_number, part_content1->part_number.data);
        aos_str_set(&complete_content1->etag, part_content1->etag.data);
        aos_list_add_tail(&complete_content1->node, &complete_part_list);
    }

    aos_list_init(&params->part_list);
    aos_str_set(&params->part_number_marker, params->next_part_number_marker.data);
    s = oss_list_upload_part(options, &bucket, &object, &upload_id, params, &list_part_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 0, params->truncated);
    CuAssertPtrNotNull(tc, list_part_resp_headers);

    aos_list_for_each_entry(oss_list_part_content_t, part_content2, &params->part_list, node) {
        complete_content2 = oss_create_complete_part_content(p);
        aos_str_set(&complete_content2->part_number, part_content2->part_number.data);
        aos_str_set(&complete_content2->etag, part_content2->etag.data);
        aos_list_add_tail(&complete_content2->node, &complete_part_list);
    }

    //complete multipart
    apr_table_clear(headers);
    apr_table_add(headers, OSS_CONTENT_TYPE, content_type_for_complete);
    s = oss_complete_multipart_upload(options, &bucket, &object, &upload_id,
        &complete_part_list, headers, &complete_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, complete_resp_headers);

    aos_pool_destroy(p);


    /*get object tagging*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    aos_list_init(&tag_list);
    s = oss_get_object_tagging(options, &bucket, &object,
        &tag_list, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
    index = 0;
    aos_list_for_each_entry(oss_tag_content_t, tag_content, &tag_list, node) {
        char *value = NULL;
        if (index == 0) {
            value = apr_psprintf(p, "%.*s", tag_content->key.len,
                tag_content->key.data);
            CuAssertStrEquals(tc, "key1", value);
            value = apr_psprintf(p, "%.*s", tag_content->value.len,
                tag_content->value.data);
            CuAssertStrEquals(tc, "value1", value);
        }
        else if (index == 1) {
            value = apr_psprintf(p, "%.*s", tag_content->key.len,
                tag_content->key.data);
            CuAssertStrEquals(tc, "key2", value);
            value = apr_psprintf(p, "%.*s", tag_content->value.len,
                tag_content->value.data);
            CuAssertStrEquals(tc, "value2", value);
        }
        index++;
    }
    CuAssertIntEquals(tc, 2, index);
    aos_pool_destroy(p);

    printf("test_object_tagging_multipart ok\n");
}

void test_object_tagging_resumale_upload(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_object_tagging_resumable_upload";
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
    aos_list_t tag_list;
    oss_tag_content_t *tag_content;
    int index;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    apr_table_add(headers, "x-oss-tagging", "key1=value1&key2=value2");
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

    /*get object tagging*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    init_test_request_options(options, is_cname);
    aos_list_init(&tag_list);
    apr_table_clear(resp_headers);
    s = oss_get_object_tagging(options, &bucket, &object,
        &tag_list, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    index = 0;
    aos_list_for_each_entry(oss_tag_content_t, tag_content, &tag_list, node) {
        char *value = NULL;
        if (index == 0) {
            value = apr_psprintf(p, "%.*s", tag_content->key.len,
                tag_content->key.data);
            CuAssertStrEquals(tc, "key1", value);
            value = apr_psprintf(p, "%.*s", tag_content->value.len,
                tag_content->value.data);
            CuAssertStrEquals(tc, "value1", value);
        }
        else if (index == 1) {
            value = apr_psprintf(p, "%.*s", tag_content->key.len,
                tag_content->key.data);
            CuAssertStrEquals(tc, "key2", value);
            value = apr_psprintf(p, "%.*s", tag_content->value.len,
                tag_content->value.data);
            CuAssertStrEquals(tc, "value2", value);
        }
        index++;
    }
    CuAssertIntEquals(tc, 2, index);
    aos_pool_destroy(p);

    printf("test_object_tagging_resumale_upload ok\n");
}

void test_lifecycle_tag(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t lifecycle_rule_list;
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
    oss_tag_content_t *tag_content = NULL;
    int tag_index = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);

    //put lifecycle
    resp_headers = NULL;
    aos_list_init(&lifecycle_rule_list);
    rule_content1 = oss_create_lifecycle_rule_content(p);
    aos_str_set(&rule_content1->id, "1");
    aos_str_set(&rule_content1->prefix, "pre1");
    aos_str_set(&rule_content1->status, "Enabled");
    rule_content1->days = 1;
    tag_content = oss_create_tag_content(p);
    aos_str_set(&tag_content->key, "pre1key1");
    aos_str_set(&tag_content->value, "pre1value1");
    aos_list_add_tail(&tag_content->node, &rule_content1->tag_list);
    tag_content = oss_create_tag_content(p);
    aos_str_set(&tag_content->key, "pre1key2");
    aos_str_set(&tag_content->value, "pre1value2");
    aos_list_add_tail(&tag_content->node, &rule_content1->tag_list);

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
    rule_content3->abort_multipart_upload_dt.days = 1;

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
            tag_index = 0;
            aos_list_for_each_entry(oss_tag_content_t, tag_content, &rule_content->tag_list, node) {
                if (tag_index == 0) {
                    CuAssertStrEquals(tc, "pre1key1", tag_content->key.data);
                    CuAssertStrEquals(tc, "pre1value1", tag_content->value.data);
                }
                else if (tag_index == 1) {
                    CuAssertStrEquals(tc, "pre1key2", tag_content->key.data);
                    CuAssertStrEquals(tc, "pre1value2", tag_content->value.data);
                }
                tag_index++;
            }
            CuAssertIntEquals(tc, 2, tag_index);
        }
        else if (size == 1) {
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
            CuAssertIntEquals(tc, 1, aos_list_empty(&rule_content->tag_list));
        }
        else if (size == 2) {
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
            CuAssertIntEquals(tc, 1, aos_list_empty(&rule_content->tag_list));
        }
        else if (size == 3) {
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
            CuAssertIntEquals(tc, 1, aos_list_empty(&rule_content->tag_list));
        }

        ++size;
    }
    CuAssertIntEquals(tc, 4, size);

    //delete lifecycle
    resp_headers = NULL;
    s = oss_delete_bucket_lifecycle(options, &bucket, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    aos_pool_destroy(p);

    printf("test_lifecycle ok\n");
}

void test_object_tagging_invalid_parameter(CuTest *tc)
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
        aos_str_set(&bucket, invalid_name_list[i]);

        s = oss_put_object_tagging(options, &bucket, NULL, NULL, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_get_object_tagging(options, &bucket, NULL, NULL, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_delete_object_tagging(options, &bucket, NULL, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);
    }
    aos_pool_destroy(p);

    printf("test_object_tagging_invalid_parameter ok\n");
}


CuSuite *test_oss_object_tagging()
{
    CuSuite* suite = CuSuiteNew();   

    SUITE_ADD_TEST(suite, test_object_tagging_setup);
    SUITE_ADD_TEST(suite, test_object_tagging_basic);
    SUITE_ADD_TEST(suite, test_object_tagging_tag_list);
    SUITE_ADD_TEST(suite, test_object_tagging_put_object);
    SUITE_ADD_TEST(suite, test_object_tagging_append_object);
    SUITE_ADD_TEST(suite, test_object_tagging_put_symlink);
    SUITE_ADD_TEST(suite, test_object_tagging_copy_object);
    SUITE_ADD_TEST(suite, test_object_tagging_multipart_upload);
    SUITE_ADD_TEST(suite, test_object_tagging_resumale_upload);
    SUITE_ADD_TEST(suite, test_lifecycle_tag);
    SUITE_ADD_TEST(suite, test_object_tagging_invalid_parameter);
    SUITE_ADD_TEST(suite, test_object_tagging_cleanup);
    
    return suite;
}
