#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void put_and_get_from_buffer_with_progress()
{
    aos_pool_t *p = NULL;
    char *str = "oss c sdk";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t resp_body;
    aos_list_t buffer;
    aos_buf_t *content;

    /* init test*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
   
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    aos_list_init(&resp_body);

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    
    /* test put object */
    s = oss_do_put_object_from_buffer(options, &bucket, &object, &buffer, 
        headers, params, percentage, &resp_headers, &resp_body);
    if (aos_status_is_ok(s)) {
        printf("put object from buffer succeeded\n");
    } else {
        printf("put object from buffer failed\n");
        aos_pool_destroy(p);
        return;
    }

    aos_pool_destroy(p);

    /* test get object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);

    s = oss_do_get_object_to_buffer(options, &bucket, &object, NULL, NULL, 
        &buffer, percentage, NULL);
    if (aos_status_is_ok(s)) {
        printf("get object to buffer succeeded\n");
    } else {
        printf("get object to buffer failed\n");
    }

    aos_pool_destroy(p);
}

void put_and_get_from_file_with_progress()
{
    aos_pool_t *p = NULL;
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t resp_body;
    char *download_filename = "get_object_to_local_file.txt";

    /* init test*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
   
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    aos_str_set(&filename, __FILE__);
    aos_list_init(&resp_body);

    /* put object */
    s = oss_do_put_object_from_file(options, &bucket, &object, &filename, 
        NULL, NULL, percentage, &resp_headers, &resp_body);
    if (aos_status_is_ok(s)) {
        printf("put object from file succeeded\n");
    } else {
        printf("put object from file failed\n");
        aos_pool_destroy(p);
        return;
    }
    
    aos_pool_destroy(p);

    /* get object */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&filename, download_filename);

    s = oss_do_get_object_to_file(options, &bucket, &object, NULL, NULL, 
        &filename, percentage, NULL);
    if (aos_status_is_ok(s)) {
        printf("get object to file succeeded\n");
    } else {
        printf("get object to file failed\n");
    }

    aos_pool_destroy(p);
}

void append_object_with_progress()
{
    aos_pool_t *p = NULL;
    char *object_name = "oss_append_object.ts";
    char *str = "oss c sdk";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    uint64_t initcrc = 0;
    aos_list_t resp_body;
    aos_list_t buffer;
    aos_buf_t *content;

    /* init test*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
   
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&resp_body);

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    
    /* append object from buffer */
    s = oss_do_append_object_from_buffer(options, &bucket, &object, 0, initcrc, &buffer, 
        headers, params, percentage, &resp_headers, &resp_body);
    if (aos_status_is_ok(s)) {
        printf("progress: append object from buffer succeeded\n");
    } else {
        printf("progress: append object from buffer failed\n");
        aos_pool_destroy(p);
        return;
    }

    aos_pool_destroy(p);

    /* append object from file*/
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);

    aos_str_set(&filename, __FILE__);
    initcrc = aos_atoui64((char*)(apr_table_get(resp_headers, OSS_HASH_CRC64_ECMA)));

    s = oss_do_append_object_from_file(options, &bucket, &object, strlen(str), initcrc, &filename, 
        NULL, NULL, percentage, &resp_headers, &resp_body);
    if (aos_status_is_ok(s)) {
        printf("progress: append object from file succeeded\n");
    } else {
        printf("progress: append object from file failed\n");
    }

    oss_delete_object(options, &bucket, &object, NULL);

    aos_pool_destroy(p);
}

void multipart_upload_from_buffer_with_progress()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_status_t *s = NULL;
    aos_list_t buffer;
    oss_list_upload_part_params_t *params = NULL;
    aos_string_t upload_id;
    aos_list_t complete_part_list;
    oss_list_part_content_t *part_content1 = NULL;
    oss_complete_part_content_t *complete_content1 = NULL;
    int part_num = 1;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);

    // init mulitipart
    s = oss_init_multipart_upload(options, &bucket, &object, &upload_id, NULL, NULL);
    if (aos_status_is_ok(s)) {
        printf("Init multipart upload succeeded, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Init multipart upload failed\n");
        aos_pool_destroy(p);
        return;
    } 

    // upload part
    aos_list_init(&buffer);
    make_random_body(p, 200, &buffer);

    s = oss_do_upload_part_from_buffer(options, &bucket, &object, &upload_id,
        part_num++, &buffer, percentage, NULL, NULL, NULL, NULL);
    if (aos_status_is_ok(s)) {
        printf("Upload multipart part succeeded\n");
    } else {
        printf("Upload multipart part failed\n");
        aos_pool_destroy(p);
        return;
    }

    aos_list_init(&buffer);
    make_random_body(p, 200, &buffer);
    s = oss_do_upload_part_from_buffer(options, &bucket, &object, &upload_id,
        part_num++, &buffer, percentage, NULL, NULL, NULL, NULL);
    if (aos_status_is_ok(s)) {
        printf("Upload multipart part succeeded\n");
    } else {
        printf("Upload multipart part failed\n");
        aos_pool_destroy(p);
        return;
    }

    // list part
    params = oss_create_list_upload_part_params(p);
    params->max_ret = 1;
    aos_list_init(&complete_part_list);

    s = oss_list_upload_part(options, &bucket, &object, &upload_id, 
                             params, NULL);

    aos_list_for_each_entry(oss_list_part_content_t, part_content1, &params->part_list, node) {
        complete_content1 = oss_create_complete_part_content(p);
        aos_str_set(&complete_content1->part_number, part_content1->part_number.data);
        aos_str_set(&complete_content1->etag, part_content1->etag.data);
        aos_list_add_tail(&complete_content1->node, &complete_part_list);
    }

    // complete multipart
    s = oss_complete_multipart_upload(options, &bucket, &object, &upload_id,
            &complete_part_list, NULL, NULL);
    if (aos_status_is_ok(s)) {
        printf("Complete multipart upload succeeded, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Complete multipart upload failed\n");
    }

    aos_pool_destroy(p);
}

void multipart_upload_from_file_with_progress()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_status_t *s = NULL;
    oss_list_upload_part_params_t *params = NULL;
    aos_string_t upload_id;
    aos_list_t complete_part_list;
    oss_upload_file_t *upload_file = NULL;
    oss_list_part_content_t *part_content1 = NULL;
    oss_complete_part_content_t *complete_content1 = NULL;
    int part_num = 1;    

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);

    upload_file = oss_create_upload_file(p);
    aos_str_set(&upload_file->filename, __FILE__);

    // init mulitipart
    s = oss_init_multipart_upload(options, &bucket, &object, &upload_id, NULL, NULL);
    if (aos_status_is_ok(s)) {
        printf("Init multipart upload succeeded, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Init multipart upload failed\n");
        aos_pool_destroy(p);
        return;
    }  

    // upload part
    upload_file->file_pos = 0;
    upload_file->file_last = 1024;
    s = oss_do_upload_part_from_file(options, &bucket, &object, &upload_id,
        part_num++, upload_file, percentage, NULL, NULL, NULL, NULL);
    
    if (aos_status_is_ok(s)) {
        printf("Upload multipart part succeeded\n");
    } else {
        printf("Upload multipart part failed\n");
        aos_pool_destroy(p);
        return;
    }

    // list part
    params = oss_create_list_upload_part_params(p);
    params->max_ret = 1;
    aos_list_init(&complete_part_list);

    s = oss_list_upload_part(options, &bucket, &object, &upload_id, 
                             params, NULL);

    aos_list_for_each_entry(oss_list_part_content_t, part_content1, &params->part_list, node) {
        complete_content1 = oss_create_complete_part_content(p);
        aos_str_set(&complete_content1->part_number, part_content1->part_number.data);
        aos_str_set(&complete_content1->etag, part_content1->etag.data);
        aos_list_add_tail(&complete_content1->node, &complete_part_list);
    }

    // complete multipart
    s = oss_complete_multipart_upload(options, &bucket, &object, &upload_id,
            &complete_part_list, NULL, NULL);
    if (aos_status_is_ok(s)) {
        printf("Complete multipart upload succeeded, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Complete multipart upload failed\n");
    }

    aos_pool_destroy(p);
}

void progress_sample()
{
    put_and_get_from_buffer_with_progress();
    put_and_get_from_file_with_progress();
    append_object_with_progress();
    multipart_upload_from_buffer_with_progress();
    multipart_upload_from_file_with_progress();
}
