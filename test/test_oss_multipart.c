#include "CuTest.h"
#include "test.h"
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

void test_multipart_setup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_oss_domain = 1;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;

    //create test bucket
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);

    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);
}

void test_multipart_cleanup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_oss_domain = 1;
    aos_string_t bucket;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    char *object_name = "oss_test_multipart_upload";
    char *object_name1 = "oss_test_multipart_upload_from_file";
    char *object_name2 = "oss_test_upload_part_copy_dest_object";
    char *object_name3 = "oss_test_upload_part_copy_source_object";
    aos_table_t *resp_headers = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);

    //delete test object
    delete_test_object(options, TEST_BUCKET_NAME, object_name);
    delete_test_object(options, TEST_BUCKET_NAME, object_name1);
    delete_test_object(options, TEST_BUCKET_NAME, object_name2);
    delete_test_object(options, TEST_BUCKET_NAME, object_name3);

    //delete test bucket
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_delete_bucket(options, &bucket, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);
}

void test_init_abort_multipart_upload(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_abort_multipart_upload";
    oss_request_options_t *options = NULL;
    int is_oss_domain = 1;
    aos_string_t upload_id;
    aos_status_t *s = NULL;

    //test init multipart
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    s = init_test_multipart_upload(options, TEST_BUCKET_NAME, object_name, &upload_id);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertTrue(tc, upload_id.len > 0);
    CuAssertPtrNotNull(tc, upload_id.data);

    //abort multipart
    s = abort_test_multipart_upload(options, TEST_BUCKET_NAME, object_name, &upload_id);
    CuAssertIntEquals(tc, 204, s->code);

    aos_pool_destroy(p);

    printf("test_init_abort_multipart_upload ok\n");
}

void test_list_multipart_upload(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name1 = "oss_test_abort_multipart_upload1";
    char *object_name2 = "oss_test_abort_multipart_upload2";
    int is_oss_domain = 1;
    oss_request_options_t *options = NULL;
    aos_string_t upload_id1;
    aos_string_t upload_id2;
    aos_status_t *s = NULL;
    aos_table_t *resp_headers;
    oss_list_multipart_upload_params_t *params = NULL;
    char *expect_next_key_marker = "oss_test_abort_multipart_upload1";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    s = init_test_multipart_upload(options, TEST_BUCKET_NAME, object_name1, &upload_id1);
    CuAssertIntEquals(tc, 200, s->code);

    s = init_test_multipart_upload(options, TEST_BUCKET_NAME, object_name2, &upload_id2);
    CuAssertIntEquals(tc, 200, s->code);

    params = oss_create_list_multipart_upload_params(p);
    params->max_ret = 1;
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_list_multipart_upload(options, &bucket, params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 1, params->truncated);
    CuAssertStrEquals(tc, expect_next_key_marker, params->next_key_marker.data);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_list_init(&params->upload_list);
    aos_str_set(&params->key_marker, params->next_key_marker.data);
    aos_str_set(&params->upload_id_marker, params->next_upload_id_marker.data);

    s = oss_list_multipart_upload(options, &bucket, params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 0, params->truncated);

    s = abort_test_multipart_upload(options, TEST_BUCKET_NAME, object_name1, &upload_id1);
    CuAssertIntEquals(tc, 204, s->code);
    s = abort_test_multipart_upload(options, TEST_BUCKET_NAME, object_name2, &upload_id2);
    CuAssertIntEquals(tc, 204, s->code);
    aos_pool_destroy(p);

    printf("test_list_multipart_upload ok\n");    
}

void test_multipart_upload(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "oss_test_multipart_upload";
    aos_string_t object;
    int is_oss_domain = 1;
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
    char *content_type_for_complete = "video/MP2T";
    char *actual_content_type = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    headers = aos_table_make(options->pool, 2);

    //init mulitipart
    s = init_test_multipart_upload(options, TEST_BUCKET_NAME, object_name, &upload_id);
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

    aos_list_for_each_entry(part_content1, &params->part_list, node) {
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

    aos_list_for_each_entry(part_content2, &params->part_list, node) {
        complete_content2 = oss_create_complete_part_content(p);
        aos_str_set(&complete_content2->part_number, part_content2->part_number.data);
        aos_str_set(&complete_content2->etag, part_content2->etag.data);
        aos_list_add_tail(&complete_content2->node, &complete_part_list);
    }

    //complete multipart
    apr_table_add(headers, OSS_CONTENT_TYPE, content_type_for_complete);
    s = oss_complete_multipart_upload(options, &bucket, &object, &upload_id,
            &complete_part_list, headers, &complete_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, complete_resp_headers);
    
    //check content type
    apr_table_clear(headers);
    s = oss_head_object(options, &bucket, &object, headers, &head_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, head_resp_headers);
   
    actual_content_type = (char*)(apr_table_get(head_resp_headers, OSS_CONTENT_TYPE));
    CuAssertStrEquals(tc, content_type_for_complete, actual_content_type);

    aos_pool_destroy(p);

    printf("test_multipart_upload ok\n");
}

void test_multipart_upload_from_file(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "oss_test_multipart_upload_from_file";
    char *file_path = TEST_DIR"/data/test_upload_part_copy.file";
    FILE* fd = NULL;
    aos_string_t object;
    int is_oss_domain = 1;
    oss_request_options_t *options = NULL;
    aos_status_t *s = NULL;
    oss_upload_file_t *upload_file = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *upload_part_resp_headers = NULL;
    oss_list_upload_part_params_t *params = NULL;
    aos_table_t *list_part_resp_headers = NULL;
    aos_table_t *head_resp_headers = NULL;
    aos_string_t upload_id;
    aos_list_t complete_part_list;
    oss_list_part_content_t *part_content1 = NULL;
    oss_complete_part_content_t *complete_content1 = NULL;
    aos_table_t *complete_resp_headers = NULL;
    char *content_type_for_complete = "video/MP2T";
    char *actual_content_type = NULL;
    aos_string_t data;
    int part_num = 1;
    int part_num1 = 2;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    headers = aos_table_make(options->pool, 1);

    // create multipart upload local file    
    make_rand_string(p, 10 * 1024 * 1024, &data);
    fd = fopen(file_path, "w");
    fwrite(data.data, sizeof(data.data[0]), data.len, fd);
    fclose(fd);

    //init mulitipart
    s = init_test_multipart_upload(options, TEST_BUCKET_NAME, object_name, &upload_id);
    CuAssertIntEquals(tc, 200, s->code);

    //upload part from file
    upload_file = oss_create_upload_file(p);
    aos_str_set(&upload_file->filename, file_path);
    upload_file->file_pos = 0;
    upload_file->file_last = 200 * 1024; //200k
    
    s = oss_upload_part_from_file(options, &bucket, &object, &upload_id,
        part_num, upload_file, &upload_part_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, upload_part_resp_headers);
    
    upload_file->file_pos = 200 *1024;//remain content start pos
    upload_file->file_last = get_file_size(file_path);
    
    s = oss_upload_part_from_file(options, &bucket, &object, &upload_id,
        part_num1, upload_file, &upload_part_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, upload_part_resp_headers);
    
    //list part
    params = oss_create_list_upload_part_params(p);
    aos_str_set(&params->part_number_marker, "");
    params->max_ret = 10;
    params->truncated = 0;
    aos_list_init(&complete_part_list);
    
    s = oss_list_upload_part(options, &bucket, &object, &upload_id, params, &list_part_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 0, params->truncated);
    CuAssertPtrNotNull(tc, list_part_resp_headers);

    aos_list_for_each_entry(part_content1, &params->part_list, node) {
        complete_content1 = oss_create_complete_part_content(p);
        aos_str_set(&complete_content1->part_number, part_content1->part_number.data);
        aos_str_set(&complete_content1->etag, part_content1->etag.data);
        aos_list_add_tail(&complete_content1->node, &complete_part_list);
    }

    //complete multipart
    s = oss_complete_multipart_upload(options, &bucket, &object, &upload_id,
            &complete_part_list, NULL, &complete_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, complete_resp_headers);

    remove(file_path);
    aos_pool_destroy(p);

    printf("test_multipart_upload_from_file ok\n");
}

void test_upload_part_copy(CuTest *tc)
{
    aos_pool_t *p = NULL;
    oss_request_options_t *options = NULL;
    int is_oss_domain = 1;
    aos_string_t upload_id;
    oss_list_upload_part_params_t *list_upload_part_params = NULL;
    oss_upload_part_copy_params_t *upload_part_copy_params1 = NULL;
    oss_upload_part_copy_params_t *upload_part_copy_params2 = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *query_params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_table_t *list_part_resp_headers = NULL;
    aos_list_t complete_part_list;
    oss_list_part_content_t *part_content = NULL;
    oss_complete_part_content_t *complete_content = NULL;
    aos_table_t *complete_resp_headers = NULL;
    aos_status_t *s = NULL;
    int part1 = 1;
    int part2 = 2;
    char *local_filename = TEST_DIR"/data/test_upload_part_copy.file";
    char *download_filename = TEST_DIR"/data/test_upload_part_copy.file.download";
    char *source_object_name = "oss_test_upload_part_copy_source_object";
    char *dest_object_name = "oss_test_upload_part_copy_dest_object";
    FILE *fd = NULL;
    aos_string_t download_file;
    aos_string_t dest_bucket;
    aos_string_t dest_object;
    int64_t range_start1 = 0;
    int64_t range_end1 = 6000000;
    int64_t range_start2 = 6000001;
    int64_t range_end2;
    aos_string_t data;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);

    // create multipart upload local file    
    make_rand_string(p, 10 * 1024 * 1024, &data);
    fd = fopen(local_filename, "w");
    fwrite(data.data, sizeof(data.data[0]), data.len, fd);
    fclose(fd);    

    init_test_request_options(options, is_oss_domain);
    headers = aos_table_make(p, 0);
    s = create_test_object_from_file(options, TEST_BUCKET_NAME, source_object_name, 
        local_filename, headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, headers);

    //init mulitipart
    s = init_test_multipart_upload(options, TEST_BUCKET_NAME, dest_object_name, &upload_id);
    CuAssertIntEquals(tc, 200, s->code);

    //upload part copy 1
    upload_part_copy_params1 = oss_create_upload_part_copy_params(p);
    aos_str_set(&upload_part_copy_params1->source_bucket, TEST_BUCKET_NAME);
    aos_str_set(&upload_part_copy_params1->source_object, source_object_name);
    aos_str_set(&upload_part_copy_params1->dest_bucket, TEST_BUCKET_NAME);
    aos_str_set(&upload_part_copy_params1->dest_object, dest_object_name);
    aos_str_set(&upload_part_copy_params1->upload_id, upload_id.data);
    upload_part_copy_params1->part_num = part1;
    upload_part_copy_params1->range_start = range_start1;
    upload_part_copy_params1->range_end = range_end1;

    headers = aos_table_make(p, 0);
    s = oss_upload_part_copy(options, upload_part_copy_params1, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    //upload part copy 2
    resp_headers = NULL;
    range_end2 = get_file_size(local_filename) - 1;
    upload_part_copy_params2 = oss_create_upload_part_copy_params(p);
    aos_str_set(&upload_part_copy_params2->source_bucket, TEST_BUCKET_NAME);
    aos_str_set(&upload_part_copy_params2->source_object, source_object_name);
    aos_str_set(&upload_part_copy_params2->dest_bucket, TEST_BUCKET_NAME);
    aos_str_set(&upload_part_copy_params2->dest_object, dest_object_name);
    aos_str_set(&upload_part_copy_params2->upload_id, upload_id.data);
    upload_part_copy_params2->part_num = part2;
    upload_part_copy_params2->range_start = range_start2;
    upload_part_copy_params2->range_end = range_end2;

    headers = aos_table_make(p, 0);
    s = oss_upload_part_copy(options, upload_part_copy_params2, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    //list part
    list_upload_part_params = oss_create_list_upload_part_params(p);
    list_upload_part_params->max_ret = 10;
    aos_list_init(&complete_part_list);
        
    aos_str_set(&dest_bucket, TEST_BUCKET_NAME);
    aos_str_set(&dest_object, dest_object_name);
    s = oss_list_upload_part(options, &dest_bucket, &dest_object, &upload_id, 
                             list_upload_part_params, &list_part_resp_headers);

    aos_list_for_each_entry(part_content, &list_upload_part_params->part_list, node) {
        complete_content = oss_create_complete_part_content(p);
        aos_str_set(&complete_content->part_number, part_content->part_number.data);
        aos_str_set(&complete_content->etag, part_content->etag.data);
        aos_list_add_tail(&complete_content->node, &complete_part_list);
    }
     
    //complete multipart
    headers = aos_table_make(p, 0);
    s = oss_complete_multipart_upload(options, &dest_bucket, &dest_object, 
            &upload_id, &complete_part_list, headers, &complete_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, complete_resp_headers);

    //check upload copy part content equal to local file
    headers = aos_table_make(p, 0);
    aos_str_set(&download_file, download_filename);
    s = oss_get_object_to_file(options, &dest_bucket, &dest_object, headers, 
                               query_params, &download_file, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, get_file_size(local_filename), get_file_size(download_filename));    
    CuAssertPtrNotNull(tc, resp_headers);

    remove(download_filename);
    remove(local_filename);
    aos_pool_destroy(p);

    printf("test_upload_part_copy ok\n");
}

void test_upload_file(CuTest *tc) 
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "oss_test_multipart_upload_from_file";
    aos_string_t object; 
    int is_oss_domain = 1; 
    oss_request_options_t *options = NULL;
    aos_status_t *s = NULL;
    int part_size = 100*1024;
    aos_string_t upload_id;
    aos_string_t filepath;
    
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_null(&upload_id);
    aos_str_set(&filepath, __FILE__);
    s = oss_upload_file(options, &bucket, &object, &upload_id, &filepath, 
                        part_size, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

    printf("test_upload_file ok\n");
}

CuSuite *test_oss_multipart()
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_multipart_setup);
    SUITE_ADD_TEST(suite, test_init_abort_multipart_upload);
    SUITE_ADD_TEST(suite, test_list_multipart_upload);
    SUITE_ADD_TEST(suite, test_multipart_upload);
    SUITE_ADD_TEST(suite, test_multipart_upload_from_file);
    SUITE_ADD_TEST(suite, test_upload_file);
    SUITE_ADD_TEST(suite, test_upload_part_copy);
    SUITE_ADD_TEST(suite, test_multipart_cleanup);

    return suite;
}
