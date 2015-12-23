#include "aos_log.h"
#include "aos_define.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_xml.h"
#include "oss_api.h"

aos_status_t *oss_create_bucket(const oss_request_options_t *options, 
                                const aos_string_t *bucket, 
                                oss_acl_e oss_acl, 
                                aos_table_t **resp_headers)
{
    const char *oss_acl_str;
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *headers;
    aos_table_t *query_params;

    //init query_params
    query_params = aos_table_make(options->pool, 0);

    //init headers
    headers = aos_table_make(options->pool, 1);
    oss_acl_str = get_oss_acl_str(oss_acl);
    if (oss_acl_str) {
        apr_table_set(headers, OSS_CANNONICALIZED_HEADER_ACL, oss_acl_str);
    }

    oss_init_bucket_request(options, bucket, HTTP_PUT, &req, 
                            query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;
}

aos_status_t *oss_delete_bucket(const oss_request_options_t *options,
                                const aos_string_t *bucket, 
                                aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;
    aos_table_t *headers;

    //init query_params
    query_params = aos_table_make(options->pool, 0);

    //init headers
    headers = aos_table_make(options->pool, 0);

    oss_init_bucket_request(options, bucket, HTTP_DELETE, &req, 
                            query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;
}

aos_status_t *oss_put_bucket_acl(const oss_request_options_t *options, 
                                 const aos_string_t *bucket, 
                                 oss_acl_e oss_acl,
                                 aos_table_t **resp_headers)
{
    aos_status_t *s;
    int res;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;
    aos_table_t *headers;
    const char *oss_acl_str;

    //init query_params
    query_params = aos_table_make(options->pool, 1);
    apr_table_add(query_params, OSS_ACL, "");

    //init headers
    headers = aos_table_make(options->pool, 1);

    oss_acl_str = get_oss_acl_str(oss_acl);
    if (oss_acl_str) {
        apr_table_set(headers, OSS_CANNONICALIZED_HEADER_ACL, oss_acl_str);
    }

    oss_init_bucket_request(options, bucket, HTTP_PUT, &req, 
                            query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;    
}

aos_status_t *oss_get_bucket_acl(const oss_request_options_t *options, 
                                 const aos_string_t *bucket, 
                                 aos_string_t *oss_acl, 
                                 aos_table_t **resp_headers)
{
    aos_status_t *s;
    int res;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;
    aos_table_t *headers;

    //init query_params
    query_params = aos_table_make(options->pool, 1);
    apr_table_add(query_params, OSS_ACL, "");

    //init headers
    headers = aos_table_make(options->pool, 0);

    oss_init_bucket_request(options, bucket, HTTP_GET, &req, 
                            query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;
    if (!aos_status_is_ok(s)) {
        return s;
    }

    res = oss_acl_parse_from_body(options->pool, &resp->body, oss_acl);
    if (res != AOSE_OK) {
        aos_xml_error_status_set(s, res);
    }

    return s;
}

aos_status_t *oss_list_object(const oss_request_options_t *options,
                              const aos_string_t *bucket, 
                              oss_list_object_params_t *params, 
                              aos_table_t **resp_headers)
{
    int res;
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;
    aos_table_t *headers;

    //init query_params
    query_params = aos_table_make(options->pool, 4);
    apr_table_add(query_params, OSS_PREFIX, params->prefix.data);
    apr_table_add(query_params, OSS_DELIMITER, params->delimiter.data);
    apr_table_add(query_params, OSS_MARKER, params->marker.data);
    aos_table_add_int(query_params, OSS_MAX_KEYS, params->max_ret);
    
    //init headers
    headers = aos_table_make(options->pool, 0);

    oss_init_bucket_request(options, bucket, HTTP_GET, &req, 
                            query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;
    if (!aos_status_is_ok(s)) {
        return s;
    }

    res = oss_list_objects_parse_from_body(options->pool, &resp->body, 
            &params->object_list, &params->common_prefix_list, 
            &params->next_marker, &params->truncated);
    if (res != AOSE_OK) {
        aos_xml_error_status_set(s, res);
    }

    return s;
}

aos_status_t *oss_put_bucket_lifecycle(const oss_request_options_t *options,
                                       const aos_string_t *bucket, 
                                       aos_list_t *lifecycle_rule_list, 
                                       aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    apr_table_t *query_params;
    aos_table_t *headers;
    aos_list_t body;

    //init query_params
    query_params = aos_table_make(options->pool, 1);
    apr_table_add(query_params, OSS_LIFECYCLE, "");

    //init headers
    headers = aos_table_make(options->pool, 5);

    oss_init_bucket_request(options, bucket, HTTP_PUT, &req, 
                            query_params, headers, &resp);

    build_lifecycle_body(options->pool, lifecycle_rule_list, &body);
    oss_write_request_body_from_buffer(&body, req);
    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;

}

aos_status_t *oss_get_bucket_lifecycle(const oss_request_options_t *options,
                                       const aos_string_t *bucket, 
                                       aos_list_t *lifecycle_rule_list, 
                                       aos_table_t **resp_headers)
{
    int res;
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;
    aos_table_t *headers;

    //init query_params
    query_params = aos_table_make(options->pool, 1);
    apr_table_add(query_params, OSS_LIFECYCLE, "");

    //init headers
    headers = aos_table_make(options->pool, 5);

    oss_init_bucket_request(options, bucket, HTTP_GET, &req, 
                            query_params, headers, &resp);
    
    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;
    if (!aos_status_is_ok(s)) {
        return s;
    }

    res = oss_lifecycle_rules_parse_from_body(options->pool, 
            &resp->body, lifecycle_rule_list);
    if (res != AOSE_OK) {
        aos_xml_error_status_set(s, res);
    }

    return s;
}

aos_status_t *oss_delete_bucket_lifecycle(const oss_request_options_t *options,
                                          const aos_string_t *bucket, 
                                          aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;
    aos_table_t *headers;

    //init query_params
    query_params = aos_table_make(options->pool, 1);
    apr_table_add(query_params, OSS_LIFECYCLE, "");

    //init headers
    headers = aos_table_make(options->pool, 5);

    oss_init_bucket_request(options, bucket, HTTP_DELETE, &req, 
                            query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;
}

aos_status_t *oss_delete_objects(const oss_request_options_t *options,
                                 const aos_string_t *bucket, 
                                 aos_list_t *object_list, 
                                 int is_quiet,
                                 aos_table_t **resp_headers, 
                                 aos_list_t *deleted_object_list)
{
    int res;
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *headers;
    aos_table_t *query_params;
    aos_list_t body;
    unsigned char *md5 = NULL;
    char *buf;
    int64_t body_len;
    char *b64_value;
    int b64_buf_len = (20 + 1) * 4 / 3;
    int b64_len;

    //init query_params
    query_params = aos_table_make(options->pool, 0);
    apr_table_add(query_params, OSS_DELETE, "");

    //init headers
    headers = aos_table_make(options->pool, 0);
    apr_table_set(headers, OSS_CONTENT_TYPE, OSS_MULTIPART_CONTENT_TYPE);

    oss_init_bucket_request(options, bucket, HTTP_POST, &req, 
                            query_params, headers, &resp);

    build_delete_objects_body(options->pool, object_list, is_quiet, &body);

    //add Content-MD5
    body_len = aos_buf_list_len(&body);
    buf = aos_buf_list_content(options->pool, &body);
    md5 = aos_md5(options->pool, buf, body_len);
    b64_value = aos_pcalloc(options->pool, b64_buf_len);
    b64_len = aos_base64_encode(md5, 20, b64_value);
    b64_value[b64_len] = '\0';
    apr_table_addn(headers, OSS_CONTENT_MD5, b64_value);

    oss_write_request_body_from_buffer(&body, req);

    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;

    if (is_quiet) {
        return s;
    }

    if (!aos_status_is_ok(s)) {
        return s;
    }

    res = oss_delete_objects_parse_from_body(options->pool, &resp->body, 
                                             deleted_object_list);
    if (res != AOSE_OK) {
        aos_xml_error_status_set(s, res);
    }

    return s;
}

aos_status_t *oss_delete_objects_by_prefix(oss_request_options_t *options,
                                           const aos_string_t *bucket, 
                                           const aos_string_t *prefix)
{
    aos_pool_t *subpool;
    aos_pool_t *parent_pool;
    int is_quiet = 1;
    aos_status_t *s;
    aos_status_t *ret;
    oss_list_object_params_t *params;
    int list_object_count = 0;
    
    parent_pool = options->pool;
    params = oss_create_list_object_params(parent_pool);
    if (prefix->data == NULL) {
        aos_str_set(&params->prefix, "");
    } else {
        aos_str_set(&params->prefix, prefix->data);
    }
    while (params->truncated) {
        aos_table_t *list_object_resp_headers;
        aos_list_t object_list;
        aos_list_t deleted_object_list;
        oss_list_object_content_t *list_content;
        aos_table_t *delete_objects_resp_headers;
        char *key;

        aos_pool_create(&subpool, parent_pool);
        options->pool = subpool;
        list_object_count = 0;
        aos_list_init(&object_list);
        s = oss_list_object(options, bucket, params, &list_object_resp_headers);
        if (!aos_status_is_ok(s)) {
            aos_error_log("list objects by prefix fail! prefix:%.*s\n", 
                          prefix->len, prefix->data);
            ret = aos_status_dup(parent_pool, s);
            aos_pool_destroy(subpool);
            options->pool = parent_pool;
            return ret;
        }

        aos_list_for_each_entry(list_content, &params->object_list, node) {
            oss_object_key_t *object_key = oss_create_oss_object_key(parent_pool);
            key = apr_psprintf(parent_pool, "%.*s", list_content->key.len, 
                               list_content->key.data);
            aos_str_set(&object_key->key, key);
            aos_list_add_tail(&object_key->node, &object_list);
            list_object_count += 1;
        }
        if (list_object_count == 0)
        {
            ret = aos_status_dup(parent_pool, s);
            aos_pool_destroy(subpool);
            options->pool = parent_pool;
            return ret;
        }
        aos_pool_destroy(subpool);

        aos_list_init(&deleted_object_list);
        aos_pool_create(&subpool, parent_pool);
        options->pool = subpool;
        s = oss_delete_objects(options, bucket, &object_list, is_quiet,
                               &delete_objects_resp_headers, &deleted_object_list);
        if (!aos_status_is_ok(s)) {
            aos_error_log("delete objects by prefix fail! prefix:%.*s error_msg:%s\n",
                          prefix->len, prefix->data, s->error_msg);
            ret = aos_status_dup(parent_pool, s);
            aos_pool_destroy(subpool);
            options->pool = parent_pool;
            return ret;
        }
        if (!params->truncated) {
            ret = aos_status_dup(parent_pool, s);
        }

        aos_pool_destroy(subpool);

        aos_list_init(&params->object_list);
        if (params->next_marker.data) {
            aos_str_set(&params->marker, params->next_marker.data);
        }
    }
    options->pool = parent_pool;
    return ret;
}
