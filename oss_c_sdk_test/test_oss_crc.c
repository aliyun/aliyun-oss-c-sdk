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

void test_crc_setup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;

    TEST_BUCKET_NAME = get_test_bucket_name(aos_global_pool, "test-c-sdk-crc");

    /* create test bucket */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);

    CuAssertIntEquals(tc, 200, s->code);
    aos_pool_destroy(p);
}

void test_crc_cleanup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    oss_request_options_t *options = NULL;
    char *object_name1 = "oss_test_crc_put_object.txt";
    char *object_name2 = "oss_test_crc_append_object.txt";
    char *object_name3 = "oss_test_crc_multipart_object.txt";

    aos_table_t *resp_headers = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    /* delete test object */
    delete_test_object(options, TEST_BUCKET_NAME, object_name1);
    delete_test_object(options, TEST_BUCKET_NAME, object_name2);
    delete_test_object(options, TEST_BUCKET_NAME, object_name3);

    /* delete test bucket */
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    oss_delete_bucket(options, &bucket, &resp_headers);
    apr_sleep(apr_time_from_sec(3));

    aos_pool_destroy(p);
}

void test_crc_append_object_from_buffer(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_crc_append_object.txt";
    aos_string_t bucket;
    aos_string_t object;
    char *str = "Time is a bird for ever on the wing.";
    aos_status_t *s = NULL;
    int is_cname = 0;
    int64_t position = 0;
    uint64_t initcrc = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t resp_body;
    oss_request_options_t *options = NULL;
    aos_list_t buffer;
    aos_buf_t *content = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);

    /* append object */
    aos_list_init(&buffer);
    content = aos_buf_pack(p, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    oss_delete_object(options, &bucket, &object, NULL);

    s = oss_do_append_object_from_buffer(options, &bucket, &object, position, 
        initcrc, &buffer, headers, NULL, NULL, &resp_headers, &resp_body);
    CuAssertIntEquals(tc, 200, s->code);

    position = aos_atoi64((char*)(apr_table_get(resp_headers, OSS_NEXT_APPEND_POSITION)));
    initcrc = aos_atoui64((char*)(apr_table_get(resp_headers, OSS_HASH_CRC64_ECMA)));

    /* append object */
    s = oss_do_append_object_from_buffer(options, &bucket, &object, position, 
        initcrc, &buffer, NULL, NULL, NULL, NULL, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    /* delete object */
    s= oss_delete_object(options, &bucket, &object, NULL);
    CuAssertIntEquals(tc, 204, s->code);

    aos_pool_destroy(p);

    printf("test_crc_append_object_from_buffer ok\n");
}

void test_crc_append_object_from_file(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_crc_append_object.txt";
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    int64_t position = 0;
    uint64_t initcrc = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t resp_body;
    oss_request_options_t *options = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    headers = aos_table_make(p, 0);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);

    make_random_file(p, object_name, 10240);
    aos_str_set(&filename, object_name);

    oss_delete_object(options, &bucket, &object, NULL);

    /* append object */
    s = oss_do_append_object_from_file(options, &bucket, &object, position, 
        initcrc, &filename, headers, NULL, NULL, &resp_headers, &resp_body);
    CuAssertIntEquals(tc, 200, s->code);

    position = aos_atoi64((char*)(apr_table_get(resp_headers, OSS_NEXT_APPEND_POSITION)));
    initcrc = aos_atoui64((char*)(apr_table_get(resp_headers, OSS_HASH_CRC64_ECMA)));

    /* append object */
    s = oss_do_append_object_from_file(options, &bucket, &object, position, 
        initcrc, &filename, NULL, NULL, NULL, NULL, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    /* delete object */
    s= oss_delete_object(options, &bucket, &object, NULL);
    CuAssertIntEquals(tc, 204, s->code);

    apr_file_remove(object_name, p);
    aos_pool_destroy(p);

    printf("test_crc_append_object_from_file ok\n");
}

void test_crc_disable_crc(CuTest *tc) 
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_crc_put_object.txt";
    char *str = "Sow nothing, reap nothing.";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    oss_request_options_t *options = NULL;
    aos_list_t resp_body;
    aos_list_t buffer;
    aos_buf_t *content;

    /* init test*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
   
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    options->ctl->options->enable_crc = AOS_FALSE;
    
    /* test put object */
    s = oss_put_object_from_buffer(options, &bucket, &object, &buffer, NULL, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

    /* test get object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    options->ctl->options->enable_crc = AOS_FALSE;

    s = oss_get_object_to_buffer(options, &bucket, &object, NULL, NULL, &buffer, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);

    printf("test_crc_disable_crc ok\n");
}

/* Test crc64() on vector[0..len-1] which should have CRC-64 crc.  Also test
   crc64_combine() on vector[] split in two. */
static void crc64_combine_test(CuTest *tc, void *vector, size_t len, uint64_t crc)
{
    uint64_t crc1, crc2;

    /* test crc64() */
    crc1 = aos_crc64(0, vector, len);
    CuAssertTrue(tc, crc1 == crc);

    /* test crc64_combine() */
    crc1 = aos_crc64(0, vector, (len + 1) >> 1);
    crc2 = aos_crc64(0, (char*)vector + ((len + 1) >> 1), len >> 1);
    crc1 = aos_crc64_combine(crc1, crc2, len >> 1);
    CuAssertTrue(tc, crc1 == crc);
}

void test_crc_combine(CuTest *tc)
{
    char *str1 = "123456789";
    size_t len1 = 9;
    uint64_t crc1 = UINT64_C(0x995dc9bbdf1939fa);
    char *str2 = "This is a test of the emergency broadcast system.";
    size_t len2 = 49;
    uint64_t crc2 = UINT64_C(0x27db187fc15bbc72);

    crc64_combine_test(tc, str1, len1, crc1);
    crc64_combine_test(tc, str2, len2, crc2);

    CuAssertTrue(tc, aos_crc64_combine(crc1, crc2, 0) == crc1);

    printf("test_crc_combine ok\n");
}

void test_crc_negative(CuTest *tc)
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_test_crc_append_object_neg.txt";
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    int is_cname = 0;
    int64_t position = 0;
    oss_request_options_t *options = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    make_random_file(p, object_name, 1024);
    aos_str_set(&filename, object_name);

    oss_delete_object(options, &bucket, &object, NULL);

    /* append object */
    s = oss_do_append_object_from_file(options, &bucket, &object, position, 1, 
        &filename, NULL, NULL, NULL, NULL, NULL);
    CuAssertIntEquals(tc, 200, s->code);

    /* delete object */
    s= oss_delete_object(options, &bucket, &object, NULL);
    CuAssertIntEquals(tc, 204, s->code);

    apr_file_remove(object_name, p);
    aos_pool_destroy(p);

    printf("test_crc_negative ok\n");
}

void test_crc_big_endian(CuTest *tc)
{
    char *str1 = "12345678";
    char *str2 = "87654321";
    char *str3 = "123456789";
    uint64_t crc1;
    uint64_t crc2;

    crc1 = aos_crc64_test(0, str1, 8, 0);
    crc2 = aos_crc64_test(0, str2, 8, 1);

    CuAssertTrue(tc, crc1 == crc2);

    crc1 = aos_crc64_test(0, str3, 9, 0);

    printf("test_crc_big_endian ok\n");
}

CuSuite *test_oss_crc()
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_crc_setup);
    SUITE_ADD_TEST(suite, test_crc_append_object_from_buffer);
    SUITE_ADD_TEST(suite, test_crc_append_object_from_file);
    SUITE_ADD_TEST(suite, test_crc_disable_crc);
    SUITE_ADD_TEST(suite, test_crc_combine);
    SUITE_ADD_TEST(suite, test_crc_negative);
    SUITE_ADD_TEST(suite, test_crc_big_endian);
    SUITE_ADD_TEST(suite, test_crc_cleanup);

    return suite;
}
