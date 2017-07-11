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

#if defined(WIN32)
static char *test_local_file = "..\\oss_c_sdk_test\\BingWallpaper-2017-01-19.jpg";
#else
static char *test_local_file = "oss_c_sdk_test/BingWallpaper-2017-01-19.jpg";
#endif

void test_resumable_setup(CuTest *tc)
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

void test_resumable_cleanup(CuTest *tc)
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
    delete_test_object(options, TEST_BUCKET_NAME, "test_resumable_upload_without_checkpoint.jpg");
    delete_test_object(options, TEST_BUCKET_NAME, "test_resumable_upload_partsize.jpg");
    delete_test_object(options, TEST_BUCKET_NAME, "test_resumable_upload_threads.jpg");
    delete_test_object(options, TEST_BUCKET_NAME, "test_resumable_upload_with_checkpoint.jpg");
    delete_test_object(options, TEST_BUCKET_NAME, "test_resumable_upload_with_checkpoint_format_invalid.jpg");
    delete_test_object(options, TEST_BUCKET_NAME, "test_resumable_upload_with_file_size_unavailable.jpg");
    delete_test_object(options, TEST_BUCKET_NAME, "test_resumable_upload_with_uploadid_unavailable.jpg");
    delete_test_object(options, TEST_BUCKET_NAME, "test_resumable_upload_with_uploadid_available.jpg");
    delete_test_object(options, TEST_BUCKET_NAME, "test_resumable_upload_callback_without_checkpoint.jpg");
    delete_test_object(options, TEST_BUCKET_NAME, "test_resumable_upload_progress_without_checkpoint.jpg");
    delete_test_object(options, TEST_BUCKET_NAME, "test_resumable_upload_callback_with_checkpoint.jpg");
    delete_test_object(options, TEST_BUCKET_NAME, "test_resumable_upload_progress_with_checkpoint.jpg");

    /* delete test bucket */
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    oss_delete_bucket(options, &bucket, &resp_headers);
    apr_sleep(apr_time_from_sec(3));

    aos_pool_destroy(p);
}

// ---------------------------- UT ----------------------------

void test_resumable_oss_get_thread_num(CuTest *tc)
{
    aos_pool_t *p = NULL;
    oss_resumable_clt_params_t *clt_params;
    int32_t thread_num = 0;

    aos_pool_create(&p, NULL);
    
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 1024, AOS_FALSE, NULL);
    thread_num = oss_get_thread_num(clt_params);
    CuAssertIntEquals(tc, 1024, thread_num);

    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 0, AOS_FALSE, NULL);
    thread_num = oss_get_thread_num(clt_params);
    CuAssertIntEquals(tc, 1, thread_num);

    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, -1, AOS_FALSE, NULL);
    thread_num = oss_get_thread_num(clt_params);
    CuAssertIntEquals(tc, 1, thread_num);

    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 1025, AOS_FALSE, NULL);
    thread_num = oss_get_thread_num(clt_params);
    CuAssertIntEquals(tc, 1, thread_num);

    aos_pool_destroy(p);

    printf("test_resumable_oss_get_thread_num ok\n");
}

void test_resumable_oss_get_checkpoint_path(CuTest *tc)
{
    aos_pool_t *p = NULL;
    oss_resumable_clt_params_t *clt_params;
    aos_string_t file_path = aos_null_string;
    aos_string_t checkpoint_path = aos_null_string;

    aos_pool_create(&p, NULL);

    aos_str_set(&file_path, "D:\\work\\oss\\BingWallpaper-2017-01-19.jpg");
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 1024, AOS_FALSE, NULL);
    oss_get_checkpoint_path(clt_params, &file_path, p, &checkpoint_path);
    CuAssertTrue(tc, checkpoint_path.data == NULL);
    CuAssertTrue(tc, checkpoint_path.len == 0);

    aos_str_set(&checkpoint_path, "BingWallpaper-2017-01-19.jpg.ucp");
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 1024, AOS_TRUE, checkpoint_path.data);
    oss_get_checkpoint_path(clt_params, &file_path, p, &checkpoint_path);
    CuAssertStrEquals(tc, "BingWallpaper-2017-01-19.jpg.ucp", checkpoint_path.data);

    // win path
    aos_str_set(&file_path, "D:\\work\\oss\\BingWallpaper-2017-01-19.jpg");
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 1024, AOS_TRUE, NULL);
    oss_get_checkpoint_path(clt_params, &file_path, p, &checkpoint_path);
    CuAssertStrEquals(tc, "D:\\work\\oss\\BingWallpaper-2017-01-19.jpg.cp", checkpoint_path.data);

    aos_str_set(&checkpoint_path, "");
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 1024, AOS_TRUE, NULL);
    oss_get_checkpoint_path(clt_params, &file_path, p, &checkpoint_path);
    CuAssertStrEquals(tc, "D:\\work\\oss\\BingWallpaper-2017-01-19.jpg.cp", checkpoint_path.data);

    // linux path
    aos_str_set(&file_path, "/home/tim/work/oss/BingWallpaper-2017-01-19.jpg");
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 1024, AOS_TRUE, NULL);
    oss_get_checkpoint_path(clt_params, &file_path, p, &checkpoint_path);
    CuAssertStrEquals(tc, "/home/tim/work/oss/BingWallpaper-2017-01-19.jpg.cp", checkpoint_path.data);

    aos_str_set(&checkpoint_path, "");
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 1024, AOS_TRUE, NULL);
    oss_get_checkpoint_path(clt_params, &file_path, p, &checkpoint_path);
    CuAssertStrEquals(tc, "/home/tim/work/oss/BingWallpaper-2017-01-19.jpg.cp", checkpoint_path.data);

    aos_pool_destroy(p);

    printf("test_resumable_oss_get_checkpoint_path ok\n");
}

void test_resumable_oss_get_file_info(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t file_path = aos_null_string;
    char *local_file = "test_resumable_oss_get_file_info.txt";
    apr_finfo_t finfo;
    int rv;

    aos_pool_create(&p, NULL);

    // invalid path
    aos_str_set(&file_path, "");
    rv = oss_get_file_info(&file_path, p, &finfo);
    CuAssertTrue(tc, APR_STATUS_IS_ENOENT(rv));

    // file not exist
    aos_str_set(&file_path, "/uvwxyz/abchij/test.udp");
    rv = oss_get_file_info(&file_path, p, &finfo);
    CuAssertTrue(tc, APR_STATUS_IS_ENOENT(rv));

    // empty file
    rv = fill_test_file(p, local_file, "");
    CuAssertIntEquals(tc, APR_SUCCESS, rv);
    aos_str_set(&file_path, local_file);
    rv = oss_get_file_info(&file_path, p, &finfo);
    CuAssertIntEquals(tc, AOSE_OK, rv);
    CuAssertTrue(tc, 0 == finfo.size);

    // normal
    rv = make_random_file(p, local_file, 1024);
    CuAssertIntEquals(tc, APR_SUCCESS, rv);
    rv = oss_get_file_info(&file_path, p, &finfo);
    CuAssertIntEquals(tc, AOSE_OK, rv);
    CuAssertTrue(tc, 1024 == finfo.size);

    apr_file_remove(local_file, p);

    aos_pool_destroy(p);

    printf("test_resumable_oss_get_file_info ok\n");
}

void test_resumable_oss_does_file_exist(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t file_path = aos_null_string;
    char *local_file = "test_resumable_oss_does_file_exist.txt";
    int rv;

    aos_pool_create(&p, NULL);

    // invalid path
    aos_str_set(&file_path, "");
    rv = oss_does_file_exist(&file_path, p);
    CuAssertTrue(tc, !rv);

    // file not exist
    aos_str_set(&file_path, "/uvwxyz/abchij/test.udp");
    rv = oss_does_file_exist(&file_path, p);
    CuAssertTrue(tc, !rv);

    // normal
    rv = make_random_file(p, local_file, 1024);
    CuAssertIntEquals(tc, APR_SUCCESS, rv);
    aos_str_set(&file_path, local_file);
    rv = oss_does_file_exist(&file_path, p);
    CuAssertTrue(tc, rv);

    apr_file_remove(local_file, p);

    aos_pool_destroy(p);

    printf("test_resumable_oss_does_file_exist ok\n");
}

void test_resumable_oss_dump_checkpoint(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t file_path = aos_null_string;
    char *cp_file = "test_resumable_oss_dump_checkpoint.ucp";
    oss_checkpoint_t *cp;
    apr_finfo_t finfo;
    aos_string_t upload_id;
    int64_t part_size;
    int rv;

    aos_pool_create(&p, NULL);

    // build checkpoint
    finfo.size = 510598;
    finfo.mtime = 1459922563;  
    aos_str_set(&file_path, "D:\\work\\oss\\BingWallpaper-2017-01-19.jpg");
    aos_str_set(&upload_id, "0004B9894A22E5B1888A1E29F8236E2D");
    part_size = 1024 * 100;

    cp = oss_create_checkpoint_content(p);
    oss_build_upload_checkpoint(p, cp, &file_path, &finfo, &upload_id, part_size);

    aos_str_set(&file_path, cp_file);
    rv = oss_open_checkpoint_file(p, &file_path, cp); 
    CuAssertIntEquals(tc, APR_SUCCESS, rv);

    rv = oss_dump_checkpoint(p, cp);
    CuAssertIntEquals(tc, AOSE_OK, rv);
    apr_file_close(cp->thefile);

    // write failed
    rv = apr_file_open(&cp->thefile, file_path.data, APR_READ, APR_UREAD | APR_GREAD, p);
    CuAssertIntEquals(tc, APR_SUCCESS, rv);

    rv = oss_dump_checkpoint(p, cp);
    CuAssertIntEquals(tc, AOSE_FILE_TRUNC_ERROR, rv);
    apr_file_close(cp->thefile);

    apr_file_remove(cp_file, p);

    aos_pool_destroy(p);

    printf("test_resumable_oss_dump_checkpoint ok\n");
}

void test_resumable_oss_load_checkpoint(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t file_path = aos_null_string;
    char *cp_file = "test_resumable_oss_load_checkpoint.ucp";
    oss_checkpoint_t *cp;
    oss_checkpoint_t *cp_l;
    apr_finfo_t finfo;
    aos_string_t upload_id;
    int64_t part_size;
    int rv;

    aos_pool_create(&p, NULL);

    // build checkpoint
    finfo.size = 510598;
    finfo.mtime = 1459922563;  
    aos_str_set(&file_path, "D:\\work\\oss\\BingWallpaper-2017-01-19.jpg");
    aos_str_set(&upload_id, "0004B9894A22E5B1888A1E29F8236E2D");
    part_size = 1024 * 100;

    cp = oss_create_checkpoint_content(p);
    oss_build_upload_checkpoint(p, cp, &file_path, &finfo, &upload_id, part_size);

    aos_str_set(&file_path, cp_file);
    rv = oss_open_checkpoint_file(p, &file_path, cp); 
    CuAssertIntEquals(tc, APR_SUCCESS, rv);

    // dump
    rv = oss_dump_checkpoint(p, cp);
    CuAssertIntEquals(tc, AOSE_OK, rv);
    apr_file_close(cp->thefile);

    // load
    cp_l = oss_create_checkpoint_content(p);
    rv = oss_load_checkpoint(p, &file_path, cp_l);
    CuAssertIntEquals(tc, AOSE_OK, rv);

    CuAssertStrEquals(tc, cp->md5.data, cp_l->md5.data);
    CuAssertIntEquals(tc, cp->cp_type, cp_l->cp_type);
    CuAssertStrEquals(tc, cp->upload_id.data, cp_l->upload_id.data);
    CuAssertIntEquals(tc, cp->part_num, cp_l->part_num);
    CuAssertTrue(tc, cp->part_size == cp_l->part_size);

    // load failed
    aos_str_set(&file_path, "/uvwxyz/abchij/test.udp");
    rv = oss_load_checkpoint(p, &file_path, cp_l);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, rv);

    // content invalid
    rv = make_random_file(p, cp_file, 1024);
    CuAssertIntEquals(tc, APR_SUCCESS, rv);
    aos_str_set(&file_path, cp_file);
    rv = oss_load_checkpoint(p, &file_path, cp_l);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, rv);

    apr_file_remove(cp_file, p);

    aos_pool_destroy(p);

    printf("test_resumable_oss_load_checkpoint ok\n");
}

void test_resumable_oss_is_upload_checkpoint_valid(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t file_path = aos_null_string;
    oss_checkpoint_t *cp;
    apr_finfo_t finfo;
    aos_string_t upload_id;
    int64_t part_size;
    int rv;

    aos_pool_create(&p, NULL);

    // build checkpoint
    finfo.size = 510598;
    finfo.mtime = 1459922563;  
    aos_str_set(&file_path, "D:\\work\\oss\\BingWallpaper-2017-01-19.jpg");
    aos_str_set(&upload_id, "0004B9894A22E5B1888A1E29F8236E2D");
    part_size = 1024 * 100;

    cp = oss_create_checkpoint_content(p);
    oss_build_upload_checkpoint(p, cp, &file_path, &finfo, &upload_id, part_size);

    rv = oss_is_upload_checkpoint_valid(p, cp, &finfo);
    CuAssertTrue(tc, rv);

    finfo.size = 510599;
    rv = oss_is_upload_checkpoint_valid(p, cp, &finfo);
    CuAssertTrue(tc, !rv);

    finfo.mtime = 1459922562; 
    rv = oss_is_upload_checkpoint_valid(p, cp, &finfo);
    CuAssertTrue(tc, !rv);

    aos_pool_destroy(p);

    printf("test_resumable_oss_is_upload_checkpoint_valid ok\n");
}

void test_resumable_checkpoint_xml(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *xml_doc = NULL;
    oss_checkpoint_t *cp;
    int64_t part_size = 0;
    int i = 0;
    oss_checkpoint_t *cp_actual;
    const char *xml_doc_expected = 
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<Checkpoint><MD5></MD5><Type>1</Type>"
        "<LocalFile>"
        "<Path>D:\\work\\oss\\BingWallpaper-2017-01-19.jpg</Path><Size>510598</Size>"
        "<LastModified>1459922563</LastModified><MD5>fba9dede5f27731c9771645a39863328</MD5>"
        "</LocalFile>"
        "<Object>"
        "<Key>~/oss/BingWallpaper-2017-01-19.jpg</Key><Size>510598</Size>"
        "<LastModified>Fri, 24 Feb 2012 06:07:48 GMT</LastModified><ETag>0F7230CAA4BE94CCBDC99C5500000000</ETag>"
        "</Object>"
        "<UploadId>0004B9894A22E5B1888A1E29F8236E2D</UploadId>"
        "<CPParts>"
        "<Number>5</Number><Size>102400</Size>"
        "<Parts>"
        "<Part><Index>0</Index><Offset>0</Offset><Size>102400</Size><Completed>1</Completed><ETag></ETag></Part>"
        "<Part><Index>1</Index><Offset>102400</Offset><Size>102400</Size><Completed>1</Completed><ETag></ETag></Part>"
        "<Part><Index>2</Index><Offset>204800</Offset><Size>102400</Size><Completed>1</Completed><ETag></ETag></Part>"
        "<Part><Index>3</Index><Offset>307200</Offset><Size>102400</Size><Completed>1</Completed><ETag></ETag></Part>"
        "<Part><Index>4</Index><Offset>409600</Offset><Size>100998</Size><Completed>1</Completed><ETag></ETag></Part>"
        "</Parts>"
        "</CPParts>"
        "</Checkpoint>\n";
    
    aos_pool_create(&p, NULL);

    cp = oss_create_checkpoint_content(p);
    cp->cp_type = OSS_CP_UPLOAD;

    aos_str_set(&cp->file_path, "D:\\work\\oss\\BingWallpaper-2017-01-19.jpg");
    cp->file_size = 510598;
    cp->file_last_modified = 1459922563;
    aos_str_set(&cp->file_md5,"fba9dede5f27731c9771645a39863328");

    aos_str_set(&cp->object_name, "~/oss/BingWallpaper-2017-01-19.jpg");
    cp->object_size = 510598;
    aos_str_set(&cp->object_last_modified, "Fri, 24 Feb 2012 06:07:48 GMT");
    aos_str_set(&cp->object_etag, "0F7230CAA4BE94CCBDC99C5500000000");

    aos_str_set(&cp->upload_id, "0004B9894A22E5B1888A1E29F8236E2D");

    part_size = 1024 * 100;
    oss_get_part_size(cp->file_size, &part_size);
    cp->part_size = part_size;
    for (i = 0; i * part_size < cp->file_size; i++) {
        cp->parts[i].index = i;
        cp->parts[i].offset = i * part_size;
        cp->parts[i].size = aos_min(part_size, (cp->file_size - i * part_size));
        cp->parts[i].completed = AOS_TRUE;
        aos_str_set(& cp->parts[i].etag, "");
    }
    cp->part_num = i;

    xml_doc = oss_build_checkpoint_xml(p ,cp);

    CuAssertStrEquals(tc, xml_doc_expected, xml_doc);

    cp_actual = oss_create_checkpoint_content(p);
    oss_checkpoint_parse_from_body(p, xml_doc, cp_actual);

    CuAssertIntEquals(tc, OSS_CP_UPLOAD, cp_actual->cp_type);
    CuAssertStrEquals(tc, "", cp_actual->md5.data);

    CuAssertStrEquals(tc, "D:\\work\\oss\\BingWallpaper-2017-01-19.jpg", cp_actual->file_path.data);
    CuAssertTrue(tc, 510598 == cp_actual->file_size);
    CuAssertTrue(tc, 1459922563 == cp_actual->file_last_modified);
    CuAssertStrEquals(tc, "fba9dede5f27731c9771645a39863328", cp_actual->file_md5.data);

    CuAssertStrEquals(tc, "~/oss/BingWallpaper-2017-01-19.jpg", cp_actual->object_name.data);
    CuAssertTrue(tc, 510598 == cp_actual->file_size);
    CuAssertStrEquals(tc, "Fri, 24 Feb 2012 06:07:48 GMT", cp_actual->object_last_modified.data);
    CuAssertStrEquals(tc, "0F7230CAA4BE94CCBDC99C5500000000", cp_actual->object_etag.data);

    CuAssertStrEquals(tc, "0004B9894A22E5B1888A1E29F8236E2D", cp_actual->upload_id.data);

    CuAssertIntEquals(tc, 5, cp_actual->part_num);
    CuAssertTrue(tc, 102400 == cp_actual->part_size);

    CuAssertIntEquals(tc, 4, cp_actual->parts[4].index);
    CuAssertTrue(tc, 409600 == cp_actual->parts[4].offset);
    CuAssertTrue(tc, 100998 == cp_actual->parts[4].size);
    CuAssertIntEquals(tc, 1, cp_actual->parts[4].completed);

    aos_pool_destroy(p);

    printf("test_resumable_checkpoint_xml ok\n");
}

// ---------------------------- FT ----------------------------

void test_resumable_upload_without_checkpoint(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_without_checkpoint.jpg";
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
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, test_local_file);

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
    CuAssertTrue(tc, content_length == get_file_size(test_local_file));

    aos_pool_destroy(p);

    printf("test_resumable_upload_without_checkpoint ok\n");
}

void test_resumable_upload_partsize(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_partsize.jpg";
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
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, test_local_file);

    // upload object with part size 10MB
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 1024 * 10, 3, AOS_FALSE, NULL);
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
    CuAssertTrue(tc, content_length == get_file_size(test_local_file));

    aos_pool_destroy(p);

    // upload object with part size 200K
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);

    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 200, 3, AOS_FALSE, NULL);
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
    CuAssertTrue(tc, content_length == get_file_size(test_local_file));

    aos_pool_destroy(p);

    printf("test_resumable_upload_partsize ok\n");
}

void test_resumable_upload_threads(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_threads.jpg";
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
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, test_local_file);

    // upload object with thread 1
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 1, AOS_FALSE, NULL);
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
    CuAssertTrue(tc, content_length == get_file_size(test_local_file));

    aos_pool_destroy(p);

    // upload object with thread 5
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);

    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 200, 5, AOS_FALSE, NULL);
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
    CuAssertTrue(tc, content_length == get_file_size(test_local_file));

    aos_pool_destroy(p);

    // upload object with thread 10
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);

    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 10, AOS_FALSE, NULL);
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
    CuAssertTrue(tc, content_length == get_file_size(test_local_file));

    aos_pool_destroy(p);

    printf("test_resumable_upload_threads ok\n");
}

void test_resumable_upload_with_checkpoint(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_with_checkpoint.jpg";
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
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, test_local_file);

    // upload object
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_TRUE, NULL);
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
    CuAssertTrue(tc, content_length == get_file_size(test_local_file));

    aos_pool_destroy(p);

    printf("test_resumable_upload_with_checkpoint ok\n");
}

void test_resumable_upload_with_checkpoint_format_invalid(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_with_checkpoint_format_invalid.jpg";
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
    aos_string_t checkpoint_path;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, test_local_file);

    // generate checkpoint
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_TRUE, NULL);
    oss_get_checkpoint_path(clt_params, &filename, p, &checkpoint_path);
    fill_test_file(p, checkpoint_path.data, "HiOSS");

    // upload object
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
    CuAssertTrue(tc, content_length == get_file_size(test_local_file));

    aos_pool_destroy(p);

    printf("test_resumable_upload_with_checkpoint_format_invalid ok\n");
}

void test_resumable_upload_with_checkpoint_path_invalid(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_with_checkpoint.jpg";
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
    char *cp_path = "/uvwxyz/abchij/test.udp";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, test_local_file);

    // upload
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_TRUE, cp_path);
    s = oss_resumable_upload_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers, &resp_body);
    CuAssertStrEquals(tc, "OpenFileFail", s->error_code);

    aos_pool_destroy(p);

    printf("test_resumable_upload_with_checkpoint_path_invalid ok\n");
}

void test_resumable_upload_with_file_size_unavailable(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_with_file_size_unavailable.jpg";
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
    aos_string_t checkpoint_path;
    char *xml_doc = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<Checkpoint>"
        "<MD5></MD5><Type>1</Type>"
        "<LocalFile>"
        "<Path>/home/baiyb/work/tmp/aliyun-oss-c-sdk-doing/BingWallpaper-2017-01-19.jpg</Path>"
        "<Size>0</Size><LastModified>1484790044000000</LastModified><MD5></MD5>"
        "</LocalFile>"
        "<Object>"
        "<Key></Key><Size>0</Size><LastModified></LastModified><ETag></ETag>"
        "</Object>"
        "<UploadId>750FBF7EB9104D4F8DDB74F0432A821F</UploadId>"
        "<CPParts>"
        "<Number>8</Number><Size>102400</Size>"
        "<Parts>"
        "<Part><Index>0</Index><Offset>0</Offset><Size>102400</Size><Completed>1</Completed><ETag>&quot;06336E9660D3D9610C79835D27F4D2EF&quot;</ETag></Part>"
        "<Part><Index>1</Index><Offset>102400</Offset><Size>102400</Size><Completed>1</Completed><ETag>&quot;D1C009C43EAA5E64B6B794E47BA37917&quot;</ETag></Part>"
        "<Part><Index>2</Index><Offset>204800</Offset><Size>102400</Size><Completed>1</Completed><ETag>&quot;073D2D906CEB0FADA1F2BA8A0BA54C1D&quot;</ETag></Part>"
        "<Part><Index>3</Index><Offset>307200</Offset><Size>102400</Size><Completed>1</Completed><ETag>&quot;7BA3B455E7B30D734F2CA29548E8BC56&quot;</ETag></Part>"
        "<Part><Index>4</Index><Offset>409600</Offset><Size>102400</Size><Completed>1</Completed><ETag>&quot;296F06C36E3746CD2A28824D3B4F0648&quot;</ETag></Part>"
        "<Part><Index>5</Index><Offset>512000</Offset><Size>102400</Size><Completed>1</Completed><ETag>&quot;06A0A19EC60DD4F51344D900BE543C53&quot;</ETag></Part>"
        "<Part><Index>6</Index><Offset>614400</Offset><Size>102400</Size><Completed>1</Completed><ETag>&quot;B7CE941E6AC00B6B3423572A87EA0B67&quot;</ETag></Part>"
        "<Part><Index>7</Index><Offset>716800</Offset><Size>52886</Size><Completed>1</Completed><ETag>&quot;AE5EEAEBB54232A6F71743AA45A32DA9&quot;</ETag></Part>"
        "</Parts>"
        "</CPParts>"
        "</Checkpoint>";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, test_local_file);

    // generate checkpoint
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_TRUE, NULL);
    oss_get_checkpoint_path(clt_params, &filename, p, &checkpoint_path);
    fill_test_file(p, checkpoint_path.data, xml_doc);

    // upload object
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
    CuAssertTrue(tc, content_length == get_file_size(test_local_file));

    aos_pool_destroy(p);

    printf("test_resumable_upload_with_file_size_unavailable ok\n");
}

void test_resumable_upload_with_uploadid_unavailable(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_with_uploadid_unavailable.jpg";
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t resp_body;
    apr_finfo_t finfo;
    oss_request_options_t *options = NULL;
    oss_resumable_clt_params_t *clt_params;
    char *cp_path = "d:\\work\\oss\\test.ucp";
    char *xml_doc = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<Checkpoint><MD5></MD5><Type>1</Type><LocalFile>"
        "<Path>oss_c_sdk_test/BingWallpaper-2017-01-19.jpg</Path>"
        "<Size>769686</Size><LastModified>%"
        APR_INT64_T_FMT
        "</LastModified><MD5></MD5></LocalFile>"
        "<Object><Key></Key><Size>0</Size><LastModified></LastModified><ETag></ETag></Object>"
        "<UploadId>F5F901B64DF34BEDA60C9B2B0984B8D4</UploadId>"
        "<CPParts><Number>1</Number><Size>1048576</Size>"
        "<Parts><Part><Index>0</Index><Offset>0</Offset><Size>769686</Size><Completed>0</Completed><ETag></ETag></Part>"
        "</Parts></CPParts></Checkpoint>";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, test_local_file);

    // generate checkpoint
    oss_get_file_info(&filename, p, &finfo);
    xml_doc = apr_psprintf(p, xml_doc, finfo.mtime);
    fill_test_file(p, cp_path, xml_doc);

    // upload object
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 1024, 1, AOS_TRUE, cp_path);
    s = oss_resumable_upload_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers, &resp_body);
    CuAssertIntEquals(tc, 404, s->code);
    CuAssertStrEquals(tc, "NoSuchUpload", s->error_code);

    apr_file_remove(cp_path, p);
    aos_pool_destroy(p);

    printf("test_resumable_upload_with_uploadid_unavailable ok\n");
}

void test_resumable_upload_with_uploadid_available(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_pool_t *pool = NULL;
    char *object_name = "test_resumable_upload_with_uploadid_available.jpg";
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
    aos_string_t checkpoint_path;
    aos_string_t upload_id;
    char *xml_doc = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<Checkpoint>"
        "<MD5></MD5><Type>1</Type>"
        "<LocalFile>"
        "<Path>/home/baiyb/work/tmp/aliyun-oss-c-sdk-doing/BingWallpaper-2017-01-19.jpg</Path>"
        "<Size>769686</Size><LastModified>1484790044000000</LastModified><MD5></MD5>"
        "</LocalFile>"
        "<Object>"
        "<Key></Key><Size>0</Size><LastModified></LastModified><ETag></ETag>"
        "</Object>"
        "<UploadId>%.*s</UploadId>"
        "<CPParts>"
        "<Number>8</Number><Size>102400</Size>"
        "<Parts>"
        "<Part><Index>0</Index><Offset>0</Offset><Size>102400</Size><Completed>0</Completed><ETag></ETag></Part>"
        "<Part><Index>1</Index><Offset>102400</Offset><Size>102400</Size><Completed>0</Completed><ETag></ETag></Part>"
        "<Part><Index>2</Index><Offset>204800</Offset><Size>102400</Size><Completed>0</Completed><ETag></ETag></Part>"
        "<Part><Index>3</Index><Offset>307200</Offset><Size>102400</Size><Completed>0</Completed><ETag></ETag></Part>"
        "<Part><Index>4</Index><Offset>409600</Offset><Size>102400</Size><Completed>0</Completed><ETag></ETag></Part>"
        "<Part><Index>5</Index><Offset>512000</Offset><Size>102400</Size><Completed>0</Completed><ETag></ETag></Part>"
        "<Part><Index>6</Index><Offset>614400</Offset><Size>102400</Size><Completed>0</Completed><ETag></ETag></Part>"
        "<Part><Index>7</Index><Offset>716800</Offset><Size>52886</Size><Completed>0</Completed><ETag></ETag></Part>"
        "</Parts>"
        "</CPParts>"
        "</Checkpoint>";

    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, test_local_file);

    // generate upload id
    aos_pool_create(&p, NULL);
    aos_pool_create(&pool, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = oss_init_multipart_upload(options, &bucket, &object, &upload_id, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    xml_doc = apr_psprintf(pool, xml_doc, upload_id.len, upload_id.data);
    aos_pool_destroy(p);

    // generate checkpoint
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);

    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_TRUE, NULL);
    oss_get_checkpoint_path(clt_params, &filename, p, &checkpoint_path);
    fill_test_file(p, checkpoint_path.data, xml_doc);

    // upload object
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
    CuAssertTrue(tc, content_length == get_file_size(test_local_file));

    aos_pool_destroy(p);
    aos_pool_destroy(pool);

    printf("test_resumable_upload_with_uploadid_available ok\n");
}

void test_resumable_upload_with_file_path_invalid(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_with_file_path_invalid.jpg";
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

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, "/uvwxyz/abchij/test.jpg");

    // upload
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 1024, 1, AOS_TRUE, NULL);
    s = oss_resumable_upload_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers, &resp_body);
    CuAssertStrEquals(tc, "OpenFileFail", s->error_code);

    aos_pool_destroy(p);

    printf("test_resumable_upload_with_file_path_invalid ok\n");
}

void test_resumable_upload_callback_without_checkpoint(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_callback_without_checkpoint.jpg";
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
    aos_buf_t *content;
    char b64_buf[1024];
    int b64_len = 64;
    char *buf = NULL;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;
    char *callback =  "{"
        "\"callbackUrl\":\"http://callback.oss-demo.com:23450\","
        "\"callbackHost\":\"oss-cn-hangzhou.aliyuncs.com\","
        "\"callbackBody\":\"bucket=${bucket}&object=${object}&size=${size}&mimeType=${mimeType}\","
        "\"callbackBodyType\":\"application/x-www-form-urlencoded\""
        "}";
    char *callback_var =  "{"
        "\"x:var1\":\"value1\","
        "\"x:var2\":\"value2\""
        "}";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, test_local_file);

    headers = aos_table_make(p, 2);
    b64_len = aos_base64_encode((unsigned char*)callback, strlen(callback), b64_buf);
    b64_buf[b64_len] = '\0';
    apr_table_set(headers, OSS_CALLBACK, apr_pstrdup(p, b64_buf));
    b64_len = aos_base64_encode((unsigned char*)callback_var, strlen(callback_var), b64_buf);
    b64_buf[b64_len] = '\0';
    apr_table_set(headers, OSS_CALLBACK_VAR, apr_pstrdup(p, b64_buf));

    // upload object
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_FALSE, NULL);
    s = oss_resumable_upload_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers, &resp_body);
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

    // head object
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = oss_head_object(options, &bucket, &object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    content_length = atol((char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH));
    CuAssertTrue(tc, content_length == get_file_size(test_local_file));

    aos_pool_destroy(p);

    printf("test_resumable_upload_callback_without_checkpoint ok\n");
}

void test_resumable_upload_progress_without_checkpoint(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_progress_without_checkpoint.jpg";
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
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, test_local_file);

    // upload object
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_FALSE, NULL);
    s = oss_resumable_upload_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, percentage, &resp_headers, &resp_body);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

    // head object
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = oss_head_object(options, &bucket, &object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    content_length = atol((char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH));
    CuAssertTrue(tc, content_length == get_file_size(test_local_file));

    aos_pool_destroy(p);

    printf("test_resumable_upload_progress_without_checkpoint ok\n");
}

void test_resumable_upload_callback_with_checkpoint(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_callback_with_checkpoint.jpg";
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
    aos_buf_t *content;
    char b64_buf[1024];
    int b64_len = 64;
    char *buf = NULL;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;
    char *callback =  "{"
        "\"callbackUrl\":\"http://callback.oss-demo.com:23450\","
        "\"callbackHost\":\"oss-cn-hangzhou.aliyuncs.com\","
        "\"callbackBody\":\"bucket=${bucket}&object=${object}&size=${size}&mimeType=${mimeType}\","
        "\"callbackBodyType\":\"application/x-www-form-urlencoded\""
        "}";
    char *callback_var =  "{"
        "\"x:var1\":\"value1\","
        "\"x:var2\":\"value2\""
        "}";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, test_local_file);

    headers = aos_table_make(p, 2);
    b64_len = aos_base64_encode((unsigned char*)callback, strlen(callback), b64_buf);
    b64_buf[b64_len] = '\0';
    apr_table_set(headers, OSS_CALLBACK, apr_pstrdup(p, b64_buf));
    b64_len = aos_base64_encode((unsigned char*)callback_var, strlen(callback_var), b64_buf);
    b64_buf[b64_len] = '\0';
    apr_table_set(headers, OSS_CALLBACK_VAR, apr_pstrdup(p, b64_buf));

    // upload object
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_TRUE, NULL);
    s = oss_resumable_upload_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers, &resp_body);
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

    // head object
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = oss_head_object(options, &bucket, &object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    content_length = atol((char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH));
    CuAssertTrue(tc, content_length == get_file_size(test_local_file));

    aos_pool_destroy(p);

    printf("test_resumable_upload_callback_with_checkpoint ok\n");
}

void test_resumable_upload_progress_with_checkpoint(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_progress_with_checkpoint.jpg";
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
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, test_local_file);

    // upload object
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_TRUE, NULL);
    s = oss_resumable_upload_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, percentage, &resp_headers, &resp_body);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

    // head object
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = oss_head_object(options, &bucket, &object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    content_length = atol((char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH));
    CuAssertTrue(tc, content_length == get_file_size(test_local_file));

    aos_pool_destroy(p);

    printf("test_resumable_upload_progress_with_checkpoint ok\n");
}

void test_resumable_upload_content_type(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_content_type.ts";
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
    char *content_type = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);
    aos_str_set(&filename, test_local_file);

    // upload object
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_TRUE, NULL);
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
    CuAssertTrue(tc, content_length == get_file_size(test_local_file));

    content_type = (char*)(apr_table_get(resp_headers, OSS_CONTENT_TYPE));
    CuAssertStrEquals(tc, "video/MP2T", content_type);

    aos_pool_destroy(p);

    printf("test_resumable_upload_content_type ok\n");
}

void test_resumable_download_without_checkpoint(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_without_checkpoint.jpg";
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    oss_resumable_clt_params_t *clt_params;
    unsigned long content_length;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&filename, object_name);

    // download
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_FALSE, NULL);
    s = oss_resumable_download_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    s = oss_head_object(options, &bucket, &object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    content_length = atol((char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH));
    CuAssertTrue(tc, content_length == get_file_size(filename.data));

    remove(filename.data);
    aos_pool_destroy(p);

    printf("test_resumable_download_without_checkpoint ok\n");
}

void test_resumable_download_with_checkpoint(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_with_checkpoint.jpg";
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    oss_resumable_clt_params_t *clt_params;
    unsigned long content_length;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&filename, object_name);

    // download
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_TRUE, NULL);
    s = oss_resumable_download_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    s = oss_head_object(options, &bucket, &object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    content_length = atol((char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH));
    CuAssertTrue(tc, content_length == get_file_size(filename.data));

    remove(filename.data);
    aos_pool_destroy(p);

    printf("test_resumable_download_with_checkpoint ok\n");
}

void test_resumable_download_partsize(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_partsize.jpg";
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    oss_resumable_clt_params_t *clt_params;
    unsigned long content_length;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&filename, object_name);

    // download
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 1024 * 10, 3, AOS_FALSE, NULL);
    s = oss_resumable_download_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    s = oss_head_object(options, &bucket, &object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    content_length = atol((char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH));
    CuAssertTrue(tc, content_length == get_file_size(filename.data));

    remove(filename.data);
    aos_pool_destroy(p);

    printf("test_resumable_download_partsize ok\n");
}

void test_resumable_download_threads(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_threads.jpg";
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    oss_resumable_clt_params_t *clt_params;
    unsigned long content_length;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&filename, object_name);

    // download with thread 1
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 1, AOS_FALSE, NULL);
    s = oss_resumable_download_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    s = oss_head_object(options, &bucket, &object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    content_length = atol((char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH));
    CuAssertTrue(tc, content_length == get_file_size(filename.data));

    remove(filename.data);
    aos_pool_destroy(p);

    printf("test_resumable_download_threads ok\n");
}

void test_resumable_download_with_checkpoint_format_invalid(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_with_checkpoint_format_invalid.jpg";
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    oss_resumable_clt_params_t *clt_params;
    unsigned long content_length;
    aos_string_t checkpoint_path;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&filename, object_name);

    // generate checkpoint
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_TRUE, NULL);
    oss_get_checkpoint_path(clt_params, &filename, p, &checkpoint_path);
    fill_test_file(p, checkpoint_path.data, "HiOSS");
    s = oss_resumable_download_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    s = oss_head_object(options, &bucket, &object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    content_length = atol((char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH));
    CuAssertTrue(tc, content_length == get_file_size(filename.data));

    remove(filename.data);
    aos_pool_destroy(p);

    printf("test_resumable_download_with_checkpoint_format_invalid ok\n");
}

void test_resumable_download_with_checkpoint_path_invalid(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_with_checkpoint_format_invalid.jpg";
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    oss_resumable_clt_params_t *clt_params;
    char *cp_path = "/uvwxyz/abchij/test.udp";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&filename, object_name);

    // generate checkpoint
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_TRUE, cp_path);
    s = oss_resumable_download_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers);
    CuAssertStrEquals(tc, "OpenFileFail", s->error_code);

    aos_pool_destroy(p);

    printf("test_resumable_download_with_checkpoint_path_invalid ok\n");
}

void test_resumable_download_without_checkpoint_object_not_found(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "/a/b/c/d/e/f/g/h/i/j/k/j/m/n.fake";
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    oss_resumable_clt_params_t *clt_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&filename, object_name);

    // download
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_FALSE, NULL);
    s = oss_resumable_download_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers);
    CuAssertIntEquals(tc, 404, s->code);

    aos_pool_destroy(p);

    printf("test_resumable_download_without_checkpoint_object_not_found ok\n");
}


void test_resumable_download_with_checkpoint_object_not_found(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "/a/b/c/d/e/f/g/h/i/j/k/j/m/n.fake";
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    oss_resumable_clt_params_t *clt_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&filename, object_name);

    // download
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_TRUE, NULL);
    s = oss_resumable_download_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, NULL, &resp_headers);
    CuAssertIntEquals(tc, 404, s->code);

    aos_pool_destroy(p);

    printf("test_resumable_download_with_checkpoint_object_not_found ok\n");
}



void test_resumable_download_progress_without_checkpoint(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_progress_without_checkpoint.jpg";
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    oss_resumable_clt_params_t *clt_params;
    unsigned long content_length;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&filename, object_name);

    // download
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_FALSE, NULL);
    s = oss_resumable_download_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, percentage, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    s = oss_head_object(options, &bucket, &object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    content_length = atol((char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH));
    CuAssertTrue(tc, content_length == get_file_size(filename.data));

    remove(filename.data);
    aos_pool_destroy(p);

    printf("test_resumable_download_progress_without_checkpoint ok\n");
}

void test_resumable_download_progress_with_checkpoint(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "test_resumable_upload_progress_with_checkpoint.jpg";
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    oss_resumable_clt_params_t *clt_params;
    unsigned long content_length;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&filename, object_name);

    // download
    clt_params = oss_create_resumable_clt_params_content(p, 1024 * 100, 3, AOS_TRUE, NULL);
    s = oss_resumable_download_file(options, &bucket, &object, &filename, headers, NULL, 
        clt_params, percentage, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    s = oss_head_object(options, &bucket, &object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    content_length = atol((char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH));
    CuAssertTrue(tc, content_length == get_file_size(filename.data));

    remove(filename.data);
    aos_pool_destroy(p);

    printf("test_resumable_download_progress_with_checkpoint ok\n");
}

CuSuite *test_oss_resumable()
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_resumable_setup);
    SUITE_ADD_TEST(suite, test_resumable_oss_get_thread_num);
    SUITE_ADD_TEST(suite, test_resumable_oss_get_checkpoint_path);
    SUITE_ADD_TEST(suite, test_resumable_oss_get_file_info);
    SUITE_ADD_TEST(suite, test_resumable_oss_does_file_exist);
    SUITE_ADD_TEST(suite, test_resumable_oss_dump_checkpoint);
    SUITE_ADD_TEST(suite, test_resumable_oss_load_checkpoint);
    SUITE_ADD_TEST(suite, test_resumable_oss_is_upload_checkpoint_valid);
    SUITE_ADD_TEST(suite, test_resumable_checkpoint_xml);
    SUITE_ADD_TEST(suite, test_resumable_upload_without_checkpoint);
    SUITE_ADD_TEST(suite, test_resumable_upload_with_checkpoint);
    SUITE_ADD_TEST(suite, test_resumable_upload_partsize);
    SUITE_ADD_TEST(suite, test_resumable_upload_threads);
    SUITE_ADD_TEST(suite, test_resumable_upload_with_checkpoint_format_invalid);
    SUITE_ADD_TEST(suite, test_resumable_upload_with_checkpoint_path_invalid);
    SUITE_ADD_TEST(suite, test_resumable_upload_with_file_size_unavailable);
    SUITE_ADD_TEST(suite, test_resumable_upload_with_uploadid_unavailable);
    SUITE_ADD_TEST(suite, test_resumable_upload_with_uploadid_available);
    SUITE_ADD_TEST(suite, test_resumable_upload_with_file_path_invalid);
    SUITE_ADD_TEST(suite, test_resumable_upload_callback_without_checkpoint);
    SUITE_ADD_TEST(suite, test_resumable_upload_progress_without_checkpoint);
    SUITE_ADD_TEST(suite, test_resumable_upload_callback_with_checkpoint);
    SUITE_ADD_TEST(suite, test_resumable_upload_progress_with_checkpoint);
    SUITE_ADD_TEST(suite, test_resumable_upload_content_type);
    SUITE_ADD_TEST(suite, test_resumable_download_without_checkpoint);
    SUITE_ADD_TEST(suite, test_resumable_download_with_checkpoint);
    SUITE_ADD_TEST(suite, test_resumable_download_partsize);
    SUITE_ADD_TEST(suite, test_resumable_download_threads);
    SUITE_ADD_TEST(suite, test_resumable_download_with_checkpoint_format_invalid);
    SUITE_ADD_TEST(suite, test_resumable_download_with_checkpoint_path_invalid);
    SUITE_ADD_TEST(suite, test_resumable_download_without_checkpoint_object_not_found);
    SUITE_ADD_TEST(suite, test_resumable_download_with_checkpoint_object_not_found);
    SUITE_ADD_TEST(suite, test_resumable_download_progress_without_checkpoint);
    SUITE_ADD_TEST(suite, test_resumable_download_progress_with_checkpoint);
    SUITE_ADD_TEST(suite, test_resumable_cleanup);

    return suite;
}
