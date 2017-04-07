#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void list_object()
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_list_object_params_t *params = NULL;
    oss_list_object_content_t *content = NULL;
    int size = 0;
    char *line = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);

    params = oss_create_list_object_params(p);
    aos_str_set(&bucket, BUCKET_NAME);
    
    s = oss_list_object(options, &bucket, params, NULL);
    if (!aos_status_is_ok(s))
    {
        printf("list object failed\n");
        return;
    }

    printf("Object\tSize\tLastModified\n");
    aos_list_for_each_entry(oss_list_object_content_t, content, &params->object_list, node) {
        ++size;
        line = apr_psprintf(p, "%.*s\t%.*s\t%.*s\n", content->key.len, content->key.data, 
            content->size.len, content->size.data, 
            content->last_modified.len, content->last_modified.data);
        printf("%s", line);
    }
    printf("Total %d\n", size);

    aos_pool_destroy(p);

    printf("List object ok\n");
}

void list_all_objects() 
{
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_status_t *s = NULL;
    oss_list_object_params_t *params = NULL;
    oss_list_object_content_t *content = NULL;
    int size = 0;
    char *line = NULL;
    char *prefix = "mingdi";
    char *nextMarker = "";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);

    params = oss_create_list_object_params(p);
    params->max_ret = 10;
    aos_str_set(&params->prefix, prefix);
    aos_str_set(&params->marker, nextMarker);

    printf("Object\tSize\tLastModified\n");

    do {
        s = oss_list_object(options, &bucket, params, NULL);
        if (!aos_status_is_ok(s))
        {
            printf("list object failed\n");
            return;
        }

        aos_list_for_each_entry(oss_list_object_content_t, content, &params->object_list, node) {
            ++size;
            line = apr_psprintf(p, "%.*s\t%.*s\t%.*s\n", content->key.len, content->key.data, 
                content->size.len, content->size.data, 
                content->last_modified.len, content->last_modified.data);
            printf("%s", line);
        }

        nextMarker = apr_psprintf(p, "%.*s", params->next_marker.len, params->next_marker.data);
        aos_str_set(&params->marker, nextMarker);
        aos_list_init(&params->object_list);
        aos_list_init(&params->common_prefix_list);
    } while (params->truncated == AOS_TRUE);
   
    printf("Total %d\n", size);

    aos_pool_destroy(p);

    printf("List object ok\n");
}

void list_object_sample()
{   
    list_object();
    list_all_objects();
}
