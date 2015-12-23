#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_xml.h"
#include "oss_api.h"

char *oss_gen_signed_url(const oss_request_options_t *options,
                         const aos_string_t *bucket, 
                         const aos_string_t *object,
                         int64_t expires, 
                         aos_http_request_t *req)
{
    int res;
    aos_string_t signed_url;
    char *expires_str;
    aos_string_t expires_time;

    expires_str = apr_psprintf(options->pool, "%" APR_INT64_T_FMT, expires);
    aos_str_set(&expires_time, expires_str);
    oss_get_object_uri(options, bucket, object, req);
    res = oss_get_signed_url(options, req, &expires_time, &signed_url);
    if (res != AOSE_OK) {
        aos_error_log("\ngen signed url fail!");
        return NULL;
    }
    return signed_url.data;
}

aos_status_t *oss_put_object_from_buffer(const oss_request_options_t *options,
                                         const aos_string_t *bucket, 
                                         const aos_string_t *object, 
                                         aos_list_t *buffer,
                                         aos_table_t *headers, 
                                         aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;

    //init query_params
    query_params = aos_table_make(options->pool, 0);

    oss_init_object_request(options, bucket, object, HTTP_PUT, 
                            &req, query_params, headers, &resp);
    oss_write_request_body_from_buffer(buffer, req);

    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;
}

aos_status_t *oss_put_object_from_file(const oss_request_options_t *options,
                                       const aos_string_t *bucket, 
                                       const aos_string_t *object, 
                                       const aos_string_t *filename,
                                       aos_table_t *headers, 
                                       aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;
    int res = AOSE_OK;

    s = aos_status_create(options->pool);

    //init query_params
    query_params = aos_table_make(options->pool, 0);    

    oss_init_object_request(options, bucket, object, HTTP_PUT, &req, 
                            query_params, headers, &resp);
    set_content_type_for_file(filename->data, headers);
    res = oss_write_request_body_from_file(options->pool, filename, req);
    if (res != AOSE_OK) {
        aos_file_error_status_set(s, res);
        return s;
    }

    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;
}

aos_status_t *oss_get_object_to_buffer(const oss_request_options_t *options, 
                                       const aos_string_t *bucket, 
                                       const aos_string_t *object,
                                       aos_table_t *headers, 
                                       aos_list_t *buffer, 
                                       aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;

    //init query_params
    query_params = aos_table_make(options->pool, 0);

    oss_init_object_request(options, bucket, object, HTTP_GET, 
                            &req, query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    oss_init_read_response_body_to_buffer(buffer, resp);
    *resp_headers = resp->headers;

    return s;
}

aos_status_t *oss_get_object_to_file(const oss_request_options_t *options,
                                     const aos_string_t *bucket, 
                                     const aos_string_t *object,
                                     aos_table_t *headers, 
                                     aos_string_t *filename, 
                                     aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;
    int res = AOSE_OK;

    //init query_params
    query_params = aos_table_make(options->pool, 0);

    oss_init_object_request(options, bucket, object, HTTP_GET, 
                            &req, query_params, headers, &resp);

    s = aos_status_create(options->pool);
    res = oss_init_read_response_body_to_file(options->pool, filename, resp);
    if (res != AOSE_OK) {
        aos_file_error_status_set(s, res);
        return s;
    }

    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;
}

aos_status_t *oss_head_object(const oss_request_options_t *options, 
                              const aos_string_t *bucket, 
                              const aos_string_t *object,
                              aos_table_t *headers, 
                              aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;

    //init query_params
    query_params = aos_table_make(options->pool, 0);

    oss_init_object_request(options, bucket, object, HTTP_HEAD, 
                            &req, query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;
}

aos_status_t *oss_delete_object(const oss_request_options_t *options,
                                const aos_string_t *bucket, 
                                const aos_string_t *object, 
                                aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *headers;
    aos_table_t *query_params;

    //init query_params
    query_params = aos_table_make(options->pool, 0);

    //init headers
    headers = aos_table_make(options->pool, 0);

    oss_init_object_request(options, bucket, object, HTTP_DELETE, 
                            &req, query_params, headers, &resp);
    oss_get_object_uri(options, bucket, object, req);

    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;
}

aos_status_t *oss_copy_object(const oss_request_options_t *options, 
                              const aos_string_t *source_bucket, 
                              const aos_string_t *source_object, 
                              const aos_string_t *dest_bucket, 
                              const aos_string_t *dest_object,
                              aos_table_t *headers, 
                              aos_table_t **resp_headers)
{
    char *copy_source;
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;

    //init query_params
    query_params = aos_table_make(options->pool, 0);

    //init headers
    copy_source = apr_psprintf(options->pool, "/%.*s/%.*s", 
                               source_bucket->len, source_bucket->data, 
                               source_object->len, source_object->data);
    apr_table_set(headers, OSS_CANNONICALIZED_HEADER_COPY_SOURCE, copy_source);

    oss_init_object_request(options, dest_bucket, dest_object, HTTP_PUT, 
                            &req, query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;
}

aos_status_t *oss_append_object_from_buffer(const oss_request_options_t *options,
                                            const aos_string_t *bucket, 
                                            const aos_string_t *object, 
                                            int64_t position,
                                            aos_list_t *buffer, 
                                            aos_table_t *headers, 
                                            aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;
    
    //init query_params
    query_params = aos_table_make(options->pool, 2);
    apr_table_add(query_params, OSS_APPEND, "");
    aos_table_add_int64(query_params, OSS_POSITION, position);

    //init headers
    oss_set_multipart_content_type(headers);
    
    oss_init_object_request(options, bucket, object, HTTP_POST, 
                            &req, query_params, headers, &resp);
    oss_write_request_body_from_buffer(buffer, req);

    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;
}

aos_status_t *oss_append_object_from_file(const oss_request_options_t *options,
                                          const aos_string_t *bucket, 
                                          const aos_string_t *object, 
                                          int64_t position,
                                          const aos_string_t *append_file, 
                                          aos_table_t *headers, 
                                          aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;
    int res = AOSE_OK;

    //init query_params
    query_params = aos_table_make(options->pool, 2);
    apr_table_add(query_params, OSS_APPEND, "");
    aos_table_add_int64(query_params, OSS_POSITION, position);

    //init headers
    oss_set_multipart_content_type(headers);

    oss_init_object_request(options, bucket, object, HTTP_POST, 
                            &req, query_params, headers, &resp);
    set_content_type_for_file(append_file->data, headers);
    res = oss_write_request_body_from_file(options->pool, append_file, req);
    s = aos_status_create(options->pool);
    if (res != AOSE_OK) {
        aos_file_error_status_set(s, res);
        return s;
    }

    s = oss_process_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;
}

aos_status_t *oss_put_object_from_buffer_by_url(const oss_request_options_t *options,
                                                const aos_string_t *signed_url, 
                                                aos_list_t *buffer, 
                                                aos_table_t *headers,
                                                aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;

    //init query_params
    query_params = aos_table_make(options->pool, 0);

    oss_init_signed_url_request(options, signed_url, HTTP_PUT, 
                                &req, query_params, headers, &resp);

    oss_write_request_body_from_buffer(buffer, req);

    s = oss_process_signed_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;
}

aos_status_t *oss_put_object_from_file_by_url(const oss_request_options_t *options,
                                              const aos_string_t *signed_url, 
                                              aos_string_t *filename, 
                                              aos_table_t *headers,
                                              aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;
    int res = AOSE_OK;

    s = aos_status_create(options->pool);

    //init query_params
    query_params = aos_table_make(options->pool, 0);

    oss_init_signed_url_request(options, signed_url, HTTP_PUT, 
                                &req, query_params, headers, &resp);
    res = oss_write_request_body_from_file(options->pool, filename, req);
    if (res != AOSE_OK) {
        aos_file_error_status_set(s, res);
        return s;
    }

    s = oss_process_signed_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;
}

aos_status_t *oss_get_object_to_buffer_by_url(const oss_request_options_t *options,
                                              const aos_string_t *signed_url, 
                                              aos_table_t *headers, 
                                              aos_list_t *buffer,
                                              aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;

    //init query_params
    query_params = aos_table_make(options->pool, 0);

    oss_init_signed_url_request(options, signed_url, HTTP_GET, 
                                &req, query_params, headers, &resp);

    s = oss_process_signed_request(options, req, resp);
    oss_init_read_response_body_to_buffer(buffer, resp);
    *resp_headers = resp->headers;

    return s;
}

aos_status_t *oss_get_object_to_file_by_url(const oss_request_options_t *options,
                                            const aos_string_t *signed_url, 
                                            aos_table_t *headers, 
                                            aos_string_t *filename,
                                            aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;
    int res = AOSE_OK;

    s = aos_status_create(options->pool);

    //init query_params
    query_params = aos_table_make(options->pool, 0);
 
    oss_init_signed_url_request(options, signed_url, HTTP_GET, 
                                &req, query_params, headers, &resp);

    res = oss_init_read_response_body_to_file(options->pool, filename, resp);
    if (res != AOSE_OK) {
        aos_file_error_status_set(s, res);
        return s;
    }

    s = oss_process_signed_request(options, req, resp);
    *resp_headers = resp->headers;
 
    return s;
}


aos_status_t *oss_head_object_by_url(const oss_request_options_t *options,
                                     const aos_string_t *signed_url, 
                                     aos_table_t *headers, 
                                     aos_table_t **resp_headers)
{
    aos_status_t *s;
    aos_http_request_t *req;
    aos_http_response_t *resp;
    aos_table_t *query_params;

    //init query_params
    query_params = aos_table_make(options->pool, 0);

    oss_init_signed_url_request(options, signed_url, HTTP_HEAD, 
                                &req, query_params, headers, &resp);

    s = oss_process_signed_request(options, req, resp);
    *resp_headers = resp->headers;

    return s;
}
