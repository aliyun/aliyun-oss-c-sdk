#include "aos_log.h"
#include "aos_define.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_xml.h"
#include "oss_api.h"

aos_status_t *oss_create_live_channel(const oss_request_options_t *options,
                                      const aos_string_t *bucket,
                                      oss_live_channel_configuration_t *config,
                                      aos_list_t *publish_url_list,
                                      aos_list_t *play_url_list,
                                      aos_table_t **resp_headers)
{
    int res = AOSE_OK;
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    aos_table_t *query_params = NULL;
    aos_table_t *headers = NULL;
    aos_list_t body;

    //init params
    query_params = aos_table_create_if_null(options, query_params, 1);
    apr_table_add(query_params, OSS_LIVE_CHANNEL, "");

    //init headers
    headers = aos_table_create_if_null(options, headers, 0);

    oss_init_live_channel_request(options, bucket, &config->name, HTTP_PUT,
                            &req, query_params, headers, &resp);

    // build body
    build_create_live_channel_body(options->pool, config, &body);
    oss_write_request_body_from_buffer(&body, req);

    s = oss_process_request(options, req, resp);
    oss_fill_read_response_header(resp, resp_headers);
    if (!aos_status_is_ok(s)) {
        return s;
    }

    // parse result
    res = oss_create_live_channel_parse_from_body(options->pool, &resp->body, 
        publish_url_list, play_url_list);
    if (res != AOSE_OK) {
        aos_xml_error_status_set(s, res);
    }

    return s;

}

aos_status_t *oss_put_live_channel_status(const oss_request_options_t *options,
                                          const aos_string_t *bucket,
                                          const aos_string_t *live_channel,
                                          const aos_string_t *live_channel_status,
                                          aos_table_t **resp_headers)
{
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    aos_table_t *query_params = NULL;
    aos_table_t *headers = NULL;

    //init params
    query_params = aos_table_create_if_null(options, query_params, 2);
    apr_table_add(query_params, OSS_LIVE_CHANNEL, "");
    apr_table_add(query_params, OSS_LIVE_CHANNEL_STATUS, live_channel_status->data);
    
    //init headers, forbid 'Expect' and 'Transfer-Encoding' of HTTP
    headers = aos_table_create_if_null(options, headers, 2);
	apr_table_set(headers, "Expect", "");
	apr_table_set(headers, "Transfer-Encoding", "");

    oss_init_live_channel_request(options, bucket, live_channel, HTTP_PUT,
                            &req, query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    oss_fill_read_response_header(resp, resp_headers);
    
    return s;
}

aos_status_t *oss_get_live_channel_info(const oss_request_options_t *options,
                                        const aos_string_t *bucket,
                                        const aos_string_t *live_channel,
                                        oss_live_channel_configuration_t *info,
                                        aos_table_t **resp_headers)
{
    int res = AOSE_OK;
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    aos_table_t *query_params = NULL;
    aos_table_t *headers = NULL;

    //init query_params
    query_params = aos_table_create_if_null(options, query_params, 1);
    apr_table_add(query_params, OSS_LIVE_CHANNEL, "");

    //init headers
    headers = aos_table_create_if_null(options, headers, 0);

    oss_init_live_channel_request(options, bucket, live_channel, HTTP_GET,
                            &req, query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    oss_fill_read_response_header(resp, resp_headers);
    if (!aos_status_is_ok(s)) {
        return s;
    }

    // parse result
    res = oss_live_channel_info_parse_from_body(options->pool, &resp->body, info);
    if (res != AOSE_OK) {
        aos_xml_error_status_set(s, res);
    }
    aos_str_set(&info->name, aos_pstrdup(options->pool, live_channel));

    return s;
}

aos_status_t *oss_get_live_channel_stat(const oss_request_options_t *options,
                                        const aos_string_t *bucket,
                                        const aos_string_t *live_channel,
                                        oss_live_channel_stat_t *stat,
                                        aos_table_t **resp_headers)
{
    int res = AOSE_OK;
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    aos_table_t *query_params = NULL;
    aos_table_t *headers = NULL;

    //init params
    query_params = aos_table_create_if_null(options, query_params, 2);
    apr_table_add(query_params, OSS_LIVE_CHANNEL, "");
    apr_table_add(query_params, OSS_COMP, OSS_LIVE_CHANNEL_STAT);

    //init headers
    headers = aos_table_create_if_null(options, headers, 0);

    oss_init_live_channel_request(options, bucket, live_channel, HTTP_GET,
                            &req, query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    oss_fill_read_response_header(resp, resp_headers);
    if (!aos_status_is_ok(s)) {
        return s;
    }

    // parse result
    res = oss_live_channel_stat_parse_from_body(options->pool, &resp->body, stat);
    if (res != AOSE_OK) {
        aos_xml_error_status_set(s, res);
    }

    return s;
}

aos_status_t *oss_delete_live_channel(const oss_request_options_t *options,
                                      const aos_string_t *bucket,
                                      const aos_string_t *live_channel,
                                      aos_table_t **resp_headers)
{
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    aos_table_t *query_params = NULL;
    aos_table_t *headers = NULL;

    //init params
    query_params = aos_table_create_if_null(options, query_params, 1);
    apr_table_add(query_params, OSS_LIVE_CHANNEL, "");

    //init headers
    headers = aos_table_create_if_null(options, headers, 0);

    oss_init_live_channel_request(options, bucket, live_channel, HTTP_DELETE,
                            &req, query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    oss_fill_read_response_header(resp, resp_headers);

    return s;
}

aos_status_t *oss_list_live_channel(const oss_request_options_t *options,
                                    const aos_string_t *bucket,
                                    oss_list_live_channel_params_t *params,
                                    aos_table_t **resp_headers)
{
    int res = AOSE_OK;
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    aos_table_t *query_params = NULL;
    aos_table_t *headers = NULL;

    //init params
    query_params = aos_table_create_if_null(options, query_params, 4);
    apr_table_add(query_params, OSS_LIVE_CHANNEL, "");
    apr_table_add(query_params, OSS_PREFIX, params->prefix.data);
    apr_table_add(query_params, OSS_MARKER, params->marker.data);
    aos_table_add_int(query_params, OSS_MAX_KEYS, params->max_keys);

    //init headers
    headers = aos_table_create_if_null(options, headers, 0);

    oss_init_bucket_request(options, bucket, HTTP_GET, &req,
                            query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    oss_fill_read_response_header(resp, resp_headers);
    if (!aos_status_is_ok(s)) {
        return s;
    }

    // parse result
    res = oss_list_live_channel_parse_from_body(options->pool, &resp->body,
        &params->live_channel_list, &params->next_marker, &params->truncated);
    if (res != AOSE_OK) {
        aos_xml_error_status_set(s, res);
    }

    return s;
}

aos_status_t *oss_get_live_channel_history(const oss_request_options_t *options,
                                           const aos_string_t *bucket,
                                           const aos_string_t *live_channel,
                                           aos_list_t *live_record_list,
                                           aos_table_t **resp_headers)
{
    int res = AOSE_OK;
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    aos_table_t *query_params = NULL;
    aos_table_t *headers = NULL;

    //init params
    query_params = aos_table_create_if_null(options, query_params, 2);
    apr_table_add(query_params, OSS_LIVE_CHANNEL, "");
    apr_table_add(query_params, OSS_COMP, OSS_LIVE_CHANNEL_HISTORY);

    //init headers
    headers = aos_table_create_if_null(options, headers, 0);

    oss_init_live_channel_request(options, bucket, live_channel, HTTP_GET,
                            &req, query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    oss_fill_read_response_header(resp, resp_headers);
    if (!aos_status_is_ok(s)) {
        return s;
    }

    // parse result
    res = oss_live_channel_history_parse_from_body(options->pool, &resp->body, live_record_list);
    if (res != AOSE_OK) {
        aos_xml_error_status_set(s, res);
    }

    return s;
}

aos_status_t *oss_gen_vod_play_list(const oss_request_options_t *options,
                                     const aos_string_t *bucket,
                                     const aos_string_t *live_channel,
                                     const aos_string_t *play_list_name,
                                     const int64_t start_time,
                                     const int64_t end_time,
                                     aos_table_t **resp_headers)
{
    aos_status_t *s = NULL;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    aos_table_t *query_params = NULL;
    aos_table_t *headers = NULL;
    char *resource = NULL;
    aos_string_t resource_str;

    //init params
    query_params = aos_table_create_if_null(options, query_params, 3);
    apr_table_add(query_params, OSS_LIVE_CHANNEL_VOD, "");
    apr_table_add(query_params, OSS_LIVE_CHANNEL_START_TIME,
        apr_psprintf(options->pool, "%" APR_INT64_T_FMT, start_time));
    apr_table_add(query_params, OSS_LIVE_CHANNEL_END_TIME,
        apr_psprintf(options->pool, "%" APR_INT64_T_FMT, end_time));

    //init headers
    headers = aos_table_create_if_null(options, headers, 1);
    apr_table_set(headers, OSS_CONTENT_TYPE, OSS_MULTIPART_CONTENT_TYPE);

    resource = apr_psprintf(options->pool, "%s/%s", live_channel->data, play_list_name->data);
    aos_str_set(&resource_str, resource);

    oss_init_live_channel_request(options, bucket, &resource_str, HTTP_POST,
                            &req, query_params, headers, &resp);

    s = oss_process_request(options, req, resp);
    oss_fill_read_response_header(resp, resp_headers);

    return s;
}

char *oss_gen_rtmp_signed_url(const oss_request_options_t *options,
                              const aos_string_t *bucket,
                              const aos_string_t *live_channel,
                              const aos_string_t *play_list_name,
                              const int64_t expires)
{
    aos_string_t signed_url;
    char *expires_str = NULL;
    aos_string_t expires_time;
    int res = AOSE_OK;
    aos_http_request_t *req = NULL;
    aos_table_t *params = NULL;

    expires_str = apr_psprintf(options->pool, "%" APR_INT64_T_FMT, expires);
    aos_str_set(&expires_time, expires_str);
    req = aos_http_request_create(options->pool);
    oss_get_rtmp_uri(options, bucket, live_channel, req);
    res = oss_get_rtmp_signed_url(options, req, &expires_time, play_list_name,
        params, &signed_url);
    if (res != AOSE_OK) {
        return NULL;
    }
    return signed_url.data;
}
