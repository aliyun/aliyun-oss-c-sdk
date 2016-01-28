#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void head_object()
{
    aos_pool_t *p;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_status_t *s;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    headers = aos_table_make(p, 0);

    s = oss_head_object(options, &bucket, &object, headers, &resp_headers);
    
    if (NULL != s && 2 == s->code / 100) {
        printf("head object succeeded\n");
    } else {
        printf("head object failed\n");
    }

    aos_pool_destroy(p);
}

int main(int argc, char *argv[])
{
    if (aos_http_io_initialize("oss_sample", 0) != AOSE_OK) {
        exit(1);
    }

    head_object();

    aos_http_io_deinitialize();

    return 0;
}
