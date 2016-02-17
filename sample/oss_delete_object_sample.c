#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void delete_object()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);

    s = oss_delete_object(options, &bucket, &object, &resp_headers);

    if (NULL != s && 204 == s->code) {
        printf("delete object succeed\n");
    } else {
        printf("delete object failed\n");
    }

    aos_pool_destroy(p);
}

int main(int argc, char *argv[])
{
    if (aos_http_io_initialize(0) != AOSE_OK) {
        exit(1);
    }

    delete_object();

    aos_http_io_deinitialize();

    return 0;
}
