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
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    char *content_length_str = NULL;
    char *object_type = NULL;
    int64_t content_length = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);
    headers = aos_table_make(p, 0);

    s = oss_head_object(options, &bucket, &object, headers, &resp_headers);
    
    if (aos_status_is_ok(s)) {
        content_length_str = (char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH);
        if (content_length_str != NULL) {
            content_length = atol(content_length_str);
        }

        object_type = (char*)apr_table_get(resp_headers, OSS_OBJECT_TYPE);
        
        printf("head object succeeded, object type:%s, content length:%"APR_INT64_T_FMT"\n", object_type, content_length);
    } else {
        printf("head object failed\n");
    }

    aos_pool_destroy(p);
}

static void get_object_meta()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    char *content_length_str = NULL;
    char *object_type = NULL;
    int64_t content_length = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);

    s = oss_get_object_meta(options, &bucket, &object, &resp_headers);

    if (aos_status_is_ok(s)) {
        content_length_str = (char*)apr_table_get(resp_headers, OSS_CONTENT_LENGTH);
        if (content_length_str != NULL) {
            content_length = atol(content_length_str);
        }

        object_type = (char*)apr_table_get(resp_headers, OSS_OBJECT_TYPE);

        printf("get object meta succeeded, object type:%s, content_length:%"APR_INT64_T_FMT"\n",
            object_type, content_length);
    }
    else {
        printf("get object meta failed\n");
    }

    aos_pool_destroy(p);
}



void head_object_sample()
{
    head_object();
    get_object_meta();
}
