#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void put_object_from_buffer_with_callback()
{
    aos_pool_t *p = NULL;
    char *str = "test oss c sdk";
    aos_status_t *s = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *headers = NULL;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t resp_body;
    aos_list_t buffer;
    aos_buf_t *content;
    char *buf = NULL;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;
    char b64_buf[1024];
    int b64_len;
    
    /* JSON format */
    char *callback =  "{"
        "\"callbackUrl\":\"http://callback.oss-demo.com:23450\","
        "\"callbackHost\":\"oss-cn-hangzhou.aliyuncs.com\","
        "\"callbackBody\":\"bucket=${bucket}&object=${object}&size=${size}&mimeType=${mimeType}\","
        "\"callbackBodyType\":\"application/x-www-form-urlencoded\""
        "}";

    /* init sample */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
   
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    aos_list_init(&resp_body);
    aos_list_init(&buffer);

    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    /* put call into header */
    b64_len = aos_base64_encode((unsigned char*)callback, strlen(callback), b64_buf);
    b64_buf[b64_len] = '\0';

    headers = aos_table_make(p, 1);
    apr_table_set(headers, OSS_CALLBACK, b64_buf);
    
    /* test put object */
    s = oss_do_put_object_from_buffer(options, &bucket, &object, &buffer, 
        headers, NULL, NULL, &resp_headers, &resp_body);
    if (aos_status_is_ok(s)) {
        printf("put object from buffer succeeded\n");
    } else {
        printf("put object from buffer failed\n");      
    }    

    /* get buffer len */
    len = aos_buf_list_len(&resp_body);
    buf = (char *)aos_pcalloc(p, (apr_size_t)(len + 1));
    buf[len] = '\0';

    /* copy buffer content to memory */
    aos_list_for_each_entry(aos_buf_t, content, &resp_body, node) {
        size = aos_buf_size(content);
        memcpy(buf + pos, content->pos, (size_t)size);
        pos += size;
    }

    aos_pool_destroy(p);
}

void multipart_from_buffer_with_callback()
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
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    oss_list_part_content_t *part_content1 = NULL;
    oss_complete_part_content_t *complete_content1 = NULL;
    int part_num = 1;
    aos_list_t resp_body;
    aos_buf_t *content;
    char *buf = NULL;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;
    char b64_buf[1024];
    int b64_len;

    /* JSON format */
    char *callback =  "{"
        "\"callbackUrl\":\"http://callback.oss-demo.com:23450\","
        "\"callbackHost\":\"oss-cn-hangzhou.aliyuncs.com\","
        "\"callbackBody\":\"bucket=${bucket}&object=${object}&size=${size}&mimeType=${mimeType}\","
        "\"callbackBodyType\":\"application/x-www-form-urlencoded\""
        "}";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);

    /* put call into header */
    b64_len = aos_base64_encode((unsigned char*)callback, strlen(callback), b64_buf);
    b64_buf[b64_len] = '\0';

    headers = aos_table_make(p, 3);
    apr_table_set(headers, OSS_CALLBACK, b64_buf);
    apr_table_set(headers, OSS_CALLBACK, b64_buf);
    apr_table_set(headers, OSS_CALLBACK, b64_buf);

    /* init mulitipart */
    s = oss_init_multipart_upload(options, &bucket, &object, &upload_id, NULL, &resp_headers);
    if (aos_status_is_ok(s)) {
        printf("Init multipart upload succeeded, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Init multipart upload failed\n");
        aos_pool_destroy(p);
        return;
    }

    /* upload part */
    aos_list_init(&buffer);
    make_random_body(p, 2, &buffer);

    s = oss_upload_part_from_buffer(options, &bucket, &object, &upload_id,
        part_num++, &buffer, NULL);
    if (aos_status_is_ok(s)) {
        printf("Multipart upload part from file succeeded\n");
    } else {
        printf("Multipart upload part from file failed\n");
        aos_pool_destroy(p);
        return;
    }

    aos_list_init(&buffer);
    make_random_body(p, 200, &buffer);
    s = oss_upload_part_from_buffer(options, &bucket, &object, &upload_id,
        part_num++, &buffer, NULL);
    if (aos_status_is_ok(s)) {
        printf("Multipart upload part from file succeeded\n");
    } else {
        printf("Multipart upload part from file failed\n");
        aos_pool_destroy(p);
        return;
    }

    /* list part */
    params = oss_create_list_upload_part_params(p);
    params->max_ret = 1;
    aos_list_init(&complete_part_list);

    s = oss_list_upload_part(options, &bucket, &object, &upload_id, 
                             params, NULL);
    if (aos_status_is_ok(s)) {
        printf("List multipart succeeded\n");
    } else {
        printf("List multipart failed\n");
        aos_pool_destroy(p);
        return;
    }

    aos_list_for_each_entry(oss_list_part_content_t, part_content1, &params->part_list, node) {
        complete_content1 = oss_create_complete_part_content(p);
        aos_str_set(&complete_content1->part_number, part_content1->part_number.data);
        aos_str_set(&complete_content1->etag, part_content1->etag.data);
        aos_list_add_tail(&complete_content1->node, &complete_part_list);
    }

    /* complete multipart */
    s = oss_do_complete_multipart_upload(options, &bucket, &object, &upload_id,
            &complete_part_list, headers, NULL, &resp_headers, &resp_body);
    if (aos_status_is_ok(s)) {
        printf("Complete multipart upload from file succeeded, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Complete multipart upload from file failed\n");
        printf("%d, %s, %s, %s\n", s->code, s->error_code, s->error_msg, s->req_id);
    }

    /* get buffer len */
    len = aos_buf_list_len(&resp_body);
    buf = (char *)aos_pcalloc(p, (apr_size_t)(len + 1));
    buf[len] = '\0';

    /* copy buffer content to memory */
    aos_list_for_each_entry(aos_buf_t, content, &resp_body, node) {
        size = aos_buf_size(content);
        memcpy(buf + pos, content->pos, (size_t)size);
        pos += size;
    }

    aos_pool_destroy(p);
}

void callback_sample()
{
    put_object_from_buffer_with_callback();
    multipart_from_buffer_with_callback();
}
