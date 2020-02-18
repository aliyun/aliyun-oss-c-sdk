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

static char test_file[1024];
static char test_file_gz[1024];

static void test_select_object_setup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;

    TEST_BUCKET_NAME = get_test_bucket_name(aos_global_pool, "selectobject");

    /* create test bucket */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);
    CuAssertIntEquals(tc, 200, s->code);

    sprintf(test_file, "%ssample_data.csv", get_test_file_path());
    sprintf(test_file_gz, "%ssample_data.csv.gz", get_test_file_path());

    create_test_object_from_file(options, TEST_BUCKET_NAME, "sampleusers.csv", test_file, NULL);

    aos_pool_destroy(p);
}

static void test_select_object_cleanup(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);

    delete_test_object_by_prefix(options, TEST_BUCKET_NAME, "");

    /* delete test bucket */
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    oss_delete_bucket(options, &bucket, &resp_headers);
    apr_sleep(apr_time_from_sec(3));

    aos_pool_destroy(p);
}

void test_select_object_csv_data(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "oss_test_select_objec_sample_data.csv";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;
    char *select_data = NULL;
    oss_select_object_meta_params_t *meta_params;
    long content_length = 0;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;
    char Year[128];
    char StateAbbr[128];
    char CityName[128];
    char PopulationCount[128];
    char line_buff[1024];
    char *ptr = NULL;
    int32_t col = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&buffer);

    create_test_object_from_file(options, TEST_BUCKET_NAME, object_name, test_file, NULL);

    resp_headers = NULL;
    meta_params = oss_create_select_object_meta_params(options->pool);
    s = oss_create_select_object_meta(options, &bucket, &object, meta_params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    s = oss_head_object(options, &bucket, &object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    content_length = atol((char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH));
    CuAssertTrue(tc, content_length == get_file_size(test_file));

    sql = "select Year, StateAbbr, CityName, PopulationCount from ossobject where CityName != '' limit 20";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    aos_str_set(&select_params->input_param.file_header_info, "USE");
    select_params->output_param.enable_payload_crc = AOS_TRUE;
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = aos_buf_list_content(options->pool, &buffer);
    ptr = select_data;

    while ((ptr[0] != '\0') && sscanf(ptr, "%s\n", line_buff)) {
        memset(Year, 0, sizeof(Year));
        memset(StateAbbr, 0, sizeof(Year));
        memset(CityName, 0, sizeof(Year));
        memset(PopulationCount, 0, sizeof(Year));
        sscanf(line_buff, "%[^,],%[^,],%[^,],%[^,]", Year, StateAbbr, CityName, PopulationCount);
        ptr += strlen(line_buff) + 1;
        CuAssertTrue(tc, strlen(CityName) > 0);
        col++;
    }
    CuAssertTrue(tc, col == 20);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_with_output_header(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_with_output_header.csv";
    char *object_data = "name,job\nabc,def\n";
    char *select_data = NULL;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    sql = "select name from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    aos_str_set(&select_params->input_param.file_header_info, "USE");
    select_params->output_param.output_header = AOS_TRUE;
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = aos_buf_list_content(options->pool, &buffer);
    CuAssertStrEquals(tc, "name\nabc\n", select_data);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_without_output_header(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_without_output_header.csv";
    char *object_data = "name,job\nabc,def\n";
    char *select_data = NULL;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    sql = "select name from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    aos_str_set(&select_params->input_param.file_header_info, "USE");
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = aos_buf_list_content(options->pool, &buffer);
    CuAssertStrEquals(tc, "abc\n", select_data);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_with_keep_columns(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_with_keep_columns.csv";
    char *object_data = "abc,def,ghi,jkl\n";
    char *select_data = NULL;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    sql = "select _1, _4 from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    select_params->output_param.keep_all_columns = AOS_TRUE;
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = aos_buf_list_content(options->pool, &buffer);
    CuAssertStrEquals(tc, "abc,,,jkl\n", select_data);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_without_keep_columns(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_without_keep_columns.csv";
    char *object_data = "abc,def,ghi,jkl\n";
    char *select_data = NULL;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    sql = "select _1, _4 from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    select_params->output_param.keep_all_columns = AOS_FALSE;
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = aos_buf_list_content(options->pool, &buffer);
    CuAssertStrEquals(tc, "abc,jkl\n", select_data);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_with_output_raw(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_with_output_raw.csv";
    char *object_data = "abc,def\n";
    char *select_data = NULL;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    sql = "select _1 from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    select_params->output_param.output_rawdata = AOS_TRUE;
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 206, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = aos_buf_list_content(options->pool, &buffer);
    CuAssertStrEquals(tc, "abc\n", select_data);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_without_output_raw(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_without_output_raw.csv";
    char *object_data = "abc,def\n";
    char *select_data = NULL;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    sql = "select _1 from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    select_params->output_param.output_rawdata = AOS_FALSE;
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = aos_buf_list_content(options->pool, &buffer);
    CuAssertStrEquals(tc, "abc\n", select_data);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_with_skip_partial_data_true(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_with_skip_partial_data.csv";
    char *object_data = "abc,def\nefg\nhij,klm\n";
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    sql = "select _1, _2 from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    select_params->option_param.skip_partial_data_record = AOS_TRUE;
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 400, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_with_skip_partial_data_false(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_without_skip_partial_data.csv";
    char *object_data = "abc,def\nefg\nhij,klm\n123,456\n";
    char *select_data = NULL;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    sql = "select _1, _2 from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    select_params->option_param.skip_partial_data_record = AOS_FALSE;
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = aos_buf_list_content(options->pool, &buffer);
    CuAssertStrEquals(tc, "abc,def\nefg,\nhij,klm\n123,456\n", select_data);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_with_crc(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_with_crc.csv";
    char *object_data = "abc,def\n";
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    sql = "select _1, _2 from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    select_params->output_param.enable_payload_crc = AOS_TRUE;
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_without_crc(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_without_crc.csv";
    char *object_data = "abc,def\n";
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    sql = "select _1, _2 from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    select_params->output_param.enable_payload_crc = AOS_FALSE;
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_with_output_delimiters(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_with_output_delimiters.csv";
    char *object_data = "abc,def\n";
    char *select_data = NULL;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    sql = "select _1,_2 from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    aos_str_set(&select_params->output_param.record_delimiter, "\r\n");
    aos_str_set(&select_params->output_param.field_delimiter, "|");
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = aos_buf_list_content(options->pool, &buffer);
    CuAssertStrEquals(tc, "abc|def\r\n", select_data);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_with_line_range(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_with_line_range.csv";
    char *object_data = "abc,def\n123,456\n789,efg\nhij,klm\n";
    char *select_data = NULL;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;
    oss_select_object_meta_params_t *meta_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    meta_params = oss_create_select_object_meta_params(options->pool);
    s = oss_create_select_object_meta(options, &bucket, &object, meta_params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    sql = "select _1,_2 from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    aos_str_set(&select_params->input_param.range, "line-range=1-2");
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = aos_buf_list_content(options->pool, &buffer);
    CuAssertStrEquals(tc, "123,456\n789,efg\n", select_data);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_with_input_comment_character(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_with_input_comment_character.csv";
    char *object_data = "abc,def\n`123,456\n#ghi,jkl\n";
    char *select_data = NULL;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    sql = "select _1 from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    aos_str_set(&select_params->input_param.comment_character, "`");
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = aos_buf_list_content(options->pool, &buffer);
    CuAssertStrEquals(tc, "abc\n#ghi\n", select_data);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_with_input_quote_character(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_with_input_quote_character.csv";
    char *object_data = "'abc','def\n123','456'\n";
    char *select_data = NULL;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    sql = "select _2 from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    select_data = aos_buf_list_content(options->pool, &buffer);
    CuAssertStrEquals(tc, "'def\n'456'\n", select_data);

    //use '
    select_params = oss_create_select_object_params(options->pool);
    aos_str_set(&select_params->input_param.quote_character, "'");
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    select_data = aos_buf_list_content(options->pool, &buffer);
    CuAssertStrEquals(tc, "'def\n123'\n", select_data);

    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_with_input_delimiters(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_with_input_delimiters.csv";
    char *object_data = "abc,def|123,456|7891334\n\n777,888|999,222|012345\n\n";
    char *select_data = NULL;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    sql = "select _2,_3 from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    aos_str_set(&select_params->input_param.record_delimiter, "\n\n");
    aos_str_set(&select_params->input_param.field_delimiter, "|");
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = aos_buf_list_content(options->pool, &buffer);
    CuAssertStrEquals(tc, "123,456|7891334\n\n999,222|012345\n\n", select_data);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_with_gzip_data(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "test_select_object_with_gzip_data.csv.gz";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;
    char *select_data = NULL;
    int32_t select_data_len = 0;
    //char *cmp_data = NULL;
    //int32_t cmp_data_len = 0;
    long content_length = 0;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;
    int64_t select_data_crc = 0;
    //int64_t cmp_data_crc = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&buffer);

    create_test_object_from_file(options, TEST_BUCKET_NAME, object_name, test_file_gz, NULL);

    s = oss_head_object(options, &bucket, &object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    content_length = atol((char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH));
    CuAssertTrue(tc, content_length == get_file_size(test_file_gz));

    sql = "select * from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    aos_str_set(&select_params->input_param.file_header_info, "None");
    aos_str_set(&select_params->input_param.record_delimiter, "\n");
    aos_str_set(&select_params->input_param.field_delimiter, ",");
    aos_str_set(&select_params->input_param.quote_character, "\"");
    aos_str_set(&select_params->input_param.compression_type, "GZIP");
    select_params->output_param.output_rawdata = AOS_FALSE;
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = aos_buf_list_content(options->pool, &buffer);
    select_data_len = strlen(select_data);
    select_data_crc = aos_crc64(0, select_data, (size_t)select_data_len);

    //cmp_data = get_text_file_data(options->pool, test_file);
    //cmp_data_len = strlen(select_data);
    //cmp_data_crc = aos_crc64(0, cmp_data, (size_t)cmp_data_len);
    CuAssertTrue(tc, select_data_crc == 0x42DF5EE66341E3C3);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_with_gzip_data_to_file(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "test_select_object_with_gzip_data.csv.gz";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;
    char *select_data = NULL;
    int32_t select_data_len = 0;
    //char *cmp_data = NULL;
    //int32_t cmp_data_len = 0;
    char *tmpfile = "test_select_object_with_gzip_data.csv.tmp";
    long content_length = 0;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;
    aos_string_t filename;
    int64_t select_data_crc = 0;
    //int64_t cmp_data_crc = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&buffer);

    create_test_object_from_file(options, TEST_BUCKET_NAME, object_name, test_file_gz, NULL);

    s = oss_head_object(options, &bucket, &object, NULL, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    content_length = atol((char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH));
    CuAssertTrue(tc, content_length == get_file_size(test_file_gz));

    sql = "select * from ossobject";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    aos_str_set(&select_params->input_param.file_header_info, "None");
    aos_str_set(&select_params->input_param.record_delimiter, "\n");
    aos_str_set(&select_params->input_param.field_delimiter, ",");
    aos_str_set(&select_params->input_param.quote_character, "\"");
    aos_str_set(&select_params->input_param.compression_type, "GZIP");
    aos_str_set(&filename, tmpfile);
    s = oss_select_object_to_file(options, &bucket, &object, &expression, select_params, &filename, &resp_headers);
    CuAssertIntEquals(tc, 206, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = get_text_file_data(options->pool, tmpfile);
    select_data_len = strlen(select_data);
    select_data_crc = aos_crc64(0, select_data, (size_t)select_data_len);

    //cmp_data = get_text_file_data(options->pool, test_file);
    //cmp_data_len = strlen(select_data);
    //cmp_data_crc = aos_crc64(0, cmp_data, (size_t)cmp_data_len);
    CuAssertTrue(tc, select_data_crc == 0x42DF5EE66341E3C3);

    //to invalid filepath
    aos_str_set(&filename, "g:/invalid-path");
    s = oss_select_object_to_file(options, &bucket, &object, &expression, select_params, &filename, &resp_headers);
    CuAssertIntEquals(tc, AOSE_OPEN_FILE_ERROR, s->code);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_big_csv_data(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "sampleusers.csv";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;
    char *select_data = NULL;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;
    char name[128];
    char company[128];
    char age[128];
    char line_buff[1024];
    char *ptr = NULL;
    int32_t col = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&buffer);

    sql = "select Year, StateAbbr,StateDesc from ossobject limit 200";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    aos_str_set(&select_params->input_param.file_header_info, "USE");
    select_params->output_param.enable_payload_crc = AOS_TRUE;
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = aos_buf_list_content(options->pool, &buffer);
    ptr = select_data;

    memset(line_buff, 0, sizeof(line_buff));
    while ((ptr[0] != '\0') && sscanf(ptr, "%[^\n]\n", line_buff)) {
        memset(name, 0, sizeof(name));
        memset(company, 0, sizeof(company));
        memset(age, 0, sizeof(age));
        sscanf(line_buff, "%[^,],%[^,],%[^,]", name, company, age);
        ptr += strlen(line_buff) + 1;
        CuAssertTrue(tc, strlen(name) > 0);
        memset(line_buff, 0, sizeof(line_buff));
        col++;
    }
    CuAssertIntEquals(tc, 200, col);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_big_csv_data_to_file(CuTest *tc)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    char *object_name = "sampleusers.csv";
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t filename;
    char *select_data = NULL;
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;
    char name[128];
    char company[128];
    char age[128];
    char line_buff[1024];
    char *ptr = NULL;
    int32_t col = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    sql = "select Year, StateAbbr,StateDesc from ossobject limit 200";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    aos_str_set(&select_params->input_param.file_header_info, "USE");
    select_params->output_param.enable_payload_crc = AOS_TRUE;
    aos_str_set(&filename, "result_sampleusers.csv");
    s = oss_select_object_to_file(options, &bucket, &object, &expression, select_params, &filename, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    select_data = get_text_file_data(options->pool, filename.data);
    ptr = select_data;

    memset(line_buff, 0, sizeof(line_buff));
    while ((ptr[0] != '\0') && sscanf(ptr, "%[^\n]\n", line_buff)) {
        memset(name, 0, sizeof(name));
        memset(company, 0, sizeof(company));
        memset(age, 0, sizeof(age));
        sscanf(line_buff, "%[^,],%[^,],%[^,]", name, company, age);
        ptr += strlen(line_buff) + 1;
        CuAssertTrue(tc, strlen(name) > 0);
        memset(line_buff, 0, sizeof(line_buff));
        col++;
    }
    CuAssertIntEquals(tc, 200, col);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_create_meta_delimiters(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_create_meta_delimiters.csv";
    char *object_data = "abc,def123,456|7891334\n777,888|999,222012345\n\n";
    oss_select_object_meta_params_t *meta_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    meta_params = oss_create_select_object_meta_params(options->pool);
    aos_str_set(&meta_params->field_delimiter, ",");
    aos_str_set(&meta_params->record_delimiter, "\n");
    resp_headers = NULL;
    s = oss_create_select_object_meta(options, &bucket, &object, meta_params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    CuAssertIntEquals(tc, 1, meta_params->splits_count);
    CuAssertIntEquals(tc, 3, (int32_t)meta_params->rows_count);
    CuAssertIntEquals(tc, 3, meta_params->columns_count);

    //create meta without overwrite
    meta_params = oss_create_select_object_meta_params(options->pool);
    aos_str_set(&meta_params->field_delimiter, "|");
    aos_str_set(&meta_params->record_delimiter, "\n\n");
    resp_headers = NULL;
    s = oss_create_select_object_meta(options, &bucket, &object, meta_params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    CuAssertIntEquals(tc, 1, meta_params->splits_count);
    CuAssertIntEquals(tc, 3, (int32_t)meta_params->rows_count);
    CuAssertIntEquals(tc, 3, meta_params->columns_count);


    //create meta with overwrite
    meta_params = oss_create_select_object_meta_params(options->pool);
    aos_str_set(&meta_params->field_delimiter, "|");
    aos_str_set(&meta_params->record_delimiter, "\n\n");
    meta_params->over_write_if_existing = AOS_TRUE;
    resp_headers = NULL;
    s = oss_create_select_object_meta(options, &bucket, &object, meta_params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    CuAssertIntEquals(tc, 1, meta_params->splits_count);
    CuAssertIntEquals(tc, 1, (int32_t)meta_params->rows_count);
    CuAssertIntEquals(tc, 3, meta_params->columns_count);

    //create new oject
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);
    meta_params = oss_create_select_object_meta_params(options->pool);
    aos_str_set(&meta_params->field_delimiter, "|");
    aos_str_set(&meta_params->record_delimiter, "\n\n");
    meta_params->over_write_if_existing = AOS_TRUE;
    resp_headers = NULL;
    s = oss_create_select_object_meta(options, &bucket, &object, meta_params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    CuAssertIntEquals(tc, 1, meta_params->splits_count);
    CuAssertIntEquals(tc, 1, (int32_t)meta_params->rows_count);
    CuAssertIntEquals(tc, 3, meta_params->columns_count);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_create_meta_quote_character(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_create_meta_quote_character.csv";
    char *object_data = "'abc','def\n123','456'\n";
    oss_select_object_meta_params_t *meta_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    meta_params = oss_create_select_object_meta_params(options->pool);
    aos_str_set(&meta_params->quote_character, "'");
    resp_headers = NULL;
    s = oss_create_select_object_meta(options, &bucket, &object, meta_params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    CuAssertIntEquals(tc, 1, meta_params->splits_count);
    CuAssertIntEquals(tc, 1, (int32_t)meta_params->rows_count);
    CuAssertIntEquals(tc, 3, meta_params->columns_count);

    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);
    meta_params = oss_create_select_object_meta_params(options->pool);
    aos_str_set(&meta_params->quote_character, "\"");
    resp_headers = NULL;
    s = oss_create_select_object_meta(options, &bucket, &object, meta_params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    CuAssertIntEquals(tc, 1, meta_params->splits_count);
    CuAssertIntEquals(tc, 2, (int32_t)meta_params->rows_count);
    CuAssertIntEquals(tc, 2, meta_params->columns_count);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_invalid(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_invalid.csv";
    char *object_data = "abc,def|123,456|7891334\n\n777,888|999,222|012345\n\n";
    char *sql = NULL;
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    sql = "select _2,_3 from ossobject where";
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 400, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_create_meta_invalid(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "oss_test_select_object_create_meta_invalid.csv";
    char *object_data = "'abc','def\n123','456'\n";
    oss_select_object_meta_params_t *meta_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);

    meta_params = oss_create_select_object_meta_params(options->pool);
    aos_str_set(&meta_params->quote_character, "adbc");
    resp_headers = NULL;
    s = oss_create_select_object_meta(options, &bucket, &object, meta_params, &resp_headers);
    CuAssertIntEquals(tc, 400, s->code);
    CuAssertPtrNotNull(tc, resp_headers);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

void test_select_object_invalid_parameter(CuTest *tc)
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

        s = oss_select_object_to_buffer(options, &bucket, NULL, NULL, NULL, NULL, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_select_object_to_file(options, &bucket, NULL, NULL, NULL, NULL, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);

        s = oss_create_select_object_meta(options, &bucket, NULL, NULL, &resp_headers);
        CuAssertIntEquals(tc, AOSE_INVALID_ARGUMENT, s->code);
        CuAssertStrEquals(tc, AOS_BUCKET_NAME_INVALID_ERROR, s->error_code);
    }
    aos_pool_destroy(p);

    printf("test_select_object_invalid_parameter ok\n");
}

void test_select_object_with_long_sql_expression(CuTest *tc)
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    char *object_name = "test_select_object_with_long_sql_expression.csv";
    char *object_data = "name,job\nabc,def\n";
    char *sql = NULL;
    char sql_buff[2500];
    aos_string_t expression;
    oss_select_object_params_t *select_params;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    create_test_object(options, TEST_BUCKET_NAME, object_name, object_data, NULL);
    sprintf(sql_buff, "%s", "select name from ossobject");
    memset(sql_buff + 26, 0x20, sizeof(sql_buff) - 26);
    sql_buff[2500 - 1] = '\0';
    sql  = sql_buff;
    aos_str_set(&expression, sql);
    select_params = oss_create_select_object_params(options->pool);
    aos_str_set(&select_params->input_param.file_header_info, "USE");
    select_params->output_param.output_header = AOS_TRUE;
    aos_list_init(&buffer);
    s = oss_select_object_to_buffer(options, &bucket, &object, &expression, select_params, &buffer, &resp_headers);
    CuAssertIntEquals(tc, 400, s->code);
    CuAssertPtrNotNull(tc, resp_headers);
    printf("%s ok\n", __FUNCTION__);
}


CuSuite *test_oss_select_object()
{
    CuSuite* suite = CuSuiteNew();   

    SUITE_ADD_TEST(suite, test_select_object_setup);
    SUITE_ADD_TEST(suite, test_select_object_csv_data);
    SUITE_ADD_TEST(suite, test_select_object_with_output_header);
    SUITE_ADD_TEST(suite, test_select_object_without_output_header);
    SUITE_ADD_TEST(suite, test_select_object_with_keep_columns);
    SUITE_ADD_TEST(suite, test_select_object_without_keep_columns);
    SUITE_ADD_TEST(suite, test_select_object_with_output_raw);
    SUITE_ADD_TEST(suite, test_select_object_without_output_raw);
    SUITE_ADD_TEST(suite, test_select_object_with_skip_partial_data_true);
    SUITE_ADD_TEST(suite, test_select_object_with_skip_partial_data_false);
    SUITE_ADD_TEST(suite, test_select_object_with_crc);
    SUITE_ADD_TEST(suite, test_select_object_without_crc);
    SUITE_ADD_TEST(suite, test_select_object_with_output_delimiters);
    SUITE_ADD_TEST(suite, test_select_object_with_line_range);
    SUITE_ADD_TEST(suite, test_select_object_with_input_comment_character);
    SUITE_ADD_TEST(suite, test_select_object_with_input_quote_character);
    SUITE_ADD_TEST(suite, test_select_object_with_input_delimiters);
    SUITE_ADD_TEST(suite, test_select_object_with_gzip_data);
    SUITE_ADD_TEST(suite, test_select_object_with_gzip_data_to_file);
    SUITE_ADD_TEST(suite, test_select_object_big_csv_data);
    SUITE_ADD_TEST(suite, test_select_object_big_csv_data_to_file);
    SUITE_ADD_TEST(suite, test_select_object_create_meta_delimiters);
    SUITE_ADD_TEST(suite, test_select_object_create_meta_quote_character);
    SUITE_ADD_TEST(suite, test_select_object_invalid);
    SUITE_ADD_TEST(suite, test_select_object_create_meta_invalid);
    SUITE_ADD_TEST(suite, test_select_object_invalid_parameter);
    SUITE_ADD_TEST(suite, test_select_object_with_long_sql_expression);
    SUITE_ADD_TEST(suite, test_select_object_cleanup);
    
    return suite;
}
