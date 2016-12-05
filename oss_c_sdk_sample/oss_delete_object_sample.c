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

    if (aos_status_is_ok(s)) {
        printf("delete object succeeded\n");
    } else {
        printf("delete object failed\n");
    }    

    aos_pool_destroy(p);
}

void delete_objects()
{
    aos_pool_t *p = NULL;
    int is_cname = 0;
    aos_string_t bucket;
    aos_status_t *s = NULL;
    aos_table_t *resp_headers = NULL;
    oss_request_options_t *options = NULL;
    char *object_name1 = "oss_sample_object1";
    char *object_name2 = "oss_sample_object2";
    oss_object_key_t *content1 = NULL;
    oss_object_key_t *content2 = NULL;
    aos_list_t object_list;
    aos_list_t deleted_object_list;
    int is_quiet = AOS_TRUE;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);

    aos_list_init(&object_list);
    aos_list_init(&deleted_object_list);
    content1 = oss_create_oss_object_key(p);
    aos_str_set(&content1->key, object_name1);
    aos_list_add_tail(&content1->node, &object_list);
    content2 = oss_create_oss_object_key(p);
    aos_str_set(&content2->key, object_name2);
    aos_list_add_tail(&content2->node, &object_list);

    s = oss_delete_objects(options, &bucket, &object_list, is_quiet,
        &resp_headers, &deleted_object_list);

    aos_pool_destroy(p);

    if (aos_status_is_ok(s)) {
        printf("delete objects succeeded\n");
    } else {
        printf("delete objects failed\n");
    }
}

void delete_object_sample()
{
    delete_object();
    delete_objects();
}
