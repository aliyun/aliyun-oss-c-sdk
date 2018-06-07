#include <oss_c_sdk/oss_define.h>
#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void create_select_object_metadata(int* rows, int* splits)
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    char *rows_str = NULL;
    char *splits_str = NULL;
    char *object_type = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    headers = aos_table_make(p, 0);

    csv_format_option csv_format;
    csv_format.field_delimiter = ',';
    csv_format.field_quote = '"';
    aos_str_set(&(csv_format.record_delimiter), "\n");
    csv_format.header_info = CSV_HEADER_IGNORE;

    select_input_serialization input_serialization;
    input_serialization.csv_format = csv_format;
    input_serialization.compression_info = NONE;

    oss_select_metadata_option option;
    option.overwrite = 1;
    option.input_serialization = input_serialization;
    s = oss_create_select_object_metadata(options, &bucket, &object, &option, headers, &resp_headers);
    
    if (aos_status_is_ok(s)) {
        rows_str = (char*)apr_table_get(resp_headers, OSS_SELECT_CSV_ROWS);
        if (rows_str != NULL) {
            *rows = atol(rows_str);
        }

        splits_str = (char*)apr_table_get(resp_headers, OSS_SELECT_CSV_SPLITS);
        if (splits_str != NULL) {
            *splits = atol(splits_str);
        }

        object_type = (char*)apr_table_get(resp_headers, OSS_OBJECT_TYPE);
        
        printf("csv head object succeeded, object type:%s, csv rows:%ld\n, splits:%ld\n",
               object_type, *rows, *splits);
    } else {
        printf("csv head object failed\n");
    }

    aos_pool_destroy(p);
}

char* select_object_to_buffer(aos_pool_t *p, const char* sql_str, int start, int end, select_range_option option, int64_t* plen)
{
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;
    aos_buf_t *content = NULL;
    char *buf = NULL;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;

    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    aos_list_init(&buffer);

    select_input_serialization input_serialization;
    input_serialization.csv_format = csv_format_option_default;
    input_serialization.compression_info = NONE;

    select_output_serialization output_serialization;
    output_serialization.csv_format = csv_format_option_default;
    output_serialization.keep_all_columns = 1;

    oss_select_option select_option;
    select_option.input_serialization = input_serialization;
    select_option.output_serialization = output_serialization;
    select_option.range_option = option;
    select_option.range[0] = start;
    select_option.range[1] = end;

    aos_string_t sql;
    aos_str_set(&sql, sql_str);
    select_option.expression = sql;

    s = oss_select_object_to_buffer(options, &bucket, &object, &select_option,
                                 headers, &buffer, &resp_headers);

    if (aos_status_is_ok(s)) {
        printf("get object to buffer succeeded\n");
    }
    else {
        printf("get object to buffer failed\n");  
    }

    //get buffer len
    aos_list_for_each_entry(aos_buf_t, content, &buffer, node) {
        len += aos_buf_size(content);
    }

    buf = aos_pcalloc(p, (apr_size_t)(len + 1));
    buf[len] = '\0';

    //copy buffer content to memory
    aos_list_for_each_entry(aos_buf_t, content, &buffer, node) {
        size = aos_buf_size(content);
        memcpy(buf + pos, content->pos, (size_t)size);
        pos += size;
    }

    *plen = len;
    return buf;
}

void select_object_to_buffer_sample()
{
    int64_t result_len = 0;
    aos_pool_t *p=NULL;
    aos_pool_create(&p, NULL);
    char *buf = select_object_to_buffer(p, "select count(*) from ossobject where _4 > 60 and _1 like 'Tom%'", -1, -1, NO_RANGE, &result_len);
    printf(buf);
    buf = select_object_to_buffer(p, "select _1,_4 from ossobject where _4 > 60 and _1 like 'Tom%' limit 100", -1, -1, NO_RANGE, &result_len);
    printf("\n\n%s", buf);
    aos_pool_destroy(p);
}

void select_object_to_buffer_big_file_sample()
{
    aos_pool_t *p=NULL;
    aos_pool_create(&p, NULL);
    int splits;
    int lines;
    create_select_object_metadata(&lines, &splits);
    int blockSize = 3;
    int64_t result_len = 0;
    printf("------------line range sample-----------\n");
    for(int i = 0; i < blockSize; i++){
        int start_line = i * lines/blockSize;
        int end_line = i == blockSize -1 ? lines - 1 : (i+1) * lines/blockSize - 1;
        char *buf = select_object_to_buffer(p, "select count(*) from ossobject where _4 > 50 and _1 like 'Tom%'", start_line, end_line, LINE, &result_len);
        printf("%ld\n", atol(buf));
    }

    printf("------------split range sample-----------\n");
    for(int i = 0; i < blockSize; i++){
        int start_split = i * splits/blockSize;
        int end_split = i == blockSize -1 ? splits - 1 : (i+1) * splits/blockSize - 1;
        char *buf = select_object_to_buffer(p, "select count(*) from ossobject where _4 > 50 and _1 like 'Tom%'", start_split, end_split, SPLIT, &result_len);
        printf("%ld\n", atol(buf));
    }
    aos_pool_destroy(p);
}