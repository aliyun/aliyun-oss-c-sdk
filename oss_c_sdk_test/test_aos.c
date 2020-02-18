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
#include "aos_buf.h"
#include "aos_http_io.h"
#include "aos_fstack.h"


static char test_file[1024];

void test_aos_setup(CuTest *tc)
{
    sprintf(test_file, "%sBingWallpaper-2017-01-19.jpg", get_test_file_path());
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

    ret = is_valid_ip("0.0.0.0");
    CuAssertIntEquals(tc, 0, ret);

    printf("test_is_valid_ip ok\n");
}

void test_oss_config_resolve(CuTest *tc) {
    aos_pool_t *p;
    oss_config_t config;
    aos_http_controller_t *ctl;
    aos_pool_create(&p, NULL);

    ctl = aos_http_controller_create(p, 0);
    ctl->options = aos_http_request_options_create(p);

    
    config.proxy_user.data = NULL;
    config.proxy_passwd.data = NULL;
    config.proxy_host.data = NULL;
    oss_config_resolve(p, &config, ctl);

    config.proxy_host.data = "192.168.1.1";
    config.proxy_host.len = strlen("192.168.1.1");
    config.proxy_port = 0;
    oss_config_resolve(p, &config, ctl);
    CuAssertStrEquals(tc, "192.168.1.1", ctl->options->proxy_host);

    config.proxy_user.data = NULL;
    config.proxy_passwd.data = NULL;
    oss_config_resolve(p, &config, ctl);
    CuAssertStrEquals(tc, NULL, ctl->options->proxy_auth);

    config.proxy_user.data = "test";
    config.proxy_user.len = 4;
    config.proxy_passwd.data = NULL;
    oss_config_resolve(p, &config, ctl);
    CuAssertStrEquals(tc, NULL, ctl->options->proxy_auth);

    config.proxy_user.data = "test";
    config.proxy_user.len = 4;
    config.proxy_passwd.data = "test";
    config.proxy_passwd.len = 4;
    oss_config_resolve(p, &config, ctl);
    CuAssertStrEquals(tc, "test:test", ctl->options->proxy_auth);

    aos_pool_destroy(p);

    printf("test_oss_config_resolve ok\n");
}

void test_oss_request_options_create_with_null_pool(CuTest *tc) {
    oss_request_options_t *option;
    option = oss_request_options_create(NULL);
    CuAssertTrue(tc, NULL != option);

    aos_pool_destroy(option->pool);

    printf("test_oss_request_options_create_with_null_pool ok\n");
}

void test_oss_get_service_uri(CuTest *tc) {
    aos_pool_t *p;
    oss_request_options_t *option;
    aos_http_request_t *req;

    aos_pool_create(&p, NULL);
    option = oss_request_options_create(p);
    req = aos_http_request_create(p);
    init_test_request_options(option, 0);
    option->config->is_cname = 0;
    option->config->endpoint.data = "192.168.1.1";
    option->config->endpoint.len = 11;
    oss_get_service_uri(option, req);
    CuAssertStrEquals(tc, "192.168.1.1", req->host);

    option->config->is_cname = 1;
    option->config->endpoint.data = "192.168.1.1";
    option->config->endpoint.len = 11;
    oss_get_service_uri(option, req);
    CuAssertStrEquals(tc, "192.168.1.1", req->host);

    aos_pool_destroy(p);

    printf("test_oss_get_service_uri ok\n");
}

void test_oss_get_rtmp_uri(CuTest *tc) {
    aos_pool_t *p;
    oss_request_options_t *option;
    aos_http_request_t *req;
    aos_string_t bucketname;
    aos_string_t channelid;

    aos_pool_create(&p, NULL);
    option = oss_request_options_create(p);
    req = aos_http_request_create(p);
    init_test_request_options(option, 0);
    option->config->endpoint.data = "http://192.168.1.1";
    option->config->endpoint.len = 18;

    bucketname.data = "test";
    bucketname.len = 4;

    channelid.data = "test";
    channelid.len = 4;

    oss_get_rtmp_uri(option, &bucketname, &channelid, req);
    CuAssertStrEquals(tc, "192.168.1.1", req->host);

    option->config->is_cname = 1;
    oss_get_rtmp_uri(option, &bucketname, &channelid, req);
    CuAssertStrEquals(tc, "192.168.1.1", req->host);

    aos_pool_destroy(p);

    printf("test_oss_get_rtmp_uri ok\n");
}

void test_oss_write_request_body_from_file(CuTest *tc) {
    aos_pool_t *p;
    aos_string_t filename;
    aos_http_request_t req;
    int ret;
    aos_pool_create(&p, NULL);

    filename.data = "";
    filename.len = 0;
    ret = oss_write_request_body_from_file(p, &filename, &req);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, ret);

    aos_pool_destroy(p);

    printf("test_oss_write_request_body_from_file ok\n");
}

void test_oss_write_request_body_from_upload_file(CuTest *tc) {
    aos_pool_t *p;
    oss_upload_file_t upload_file;
    aos_http_request_t req;
    int ret;
    aos_pool_create(&p, NULL);

    upload_file.filename.data = "";
    upload_file.filename.len = 0;
    upload_file.file_pos = 0;
    upload_file.file_last = 100;
    ret = oss_write_request_body_from_upload_file(p, &upload_file, &req);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, ret);

    aos_pool_destroy(p);

    printf("test_oss_write_request_body_from_upload_file ok\n");
}

void test_oss_init_read_response_body_to_file(CuTest *tc) {
    aos_pool_t *p;
    aos_string_t filename;
    aos_http_response_t resp;
    int ret;
    aos_pool_create(&p, NULL);

    filename.data = "";
    filename.len = 0;
    ret = oss_init_read_response_body_to_file(p, &filename, &resp);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, ret);

    aos_pool_destroy(p);

    printf("test_oss_init_read_response_body_to_file ok\n");
}

void test_oss_create_bucket_info(CuTest *tc) {
    aos_pool_t *p;
    oss_bucket_info_t *info;
    aos_pool_create(&p, NULL);
    info = oss_create_bucket_info(p);
    CuAssertTrue(tc, info != NULL);
    aos_pool_destroy(p);

    printf("test_oss_create_bucket_info ok\n");
}

void test_oss_get_part_size(CuTest *tc) {
    int64_t file_size = 49999;
    int64_t part_size = 2;

    oss_get_part_size(file_size, &part_size);
    CuAssertIntEquals(tc, 5, (int)part_size);

    printf("test_oss_get_part_size ok\n");
}

void test_part_sort_cmp(CuTest *tc) {
    oss_upload_part_t part1;
    oss_upload_part_t part2;
    int ret;

    part1.part_num = 2;
    part2.part_num = 1;
    ret = part_sort_cmp(&part1, &part2);
    CuAssertIntEquals(tc, 1, ret);
    printf("test_part_sort_cmp ok\n");
}

void test_set_content_type(CuTest *tc) {
    aos_pool_t *p;
    aos_table_t *headers;
    aos_pool_create(&p, NULL);
    headers = aos_table_make(p, 1);
    set_content_type(NULL, NULL, headers);
    aos_pool_destroy(p);
    printf("test_set_content_type ok\n");
}

void test_oss_check_crc_consistent(CuTest *tc) {
    aos_pool_t *p;
    aos_table_t *headers;
    aos_status_t s;
    int ret;
    aos_pool_create(&p, NULL);
    headers = aos_table_make(p, 1);
    ret = oss_check_crc_consistent(0, headers, &s);
    CuAssertIntEquals(tc, 0, ret);

    aos_pool_destroy(p);
    printf("test_oss_check_crc_consistent ok\n");
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

void test_oss_temp_file_rename(CuTest *tc) {
    aos_pool_t *p;
    int ret;
    aos_status_t s;
    ret = oss_temp_file_rename(NULL, NULL, NULL, NULL);
    CuAssertIntEquals(tc, -1, ret);

    aos_pool_create(&p, NULL);

    s.code = 400;
    ret = oss_temp_file_rename(&s, "test-path", NULL, p);
#if defined(WIN32)
    CuAssertIntEquals(tc, 720002, ret);
#else
    CuAssertIntEquals(tc, 2, ret);
#endif

    aos_pool_destroy(p);



    printf("test_oss_temp_file_rename ok\n");
}

void test_oss_init_select_object_read_response_body(CuTest *tc) {
    int ret;
    aos_pool_t *p;
    aos_http_response_t resp;
    aos_pool_create(&p, NULL);

    ret = oss_init_select_object_read_response_body(NULL, NULL);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    ret = oss_init_select_object_read_response_body(p, NULL);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    resp.type = BODY_IN_CALLBACK;
    ret = oss_init_select_object_read_response_body(p, &resp);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);
    printf("test_oss_init_select_object_read_response_body ok\n");
}

void test_oss_check_select_object_status(CuTest *tc) {
    aos_http_response_t resp;

    oss_check_select_object_status(NULL, NULL);
    oss_check_select_object_status(&resp, NULL);

    printf("test_oss_check_select_object_status ok\n");
}

void test_oss_check_create_select_object_meta_status(CuTest *tc) {
    aos_http_response_t resp;

    oss_check_create_select_object_meta_status(NULL, NULL, NULL);
    oss_check_create_select_object_meta_status(&resp, NULL, NULL);

    printf("oss_check_create_select_object_meta_status ok\n");
}

void test_oss_init_create_select_object_meta_read_response_body(CuTest *tc) {
    aos_http_response_t resp;
    aos_pool_t *p;
    aos_pool_create(&p, NULL);

    oss_init_create_select_object_meta_read_response_body(NULL, NULL);
    oss_init_create_select_object_meta_read_response_body(p, NULL);

    resp.type = BODY_IN_CALLBACK;
    oss_init_create_select_object_meta_read_response_body(p, &resp);
    aos_pool_destroy(p);

    printf("test_oss_init_create_select_object_meta_read_response_body ok\n");
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
    apr_file_t *thefile = NULL;
    aos_pool_t *p;

    aos_stderr_file = NULL;
    aos_log_print_default("abc", 3);

    aos_pool_create(&p, NULL);

    apr_file_open(&thefile, "stderr_log_file", APR_CREATE | APR_WRITE, APR_UREAD | APR_UWRITE | APR_GREAD, p);
    CuAssertTrue(tc, thefile != NULL);
    aos_stderr_file = thefile;
    aos_log_print_default("abc", 3);
    apr_file_close(thefile);
    aos_stderr_file = NULL;
    aos_pool_destroy(p);

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
    char buff[3];

    aos_str_set(&str, "\"abc\"");
    aos_unquote_str(&str);

    CuAssertStrnEquals(tc, "abc", strlen("abc"), str.data);
    CuAssertIntEquals(tc, 3, str.len);

    aos_str_set(&str, "");
    aos_unquote_str(&str);

    buff[0] = 'A';
    buff[1] = 'a';
    buff[2] = '\0';
    aos_str_set(&str, buff);
    aos_string_tolower(&str);
    CuAssertStrnEquals(tc, "aa", strlen("aa"), str.data);

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

    ret = aos_ends_with(NULL, NULL);
    CuAssertIntEquals(tc, 0, ret);

    aos_str_set(&suffix, "abc.mn.qp.adfda");
    ret = aos_ends_with(&str, &suffix);
    CuAssertIntEquals(tc, 0, ret);

    printf("test_aos_ends_with ok\n");
}

void test_aos_string_is_empty(CuTest *tc) {
    aos_string_t  str;
    int ret;

    //empty
    ret = aos_string_is_empty(NULL);
    CuAssertIntEquals(tc, 1, ret);

    str.len = 0;
    ret = aos_string_is_empty(&str);
    CuAssertIntEquals(tc, 1, ret);

    str.data = NULL;
    str.len = 1;
    ret = aos_string_is_empty(&str);
    CuAssertIntEquals(tc, 1, ret);

    str.data = "";
    str.len = 1;
    ret = aos_string_is_empty(&str);
    CuAssertIntEquals(tc, 1, ret);

    ret = aos_is_null_string(NULL);
    CuAssertIntEquals(tc, 1, ret);

    str.data = NULL;
    str.len = 1;
    ret = aos_is_null_string(&str);
    CuAssertIntEquals(tc, 1, ret);

    str.data = "";
    str.len = 0;
    ret = aos_is_null_string(&str);
    CuAssertIntEquals(tc, 1, ret);

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
    source = "abc.xx.com/a b~";
    dest = (char*)malloc(20);
    
    ret = aos_url_encode(dest, source, strlen(source));
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertStrEquals(tc, "abc.xx.com%2Fa%20b~", dest);

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
    
    ret = aos_url_decode(NULL, out);
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

/*
*aos_status.c
*/

void test_aos_should_retry(CuTest *tc) {
    aos_status_t s;
    aos_status_set(&s, 500, "", "");
    CuAssertIntEquals(tc, 1, aos_should_retry(&s));

    aos_status_set(&s, 505, "", "");
    CuAssertIntEquals(tc, 1, aos_should_retry(&s));

    aos_status_set(&s, 400, "", "");
    CuAssertIntEquals(tc, 0, aos_should_retry(&s));

    aos_status_set(&s, -992, "-992", "");
    CuAssertIntEquals(tc, 1, aos_should_retry(&s));

    aos_status_set(&s, -995, "-995", "");
    CuAssertIntEquals(tc, 1, aos_should_retry(&s));

    aos_status_set(&s, -998, "-998", "");
    CuAssertIntEquals(tc, 1, aos_should_retry(&s));

    aos_status_set(&s, -986, "-986", "");
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

void test_aos_status_parse_from_body_fail(CuTest *tc) {
    aos_pool_t *p;
    aos_status_t *s;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "invalid";
    char *without_error_xml = 
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<BucketLoggingStatus></BucketLoggingStatus>";
    char *without_code_xml = 
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Error><RequestId>aaaaa</RequestId></Error>";
    char *without_message_xml = 
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Error><Code>aaaaa</Code></Error>";

    aos_pool_create(&p, NULL);

    s = aos_status_parse_from_body(p, NULL, 200, NULL);
    CuAssertTrue(tc, s->code == 200);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    s = aos_status_parse_from_body(p, &buffer, 400, NULL);
    CuAssertStrEquals(tc, s->error_code, (char *)AOS_UNKNOWN_ERROR_CODE);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, without_error_xml, strlen(without_error_xml));
    aos_list_add_tail(&content->node, &buffer);
    s = aos_status_parse_from_body(p, &buffer, 400, NULL);
    CuAssertStrEquals(tc, s->error_code, (char *)AOS_UNKNOWN_ERROR_CODE);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, without_code_xml, strlen(without_code_xml));
    aos_list_add_tail(&content->node, &buffer);
    s = aos_status_parse_from_body(p, &buffer, 400, NULL);
    CuAssertTrue(tc, s->error_code == NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, without_message_xml, strlen(without_message_xml));
    aos_list_add_tail(&content->node, &buffer);
    s = aos_status_parse_from_body(p, &buffer, 400, NULL);
    CuAssertTrue(tc, s->error_msg == NULL);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_aos_strtoll(CuTest *tc)
{
    int64_t val = 0;
    char *endptr = NULL;

    val = aos_strtoll("0", NULL, 10);
    CuAssertTrue(tc, val == 0);

    val = aos_strtoll("9223372036854775807", NULL, 10);
    CuAssertTrue(tc, val == 9223372036854775807);

    val = aos_strtoll("+9223372036854775807", NULL, 10);
    CuAssertTrue(tc, val == 9223372036854775807);

    val = aos_strtoll("-9223372036854775808", NULL, 10);
    CuAssertTrue(tc, val == INT64_MIN);

    val = aos_strtoll("-92233720368547758080", NULL, 10);
    CuAssertTrue(tc, val == INT64_MIN);

    val = aos_strtoll("9223372036854775809", NULL, 10);
    CuAssertTrue(tc, val == INT64_MAX);

    val = aos_strtoll("2147483648ABC", &endptr, 10);
    CuAssertTrue(tc, val == 2147483648);
    CuAssertStrEquals(tc, endptr, "ABC");

    val = aos_strtoll(" 1234", NULL, 0);
    CuAssertTrue(tc, val == 1234);

    //hex
    val = aos_strtoll("0x1234Ac", NULL, 0);
    CuAssertTrue(tc, val == 1193132);

    val = aos_strtoll("0X1234", NULL, 0);
    CuAssertTrue(tc, val == 4660);

    val = aos_strtoll("0x1234", NULL, 16);
    CuAssertTrue(tc, val == 4660);

    val = aos_strtoll("0X1234", NULL, 16);
    CuAssertTrue(tc, val == 4660);

    //Oct
    val = aos_strtoll("01234", NULL, 0);
    CuAssertTrue(tc, val == 668);

    val = aos_atoi64("0");
    CuAssertTrue(tc, val == 0);

    val = aos_atoi64("9223372036854775807");
    CuAssertTrue(tc, val == 9223372036854775807);

    val = aos_atoi64("-9223372036854775808");
    CuAssertTrue(tc, val == INT64_MIN);

    printf("%s ok\n", __FUNCTION__);
}

void test_aos_strtoull(CuTest *tc)
{
    uint64_t val = 0;
    char *endptr = NULL;

    val = aos_strtoull("0", NULL, 10);
    CuAssertTrue(tc, val == 0);

    val = aos_strtoull("9223372036854775807", NULL, 10);
    CuAssertTrue(tc, val == 9223372036854775807);

    val = aos_strtoull("+922337203685477580", NULL, 10);
    CuAssertTrue(tc, val == 922337203685477580);

    val = aos_strtoull("18446744073709551615", NULL, 10);
    CuAssertTrue(tc, val == UINT64_MAX);

    val = aos_strtoull("2147483648ABC", &endptr, 10);
    CuAssertTrue(tc, val == 2147483648);
    CuAssertStrEquals(tc, endptr, "ABC");

    val = aos_strtoull(" 1234", NULL, 0);
    CuAssertTrue(tc, val == 1234);

    //hex
    val = aos_strtoull("0x1234Ac", NULL, 0);
    CuAssertTrue(tc, val == 1193132);

    val = aos_strtoull("0X1234", NULL, 0);
    CuAssertTrue(tc, val == 4660);

    val = aos_strtoull("0x1234", NULL, 16);
    CuAssertTrue(tc, val == 4660);

    val = aos_strtoull("0X1234", NULL, 16);
    CuAssertTrue(tc, val == 4660);

    //Oct
    val = aos_strtoull("01234", NULL, 0);
    CuAssertTrue(tc, val == 668);

    val = aos_atoui64("0");
    CuAssertTrue(tc, val == 0);

    val = aos_atoui64("9223372036854775807");
    CuAssertTrue(tc, val == 9223372036854775807);

    val = aos_atoui64("18446744073709551615");
    CuAssertTrue(tc, val == UINT64_MAX);

    printf("%s ok\n", __FUNCTION__);
}

void test_aos_base64_encode(CuTest *tc) {
    char buff[32];
    int ret;

    memset(buff, 0, 32);
    ret = aos_base64_encode((unsigned char *)"abc", 3, buff);
    CuAssertStrEquals(tc, "YWJj", buff);
    CuAssertTrue(tc, ret == 4);

    ret = aos_base64_encode((unsigned char *)"abcd", 4, buff);
    CuAssertStrEquals(tc, "YWJjZA==", buff);
    CuAssertTrue(tc, ret == 8);

    ret = aos_base64_encode((unsigned char *)"abcde", 5, buff);
    CuAssertStrEquals(tc, "YWJjZGU=", buff);
    CuAssertTrue(tc, ret == 8);

    memset(buff, 0, 32);
    ret = aos_base64_encode((unsigned char *)"", 0, buff);
    CuAssertStrEquals(tc, "", buff);
    CuAssertTrue(tc, ret == 0);

    memset(buff, 0, 32);
    ret = aos_base64_encode((unsigned char *)"A", 1, buff);
    CuAssertStrEquals(tc, "QQ==", buff);
    CuAssertTrue(tc, ret == 4);

    printf("%s ok\n", __FUNCTION__);
}

void test_oss_get_file_info(CuTest *tc) {
    aos_pool_t *p;
    apr_finfo_t finfo;
    aos_string_t filepath;
    apr_status_t s;

    aos_pool_create(&p, NULL);
    aos_str_set(&filepath, test_file);

    s = oss_get_file_info(&filepath, p, &finfo); 
    CuAssertIntEquals(tc, AOSE_OK, s);

    CuAssertTrue(tc, finfo.size == 769686);
    CuAssertTrue(tc, finfo.mtime > 1484755200000000L);

    // negative
    aos_str_set(&filepath, "");

    s = oss_get_file_info(&filepath, p, &finfo); 
    CuAssertTrue(tc, AOSE_OK != s);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

/*
 * aos_buf.c
 */

void test_aos_open_file_for_read(CuTest *tc) {
    aos_pool_t *p;
    aos_file_buf_t *fb;
    apr_status_t s;

    aos_pool_create(&p, NULL);

    fb = aos_create_file_buf(p);
    s = aos_open_file_for_read(p, test_file, fb);
    CuAssertIntEquals(tc, AOSE_OK, s);
    CuAssertTrue(tc, fb->file_pos == 0);
    CuAssertTrue(tc, fb->file_last == 769686);

    apr_file_close(fb->file);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_aos_create_buf_fail(CuTest *tc) {
    aos_pool_t *p = NULL;
    aos_buf_t *buff;
    aos_pool_create(&p, NULL);
    buff = aos_create_buf(p, -100);
    CuAssertTrue(tc, buff == NULL);
    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_aos_open_file_for_read_fail(CuTest *tc) {
    aos_pool_t *p;
    aos_file_buf_t *fb;
    apr_status_t s;

    aos_pool_create(&p, NULL);
    fb = aos_create_file_buf(p);
    s = aos_open_file_for_read(p, "invalid-path", fb);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, s);

#if !defined(WIN32)
    s = aos_open_file_for_read(p, NULL, fb);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, s);
#endif

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_aos_open_file_for_range_read_fail(CuTest *tc) {
    aos_pool_t *p;
    aos_file_buf_t *fb;
    apr_status_t s;

    aos_pool_create(&p, NULL);

    fb = aos_create_file_buf(p);
    s = aos_open_file_for_range_read(p, test_file, 769696, 769686, fb);
    CuAssertIntEquals(tc, AOSE_OK, s);
    CuAssertTrue(tc, fb->file_pos == 769686);
    CuAssertTrue(tc, fb->file_last == 769686);

    apr_file_close(fb->file);

    s = aos_open_file_for_range_read(p, "invalid-path", 769696, 769686, fb);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, s);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_aos_open_file_for_write_fail(CuTest *tc) {
    aos_pool_t *p;
    aos_file_buf_t *fb;
    apr_status_t s;

    aos_pool_create(&p, NULL);
    fb = aos_create_file_buf(p);
    s = aos_open_file_for_write(p, "g:/invalid-path", fb);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, s);

#if !defined(WIN32)
    s = aos_open_file_for_write(p, NULL, fb);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, s);
#endif

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_aos_open_file_for_write_notrunc_fail(CuTest *tc) {
    aos_pool_t *p;
    aos_file_buf_t *fb;
    apr_status_t s;

    aos_pool_create(&p, NULL);
    fb = aos_create_file_buf(p);
    s = aos_open_file_for_write_notrunc(p, "g:/invalid-path", fb);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, s);

    s = aos_open_file_for_write_notrunc(p, "", fb);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, s);

#if !defined(WIN32)
    s = aos_open_file_for_write_notrunc(p, NULL, fb);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, s);
#endif

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_aos_buf_append_string_fail(CuTest *tc) {

    aos_buf_append_string(NULL, NULL, NULL, -1);

    printf("%s ok\n", __FUNCTION__);
}

/*
*aos_http_io.c
*/

void test_aos_request_get_release(CuTest *tc) {

    CURL * handle[AOS_REQUEST_STACK_SIZE + 2];
    int i;

    for (i = 0; i < AOS_REQUEST_STACK_SIZE + 2; i++) {
        handle[i] = aos_request_get();
    }

    for (i = 0; i < AOS_REQUEST_STACK_SIZE + 2; i++) {
        request_release(handle[i]);
    }

    printf("%s ok\n", __FUNCTION__);
}

void test_aos_http_controller_create(CuTest *tc) {
    aos_pool_t *p = NULL;
    aos_http_controller_t * ctr = NULL;

    ctr = aos_http_controller_create(p, 0);
    CuAssertTrue(tc, p == NULL);
    CuAssertTrue(tc, ctr != NULL);
    CuAssertTrue(tc, ctr->pool != NULL);
    aos_pool_destroy(ctr->pool);

    printf("%s ok\n", __FUNCTION__);
}

void test_aos_read_http_body_file_fail(CuTest *tc) {
    aos_pool_t *p = NULL;
    aos_http_request_t *req;
    aos_file_buf_t *fb;
    int ret;
    apr_file_t *thefile;
    char buffer[16];

    aos_pool_create(&p, NULL);
    fb = aos_create_file_buf(p);
    req = aos_http_request_create(p);

    CuAssertTrue(tc, req != NULL);
    CuAssertTrue(tc, fb != NULL);

    ret = aos_read_http_body_file(req, NULL, 0);
    CuAssertTrue(tc, ret == AOSE_INVALID_ARGUMENT);

    req->file_buf = fb;
    ret = aos_read_http_body_file(req, NULL, 0);
    CuAssertTrue(tc, ret == AOSE_INVALID_ARGUMENT);

    req->file_buf = aos_create_file_buf(p);
    apr_file_open(&thefile, "test_file_to_read", APR_CREATE | APR_WRITE, APR_UREAD | APR_UWRITE | APR_GREAD, p);
    CuAssertTrue(tc, thefile != NULL);
    req->file_buf->file = thefile;
    apr_file_close(thefile);
    req->file_buf->file_pos = 0;
    req->file_buf->file_last = 2;
    ret = aos_read_http_body_file(req, buffer, 1);
    CuAssertTrue(tc, ret == AOSE_FILE_READ_ERROR);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_aos_write_http_body_file_fail(CuTest *tc) {
    aos_pool_t *p = NULL;
    aos_http_response_t *resp;
    int ret;
    apr_file_t *thefile;

    aos_pool_create(&p, NULL);
    resp = aos_http_response_create(p);

    CuAssertTrue(tc, resp != NULL);

    ret = aos_write_http_body_file(resp, NULL, 0);
    CuAssertTrue(tc, resp->file_buf != NULL);
    CuAssertTrue(tc, ret == AOSE_INVALID_ARGUMENT);

    resp->file_path = "g:/invalid-path";
    ret = aos_write_http_body_file(resp, NULL, 0);
    CuAssertTrue(tc, ret == AOSE_OPEN_FILE_ERROR);

    resp->file_buf = aos_create_file_buf(p);
    apr_file_open(&thefile, "test_file_to_write", APR_CREATE | APR_WRITE, APR_UREAD | APR_UWRITE | APR_GREAD, p);
    CuAssertTrue(tc, thefile != NULL);
    resp->file_buf->file = thefile;
    apr_file_close(thefile);
    ret = aos_write_http_body_file(resp, NULL, 0);
    CuAssertTrue(tc, ret == AOSE_FILE_WRITE_ERROR);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

/*
 * aos_util.c
 */
void test_aos_parse_xml_body_fail(CuTest *tc) {
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    mxml_node_t *root;
    char *invalid_xml = "invalid";
    int ret;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = aos_parse_xml_body(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_INTERNAL_ERROR, ret);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_aos_http_method_to_string(CuTest *tc) {
    const char *msg = NULL;
    msg = aos_http_method_to_string((http_method_e)10);
    CuAssertStrEquals(tc, "UNKNOWN", msg);

    printf("%s ok\n", __FUNCTION__);
}

static void fstack_func1_pt(void *a)
{
}
static void fstack_func2_pt()
{
}
static int fstack_func3_pt(void *a)
{
    return 0;
}
static int fstack_func4_pt()
{
    return 0;
}

void test_aos_fstack(CuTest *tc) {
    aos_pool_t *p;
    aos_array_header_t *header;
    aos_fstack_item_t *item;
    aos_func_u func_u;

    aos_pool_create(&p, NULL);

    header = aos_fstack_create(p, 5);
    CuAssertTrue(tc, header != NULL);

    func_u.func1 = fstack_func1_pt;
    aos_fstack_push(header, NULL, func_u, 1);
    func_u.func2 = fstack_func2_pt;
    aos_fstack_push(header, NULL, func_u, 2);
    func_u.func3 = fstack_func3_pt;
    aos_fstack_push(header, NULL, func_u, 3);
    func_u.func4 = fstack_func4_pt;
    aos_fstack_push(header, NULL, func_u, 4);
    aos_fstack_push(header, NULL, func_u, 5);

    item = aos_fstack_pop(header);
    CuAssertTrue(tc, item != NULL);
    CuAssertTrue(tc, item->order == 5);
    item = aos_fstack_pop(header);
    CuAssertTrue(tc, item != NULL);
    CuAssertTrue(tc, item->order == 4);
    item = aos_fstack_pop(header);
    CuAssertTrue(tc, item != NULL);
    CuAssertTrue(tc, item->order == 3);
    item = aos_fstack_pop(header);
    CuAssertTrue(tc, item != NULL);
    CuAssertTrue(tc, item->order == 2);
    item = aos_fstack_pop(header);
    CuAssertTrue(tc, item != NULL);
    CuAssertTrue(tc, item->order == 1);

    item = aos_fstack_pop(header);
    CuAssertTrue(tc, item == NULL);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

void test_oss_create_api_result_content(CuTest *tc) {
    aos_pool_t *p;
    void *ret_void;
    aos_pool_create(&p, NULL);

    ret_void = oss_create_api_result_content(p, -10);
    CuAssertTrue(tc, ret_void == NULL);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_get_string_to_sign_negative(CuTest *tc) {
    aos_pool_t *p;
    int ret;
    aos_string_t canon_res;
    aos_table_t *headers;
    aos_table_t *params;
    aos_string_t signstr;
    char special_query[AOS_MAX_QUERY_ARG_LEN+1];
    char special_header[AOS_MAX_HEADER_LEN + 1];

    aos_pool_create(&p, NULL);

    aos_str_set(&canon_res, "");
    headers = aos_table_make(p, 1);
    params = aos_table_make(p, 1);
    apr_table_set(headers, OSS_CONTENT_TYPE, "image/jpeg");
    ret = oss_get_string_to_sign(p, HTTP_GET, &canon_res, headers, params, &signstr);
    CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, ret);

    aos_str_set(&canon_res, "");
    headers = aos_table_make(p, 1);
    params = aos_table_make(p, 1);
    apr_table_set(headers, OSS_CANNONICALIZED_HEADER_DATE, "Fri, 06 Sep 2019 08:54:24 GMT");
    ret = oss_get_string_to_sign(p, HTTP_GET, &canon_res, headers, NULL, &signstr);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    //long params 
    aos_str_set(&canon_res, "");
    headers = aos_table_make(p, 1);
    params = aos_table_make(p, 1);
    apr_table_set(headers, OSS_CANNONICALIZED_HEADER_DATE, "Fri, 06 Sep 2019 08:54:24 GMT");
    memset(special_query, 0x30, AOS_MAX_QUERY_ARG_LEN);
    special_query[AOS_MAX_QUERY_ARG_LEN] = '\0';
    apr_table_set(params, "x-oss-process", special_query);
    ret = oss_get_string_to_sign(p, HTTP_GET, &canon_res, headers, params, &signstr);
    CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, ret);

    //user meta header too many
    aos_str_set(&canon_res, "");
    headers = aos_table_make(p, 1);
    params = aos_table_make(p, 1);
    apr_table_set(headers, OSS_CANNONICALIZED_HEADER_DATE, "Fri, 06 Sep 2019 08:54:24 GMT");
    memset(special_header, 0x30, AOS_MAX_HEADER_LEN);
    special_header[AOS_MAX_HEADER_LEN] = '\0';
    apr_table_set(headers, "x-oss-meta-user1", special_header);
    ret = oss_get_string_to_sign(p, HTTP_GET, &canon_res, headers, params, &signstr);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_sign_request_negative(CuTest *tc) {
    aos_pool_t *p;
    oss_config_t config;
    aos_http_request_t *req;
    int ret;
    char special_res[AOS_MAX_URI_LEN + 1];

    aos_pool_create(&p, NULL);

    req = aos_http_request_create(p);
    memset(special_res, 0x30, AOS_MAX_URI_LEN);
    special_res[AOS_MAX_URI_LEN] = '\0';
    req->resource = special_res;
    ret = oss_sign_request(req, &config);
    CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, ret);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

static void test_get_oss_request_signature_negative(CuTest *tc) {
    aos_pool_t *p;
    oss_request_options_t *option;
    aos_http_request_t *req;
    char special_query[AOS_MAX_QUERY_ARG_LEN + 1];
    int ret;
    aos_string_t expires;
    aos_string_t signature;

    aos_pool_create(&p, NULL);
    option = oss_request_options_create(p);
    req = aos_http_request_create(p);
    req->headers = aos_table_make(p, 1);
    req->query_params = aos_table_make(p, 1);
    init_test_request_options(option, 0);
    memset(special_query, 0x30, AOS_MAX_QUERY_ARG_LEN);
    special_query[AOS_MAX_QUERY_ARG_LEN] = '\0';
    apr_table_set(req->query_params, "x-oss-process", special_query);
    aos_str_set(&expires, "12343");
    ret = get_oss_request_signature(option, req, &expires, &signature);
    CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, ret);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_get_signed_url_negative(CuTest *tc) {
    aos_pool_t *p;
    oss_request_options_t *option;
    aos_http_request_t *req;
    char special_query[AOS_MAX_QUERY_ARG_LEN + 1];
    char speical_path[AOS_MAX_URI_LEN + 1];
    int ret;
    aos_string_t expires;
    aos_string_t signed_url;

    aos_pool_create(&p, NULL);

    memset(special_query, 0x30, AOS_MAX_QUERY_ARG_LEN);
    special_query[AOS_MAX_QUERY_ARG_LEN] = '\0';
    memset(speical_path, 0x30, AOS_MAX_URI_LEN);
    speical_path[AOS_MAX_URI_LEN] = '\0';
    aos_str_set(&expires, "12343");

    option = oss_request_options_create(p);
    req = aos_http_request_create(p);
    req->query_params = aos_table_make(p, 1);
    init_test_request_options(option, 0);
    apr_table_set(req->query_params, "x-oss-process", special_query);
    ret = oss_get_signed_url(option, req, &expires, &signed_url);
    CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, ret);

/*
    init_test_request_options(option, 0);
    req = aos_http_request_create(p);
    //req->resource = "TEST";
    ret = oss_get_signed_url(option, req, &expires, &signed_url);
    CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, ret);



    //aos_str_set(&option->config->sts_token, "test");
    aos_str_set(&option->config->endpoint, "");
    req->query_params = aos_table_make(p, 1);
    req->uri = speical_path;
    ret = oss_get_signed_url(option, req, &expires, &signed_url);
    CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, ret);
    aos_pool_destroy(p);
*/
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_get_rtmp_signed_url_negative(CuTest *tc) {
    aos_pool_t *p;
    oss_request_options_t *option;
    aos_http_request_t *req;
    char special_query[AOS_MAX_QUERY_ARG_LEN + 1];
    int ret;
    aos_string_t expires;
    aos_string_t play_list_name;
    aos_string_t signed_url;
    aos_table_t *params;

    aos_pool_create(&p, NULL);

    memset(special_query, 0x30, AOS_MAX_QUERY_ARG_LEN);
    special_query[AOS_MAX_QUERY_ARG_LEN] = '\0';
    aos_str_set(&expires, "12343");
    aos_str_set(&play_list_name, "test");

    option = oss_request_options_create(p);
    req = aos_http_request_create(p);
    init_test_request_options(option, 0);
    apr_table_set(req->query_params, "x-oss-process", special_query);

    params = aos_table_make(p, 1);
    apr_table_set(params, "x-oss-param1", "test1");
    apr_table_set(params, "x-oss-param2", "test2");

    req->uri = "test";
    ret = oss_get_rtmp_signed_url(option, req, &expires, &play_list_name, params, &signed_url);
    CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, ret);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_is_valid_bucket_name(CuTest *tc) {
    int i;
    aos_string_t name;
    char *invalid_name_list[] =
    { "a", "1", "!", "aa", "12", "a1",
        "a!", "1!", "aAa", "1A1", "a!a", "FengChao@123", "-a123", "a_123", "a123-",
        "1234567890123456789012345678901234567890123456789012345678901234", ""
    };

    for (i = 0; i < sizeof(invalid_name_list) / sizeof(invalid_name_list[0]); i++) {
        aos_str_set(&name, invalid_name_list[i]);
        CuAssertIntEquals(tc, 0, oss_is_valid_bucket_name(&name));
    }

    aos_str_null(&name);
    CuAssertIntEquals(tc, 0, oss_is_valid_bucket_name(&name));

    CuAssertIntEquals(tc, 0, oss_is_valid_bucket_name(NULL));

    aos_str_set(&name, "valid-bucket-name-1234");
    CuAssertIntEquals(tc, 1, oss_is_valid_bucket_name(&name));

    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_preprocess_endpoint(CuTest *tc) {
    int i;
    aos_string_t name;
    char *endpoints[] =
    {
        "www.test.com/abc",
        "www.test.com/abc?test=1",
        "www.test.com/abc?test=1#segment",
        "www.test.com?test=1#segment",
        "www.test.com#abc",
        "www.test.com"
    };

    char *ip_endpoints[] = 
    {
        "192.168.1.1:8080/abc",
        "192.168.1.1:8080/abc?test=1",
        "192.168.1.1:8080?test=1#segment",
        "192.168.1.1:8080"
    };

    char *ip_endpoints2[] =
    {
        "192.168.1.1/abc",
        "192.168.1.1/abc?test=1",
        "192.168.1.1?test=1#segment",
        "192.168.1.1"
    };

    for (i = 0; i < sizeof(endpoints) / sizeof(endpoints[0]); i++) {
        aos_str_set(&name, endpoints[i]);
        oss_preprocess_endpoint(&name);
        CuAssertIntEquals(tc, strlen("www.test.com"), name.len);
        CuAssertIntEquals(tc, 0, strncmp("www.test.com", name.data, name.len));
    }
    CuAssertIntEquals(tc, 6, i);

    for (i = 0; i < sizeof(ip_endpoints) / sizeof(ip_endpoints[0]); i++) {
        aos_str_set(&name, ip_endpoints[i]);
        oss_preprocess_endpoint(&name);
        CuAssertIntEquals(tc, strlen("192.168.1.1:8080"), name.len);
        CuAssertIntEquals(tc, 0, strncmp("192.168.1.1:8080", name.data, name.len));
    }
    CuAssertIntEquals(tc, 4, i);

    for (i = 0; i < sizeof(ip_endpoints2) / sizeof(ip_endpoints2[0]); i++) {
        aos_str_set(&name, ip_endpoints2[i]);
        oss_preprocess_endpoint(&name);
        CuAssertIntEquals(tc, strlen("192.168.1.1"), name.len);
        CuAssertIntEquals(tc, 0, strncmp("192.168.1.1", name.data, name.len));
    }
    CuAssertIntEquals(tc, 4, i);

    aos_str_set(&name, "");
    oss_preprocess_endpoint(&name);
    CuAssertIntEquals(tc, 0, name.len);


    printf("%s ok\n", __FUNCTION__);
}


void test_oss_fill_read_response_header(CuTest *tc) {
    aos_table_t *headers;
    oss_fill_read_response_header(NULL, &headers);
    printf("%s ok\n", __FUNCTION__);
}


CuSuite *test_aos()
{
    CuSuite* suite = CuSuiteNew();
    
    SUITE_ADD_TEST(suite, test_aos_setup);
    SUITE_ADD_TEST(suite, test_get_xml_doc_with_empty_aos_list);
    SUITE_ADD_TEST(suite, test_aos_list_movelist_with_empty_list);
    SUITE_ADD_TEST(suite, test_starts_with_failed);
    SUITE_ADD_TEST(suite, test_is_valid_ip);
    SUITE_ADD_TEST(suite, test_oss_config_resolve);
    SUITE_ADD_TEST(suite, test_oss_request_options_create_with_null_pool);
    SUITE_ADD_TEST(suite, test_oss_get_service_uri);
    SUITE_ADD_TEST(suite, test_oss_get_rtmp_uri);
    SUITE_ADD_TEST(suite, test_oss_write_request_body_from_file);
    SUITE_ADD_TEST(suite, test_oss_write_request_body_from_upload_file);
    SUITE_ADD_TEST(suite, test_oss_init_read_response_body_to_file);
    SUITE_ADD_TEST(suite, test_oss_create_bucket_info);
    SUITE_ADD_TEST(suite, test_oss_get_part_size);
    SUITE_ADD_TEST(suite, test_part_sort_cmp);
    SUITE_ADD_TEST(suite, test_set_content_type);
    SUITE_ADD_TEST(suite, test_oss_check_crc_consistent);
    SUITE_ADD_TEST(suite, test_oss_get_object_uri_with_cname);
    SUITE_ADD_TEST(suite, test_oss_get_object_uri_with_ip);
    SUITE_ADD_TEST(suite, test_oss_get_bucket_uri_with_cname);
    SUITE_ADD_TEST(suite, test_oss_get_bucket_uri_with_ip);
    SUITE_ADD_TEST(suite, test_oss_temp_file_rename);
    SUITE_ADD_TEST(suite, test_oss_init_select_object_read_response_body);
    SUITE_ADD_TEST(suite, test_oss_check_select_object_status);
    SUITE_ADD_TEST(suite, test_oss_check_create_select_object_meta_status);
    SUITE_ADD_TEST(suite, test_oss_init_create_select_object_meta_read_response_body);
    SUITE_ADD_TEST(suite, test_aos_log_format_default);
    SUITE_ADD_TEST(suite, test_aos_log_print_default_with_null_file);
    SUITE_ADD_TEST(suite, test_aos_curl_code_to_status);
    SUITE_ADD_TEST(suite, test_aos_unquote_str);
    SUITE_ADD_TEST(suite, test_aos_ends_with);
    SUITE_ADD_TEST(suite, test_aos_string_is_empty);
    SUITE_ADD_TEST(suite, test_aos_url_encode_failed);
    SUITE_ADD_TEST(suite, test_aos_url_encode_with_blank_char);
    SUITE_ADD_TEST(suite, test_aos_url_decode_with_percent);
    SUITE_ADD_TEST(suite, test_aos_url_decode_with_add);
    SUITE_ADD_TEST(suite, test_aos_url_decode_failed);
    SUITE_ADD_TEST(suite, test_aos_should_retry);
    SUITE_ADD_TEST(suite, test_aos_status_parse_from_body_fail);
    SUITE_ADD_TEST(suite, test_aos_strtoll);
    SUITE_ADD_TEST(suite, test_aos_strtoull);
    SUITE_ADD_TEST(suite, test_aos_base64_encode);
    SUITE_ADD_TEST(suite, test_oss_get_file_info); 
    SUITE_ADD_TEST(suite, test_aos_open_file_for_read);
    SUITE_ADD_TEST(suite, test_aos_create_buf_fail);
    SUITE_ADD_TEST(suite, test_aos_open_file_for_read_fail);
    SUITE_ADD_TEST(suite, test_aos_open_file_for_range_read_fail);
    SUITE_ADD_TEST(suite, test_aos_open_file_for_write_fail);
    SUITE_ADD_TEST(suite, test_aos_open_file_for_write_notrunc_fail);
    SUITE_ADD_TEST(suite, test_aos_buf_append_string_fail);
    SUITE_ADD_TEST(suite, test_aos_request_get_release);
    SUITE_ADD_TEST(suite, test_aos_http_controller_create);
    SUITE_ADD_TEST(suite, test_aos_read_http_body_file_fail);
    SUITE_ADD_TEST(suite, test_aos_write_http_body_file_fail);
    SUITE_ADD_TEST(suite, test_aos_parse_xml_body_fail);
    SUITE_ADD_TEST(suite, test_aos_http_method_to_string);
    SUITE_ADD_TEST(suite, test_aos_fstack);
    SUITE_ADD_TEST(suite, test_oss_create_api_result_content);
    SUITE_ADD_TEST(suite, test_oss_get_string_to_sign_negative);
    SUITE_ADD_TEST(suite, test_oss_sign_request_negative);
    SUITE_ADD_TEST(suite, test_get_oss_request_signature_negative);
    SUITE_ADD_TEST(suite, test_oss_get_signed_url_negative);
    SUITE_ADD_TEST(suite, test_oss_get_rtmp_signed_url_negative);
    SUITE_ADD_TEST(suite, test_oss_is_valid_bucket_name);
    SUITE_ADD_TEST(suite, test_oss_preprocess_endpoint);
    SUITE_ADD_TEST(suite, test_oss_fill_read_response_header);
    
    return suite;
}
