#include <sys/stat.h>
#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void init_and_complete_multipart_upload_from_file()
{
    aos_pool_t *p;
    aos_string_t bucket;
    aos_string_t object;
    int is_oss_domain = 1;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    oss_request_options_t *options;
    char *object_name = "oss_init_complete_multipart_from_file";
    aos_string_t upload_id;
    oss_upload_file_t *upload_file;
    aos_status_t *s;
    oss_list_upload_part_params_t *params;
    aos_list_t complete_part_list;
    oss_list_part_content_t *part_content;
    oss_complete_part_content_t *complete_part_content;
    int part_num1 = 1;
    int part_num2 = 2;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_oss_domain);
    headers = aos_table_make(p, 1);
    resp_headers = aos_table_make(options->pool, 5); 
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, object_name);
    
    //init mulitipart
    s = oss_init_multipart_upload(options, &bucket, &object, headers, &upload_id, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("Init multipart upload succeeded, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Init multipart upload failed, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    }

    //upload part from file
    upload_file = oss_create_upload_file(p);
    aos_str_set(&upload_file->filename, MULTIPART_UPLOAD_FILE_PATH);
    upload_file->file_pos = 0;
    upload_file->file_last = 200 * 1024; //200k
    s = oss_upload_part_from_file(options, &bucket, &object, &upload_id,
        part_num1, upload_file, &resp_headers);    

    if (NULL != s && 2 == s->code / 100) {
        printf("Multipart upload from file succeeded\n");
    } else {
        printf("Multipart upload from file failed\n");
    }

    upload_file->file_pos = 200 *1024;//remain content start pos
    upload_file->file_last = get_file_size(MULTIPART_UPLOAD_FILE_PATH);
    s = oss_upload_part_from_file(options, &bucket, &object, &upload_id,
        part_num2, upload_file, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("Multipart upload from file succeeded\n");
    } else {
        printf("Multipart upload from file failed\n");
    }

    //list part
    params = oss_create_list_upload_part_params(p);
    params->max_ret = 1000;
    aos_list_init(&complete_part_list);
    s = oss_list_upload_part(options, &bucket, &object, &upload_id, 
                             params, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("List multipart succeeded\n");
    } else {
        printf("List multipart failed\n");
    }

    aos_list_for_each_entry(part_content, &params->part_list, node) {
        complete_part_content = oss_create_complete_part_content(p);
        aos_str_set(&complete_part_content->part_number, part_content->part_number.data);
        aos_str_set(&complete_part_content->etag, part_content->etag.data);
        aos_list_add_tail(&complete_part_content->node, &complete_part_list);
    }

    //complete multipart
    s = oss_complete_multipart_upload(options, &bucket, &object, &upload_id,
        &complete_part_list, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("Complete multipart upload from file succeeded, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Complete multipart upload from file failed\n");
    }

    aos_pool_destroy(p);
}

int main(int argc, char *argv[])
{
    //aos_http_io_initialize first 
    if (aos_http_io_initialize("oss_sample", 0) != AOSE_OK) {
        exit(1);
    }

    init_and_complete_multipart_upload_from_file();

    //aos_http_io_deinitialize last
    aos_http_io_deinitialize();

    return 0;
}
