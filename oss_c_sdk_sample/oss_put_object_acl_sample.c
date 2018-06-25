#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"

void put_object_acl(){
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_acl_e oss_acl = OSS_ACL_DEFAULT;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);

    s = oss_put_object_acl(options, &bucket, &object, oss_acl, &resp_headers);
    
    if (aos_status_is_ok(s)) {
        printf("put object acl success!\n"); 
    } else {
        printf("put object acl failed!\n"); 
    }

    aos_pool_destroy(p);
}

void put_object_acl_sample(){
    put_object_acl();
}
