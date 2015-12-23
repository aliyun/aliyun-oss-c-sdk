#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void init_and_complete_multipart_upload()
{
    aos_pool_t *p;
    aos_string_t bucket;
    aos_string_t object;
    int is_oss_domain = 1;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    oss_request_options_t *options;
    char *object_name = "oss_init_and_complete_multipart";
    aos_string_t upload_id;
    aos_status_t *s;
    aos_list_t buffer;
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
        printf("Init multipart upload failed\n");
    }    

    //upload part
    aos_list_init(&buffer);
    make_random_body(p, 200, &buffer);
    s = oss_upload_part_from_buffer(options, &bucket, &object, &upload_id,
        part_num1, &buffer, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("Upload multipart part succeeded\n");
    } else {
        printf("Upload multipart part failed\n");
    }

    aos_list_init(&buffer);
    make_random_body(p, 10, &buffer);
    s = oss_upload_part_from_buffer(options, &bucket, &object, &upload_id,
        part_num2, &buffer, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("Upload multipart part succeeded\n");
    } else {
        printf("Upload multipart part failed\n");
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
        aos_str_set(&complete_part_content->part_number, 
                    part_content->part_number.data);
        aos_str_set(&complete_part_content->etag, part_content->etag.data);
        aos_list_add_tail(&complete_part_content->node, &complete_part_list);
    }

    //complete multipart
    s = oss_complete_multipart_upload(options, &bucket, &object, &upload_id,
        &complete_part_list, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("Complete multipart upload succeeded, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Complete multipart upload failed\n");
    }

    aos_pool_destroy(p);
}

int main(int argc, char *argv[])
{
    //aos_http_io_initialize first 
    if (aos_http_io_initialize("oss_sample", 0) != AOSE_OK) {
        exit(1);
    }

    init_and_complete_multipart_upload();

    //aos_http_io_deinitialize last
    aos_http_io_deinitialize();

    return 0;
}
