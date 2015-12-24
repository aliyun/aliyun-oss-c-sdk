#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void put_object_by_signed_url()
{
    aos_pool_t *p;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t url;
    int is_oss_domain = 1;
    aos_http_request_t *request = NULL;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    oss_request_options_t *options;
    char *object_name = "key-1";
    char *filename = __FILE__;
    aos_status_t *s;
    aos_string_t file;
    char *signed_url = NULL;
    int64_t expires_time;

    aos_pool_create(&p, NULL);

    options = oss_request_options_create(p);
    init_sample_request_options(options, is_oss_domain);

    // create request
    request = aos_http_request_create(p);
    request->method = HTTP_PUT;

    // create headers
    headers = aos_table_make(options->pool, 0);

    // set value
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&file, filename);

    // expires time
    expires_time = apr_time_now() / 1000000 + 120;    

    // generate signed url for put 
    signed_url = oss_gen_signed_url(options, &bucket, &object, 
                                    expires_time, request);
    aos_str_set(&url, signed_url);
    
    printf("signed put url : %s\n", signed_url);

    // put object by signed url
    s = oss_put_object_from_file_by_url(options, &url, &file, 
                                        headers, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("put object by signed url succeeded\n");
    }
    else {
	printf("put object by signed url failed\n");
    }

    aos_pool_destroy(p);
}

void get_object_by_signed_url()
{
    aos_pool_t *p;
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t url;
    int is_oss_domain = 1;
    aos_http_request_t *request = NULL;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    oss_request_options_t *options;
    char *object_name = "key-1";
    aos_list_t buffer;
    aos_status_t *s;
    aos_string_t file;
    char *signed_url = NULL;
    int64_t expires_time;

    aos_pool_create(&p, NULL);

    options = oss_request_options_create(p);
    init_sample_request_options(options, is_oss_domain);

    // create request
    request = aos_http_request_create(p);
    request->method = HTTP_GET;

    // create headers
    headers = aos_table_make(options->pool, 0);

    // set value
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_list_init(&buffer);

    // expires time
    expires_time = apr_time_now() / 1000000 + 120;    

    // generate signed url for put 
    signed_url = oss_gen_signed_url(options, &bucket, &object, 
                                    expires_time, request);
    aos_str_set(&url, signed_url);
    
    printf("signed get url : %s\n", signed_url);

    // put object by signed url
    s = oss_get_object_to_buffer_by_url(options, &url, headers, 
            &buffer, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("get object by signed url succeeded\n");
    }
    else {
	printf("get object by signed url failed\n");
    }

    aos_pool_destroy(p);
}

int main(int argc, char *argv[])
{
    //aos_http_io_initialize first 
    if (aos_http_io_initialize("oss_sample", 0) != AOSE_OK) {
        exit(1);
    }

    put_object_by_signed_url();
    get_object_by_signed_url();

    //aos_http_io_deinitialize last
    aos_http_io_deinitialize();

    return 0;
}
