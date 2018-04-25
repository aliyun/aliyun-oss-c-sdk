#include "aos_string.h"
#include "aos_status.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"
#include <assert.h>

void get_object_meta()
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
        printf("get object meta succeeded, object type:%s, content_length:%ld\n", 
               object_type, content_length);
    } else {
		printf("req:%s, get object meta failed\n", s->req_id);
    }
	aos_pool_destroy(p);
}

/*
void get_object_meta_by_signed_url(){
	aos_pool_t *p = NULL;
	aos_string_t bucket;
	aos_string_t object;
	aos_string_t url;
	int is_cname = 0;
	aos_http_request_t *request = NULL;
	aos_table_t *headers = NULL;
	aos_table_t *params = NULL;
	aos_table_t *resp_headers = NULL;
	oss_request_options_t *options = NULL;
	aos_status_t *s = NULL;
	char *signed_url = NULL;
	int64_t expires_time;

	aos_pool_create(&p, NULL);

	options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);

	// create request
    request = aos_http_request_create(p);
    request->method = HTTP_HEAD;
	apr_table_add(request->query_params, OSS_OBJECT_META, "");

    // create headers
    headers = aos_table_make(options->pool, 0);

	// set value
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, OBJECT_NAME);

	// expires time
    expires_time = apr_time_now() / 1000000 + 3600;    

    // generate signed url for put 
    signed_url = oss_gen_signed_url(options, &bucket, &object, 
                                    expires_time, request);
    aos_str_set(&url, signed_url);
    printf("signed get url : %s\n", signed_url);

	s = oss_get_object_meta_by_url(options, &url, &resp_headers);
	
	if (aos_status_is_ok(s)) {
        printf("get object meta by signed url succeeded\n");
    } else {
        printf("get object meta by signed url failed\n");
    }

    aos_pool_destroy(p);
}
*/

void get_object_meta_sample()
{
    get_object_meta();
	//get_object_meta_by_signed_url();
}