#include "aos_log.h"
#include "aos_define.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_xml.h"
#include "oss_api.h"

aos_status_t *oss_init_multipart_upload(const oss_request_options_t *options, 
                                        const aos_string_t *bucket, 
                                        const aos_string_t *object, 
                                        aos_string_t *upload_id, 
                                        aos_table_t *headers,
                                        aos_table_t **resp_headers)
{
    int res = AOSE_OK;
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    aos_table_t *query_params = NULL;

    //init query_params
    query_params = aos_table_create_if_null(options, query_params, 1);
    apr_table_add(query_params, OSS_UPLOADS, "");

    //init headers
    headers = aos_table_create_if_null(options, headers, 1);
    set_content_type(NULL, object->data, headers);

    oss_init_object_request(options, bucket, object, HTTP_POST, 
                            &req, query_params, headers, NULL, 0, &resp);

    s = oss_process_request(options, req, resp);
    oss_fill_read_response_header(resp, resp_headers);
    if (!aos_status_is_ok(s)) {
        return s;
    }

    res = oss_upload_id_parse_from_body(options->pool, &resp->body, upload_id);
    if (res != AOSE_OK) {
        aos_xml_error_status_set(s, res);
    }

    return s;
}

aos_status_t *oss_abort_multipart_upload(const oss_request_options_t *options,
                                         const aos_string_t *bucket, 
                                         const aos_string_t *object, 
                                         aos_string_t *upload_id, 
                                         aos_table_t **resp_headers)
{
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    aos_table_t *query_params = NULL;
    aos_table_t *headers = NULL;

    //init query_params
    query_params = aos_table_create_if_null(options, query_params, 1);
    apr_table_add(query_params, OSS_UPLOAD_ID, upload_id->data);

    //init headers
    headers = aos_table_create_if_null(options, headers, 0);

    oss_init_object_request(options, bucket, object, HTTP_DELETE, 
                            &req, query_params, headers, NULL, 0, &resp);

    s = oss_process_request(options, req, resp);
    oss_fill_read_response_header(resp, resp_headers);

    return s;
}

aos_status_t *oss_list_upload_part(const oss_request_options_t *options,
                                   const aos_string_t *bucket, 
                                   const aos_string_t *object, 
                                   const aos_string_t *upload_id, 
                                   oss_list_upload_part_params_t *params,
                                   aos_table_t **resp_headers)
{
    int res = AOSE_OK;
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    aos_table_t *query_params = NULL;
    aos_table_t *headers = NULL;

    //init query_params
    query_params = aos_table_create_if_null(options, query_params, 3);
    apr_table_add(query_params, OSS_UPLOAD_ID, upload_id->data);
    aos_table_add_int(query_params, OSS_MAX_PARTS, params->max_ret);
    apr_table_add(query_params, OSS_PART_NUMBER_MARKER, 
                  params->part_number_marker.data);

    //init headers
    headers = aos_table_create_if_null(options, headers, 0);

    oss_init_object_request(options, bucket, object, HTTP_GET, 
                            &req, query_params, headers, NULL, 0, &resp);

    s = oss_process_request(options, req, resp);
    oss_fill_read_response_header(resp, resp_headers);
    if (!aos_status_is_ok(s)) {
        return s;
    }

    res = oss_list_parts_parse_from_body(options->pool, &resp->body,
            &params->part_list, &params->next_part_number_marker,
            &params->truncated);
    if (res != AOSE_OK) {
        aos_xml_error_status_set(s, res);
    }

    return s;
}

aos_status_t *oss_list_multipart_upload(const oss_request_options_t *options,
                                        const aos_string_t *bucket, 
                                        oss_list_multipart_upload_params_t *params, 
                                        aos_table_t **resp_headers)
{
    int res = AOSE_OK;
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    aos_table_t *query_params = NULL;
    aos_table_t *headers = NULL;

    //init query_params
    query_params = aos_table_create_if_null(options, query_params, 6);
    apr_table_add(query_params, OSS_UPLOADS, "");
    apr_table_add(query_params, OSS_PREFIX, params->prefix.data);
    apr_table_add(query_params, OSS_DELIMITER, params->delimiter.data);
    apr_table_add(query_params, OSS_KEY_MARKER, params->key_marker.data);
    apr_table_add(query_params, OSS_UPLOAD_ID_MARKER, params->upload_id_marker.data);
    aos_table_add_int(query_params, OSS_MAX_UPLOADS, params->max_ret);

    //init headers
    headers = aos_table_create_if_null(options, headers, 0);

    oss_init_bucket_request(options, bucket, HTTP_GET, &req, 
                            query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    oss_fill_read_response_header(resp, resp_headers);
    if (!aos_status_is_ok(s)) {
        return s;
    }

    res = oss_list_multipart_uploads_parse_from_body(options->pool, &resp->body, 
            &params->upload_list, &params->next_key_marker, 
            &params->next_upload_id_marker, &params->truncated);
    if (res != AOSE_OK) {
        aos_xml_error_status_set(s, res);
    }

    return s;
}

aos_status_t *oss_complete_multipart_upload(const oss_request_options_t *options,
                                            const aos_string_t *bucket, 
                                            const aos_string_t *object, 
                                            const aos_string_t *upload_id, 
                                            aos_list_t *part_list, 
                                            aos_table_t *headers,
                                            aos_table_t **resp_headers)
{
    return oss_do_complete_multipart_upload(options, bucket, object, upload_id, part_list,
                                            headers, NULL, resp_headers, NULL);
}

aos_status_t *oss_do_complete_multipart_upload(const oss_request_options_t *options, 
                                               const aos_string_t *bucket, 
                                               const aos_string_t *object, 
                                               const aos_string_t *upload_id, 
                                               aos_list_t *part_list, 
                                               aos_table_t *headers,
                                               aos_table_t *params,
                                               aos_table_t **resp_headers,
                                               aos_list_t *resp_body)
{
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    apr_table_t *query_params = NULL;
    aos_list_t body;

    //init query_params
    query_params = aos_table_create_if_null(options, params, 1);
    apr_table_add(query_params, OSS_UPLOAD_ID, upload_id->data);

    //init headers
    headers = aos_table_create_if_null(options, headers, 1);
    apr_table_set(headers, OSS_CONTENT_TYPE, OSS_MULTIPART_CONTENT_TYPE);

    oss_init_object_request(options, bucket, object, HTTP_POST, 
                            &req, query_params, headers, NULL, 0, &resp);

    build_complete_multipart_upload_body(options->pool, part_list, &body);
    oss_write_request_body_from_buffer(&body, req);

    s = oss_process_request(options, req, resp); 
    oss_fill_read_response_header(resp, resp_headers);
    oss_fill_read_response_body(resp, resp_body);

    return s;
}

aos_status_t *oss_upload_part_from_buffer(const oss_request_options_t *options,
                                          const aos_string_t *bucket, 
                                          const aos_string_t *object, 
                                          const aos_string_t *upload_id, 
                                          int part_num, 
                                          aos_list_t *buffer, 
                                          aos_table_t **resp_headers)
{
    return oss_do_upload_part_from_buffer(options, bucket, object, upload_id, part_num, 
                                          buffer, NULL, NULL, NULL, resp_headers, NULL);
}

aos_status_t *oss_do_upload_part_from_buffer(const oss_request_options_t *options, 
                                             const aos_string_t *bucket, 
                                             const aos_string_t *object, 
                                             const aos_string_t *upload_id,
                                             int part_num, 
                                             aos_list_t *buffer, 
                                             oss_progress_callback progress_callback,
                                             aos_table_t *headers, 
                                             aos_table_t *params,
                                             aos_table_t **resp_headers,
                                             aos_list_t *resp_body)
{
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    aos_table_t *query_params = NULL;

    //init query_params
    query_params = aos_table_create_if_null(options, params, 2);
    apr_table_add(query_params, OSS_UPLOAD_ID, upload_id->data);
    aos_table_add_int(query_params, OSS_PARTNUMBER, part_num);

    //init headers
    headers = aos_table_create_if_null(options, headers, 0);

    oss_init_object_request(options, bucket, object, HTTP_PUT, &req, query_params, 
                            headers, progress_callback, 0, &resp);

    oss_write_request_body_from_buffer(buffer, req);

    s = oss_process_request(options, req, resp);
    oss_fill_read_response_header(resp, resp_headers);
    oss_fill_read_response_body(resp, resp_body);

    if (is_enable_crc(options) && has_crc_in_response(resp)) {
        oss_check_crc_consistent(req->crc64, resp->headers, s);
    }

    return s; 
}

aos_status_t *oss_upload_part_from_file(const oss_request_options_t *options,
                                        const aos_string_t *bucket, 
                                        const aos_string_t *object,
                                        const aos_string_t *upload_id, 
                                        int part_num, 
                                        oss_upload_file_t *upload_file,
                                        aos_table_t **resp_headers)
{
    return oss_do_upload_part_from_file(options, bucket, object, upload_id, part_num, 
                                        upload_file, NULL, NULL, NULL, resp_headers, NULL);
}

aos_status_t *oss_do_upload_part_from_file(const oss_request_options_t *options,
                                           const aos_string_t *bucket, 
                                           const aos_string_t *object,
                                           const aos_string_t *upload_id, 
                                           int part_num, 
                                           oss_upload_file_t *upload_file,
                                           oss_progress_callback progress_callback,
                                           aos_table_t *headers, 
                                           aos_table_t *params,
                                           aos_table_t **resp_headers,
                                           aos_list_t *resp_body)
{
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL; 
    aos_table_t *query_params = NULL;
    int res = AOSE_OK;

    s = aos_status_create(options->pool);

    //init query_params
    query_params = aos_table_create_if_null(options, params, 2);
    apr_table_add(query_params, OSS_UPLOAD_ID, upload_id->data);
    aos_table_add_int(query_params, OSS_PARTNUMBER, part_num);

    //init headers
    headers = aos_table_create_if_null(options, headers, 0);

    oss_init_object_request(options, bucket, object, HTTP_PUT, &req, 
                            query_params, headers, progress_callback, 0, &resp);

    res = oss_write_request_body_from_upload_file(options->pool, upload_file, req);
    if (res != AOSE_OK) {
        aos_file_error_status_set(s, res);
        return s;
    }

    s = oss_process_request(options, req, resp);
    oss_fill_read_response_header(resp, resp_headers);
    oss_fill_read_response_body(resp, resp_body);

    if (is_enable_crc(options) && has_crc_in_response(resp)) {
        oss_check_crc_consistent(req->crc64, resp->headers, s);
    }

    return s;
}

aos_status_t *oss_upload_part_copy(const oss_request_options_t *options,
                                   oss_upload_part_copy_params_t *params, 
                                   aos_table_t *headers, 
                                   aos_table_t **resp_headers)
{
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    aos_table_t *query_params = NULL;
    char *copy_source = NULL;
    char *copy_source_range = NULL;

    s = aos_status_create(options->pool);

    //init query_params
    query_params = aos_table_create_if_null(options, query_params, 2);
    apr_table_add(query_params, OSS_UPLOAD_ID, params->upload_id.data);
    aos_table_add_int(query_params, OSS_PARTNUMBER, params->part_num);

    //init headers
    headers = aos_table_create_if_null(options, headers, 2);
    copy_source = apr_psprintf(options->pool, "/%.*s/%.*s", 
        params->source_bucket.len, params->source_bucket.data, 
        params->source_object.len, params->source_object.data);
    apr_table_add(headers, OSS_COPY_SOURCE, copy_source);
    copy_source_range = apr_psprintf(options->pool, 
            "bytes=%" APR_INT64_T_FMT "-%" APR_INT64_T_FMT,
            params->range_start, params->range_end);
    apr_table_add(headers, OSS_COPY_SOURCE_RANGE, copy_source_range);

    oss_init_object_request(options, &params->dest_bucket, &params->dest_object, 
                            HTTP_PUT, &req, query_params, headers, NULL, 0, &resp);

    s = oss_process_request(options, req, resp);
    oss_fill_read_response_header(resp, resp_headers);

    return s;
}

aos_status_t *oss_get_sorted_uploaded_part(oss_request_options_t *options,
                                           const aos_string_t *bucket, 
                                           const aos_string_t *object, 
                                           const aos_string_t *upload_id, 
                                           aos_list_t *complete_part_list, 
                                           int *part_count)
{
    aos_pool_t *subpool = NULL;
    aos_pool_t *parent_pool = NULL;
    aos_status_t *s = NULL;
    aos_status_t *ret = NULL;
    oss_upload_part_t *part_arr = NULL;
    int part_index = 0;
    int index = 0;
    int uploaded_part_count = 0;
    oss_list_upload_part_params_t *params = NULL;
    oss_list_part_content_t *part_content = NULL;
    oss_complete_part_content_t *complete_content = NULL;
    aos_table_t *list_part_resp_headers = NULL;
    char *part_num_str = NULL;

    parent_pool = options->pool;
    part_arr = aos_palloc(parent_pool, OSS_MAX_PART_NUM * sizeof(oss_upload_part_t *));
    params = oss_create_list_upload_part_params(parent_pool);
    while (params->truncated) {
        aos_pool_create(&subpool, parent_pool);
        options->pool = subpool;
        s = oss_list_upload_part(options, bucket, object,
                upload_id, params, &list_part_resp_headers);
        if (!aos_status_is_ok(s)) {
            ret = aos_status_dup(parent_pool, s);
            aos_pool_destroy(subpool);
            options->pool = parent_pool;
            return ret;
        }
        if (!params->truncated) {
            ret = aos_status_dup(parent_pool, s);
        }
        aos_list_for_each_entry(oss_list_part_content_t, part_content, &params->part_list, node) {
            oss_upload_part_t upload_part;
            upload_part.etag = part_content->etag.data;
            upload_part.part_num = atoi(part_content->part_number.data);
            part_arr[part_index++] = upload_part;
            uploaded_part_count++;
        }

        aos_list_init(&params->part_list);
        if (params->next_part_number_marker.data != NULL) {
            aos_str_set(&params->part_number_marker, 
                        params->next_part_number_marker.data);
        }
        
        //sort multipart upload part content
        qsort(part_arr, uploaded_part_count, sizeof(part_arr[0]), part_sort_cmp);

        for (index = 0; index < part_index; ++index) {
            complete_content = oss_create_complete_part_content(parent_pool);
            part_num_str = apr_psprintf(parent_pool, "%d", part_arr[index].part_num);
            aos_str_set(&complete_content->part_number, part_num_str);
            aos_str_set(&complete_content->etag, part_arr[index].etag);
            aos_list_add_tail(&complete_content->node, complete_part_list);
        }
        part_index = 0;
        aos_pool_destroy(subpool);
    }

    *part_count = uploaded_part_count;
    options->pool = parent_pool;

    return ret;
}

aos_status_t *oss_upload_file(oss_request_options_t *options,
                              const aos_string_t *bucket, 
                              const aos_string_t *object, 
                              aos_string_t *upload_id,
                              aos_string_t *filepath, 
                              int64_t part_size,
                              aos_table_t *headers)
{
    aos_pool_t *subpool = NULL;
    aos_pool_t *parent_pool = NULL;
    int64_t start_pos;
    int64_t end_pos;
    int part_num;
    int part_count = 0;
    int res = AOSE_OK;
    aos_status_t *s = NULL;
    aos_status_t *ret = NULL;
    aos_file_buf_t *fb = NULL;
    oss_upload_file_t *upload_file = NULL;
    aos_table_t *upload_part_resp_headers = NULL;
    char *part_num_str = NULL;
    char *etag = NULL;
    aos_list_t complete_part_list;
    oss_complete_part_content_t *complete_content = NULL;
    aos_table_t *complete_resp_headers = NULL;

    aos_list_init(&complete_part_list);
    parent_pool = options->pool;

    //get upload_id and uploaded part
    aos_pool_create(&subpool, options->pool);
    options->pool = subpool;
    if (NULL == upload_id->data) {
        aos_table_t *init_multipart_headers = NULL;
        aos_table_t *init_multipart_resp_headers = NULL;

        init_multipart_headers = aos_table_make(subpool, 0);
        s = oss_init_multipart_upload(options, bucket, object, 
                upload_id, init_multipart_headers, &init_multipart_resp_headers);
        if (!aos_status_is_ok(s)) {
            ret = aos_status_dup(parent_pool, s);
            aos_pool_destroy(subpool);
            options->pool = parent_pool;
            return ret;
        }
    } else {
        s = oss_get_sorted_uploaded_part(options, bucket, object, upload_id, 
                &complete_part_list, &part_count);
        if (!aos_status_is_ok(s)) {
            ret = aos_status_dup(parent_pool, s);
            aos_pool_destroy(subpool);
            options->pool = parent_pool;
            return ret;
        }
    }
    aos_pool_destroy(subpool);

    //get part size
    fb = aos_create_file_buf(parent_pool);
    res = aos_open_file_for_read(parent_pool, filepath->data, fb);
    if (res != AOSE_OK) {
        s = aos_status_create(parent_pool);
        aos_file_error_status_set(s, res);
        options->pool = parent_pool;
        return s;
    }
    oss_get_part_size(fb->file_last, &part_size);

    //upload part from file
    upload_file = oss_create_upload_file(parent_pool);
    aos_str_set(&upload_file->filename, filepath->data);
    start_pos = part_size * part_count;
    end_pos = start_pos + part_size;
    part_num = part_count + 1;

    while (1) {
        aos_pool_create(&subpool, parent_pool);
        options->pool = subpool;
        upload_file->file_pos = start_pos;
        upload_file->file_last = end_pos;
        
        s = oss_upload_part_from_file(options, bucket, object, upload_id,
            part_num, upload_file, &upload_part_resp_headers);
        if (!aos_status_is_ok(s)) {
            ret = aos_status_dup(parent_pool, s);
            aos_pool_destroy(subpool);
            options->pool = parent_pool;
            return ret;
        }
         
        complete_content = oss_create_complete_part_content(parent_pool);
        part_num_str = apr_psprintf(parent_pool, "%d", part_num);
        aos_str_set(&complete_content->part_number, part_num_str);
        etag = apr_pstrdup(parent_pool, 
                           (char*)apr_table_get(upload_part_resp_headers, "ETag"));
        aos_str_set(&complete_content->etag, etag);
        aos_list_add_tail(&complete_content->node, &complete_part_list);
        aos_pool_destroy(subpool);
        if (end_pos >= fb->file_last) {
            break;
        }
        start_pos += part_size;
        end_pos += part_size;
        if (end_pos > fb->file_last)
            end_pos = fb->file_last;
        part_num += 1;
    }

    //complete multipart
    aos_pool_create(&subpool, parent_pool);
    options->pool = subpool;

    headers = aos_table_create_if_null(options, headers, 0);

    s = oss_complete_multipart_upload(options, bucket, object, upload_id,
            &complete_part_list, headers, &complete_resp_headers);
    ret = aos_status_dup(parent_pool, s);
    aos_pool_destroy(subpool);
    options->pool = parent_pool;
    return ret;
}
