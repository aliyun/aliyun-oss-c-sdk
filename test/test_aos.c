#include "CuTest.h"
#include "test.h"
#include "apr_portable.h"
#include "apr_file_info.h"
#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.c"
#include "aos_transport.c"

/*
 * oss_xml.c
 */
void test_get_xml_doc_with_empty_aos_list(CuTest *tc)
{
    int ret;
    aos_list_t bc;
    aos_list_init(&bc);
    
    ret = get_xmldoc(&bc, NULL);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);
}

void test_get_xml_doc_with_parse_failed(CuTest *tc)
{
    int ret;
    mxml_node_t *xml_node;

    aos_list_t head;
    aos_list_init(&head);

    aos_list_t node;
    aos_list_init(&node);

    aos_list_add_tail(&node, &head);
    
    ret = get_xmldoc(&head, &xml_node);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);
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
}

void test_is_valid_ip(CuTest *tc) {
    int ret;

    ret = is_valid_ip("140.205.63.8");
    CuAssertIntEquals(tc, 1, ret);
}

void test_oss_request_options_create_with_null_pool(CuTest *tc) {
    oss_request_options_t *option;
    option = oss_request_options_create(NULL);
    CuAssertTrue(tc, NULL != option);

    aos_pool_destroy(option->pool);
}

void test_oss_get_part_size(CuTest *tc) {
    int64_t file_size = 49999;
    int64_t part_size = 2;

    oss_get_part_size(file_size, &part_size);
    CuAssertIntEquals(tc, 5, part_size);
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
}

void test_aos_log_format_default(CuTest *tc) {
    /*
     * check is coredump
     */
    aos_log_format_default(AOS_LOG_INFO, "/tmp/a", 10, "fun1", "%d-%d", 1, 2);
}

void test_aos_log_print_default_with_null_file(CuTest *tc) {
    /*
     * check is coredump
     */
    aos_stderr_file = NULL;
    aos_log_print_default("abc", 3);
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
}

CuSuite *test_aos()
{
    CuSuite* suite = CuSuiteNew();   

    SUITE_ADD_TEST(suite, test_get_xml_doc_with_empty_aos_list);
    SUITE_ADD_TEST(suite, test_get_xml_doc_with_parse_failed);
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
    
    return suite;
}
