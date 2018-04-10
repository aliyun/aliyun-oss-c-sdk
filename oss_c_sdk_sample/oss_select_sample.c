#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

int head_csv_object()
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
    char *object_type = NULL;
    int64_t rows = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    headers = aos_table_make(p, 0);

    csv_format_option csv_format;
    csv_format.field_delimiter = ',';
    csv_format.field_quote = '"';
    aos_str_set(&(csv_format.new_line), "\r\n");
    csv_format.header_info = CSV_HEADER_IGNORE;

    s = oss_head_csv_object(options, &bucket, &object, &csv_format, headers, &resp_headers);
    
    if (aos_status_is_ok(s)) {
        rows_str = (char*)apr_table_get(resp_headers, OSS_SELECT_CSV_ROWS);
        if (rows_str != NULL) {
            rows = atol(rows_str);
        }

        object_type = (char*)apr_table_get(resp_headers, OSS_OBJECT_TYPE);
        
        printf("csv head object succeeded, object type:%s, csv rows:%ld\n", 
               object_type, rows);
    } else {
        printf("csv head object failed\n");
    }

    aos_pool_destroy(p);
    return rows;
}

char* select_object_to_buffer(aos_pool_t *p, const char* sql_str, int start_line, int end_line, int64_t* plen)
{
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
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

    csv_format_option csv_format;
    csv_format.field_delimiter = ',';
    csv_format.field_quote = '"';
    aos_str_set(&(csv_format.new_line), "\r\n");
    csv_format.header_info = CSV_HEADER_IGNORE;

    oss_select_option select_option;
    select_option.keep_all_columns = 1;
    select_option.raw_output = 1;
    select_option.start_line = start_line;
    select_option.end_line = end_line;

    aos_string_t sql;
    aos_str_set(&sql, sql_str);
    s = oss_select_object_to_buffer(options, &bucket, &object, &sql, &csv_format, &select_option,
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
    char *buf = select_object_to_buffer(p, "select count(*) from ossobject where _4 > 60 and _1 like 'Tom%'", -1, -1, &result_len);
    printf(buf);
    buf = select_object_to_buffer(p, "select _1,_4 from ossobject where _4 > 60 and _1 like 'Tom%' limit 100", -1, -1, &result_len);
    printf("\n\n%s", buf);
    aos_pool_destroy(p);
}

void select_object_to_buffer_big_file_sample()
{
    aos_pool_t *p=NULL;
    aos_pool_create(&p, NULL);
    int lines = head_csv_object();
    int blockSize = 8;
    int64_t result_len = 0;
    for(int i = 0; i < blockSize; i++){
        int start_line = i * lines/blockSize;
        int end_line = i == blockSize -1 ? lines - 1 : (i+1) * lines/blockSize - 1;
        char *buf = select_object_to_buffer(p, "select count(*) from ossobject where _4 > 60 and _1 like 'Tom%'", start_line, end_line, &result_len);
        printf(buf);
    }

    aos_pool_destroy(p);
}