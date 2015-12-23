#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void init_and_abort_multipart_upload()
{
    aos_pool_t *p;
    aos_string_t bucket;
    aos_string_t object;
    int is_oss_domain = 1;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    oss_request_options_t *options;
    char *object_name = "oss_init_and_abort_multipart";
    aos_string_t upload_id;
    aos_status_t *s;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_oss_domain);
    headers = aos_table_make(p, 1);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, object_name);
    
    s = oss_init_multipart_upload(options, &bucket, &object, headers, &upload_id, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("Init multipart upload succeeded, upload_id:%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Init multipart upload failed\n"); 
    }
    
    s = oss_abort_multipart_upload(options, &bucket, &object, &upload_id, &resp_headers);

    if (NULL != s && 2 == s->code / 100) {
        printf("Abort multipart upload succeeded, upload_id::%.*s\n", 
               upload_id.len, upload_id.data);
    } else {
        printf("Abort multipart upload failed\n"); 
    }    

    aos_pool_destroy(p);
}

int main(int argc, char *argv[])
{
    //aos_http_io_initialize first 
    if (aos_http_io_initialize("oss_sample", 0) != AOSE_OK) {
        exit(1);
    }

    init_and_abort_multipart_upload();

    //aos_http_io_deinitialize last
    aos_http_io_deinitialize();

    return 0;
}
