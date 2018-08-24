#include "CuTest.h"
#include "apr_portable.h"
#include "apr_file_info.h"
#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_xml.h"
#include "oss_util.c"
#include "aos_transport.c"
#include "oss_test_util.h"


static char local_file[1024];

void test_aos_setup(CuTest *tc)
{
    sprintf(local_file, "%sBingWallpaper-2017-01-19.jpg", get_test_file_path());
}

/*
 * oss_xml.c
 */
void test_get_xml_doc_with_empty_aos_list(CuTest *tc)
{
    int ret;
    mxml_node_t *xml_node;
    aos_list_t bc;
    aos_list_init(&bc);

    
    ret = get_xmldoc(&bc, &xml_node);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    printf("test_get_xml_doc_with_empty_aos_list ok\n");
}

/*
 * aos_list.h
 */

void test_aos_list_movelist_with_empty_list(CuTest *tc) {
    aos_list_t list;
    aos_list_t new_list;

    aos_list_init(&list);

    aos_list_movelist(&list, &new_list);
    CuAssertTrue(tc, new_list.prev == &new_list);
    CuAssertTrue(tc, new_list.next == &new_list);

    printf("test_aos_list_movelist_with_empty_list ok\n");
}

/*
 * oss_util.c
 */
void test_starts_with_failed(CuTest *tc) {
    int ret;
    aos_string_t str;
    aos_str_set(&str, "hangzhou.city");
    
    ret = starts_with(&str, "xixi");
    CuAssertIntEquals(tc, 0, ret);

    printf("test_starts_with_failed ok\n");
}

void test_is_valid_ip(CuTest *tc) {
    int ret;

    ret = is_valid_ip("140.205.63.8");
    CuAssertIntEquals(tc, 1, ret);

    printf("test_is_valid_ip ok\n");
}

void test_oss_request_options_create_with_null_pool(CuTest *tc) {
    oss_request_options_t *option;
    option = oss_request_options_create(NULL);
    CuAssertTrue(tc, NULL != option);

    aos_pool_destroy(option->pool);

    printf("test_oss_request_options_create_with_null_pool ok\n");
}

void test_oss_get_part_size(CuTest *tc) {
    int64_t file_size = 49999;
    int64_t part_size = 2;

    oss_get_part_size(file_size, &part_size);
    CuAssertIntEquals(tc, 5, (int)part_size);

    printf("test_oss_get_part_size ok\n");
}

void test_oss_get_object_uri_with_cname(CuTest *tc) {
    aos_pool_t *p;
    oss_request_options_t *options;
    aos_string_t bucket;
    aos_string_t object;
    aos_http_request_t req;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    options->config = oss_config_create(options->pool);
    options->config->is_cname = 1;
    aos_str_set(&options->config->endpoint, "img.abc.com");

    aos_str_set(&bucket, "bucket-1");
    aos_str_set(&object, "key-2");
    
    oss_get_object_uri(options, &bucket, &object, &req);
    CuAssertStrEquals(tc, "", req.proto);
    CuAssertStrEquals(tc, "key-2", req.uri);
    CuAssertStrEquals(tc, "img.abc.com", req.host);
    
    aos_pool_destroy(p);

    printf("test_oss_get_object_uri_with_cname ok\n");
}

void test_oss_get_object_uri_with_ip(CuTest *tc) {
    aos_pool_t *p;
    oss_request_options_t *options;
    aos_string_t bucket;
    aos_string_t object;
    aos_http_request_t req;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    options->config = oss_config_create(options->pool);
    options->config->is_cname = 0;
    aos_str_set(&options->config->endpoint, "http://140.205.63.8");

    aos_str_set(&bucket, "bucket-1");
    aos_str_set(&object, "key-2");
    
    oss_get_object_uri(options, &bucket, &object, &req);
    CuAssertStrEquals(tc, "http://", req.proto);
    CuAssertStrEquals(tc, "bucket-1/key-2", req.uri);
    CuAssertStrEquals(tc, "140.205.63.8", req.host);
    
    aos_pool_destroy(p);

    printf("test_oss_get_object_uri_with_ip ok\n");
}

void test_oss_get_bucket_uri_with_ip(CuTest *tc) {
    aos_pool_t *p;
    oss_request_options_t *options;
    aos_string_t bucket;
    aos_http_request_t req;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    options->config = oss_config_create(options->pool);
    options->config->is_cname = 0;
    aos_str_set(&options->config->endpoint, "140.205.63.8");

    aos_str_set(&bucket, "bucket-1");
    
    oss_get_bucket_uri(options, &bucket, &req);
    CuAssertStrEquals(tc, "", req.proto);
    CuAssertStrEquals(tc, "bucket-1", req.uri);
    CuAssertStrEquals(tc, "140.205.63.8", req.host);
    CuAssertStrEquals(tc, "bucket-1", req.resource);
    
    aos_pool_destroy(p);

    printf("test_oss_get_bucket_uri_with_ip ok\n");
}

void test_oss_get_bucket_uri_with_cname(CuTest *tc) {
    aos_pool_t *p;
    oss_request_options_t *options;
    aos_string_t bucket;
    aos_http_request_t req;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    options->config = oss_config_create(options->pool);
    options->config->is_cname = 1;
    aos_str_set(&options->config->endpoint, "https://img.abc.com");

    aos_str_set(&bucket, "bucket-1");
    
    oss_get_bucket_uri(options, &bucket, &req);
    CuAssertStrEquals(tc, "https://", req.proto);
    CuAssertStrEquals(tc, "bucket-1", req.uri);
    CuAssertStrEquals(tc, "img.abc.com", req.host);
    CuAssertStrEquals(tc, "bucket-1/", req.resource);
    
    aos_pool_destroy(p);

    printf("test_oss_get_bucket_uri_with_cname ok\n");
}

void test_aos_log_format_default(CuTest *tc) {
    /*
     * check is coredump
     */
    aos_log_format_default(AOS_LOG_INFO, "/tmp/a", 10, "fun1", "%d-%d", 1, 2);

    printf("test_aos_log_format_default ok\n");
}

void test_aos_log_print_default_with_null_file(CuTest *tc) {
    /*
     * check is coredump
     */
    aos_stderr_file = NULL;
    aos_log_print_default("abc", 3);

    printf("test_aos_log_print_default_with_null_file ok\n");
}

/*
 * aos_transport
 */
void test_aos_curl_code_to_status(CuTest *tc) {
    int code = aos_curl_code_to_status(CURLE_OUT_OF_MEMORY);
    CuAssertIntEquals(tc, AOSE_OUT_MEMORY, code);

    code = aos_curl_code_to_status(CURLE_COULDNT_RESOLVE_PROXY);
    CuAssertIntEquals(tc, AOSE_NAME_LOOKUP_ERROR, code);

    code = aos_curl_code_to_status(CURLE_COULDNT_RESOLVE_HOST);
    CuAssertIntEquals(tc, AOSE_NAME_LOOKUP_ERROR, code);

    code = aos_curl_code_to_status(CURLE_COULDNT_CONNECT);
    CuAssertIntEquals(tc, AOSE_FAILED_CONNECT, code);

    code = aos_curl_code_to_status(CURLE_WRITE_ERROR);
    CuAssertIntEquals(tc, AOSE_CONNECTION_FAILED, code);

    code = aos_curl_code_to_status(CURLE_OPERATION_TIMEDOUT);
    CuAssertIntEquals(tc, AOSE_CONNECTION_FAILED, code);

    code = aos_curl_code_to_status(CURLE_PARTIAL_FILE);
    CuAssertIntEquals(tc, AOSE_OK, code);

    code = aos_curl_code_to_status(CURLE_SSL_CACERT);
    CuAssertIntEquals(tc, AOSE_FAILED_VERIFICATION, code);

    code = aos_curl_code_to_status(CURLE_FTP_WEIRD_PASV_REPLY);
    CuAssertIntEquals(tc, AOSE_INTERNAL_ERROR, code);

    printf("test_aos_curl_code_to_status ok\n");
}

/*
 * aos_string.h
 */
void test_aos_unquote_str(CuTest *tc) {
    aos_string_t str;
    aos_str_set(&str, "\"abc\"");
    aos_unquote_str(&str);

    CuAssertStrnEquals(tc, "abc", strlen("abc"), str.data);
    CuAssertIntEquals(tc, 3, str.len);

    printf("test_aos_unquote_str ok\n");
}

void test_aos_ends_with(CuTest *tc) {
    int ret;
    aos_string_t str;
    aos_string_t suffix;

    aos_str_set(&str, "abc.mn.qp");

    aos_str_set(&suffix, ".qp");
    ret = aos_ends_with(&str, &suffix);
    CuAssertIntEquals(tc, 1, ret);

    aos_str_set(&suffix, ".mn");
    ret = aos_ends_with(&str, &suffix);
    CuAssertIntEquals(tc, 0, ret);

    ret = aos_ends_with(&str, NULL);
    CuAssertIntEquals(tc, 0, ret);

    printf("test_aos_ends_with ok\n");
}

/*
 * aos_util.h
 */

void test_aos_url_encode_failed(CuTest *tc) {
    int ret;
    char *dest;
    dest = (char*)malloc(1024);
    
    ret = aos_url_encode(dest, "/mingdi-hz-3/./xxx/./ddd/", 1);
    CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, ret);

    free(dest);

    printf("test_aos_url_encode_failed ok\n");
}

void test_aos_url_encode_with_blank_char(CuTest *tc) {
    int ret;
    char *source;
    char *dest;
    source = "abc.xx.com/a b";
    dest = (char*)malloc(20);
    
    ret = aos_url_encode(dest, source, strlen(source));
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertStrEquals(tc, "abc.xx.com%2Fa%20b", dest);

    free(dest);

    printf("test_aos_url_encode_with_blank_char ok\n");
}

void test_aos_url_decode_with_percent(CuTest *tc) {
    int ret;
    char *in;
    char *out;

    in = "abc.xx.com/a%20b";
    out = (char*)malloc(20);
    
    ret = aos_url_decode(in, out);
    CuAssertIntEquals(tc, 0, ret);

    free(out);
    
    printf("test_aos_url_decode_with_percent ok\n");
}

void test_aos_url_decode_with_add(CuTest *tc) {
    int ret;
    char *in;
    char *out;

    in = "abc.xx.com/a+b";
    out = (char*)malloc(20);
    
    ret = aos_url_decode(in, out);
    CuAssertIntEquals(tc, 0, ret);

    free(out);

    printf("test_aos_url_decode_with_add ok\n");
}

void test_aos_url_decode_failed(CuTest *tc) {
    int ret;
    char *in;
    char *out;

    in = "abc.xx.com/a%xb";
    out = (char*)malloc(20);
    
    ret = aos_url_decode(in, out);
    CuAssertIntEquals(tc, -1, ret);

    free(out);
    
    printf("test_aos_url_decode_failed ok\n");
}

void test_aos_should_retry(CuTest *tc) {
    aos_status_t s;
    aos_status_set(&s, 500, "", "");
    CuAssertIntEquals(tc, 1, aos_should_retry(&s));

    aos_status_set(&s, 505, "", "");
    CuAssertIntEquals(tc, 1, aos_should_retry(&s));

    aos_status_set(&s, 400, "", "");
    CuAssertIntEquals(tc, 0, aos_should_retry(&s));

    aos_status_set(&s, -995, "-995", "");
    CuAssertIntEquals(tc, 1, aos_should_retry(&s));

    aos_status_set(&s, -993, "-993", "");
    CuAssertIntEquals(tc, 0, aos_should_retry(&s));

    aos_status_set(&s, 0, "0", "NULL");
    CuAssertIntEquals(tc, 0, aos_should_retry(&s));

    CuAssertIntEquals(tc, 0, aos_should_retry(NULL));

    aos_status_set(&s, 200, "", "");
    CuAssertIntEquals(tc, 0, aos_should_retry(&s));

    aos_status_set(&s, 200, NULL, NULL);
    CuAssertIntEquals(tc, 0, aos_should_retry(&s));

    printf("test_aos_should_retry ok\n");
}

void test_aos_strtoll(CuTest *tc)
{
    int64_t val = 0;
    char *endptr = NULL;

    val = aos_strtoll("0", NULL, 10);
    CuAssertTrue(tc, val == 0);

    val = aos_strtoll("9223372036854775807", NULL, 10);
    CuAssertTrue(tc, val == 9223372036854775807);

    val = aos_strtoll("-9223372036854775808", NULL, 10);
    CuAssertTrue(tc, val == INT64_MIN);

    val = aos_strtoll("2147483648ABC", &endptr, 10);
    CuAssertTrue(tc, val == 2147483648);
    CuAssertStrEquals(tc, endptr, "ABC");

    val = aos_atoi64("0");
    CuAssertTrue(tc, val == 0);

    val = aos_atoi64("9223372036854775807");
    CuAssertTrue(tc, val == 9223372036854775807);

    val = aos_atoi64("-9223372036854775808");
    CuAssertTrue(tc, val == INT64_MIN);
}

void test_aos_strtoull(CuTest *tc)
{
    uint64_t val = 0;
    char *endptr = NULL;

    val = aos_strtoull("0", NULL, 10);
    CuAssertTrue(tc, val == 0);

    val = aos_strtoull("9223372036854775807", NULL, 10);
    CuAssertTrue(tc, val == 9223372036854775807);

    val = aos_strtoull("18446744073709551615", NULL, 10);
    CuAssertTrue(tc, val == UINT64_MAX);

    val = aos_strtoll("2147483648ABC", &endptr, 10);
    CuAssertTrue(tc, val == 2147483648);
    CuAssertStrEquals(tc, endptr, "ABC");

    val = aos_atoui64("0");
    CuAssertTrue(tc, val == 0);

    val = aos_atoui64("9223372036854775807");
    CuAssertTrue(tc, val == 9223372036854775807);

    val = aos_atoui64("18446744073709551615");
    CuAssertTrue(tc, val == UINT64_MAX);
}

void test_oss_get_file_info(CuTest *tc) {
    aos_pool_t *p;
    apr_finfo_t finfo;
    aos_string_t filepath;
    apr_status_t s;

    aos_pool_create(&p, NULL);
    aos_str_set(&filepath, local_file);

    s = oss_get_file_info(&filepath, p, &finfo); 
    CuAssertIntEquals(tc, AOSE_OK, s);

    CuAssertTrue(tc, finfo.size == 769686);
    CuAssertTrue(tc, finfo.mtime > 1484755200000000L);

    // negative
    aos_str_set(&filepath, "");

    s = oss_get_file_info(&filepath, p, &finfo); 
    CuAssertTrue(tc, AOSE_OK != s);

    aos_pool_destroy(p);
}

void test_aos_open_file_for_read(CuTest *tc) {
    aos_pool_t *p;
    aos_file_buf_t *fb;
    apr_status_t s;

    aos_pool_create(&p, NULL);

    fb = aos_create_file_buf(p);
    s = aos_open_file_for_read(p, local_file, fb);
    CuAssertIntEquals(tc, AOSE_OK, s);
    CuAssertTrue(tc, fb->file_pos == 0);
    CuAssertTrue(tc, fb->file_last == 769686);

    apr_file_close(fb->file);

    aos_pool_destroy(p);
}

void test_aos_urlsafe_base64_encode(CuTest *tc) {
    char buff[5];
    char urlsafe_base64[8];
    
    buff[0] = 0xFF; buff[1] = 0xE2;  buff[2] = 'a';  buff[3] = '0';  buff[4] = '2';
    aos_urlsafe_base64_encode(buff, 5,urlsafe_base64);

    CuAssertStrEquals(tc,"_-JhMDI",urlsafe_base64);
    
    printf("test_aos_urlsafe_base64_encode ok\n");
}

CuSuite *test_aos()
{
    CuSuite* suite = CuSuiteNew();
    
    SUITE_ADD_TEST(suite, test_aos_setup);
    SUITE_ADD_TEST(suite, test_get_xml_doc_with_empty_aos_list);
    SUITE_ADD_TEST(suite, test_aos_list_movelist_with_empty_list);
    SUITE_ADD_TEST(suite, test_starts_with_failed);
    SUITE_ADD_TEST(suite, test_is_valid_ip);
    SUITE_ADD_TEST(suite, test_oss_request_options_create_with_null_pool);
    SUITE_ADD_TEST(suite, test_oss_get_part_size);
    SUITE_ADD_TEST(suite, test_oss_get_object_uri_with_cname);
    SUITE_ADD_TEST(suite, test_oss_get_object_uri_with_ip);
    SUITE_ADD_TEST(suite, test_oss_get_bucket_uri_with_cname);
    SUITE_ADD_TEST(suite, test_oss_get_bucket_uri_with_ip);
    SUITE_ADD_TEST(suite, test_aos_log_format_default);
    SUITE_ADD_TEST(suite, test_aos_log_print_default_with_null_file);
    SUITE_ADD_TEST(suite, test_aos_curl_code_to_status);
    SUITE_ADD_TEST(suite, test_aos_unquote_str);
    SUITE_ADD_TEST(suite, test_aos_ends_with);
    SUITE_ADD_TEST(suite, test_aos_url_encode_failed);
    SUITE_ADD_TEST(suite, test_aos_url_encode_with_blank_char);
    SUITE_ADD_TEST(suite, test_aos_url_decode_with_percent);
    SUITE_ADD_TEST(suite, test_aos_url_decode_with_add);
    SUITE_ADD_TEST(suite, test_aos_url_decode_failed);
    SUITE_ADD_TEST(suite, test_aos_should_retry);
    SUITE_ADD_TEST(suite, test_aos_strtoll);
    SUITE_ADD_TEST(suite, test_aos_strtoull);
    SUITE_ADD_TEST(suite, test_oss_get_file_info); 
    SUITE_ADD_TEST(suite, test_aos_open_file_for_read);
    SUITE_ADD_TEST(suite, test_aos_urlsafe_base64_encode);

    return suite;
}
