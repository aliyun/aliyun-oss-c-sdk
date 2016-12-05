#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

#if defined(WIN32)
static char *image_file = "../oss_c_sdk_test/example.jpg";
#else
static char *image_file = "oss_c_sdk_test/example.jpg";
#endif
static char *sample_image = "example.jpg";

static void put_sample_image();

void image_resize() 
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t filename;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, sample_image);
    aos_str_set(&filename, "example-new.jpg");

    params = aos_table_make(p, 1);
    apr_table_set(params, OSS_PROCESS, "image/resize,m_fixed,w_100,h_100");

    /* get processed image to file */
    s = oss_get_object_to_file(options, &bucket, &object, headers, 
                               params, &filename, &resp_headers);
    if (aos_status_is_ok(s)) {
        printf("get object to file succeeded\n");
    } else {
        printf("get object to file failed\n");  
    }

    aos_pool_destroy(p);
}

void image_crop() 
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t filename;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, sample_image);
    aos_str_set(&filename, "example-new.jpg");

    params = aos_table_make(p, 1);
    apr_table_set(params, OSS_PROCESS, "image/crop,w_100,h_100,x_100,y_100,r_1");

    /* get processed image to file */
    s = oss_get_object_to_file(options, &bucket, &object, headers, 
                               params, &filename, &resp_headers);
    if (aos_status_is_ok(s)) {
        printf("get object to file succeeded\n");
    } else {
        printf("get object to file failed\n");  
    }

    aos_pool_destroy(p);
}

void image_rotate() 
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t filename;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, sample_image);
    aos_str_set(&filename, "example-new.jpg");

    params = aos_table_make(p, 1);
    apr_table_set(params, OSS_PROCESS, "image/rotate,90");

    /* get processed image to file */
    s = oss_get_object_to_file(options, &bucket, &object, headers, 
                               params, &filename, &resp_headers);
    if (aos_status_is_ok(s)) {
        printf("get object to file succeeded\n");
    } else {
        printf("get object to file failed\n");  
    }

    aos_pool_destroy(p);
}

void image_sharpen() 
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t filename;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, sample_image);
    aos_str_set(&filename, "example-new.jpg");

    params = aos_table_make(p, 1);
    apr_table_set(params, OSS_PROCESS, "image/sharpen,100");

    /* get processed image to file */
    s = oss_get_object_to_file(options, &bucket, &object, headers, 
                               params, &filename, &resp_headers);
    if (aos_status_is_ok(s)) {
        printf("get object to file succeeded\n");
    } else {
        printf("get object to file failed\n");  
    }

    aos_pool_destroy(p);
}

void image_watermark() 
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t filename;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, sample_image);
    aos_str_set(&filename, "example-new.jpg");

    params = aos_table_make(p, 1);
    apr_table_set(params, OSS_PROCESS, "image/watermark,text_SGVsbG8g5Zu-54mH5pyN5YqhIQ");

    /* get processed image to file */
    s = oss_get_object_to_file(options, &bucket, &object, headers, 
                               params, &filename, &resp_headers);
    if (aos_status_is_ok(s)) {
        printf("get object to file succeeded\n");
    } else {
        printf("get object to file failed\n");  
    }

    aos_pool_destroy(p);
}

void image_format() {
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t filename;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, sample_image);
    aos_str_set(&filename, "example-new.jpg");

    params = aos_table_make(p, 1);
    apr_table_set(params, OSS_PROCESS, "image/format,png");

    /* get processed image to file */
    s = oss_get_object_to_file(options, &bucket, &object, headers, 
                               params, &filename, &resp_headers);
    if (aos_status_is_ok(s)) {
        printf("get object to file succeeded\n");
    } else {
        printf("get object to file failed\n");  
    }

    aos_pool_destroy(p);
}

void iamge_info()
{
    aos_pool_t *p = NULL;
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

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, sample_image);
    aos_list_init(&buffer);

    params = aos_table_make(p, 1);
    apr_table_set(params, OSS_PROCESS, "image/info");

    /* test get object to buffer */
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, 
                                 params, &buffer, &resp_headers);
    if (aos_status_is_ok(s)) {
        printf("put object from file succeeded\n");
    } else {
        printf("put object from file failed\n");  
    }

    /* get buffer len */
    len = aos_buf_list_len(&buffer);

    buf = (char *)aos_pcalloc(p, (apr_size_t)(len + 1));
    buf[len] = '\0';

    /* copy buffer content to memory */
    aos_list_for_each_entry(aos_buf_t, content, &buffer, node) {
        size = aos_buf_size(content);
        memcpy(buf + pos, content->pos, (size_t)size);
        pos += size;
    }
    
    aos_pool_destroy(p);
}

void put_example_image()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);

    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, sample_image);
    aos_str_set(&filename, image_file);

    s = oss_put_object_from_file(options, &bucket, &object, &filename, 
        headers, &resp_headers);
    if (aos_status_is_ok(s)) {
        printf("put object from file succeeded\n");
    } else {
        printf("put object from file failed\n");  
    }

    aos_pool_destroy(p);
}



/**
 * ÉÏ´«Ê¾ÀýÍ¼Æ¬
 */
void put_sample_image()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t filename;
    aos_status_t *s = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);

    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, sample_image);
    aos_str_set(&filename, image_file);

    s = oss_put_object_from_file(options, &bucket, &object, &filename, 
        headers, &resp_headers);
    if (aos_status_is_ok(s)) {
        printf("put object from file succeeded\n");
    } else {
        printf("put object from file failed\n");
    }

    aos_pool_destroy(p);
}

void image_sample()
{
    put_sample_image();

    image_resize();
    image_crop();
    image_rotate();
    image_sharpen();
    image_watermark();
    image_format();
    iamge_info();
}
