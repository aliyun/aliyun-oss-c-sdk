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

void test_progress_setup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;

    /* create test bucket */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);

    CuAssertIntEquals(tc, 200, s->code);
    aos_pool_destroy(p);
}

void test_progress_cleanup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    oss_request_options_t *options = NULL;
    char *object_name1 = "oss_test_progress_put_object.ts";
    char *object_name2 = "oss_test_progress_append_object.ts";
    char *object_name3 = "oss_test_progress_multipart_object.ts";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    /* delete test object */
    delete_test_object(options, TEST_BUCKET_NAME, object_name1);
    delete_test_object(options, TEST_BUCKET_NAME, object_name2);
    delete_test_object(options, TEST_BUCKET_NAME, object_name3);

    /* delete test bucket */
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    oss_delete_bucket(options, &bucket, NULL);
    apr_sleep(apr_time_from_sec(3));

    aos_pool_destroy(p);
}

void test_progress_put_and_get_from_buffer(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_progress_put_object.ts";
    char *str = NULL;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    size_t length = 1024 * 16 * 10;
    aos_list_t resp_body;
    aos_list_t buffer;
    aos_buf_t *content;

    /* init test*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
   
    str = (char *)aos_palloc(p, length);
    memset(str, 'A', length - 1);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, length);
    aos_list_add_tail(&content->node, &buffer);

    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    
    /* test put object */
    s = oss_do_put_object_from_buffer(options, &bucket, &object, &buffer, 
        headers, params, percentage, &resp_headers, &resp_body);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, headers);
    aos_pool_destroy(p);

    /* test get object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    s = oss_do_get_object_to_buffer(options, &bucket, &object, NULL, NULL, 
        &buffer, percentage, NULL);
    CuAssertIntEquals(tc, 200, s->code);
    aos_pool_destroy(p);

    printf("test_progress_put_object_from_buffer ok\n");
}

void test_progress_put_and_get_from_file(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_progress_put_object.ts";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    size_t length = 1024 * 16 * 10;
    aos_list_t resp_body;

    /* init test*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
   
    make_random_file(p, object_name, length);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&filename, object_name);
    aos_list_init(&resp_body);

    /* test put object */
    s = oss_do_put_object_from_file(options, &bucket, &object, &filename, 
        NULL, NULL, percentage, &resp_headers, &resp_body);
    CuAssertIntEquals(tc, 200, s->code);
    
    aos_pool_destroy(p);

    /* test get object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    s = oss_do_get_object_to_file(options, &bucket, &object, NULL, NULL, 
        &filename, percentage, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    apr_file_remove(object_name, p);
    aos_pool_destroy(p);

    printf("test_progress_put_and_get_from_file ok\n");
}

void test_progress_append_object(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_progress_append_object.ts";
    char *str = NULL;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    size_t length = 1024 * 16 * 20;
    uint64_t initcrc = 0;
    aos_list_t resp_body;
    aos_list_t buffer;
    aos_buf_t *content;

    /* init test*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
   
    str = (char *)aos_palloc(p, length);
    memset(str, 'A', length - 1);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, length);
    aos_list_add_tail(&content->node, &buffer);

    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    
    /* test append object from buffer */
    s = oss_do_append_object_from_buffer(options, &bucket, &object, 0, initcrc, &buffer, 
        headers, params, percentage, &resp_headers, &resp_body);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

    /* test append object from file*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    aos_str_set(&filename, object_name);
    make_random_file(p, object_name, length);
    initcrc = aos_atoui64((char*)(apr_table_get(resp_headers, OSS_HASH_CRC64_ECMA)));

    s = oss_do_append_object_from_file(options, &bucket, &object, length, initcrc, &filename, 
        NULL, NULL, percentage, &resp_headers, &resp_body);
    CuAssertIntEquals(tc, 200, s->code);

    apr_file_remove(object_name, p);
    aos_pool_destroy(p);

    printf("test_progress_append_object ok\n");
}

void test_progress_multipart_from_buffer(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "oss_test_progress_multipart_object.ts";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;
    oss_list_upload_part_params_t *params = NULL;
    aos_string_t upload_id;
    aos_list_t complete_part_list;
    oss_list_part_content_t *part_content1 = NULL;
    oss_complete_part_content_t *complete_content1 = NULL;
    int part_num = 1;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    //init mulitipart
    s = init_test_multipart_upload(options, TEST_BUCKET_NAME, object_name, &upload_id);
    CuAssertIntEquals(tc, 200, s->code);

    //upload part
    aos_list_init(&buffer);
    make_random_body(p, 200, &buffer);

    s = oss_do_upload_part_from_buffer(options, &bucket, &object, &upload_id,
        part_num++, &buffer, percentage, NULL, NULL, NULL, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    aos_list_init(&buffer);
    make_random_body(p, 200, &buffer);
    s = oss_do_upload_part_from_buffer(options, &bucket, &object, &upload_id,
        part_num++, &buffer, percentage, NULL, NULL, NULL, NULL);
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
    s = oss_complete_multipart_upload(options, &bucket, &object, &upload_id,
            &complete_part_list, NULL, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

    printf("test_progress_multipart_from_buffer ok\n");
}

void test_progress_multipart_from_file(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "oss_test_progress_multipart_object.ts";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_status_t *s = NULL;
    oss_list_upload_part_params_t *params = NULL;
    aos_string_t upload_id;
    aos_list_t complete_part_list;
    oss_upload_file_t *upload_file = NULL;
    oss_list_part_content_t *part_content1 = NULL;
    oss_complete_part_content_t *complete_content1 = NULL;
    size_t length = 1024 * 16 * 10;
    int part_num = 1;    

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    make_random_file(p, object_name, length);
    upload_file = oss_create_upload_file(p);
    aos_str_set(&upload_file->filename, object_name);

    //init mulitipart
    s = init_test_multipart_upload(options, TEST_BUCKET_NAME, object_name, &upload_id);
    CuAssertIntEquals(tc, 200, s->code);

    //upload part
    upload_file->file_pos = 0;
    upload_file->file_last = length/2;
    s = oss_do_upload_part_from_file(options, &bucket, &object, &upload_id,
        part_num++, upload_file, percentage, NULL, NULL, NULL, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    upload_file->file_pos = length/2;
    upload_file->file_last = length;
    s = oss_do_upload_part_from_file(options, &bucket, &object, &upload_id,
        part_num++, upload_file, percentage, NULL, NULL, NULL, NULL);
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
    s = oss_complete_multipart_upload(options, &bucket, &object, &upload_id,
            &complete_part_list, NULL, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    apr_file_remove(object_name, p);
    aos_pool_destroy(p);

    printf("void test_progress_multipart_from_file ok\n");
}

CuSuite *test_oss_progress()
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_progress_setup);
    SUITE_ADD_TEST(suite, test_progress_put_and_get_from_buffer);
    SUITE_ADD_TEST(suite, test_progress_put_and_get_from_file);
    SUITE_ADD_TEST(suite, test_progress_append_object);
    SUITE_ADD_TEST(suite, test_progress_multipart_from_buffer); 
    SUITE_ADD_TEST(suite, test_progress_multipart_from_file); 
    SUITE_ADD_TEST(suite, test_progress_cleanup);

    return suite;
}
