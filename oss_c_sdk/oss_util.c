#include "aos_string.h"
#include "aos_util.h"
#include "aos_log.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"

#ifndef WIN32
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#endif

static char *default_content_type = "application/octet-stream";

static oss_content_type_t file_type[] = {
    {"html", "text/html"},
    {"htm", "text/html"},
    {"shtml", "text/html"},
    {"css", "text/css"},
    {"xml", "text/xml"},
    {"gif", "image/gif"},
    {"jpeg", "image/jpeg"},
    {"jpg", "image/jpeg"},
    {"js", "application/x-javascript"},
    {"atom", "application/atom+xml"},
    {"rss", "application/rss+xml"},
    {"mml", "text/mathml"},
    {"txt", "text/plain"},
    {"jad", "text/vnd.sun.j2me.app-descriptor"},
    {"wml", "text/vnd.wap.wml"},
    {"htc", "text/x-component"},
    {"png", "image/png"},
    {"tif", "image/tiff"},
    {"tiff", "image/tiff"},
    {"wbmp", "image/vnd.wap.wbmp"},
    {"ico", "image/x-icon"},
    {"jng", "image/x-jng"},
    {"bmp", "image/x-ms-bmp"},
    {"svg", "image/svg+xml"},
    {"svgz", "image/svg+xml"},
    {"webp", "image/webp"},
    {"jar", "application/java-archive"},
    {"war", "application/java-archive"},
    {"ear", "application/java-archive"},
    {"hqx", "application/mac-binhex40"},
    {"doc ", "application/msword"},
    {"pdf", "application/pdf"},
    {"ps", "application/postscript"},
    {"eps", "application/postscript"},
    {"ai", "application/postscript"},
    {"rtf", "application/rtf"},
    {"xls", "application/vnd.ms-excel"},
    {"ppt", "application/vnd.ms-powerpoint"},
    {"wmlc", "application/vnd.wap.wmlc"},
    {"kml", "application/vnd.google-earth.kml+xml"},
    {"kmz", "application/vnd.google-earth.kmz"},
    {"7z", "application/x-7z-compressed"},
    {"cco", "application/x-cocoa"},
    {"jardiff", "application/x-java-archive-diff"},
    {"jnlp", "application/x-java-jnlp-file"},
    {"run", "application/x-makeself"},
    {"pl", "application/x-perl"},
    {"pm", "application/x-perl"},
    {"prc", "application/x-pilot"},
    {"pdb", "application/x-pilot"},
    {"rar", "application/x-rar-compressed"},
    {"rpm", "application/x-redhat-package-manager"},
    {"sea", "application/x-sea"},
    {"swf", "application/x-shockwave-flash"},
    {"sit", "application/x-stuffit"},
    {"tcl", "application/x-tcl"},
    {"tk", "application/x-tcl"},
    {"der", "application/x-x509-ca-cert"},
    {"pem", "application/x-x509-ca-cert"},
    {"crt", "application/x-x509-ca-cert"},
    {"xpi", "application/x-xpinstall"},
    {"xhtml", "application/xhtml+xml"},
    {"zip", "application/zip"},
    {"wgz", "application/x-nokia-widget"},
    {"bin", "application/octet-stream"},
    {"exe", "application/octet-stream"},
    {"dll", "application/octet-stream"},
    {"deb", "application/octet-stream"},
    {"dmg", "application/octet-stream"},
    {"eot", "application/octet-stream"},
    {"iso", "application/octet-stream"},
    {"img", "application/octet-stream"},
    {"msi", "application/octet-stream"},
    {"msp", "application/octet-stream"},
    {"msm", "application/octet-stream"},
    {"mid", "audio/midi"},
    {"midi", "audio/midi"},
    {"kar", "audio/midi"},
    {"mp3", "audio/mpeg"},
    {"ogg", "audio/ogg"},
    {"m4a", "audio/x-m4a"},
    {"ra", "audio/x-realaudio"},
    {"3gpp", "video/3gpp"},
    {"3gp", "video/3gpp"},
    {"mp4", "video/mp4"},
    {"mpeg", "video/mpeg"},
    {"mpg", "video/mpeg"},
    {"mov", "video/quicktime"},
    {"webm", "video/webm"},
    {"flv", "video/x-flv"},
    {"m4v", "video/x-m4v"},
    {"mng", "video/x-mng"},
    {"asx", "video/x-ms-asf"},
    {"asf", "video/x-ms-asf"},
    {"wmv", "video/x-ms-wmv"},
    {"avi", "video/x-msvideo"},
    {"ts", "video/MP2T"},
    {"m3u8", "application/x-mpegURL"},
    {"apk", "application/vnd.android.package-archive"},
    {NULL, NULL}
};

static int starts_with(const aos_string_t *str, const char *prefix) {
    uint32_t i;
    if(NULL != str && prefix && str->len > 0 && strlen(prefix)) {
        for(i = 0; str->data[i] != '\0' && prefix[i] != '\0'; i++) {
            if(prefix[i] != str->data[i]) return 0;
        }
        return 1;
    }
    return 0;
}

static void generate_proto(const oss_request_options_t *options, 
                           aos_http_request_t *req) 
{
    const char *proto;
    proto = starts_with(&options->config->endpoint, AOS_HTTP_PREFIX) ? 
            AOS_HTTP_PREFIX : "";
    proto = starts_with(&options->config->endpoint, AOS_HTTPS_PREFIX) ? 
            AOS_HTTPS_PREFIX : proto;
    req->proto = apr_psprintf(options->pool, "%.*s", (int)strlen(proto), proto);
}

static void generate_rtmp_proto(const oss_request_options_t *options,
                           aos_http_request_t *req)
{
    const char *proto = AOS_RTMP_PREFIX;
    req->proto = apr_psprintf(options->pool, "%.*s", (int)strlen(proto), proto);
}

int is_valid_ip(const char *str)
{
    char ip[24];
    for(int i = 0;i < strlen(str)&& i <24;i++){
        if (str[i] != ':'){
            ip[i]=str[i];
        }
        else
        {
            ip[i]=0;
            break;
        }
    }

    if (INADDR_NONE == inet_addr(ip) || INADDR_ANY == inet_addr(ip)) {
        return 0;
    }
    return 1;
}

oss_config_t *oss_config_create(aos_pool_t *p)
{
    return (oss_config_t *)aos_pcalloc(p, sizeof(oss_config_t));
}

void oss_config_resolve(aos_pool_t *pool, oss_config_t *config, aos_http_controller_t *ctl)
{
    if(!aos_is_null_string(&config->proxy_host)) {
        // proxy host:port
        if (config->proxy_port == 0) {
            ctl->options->proxy_host = apr_psprintf(pool, "%.*s", config->proxy_host.len, config->proxy_host.data);
        } else {
            ctl->options->proxy_host = apr_psprintf(pool, "%.*s:%d", config->proxy_host.len, config->proxy_host.data, 
                config->proxy_port);
        }
        // authorize user:passwd
        if (!aos_is_null_string(&config->proxy_user) && !aos_is_null_string(&config->proxy_passwd)) {
            ctl->options->proxy_auth = apr_psprintf(pool, "%.*s:%.*s", config->proxy_user.len, 
                config->proxy_user.data, config->proxy_passwd.len, config->proxy_passwd.data);
        }
    }
}

oss_request_options_t *oss_request_options_create(aos_pool_t *p)
{
    int s;
    oss_request_options_t *options;

    if(p == NULL) {
        if ((s = aos_pool_create(&p, NULL)) != APR_SUCCESS) {
            aos_fatal_log("aos_pool_create failure.");
            return NULL;
        }
    }

    options = (oss_request_options_t *)aos_pcalloc(p, sizeof(oss_request_options_t));
    options->pool = p;

    return options;
}

void oss_get_object_uri(const oss_request_options_t *options,
                        const aos_string_t *bucket,
                        const aos_string_t *object,
                        aos_http_request_t *req)
{
    int32_t proto_len;
    const char *raw_endpoint_str;
    aos_string_t raw_endpoint;

    generate_proto(options, req);

    proto_len = strlen(req->proto);

    req->resource = apr_psprintf(options->pool, "%.*s/%.*s", 
                                 bucket->len, bucket->data, 
                                 object->len, object->data);

    raw_endpoint_str = aos_pstrdup(options->pool, 
            &options->config->endpoint) + proto_len;
    raw_endpoint.len = options->config->endpoint.len - proto_len;
    raw_endpoint.data = options->config->endpoint.data + proto_len;

    if (options->config->is_cname) {
        req->host = apr_psprintf(options->pool, "%.*s",
                raw_endpoint.len, raw_endpoint.data);
        req->uri = object->data;
    } else if (is_valid_ip(raw_endpoint_str)) {
        req->host = apr_psprintf(options->pool, "%.*s",
                raw_endpoint.len, raw_endpoint.data);
        req->uri = apr_psprintf(options->pool, "%.*s/%.*s",
                                bucket->len, bucket->data, 
                                object->len, object->data);
    } else {
        req->host = apr_psprintf(options->pool, "%.*s.%.*s",
                bucket->len, bucket->data, 
                raw_endpoint.len, raw_endpoint.data);
        req->uri = object->data;
    }

}

void oss_get_bucket_uri(const oss_request_options_t *options, 
                        const aos_string_t *bucket,
                        aos_http_request_t *req)
{
    int32_t proto_len;
    const char *raw_endpoint_str;
    aos_string_t raw_endpoint;

    generate_proto(options, req);

    proto_len = strlen(req->proto);
    raw_endpoint_str = aos_pstrdup(options->pool, 
            &options->config->endpoint) + proto_len;
    raw_endpoint.len = options->config->endpoint.len - proto_len;
    raw_endpoint.data = options->config->endpoint.data + proto_len;

    if (is_valid_ip(raw_endpoint_str)) {
        req->resource = apr_psprintf(options->pool, "%.*s", 
                bucket->len, bucket->data);
    } else {
        req->resource = apr_psprintf(options->pool, "%.*s/", 
                bucket->len, bucket->data);
    }
    
    if (options->config->is_cname || 
        is_valid_ip(raw_endpoint_str))
    {
        req->host = apr_psprintf(options->pool, "%.*s", 
                raw_endpoint.len, raw_endpoint.data);
        req->uri = apr_psprintf(options->pool, "%.*s", bucket->len, 
                                bucket->data);
    } else {
        req->host = apr_psprintf(options->pool, "%.*s.%.*s", 
                bucket->len, bucket->data, 
                raw_endpoint.len, raw_endpoint.data);
        req->uri = apr_psprintf(options->pool, "%s", "");
    }
}

void oss_get_rtmp_uri(const oss_request_options_t *options,
                      const aos_string_t *bucket,
                      const aos_string_t *live_channel_id,
                      aos_http_request_t *req)
{
    int32_t proto_len = 0;
    const char *raw_endpoint_str = NULL;
    aos_string_t raw_endpoint;

    generate_rtmp_proto(options, req);

    proto_len = strlen(req->proto);

    req->resource = apr_psprintf(options->pool, "%.*s/%.*s", bucket->len, bucket->data,
        live_channel_id->len, live_channel_id->data);

    raw_endpoint_str = aos_pstrdup(options->pool,
            &options->config->endpoint) + proto_len;
    raw_endpoint.len = options->config->endpoint.len - proto_len;
    raw_endpoint.data = options->config->endpoint.data + proto_len;

    if (options->config->is_cname) {
        req->host = apr_psprintf(options->pool, "%.*s",
                raw_endpoint.len, raw_endpoint.data);
        req->uri = apr_psprintf(options->pool, "live/%.*s",
            live_channel_id->len, live_channel_id->data);
    } else if (is_valid_ip(raw_endpoint_str)) {
        req->host = apr_psprintf(options->pool, "%.*s",
                raw_endpoint.len, raw_endpoint.data);
        req->uri = apr_psprintf(options->pool, "%.*s/live/%.*s",
                                bucket->len, bucket->data,
                                live_channel_id->len, live_channel_id->data);
    } else {
        req->host = apr_psprintf(options->pool, "%.*s.%.*s",
                bucket->len, bucket->data,
                raw_endpoint.len, raw_endpoint.data);
        req->uri = apr_psprintf(options->pool, "live/%.*s",
            live_channel_id->len, live_channel_id->data);
    }
}

void oss_write_request_body_from_buffer(aos_list_t *buffer, 
                                        aos_http_request_t *req)
{
    aos_list_movelist(buffer, &req->body);
    req->body_len = aos_buf_list_len(&req->body);
}

int oss_write_request_body_from_file(aos_pool_t *p, 
                                     const aos_string_t *filename, 
                                     aos_http_request_t *req)
{
    int res = AOSE_OK;
    aos_file_buf_t *fb = aos_create_file_buf(p);
    res = aos_open_file_for_all_read(p, filename->data, fb);
    if (res != AOSE_OK) {
        aos_error_log("Open read file fail, filename:%s\n", filename->data);
        return res;
    }

    req->body_len = fb->file_last;
    req->file_path = filename->data;
    req->file_buf = fb;
    req->type = BODY_IN_FILE;
    req->read_body = aos_read_http_body_file;

    return res;
}

int oss_write_request_body_from_upload_file(aos_pool_t *p, 
                                            oss_upload_file_t *upload_file, 
                                            aos_http_request_t *req)
{
    int res = AOSE_OK;
    aos_file_buf_t *fb = aos_create_file_buf(p);
    res = aos_open_file_for_range_read(p, upload_file->filename.data, 
            upload_file->file_pos, upload_file->file_last, fb);
    if (res != AOSE_OK) {
        aos_error_log("Open read file fail, filename:%s\n", 
                      upload_file->filename.data);
        return res;
    }

    req->body_len = fb->file_last - fb->file_pos;
    req->file_path = upload_file->filename.data;
    req->file_buf = fb;
    req->type = BODY_IN_FILE;
    req->read_body = aos_read_http_body_file;

    return res;
}

void oss_fill_read_response_body(aos_http_response_t *resp, 
                                 aos_list_t *buffer)
{
    if (NULL != buffer) {
        aos_list_movelist(&resp->body, buffer);
    }
}

int oss_init_read_response_body_to_file(aos_pool_t *p, 
                                        const aos_string_t *filename, 
                                        aos_http_response_t *resp)
{
    int res = AOSE_OK;
    aos_file_buf_t *fb = aos_create_file_buf(p);
    res = aos_open_file_for_write(p, filename->data, fb);
    if (res != AOSE_OK) {
        aos_error_log("Open write file fail, filename:%s\n", filename->data);
        return res;
    }
    resp->file_path = filename->data;
    resp->file_buf = fb;
    resp->write_body = aos_write_http_body_file;
    resp->type = BODY_IN_FILE;

    return res;
}

int oss_init_read_response_body_to_fb(aos_file_buf_t *fb,
                                      const aos_string_t *filename,
                                      aos_http_response_t *resp)
{
    int res = AOSE_OK;

    resp->file_path = filename->data;
    resp->file_buf = fb;
    resp->write_body = aos_write_http_body_file;
    resp->type = BODY_IN_FILE;

    return res;
}

void oss_fill_read_response_header(aos_http_response_t *resp, 
                                   aos_table_t **headers)
{
    if (NULL != headers && NULL != resp) {        
        *headers = resp->headers;
    }
}

void *oss_create_api_result_content(aos_pool_t *p, size_t size)
{
    void *result_content = aos_palloc(p, size);
    if (NULL == result_content) {
        return NULL;
    }
    
    aos_list_init((aos_list_t *)result_content);

    return result_content;
}

oss_list_object_content_t *oss_create_list_object_content(aos_pool_t *p)
{
    return (oss_list_object_content_t *)oss_create_api_result_content(
            p, sizeof(oss_list_object_content_t));
}

oss_list_object_common_prefix_t *oss_create_list_object_common_prefix(aos_pool_t *p)
{
    return (oss_list_object_common_prefix_t *)oss_create_api_result_content(
            p, sizeof(oss_list_object_common_prefix_t));
}

oss_list_multipart_upload_content_t *oss_create_list_multipart_upload_content(aos_pool_t *p)
{
    return (oss_list_multipart_upload_content_t*)oss_create_api_result_content(
            p, sizeof(oss_list_multipart_upload_content_t));
}

oss_list_part_content_t *oss_create_list_part_content(aos_pool_t *p)
{
    oss_list_part_content_t *list_part_content = NULL;
    list_part_content = (oss_list_part_content_t*)oss_create_api_result_content(p,
        sizeof(oss_list_part_content_t));

    return list_part_content;
}

oss_complete_part_content_t *oss_create_complete_part_content(aos_pool_t *p)
{
    oss_complete_part_content_t *complete_part_content = NULL;
    complete_part_content = (oss_complete_part_content_t*)oss_create_api_result_content(
            p, sizeof(oss_complete_part_content_t));

    return complete_part_content;
}

oss_list_object_params_t *oss_create_list_object_params(aos_pool_t *p)
{
    oss_list_object_params_t * params;
    params = (oss_list_object_params_t *)aos_pcalloc(
            p, sizeof(oss_list_object_params_t));
    aos_list_init(&params->object_list);
    aos_list_init(&params->common_prefix_list);
    aos_str_set(&params->prefix, "");
    aos_str_set(&params->marker, "");
    aos_str_set(&params->delimiter, "");
    params->truncated = 1;
    params->max_ret = OSS_PER_RET_NUM;
    return params;
}

oss_list_upload_part_params_t *oss_create_list_upload_part_params(aos_pool_t *p)
{
    oss_list_upload_part_params_t *params;
    params = (oss_list_upload_part_params_t *)aos_pcalloc(
            p, sizeof(oss_list_upload_part_params_t));
    aos_list_init(&params->part_list);
    aos_str_set(&params->part_number_marker, "");
    params->max_ret = OSS_PER_RET_NUM;
    params->truncated = 1;
    return params;
}

oss_list_multipart_upload_params_t *oss_create_list_multipart_upload_params(aos_pool_t *p)
{
    oss_list_multipart_upload_params_t *params;
    params = (oss_list_multipart_upload_params_t *)aos_pcalloc(
            p, sizeof(oss_list_multipart_upload_params_t));
    aos_list_init(&params->upload_list);
    aos_str_set(&params->prefix, "");
    aos_str_set(&params->key_marker, "");
    aos_str_set(&params->upload_id_marker, "");
    aos_str_set(&params->delimiter, "");
    params->truncated = 1;
    params->max_ret = OSS_PER_RET_NUM;
    return params;
}

oss_upload_part_copy_params_t *oss_create_upload_part_copy_params(aos_pool_t *p)
{
    return (oss_upload_part_copy_params_t *)aos_pcalloc(
            p, sizeof(oss_upload_part_copy_params_t));
}

oss_lifecycle_rule_content_t *oss_create_lifecycle_rule_content(aos_pool_t *p)
{
    oss_lifecycle_rule_content_t *rule;
    rule = (oss_lifecycle_rule_content_t *)aos_pcalloc(
            p, sizeof(oss_lifecycle_rule_content_t));
    aos_str_set(&rule->id, "");
    aos_str_set(&rule->prefix, "");
    aos_str_set(&rule->status, "");
    aos_str_set(&rule->date, "");
    rule->days = INT_MAX;
    return rule;
}

oss_upload_file_t *oss_create_upload_file(aos_pool_t *p)
{
    return (oss_upload_file_t *)aos_pcalloc(p, sizeof(oss_upload_file_t));
}

oss_object_key_t *oss_create_oss_object_key(aos_pool_t *p)
{
    return (oss_object_key_t *)aos_pcalloc(p, sizeof(oss_object_key_t));
}

oss_live_channel_publish_url_t *oss_create_live_channel_publish_url(aos_pool_t *p)
{
    return (oss_live_channel_publish_url_t *)aos_pcalloc(p, sizeof(oss_live_channel_publish_url_t));
}

oss_live_channel_play_url_t *oss_create_live_channel_play_url(aos_pool_t *p)
{
    return (oss_live_channel_play_url_t *)aos_pcalloc(p, sizeof(oss_live_channel_play_url_t));
}

oss_live_channel_content_t *oss_create_list_live_channel_content(aos_pool_t *p)
{
    oss_live_channel_content_t *list_live_channel_content = NULL;
    list_live_channel_content = (oss_live_channel_content_t*)oss_create_api_result_content(p,
        sizeof(oss_live_channel_content_t));
    aos_list_init(&list_live_channel_content->publish_url_list);
    aos_list_init(&list_live_channel_content->play_url_list);
    return list_live_channel_content;
}

oss_live_record_content_t *oss_create_live_record_content(aos_pool_t *p)
{
    oss_live_record_content_t *live_record_content = NULL;
    live_record_content = (oss_live_record_content_t*)oss_create_api_result_content(p,
        sizeof(oss_live_record_content_t));
    return live_record_content;
}

oss_live_channel_configuration_t *oss_create_live_channel_configuration_content(aos_pool_t *p)
{
    oss_live_channel_configuration_t *config;
    config = (oss_live_channel_configuration_t *)aos_pcalloc(
            p, sizeof(oss_live_channel_configuration_t));

    aos_str_set(&config->name, "");
    aos_str_set(&config->description, "");
    aos_str_set(&config->status, LIVE_CHANNEL_STATUS_ENABLED);
    aos_str_set(&config->target.type, LIVE_CHANNEL_DEFAULT_TYPE);
    aos_str_set(&config->target.play_list_name, LIVE_CHANNEL_DEFAULT_PLAYLIST);
    config->target.frag_duration = LIVE_CHANNEL_DEFAULT_FRAG_DURATION;
    config->target.frag_count = LIVE_CHANNEL_DEFAULT_FRAG_COUNT;

    return config;
}

oss_checkpoint_t *oss_create_checkpoint_content(aos_pool_t *p) 
{
    oss_checkpoint_t *cp;
    cp = (oss_checkpoint_t *)aos_pcalloc(p, sizeof(oss_checkpoint_t));
    cp->parts = (oss_checkpoint_part_t *)aos_pcalloc(p, sizeof(oss_checkpoint_part_t) * OSS_MAX_PART_NUM);
    aos_str_set(&cp->md5, "");
    aos_str_set(&cp->file_path, "");
    aos_str_set(&cp->file_md5, "");
    aos_str_set(&cp->object_name, "");
    aos_str_set(&cp->object_last_modified, "");
    aos_str_set(&cp->object_etag, "");
    aos_str_set(&cp->upload_id, "");
    return cp;
}

oss_resumable_clt_params_t *oss_create_resumable_clt_params_content(aos_pool_t *p, int64_t part_size, int32_t thread_num,
                                                                    int enable_checkpoint, const char *checkpoint_path)
{
    oss_resumable_clt_params_t *clt;
    clt = (oss_resumable_clt_params_t *)aos_pcalloc(p, sizeof(oss_resumable_clt_params_t));
    clt->part_size = part_size;
    clt->thread_num = thread_num;
    clt->enable_checkpoint = enable_checkpoint;
    if (enable_checkpoint && NULL != checkpoint_path) {
        aos_str_set(&clt->checkpoint_path, checkpoint_path);
    }
    return clt;
}

oss_list_live_channel_params_t *oss_create_list_live_channel_params(aos_pool_t *p)
{
    oss_list_live_channel_params_t *params;
    params = (oss_list_live_channel_params_t *)aos_pcalloc(
            p, sizeof(oss_list_live_channel_params_t));
    aos_list_init(&params->live_channel_list);
    aos_str_set(&params->prefix, "");
    aos_str_set(&params->marker, "");
    params->truncated = 1;
    params->max_keys = OSS_PER_RET_NUM;
    return params;
}

const char *get_oss_acl_str(oss_acl_e oss_acl)
{
    switch (oss_acl) {
        case OSS_ACL_PRIVATE:
            return  "private";
        case OSS_ACL_PUBLIC_READ:
            return "public-read";
        case OSS_ACL_PUBLIC_READ_WRITE:
            return "public-read-write";
        default:
            return NULL;
    }
}

void oss_init_request(const oss_request_options_t *options, 
                      http_method_e method,
                      aos_http_request_t **req, 
                      aos_table_t *params, 
                      aos_table_t *headers, 
                      aos_http_response_t **resp)
{
    *req = aos_http_request_create(options->pool);
    *resp = aos_http_response_create(options->pool);
    (*req)->method = method;
    init_sts_token_header();
    (*req)->headers = headers;
    (*req)->query_params = params;
}

void oss_init_bucket_request(const oss_request_options_t *options, 
                             const aos_string_t *bucket,
                             http_method_e method, 
                             aos_http_request_t **req, 
                             aos_table_t *params, 
                             aos_table_t *headers,
                             aos_http_response_t **resp)
{
    oss_init_request(options, method, req, params, headers, resp);
    oss_get_bucket_uri(options, bucket, *req);
}

void oss_init_object_request(const oss_request_options_t *options, 
                             const aos_string_t *bucket,
                             const aos_string_t *object, 
                             http_method_e method, 
                             aos_http_request_t **req, 
                             aos_table_t *params, 
                             aos_table_t *headers,
                             oss_progress_callback cb,
                             uint64_t init_crc,
                             aos_http_response_t **resp)
{
    oss_init_request(options, method, req, params, headers, resp);
    if (HTTP_GET == method) {
        (*resp)->progress_callback = cb;
    } else if (HTTP_PUT == method || HTTP_POST == method) {
        (*req)->progress_callback = cb;
        (*req)->crc64 = init_crc;
    }

    oss_get_object_uri(options, bucket, object, *req);
}

void oss_init_live_channel_request(const oss_request_options_t *options, 
                                   const aos_string_t *bucket,
                                   const aos_string_t *live_channel,
                                   http_method_e method,
                                   aos_http_request_t **req,
                                   aos_table_t *params,
                                   aos_table_t *headers,
                                   aos_http_response_t **resp)
{
    oss_init_request(options, method, req, params, headers, resp);
    oss_get_object_uri(options, bucket, live_channel, *req);
}

void oss_init_signed_url_request(const oss_request_options_t *options, 
                                 const aos_string_t *signed_url,
                                 http_method_e method, 
                                 aos_http_request_t **req, 
                                 aos_table_t *params, 
                                 aos_table_t *headers, 
                                 aos_http_response_t **resp)
{
    *req = aos_http_request_create(options->pool);
    *resp = aos_http_response_create(options->pool);
    (*req)->method = method;
    (*req)->headers = headers;
    (*req)->query_params = params;
    (*req)->signed_url = signed_url->data;
}

aos_status_t *oss_send_request(aos_http_controller_t *ctl, 
                               aos_http_request_t *req,
                               aos_http_response_t *resp)
{
    aos_status_t *s;
    const char *reason;
    int res = AOSE_OK;

    s = aos_status_create(ctl->pool);
    res = aos_http_send_request(ctl, req, resp);

    if (res != AOSE_OK) {
        reason = aos_http_controller_get_reason(ctl);
        aos_status_set(s, res, AOS_HTTP_IO_ERROR_CODE, reason);
    } else if (!aos_http_is_ok(resp->status)) {
        s = aos_status_parse_from_body(ctl->pool, &resp->body, resp->status, s);
    } else {
        s->code = resp->status;
    }

    s->req_id = (char*)(apr_table_get(resp->headers, "x-oss-request-id"));
    if (s->req_id == NULL) {
        s->req_id = (char*)(apr_table_get(resp->headers, "x-img-request-id"));
        if (s->req_id == NULL) {
            s->req_id = "";
        }
    }

    return s;
}

aos_status_t *oss_process_request(const oss_request_options_t *options,
                                  aos_http_request_t *req, 
                                  aos_http_response_t *resp)
{
    int res = AOSE_OK;
    aos_status_t *s;

    s = aos_status_create(options->pool);
    res = oss_sign_request(req, options->config);
    if (res != AOSE_OK) {
        aos_status_set(s, res, AOS_CLIENT_ERROR_CODE, NULL);
        return s;
    }

    return oss_send_request(options->ctl, req, resp);
}

aos_status_t *oss_process_signed_request(const oss_request_options_t *options,
                                         aos_http_request_t *req, 
                                         aos_http_response_t *resp)
{
    return oss_send_request(options->ctl, req, resp);
}

void oss_get_part_size(int64_t filesize, int64_t *part_size)
{
    if (filesize > (*part_size) * OSS_MAX_PART_NUM) {
        *part_size = (filesize + OSS_MAX_PART_NUM - 
                      filesize % OSS_MAX_PART_NUM) / OSS_MAX_PART_NUM;

        aos_warn_log("Part number larger than max limit, "
                     "part size Changed to:%" APR_INT64_T_FMT "\n",
                     *part_size);
    } 
}

int part_sort_cmp(const void *a, const void *b)
{
    return (((oss_upload_part_t*)a)->part_num -
            ((oss_upload_part_t*)b)->part_num > 0 ? 1 : -1);
}


void oss_headers_add_range(apr_pool_t *pool, apr_table_t *headers, int64_t offset, int64_t size)
{
    char *range;
    range = apr_psprintf(pool, "bytes=%" APR_INT64_T_FMT "-%" APR_INT64_T_FMT, 
            offset, offset + size - 1);
    apr_table_set(headers, "Range", range);
}


char *get_content_type_by_suffix(const char *suffix)
{
    oss_content_type_t *content_type;

    for (content_type = file_type; content_type->suffix; ++content_type) {
        if (strcasecmp(content_type->suffix, suffix) == 0)
        {
            return content_type->type;
        }
    }
    return default_content_type;
}

char *get_content_type(const char *name)
{
    char *begin;
    char *content_type = NULL;
    begin = strrchr(name, '.');
    if (begin) {
        content_type = get_content_type_by_suffix(begin + 1);
    }
    return content_type;
}

void set_content_type(const char* file_name,
                      const char* key,
                      aos_table_t *headers)
{
    char *user_content_type = NULL;
    char *content_type = NULL;
    const char *mime_key = NULL;

    mime_key = file_name == NULL ? key : file_name;

    user_content_type = (char*)apr_table_get(headers, OSS_CONTENT_TYPE);
    if (NULL == user_content_type && mime_key != NULL) {
        content_type = get_content_type(mime_key);
        if (content_type) {
            apr_table_set(headers, OSS_CONTENT_TYPE, content_type);
        } else {
            apr_table_set(headers, OSS_CONTENT_TYPE, default_content_type);
        }
    }
}

aos_table_t* aos_table_create_if_null(const oss_request_options_t *options, 
                                      aos_table_t *table, 
                                      int table_size) 
{
    if (table == NULL) {
        table = aos_table_make(options->pool, table_size);
    }
    return table;
}

int is_enable_crc(const oss_request_options_t *options) 
{
    return options->ctl->options->enable_crc;
}

int has_crc_in_response(const aos_http_response_t *resp) 
{
    if (NULL != apr_table_get(resp->headers, OSS_HASH_CRC64_ECMA)) {
        return AOS_TRUE;
    }

    return AOS_FALSE;
}



int has_range_or_process_in_request(const aos_http_request_t *req) 
{
    if (NULL != apr_table_get(req->headers, "Range") || 
        NULL != apr_table_get(req->query_params, OSS_PROCESS)) {
        return AOS_TRUE;
    }

    return AOS_FALSE;
}

static int check_crc(uint64_t crc, const apr_table_t *headers) 
{
    char * srv_crc = (char*)(apr_table_get(headers, OSS_HASH_CRC64_ECMA));
    if (NULL != srv_crc && crc != aos_atoui64(srv_crc)) {
        return AOSE_CRC_INCONSISTENT_ERROR;
    }
    return AOSE_OK;
}

int oss_check_crc_consistent(uint64_t crc, const apr_table_t *resp_headers, aos_status_t *s) 
{
    int res = check_crc(crc, resp_headers);
    if (res != AOSE_OK) {
        aos_inconsistent_error_status_set(s, res);
    }
    return res;
}

int oss_get_temporary_file_name(aos_pool_t *p, const aos_string_t *filename, aos_string_t *temp_file_name)
{
    int len = filename->len + 1;
    char *temp_file_name_ptr = NULL;

    len += strlen(AOS_TEMP_FILE_SUFFIX);
    temp_file_name_ptr = aos_pcalloc(p, len);

    apr_snprintf(temp_file_name_ptr, len, "%.*s%s", filename->len, filename->data, AOS_TEMP_FILE_SUFFIX);
    aos_str_set(temp_file_name, temp_file_name_ptr);

    return len;
}

int oss_temp_file_rename(aos_status_t *s, const char *from_path, const char *to_path, apr_pool_t *pool)
{
    int res = -1;

    if (s != NULL) {
        if (aos_status_is_ok(s)) {
            res = apr_file_rename(from_path, to_path, pool);
        } else {
            res = apr_file_remove(from_path, pool);
        }
    }

    return res;
}

char* delimiter_to_string(char c, char* str)
{
    switch(c)
    {
        case '\t':
            *str = '\\';
            *(str+1) = 't';
            *(str+2) = 0;
            break;
        case '\v':
            *str = '\\';
            *(str+1) = 'v';
            *(str+2) = 0;
            break;
        default:
            *str = c;
            *(str+1) = 0;
            break;
    }

    return str;
}

char* newline_to_string(const aos_string_t *newline, char* newline_str)
{
    int j = 0;
    for(int i=0; i < newline->len; i++, j++){
        if (newline->data[i] == '\n'){
            newline_str[j] = '\\';
            newline_str[j+1] = 'n';
            j++;
        }
        else if (newline->data[i] == '\r'){
            newline_str[j] = '\\';
            newline_str[j+1] = 'r';
            j++;
        }
        else{
            newline_str[j] = newline->data[i];
        }
    }

    newline_str[j] = '\0';
    return newline_str;
}

char* file_header_to_string(const csv_header_info header, char* file_header_str)
{
    if (header == CSV_HEADER_IGNORE)
    {
        strcpy(file_header_str, "Ignore");
    }
    else if (header == CSV_HEADER_USE)
    {
        strcpy(file_header_str, "Use");
    }
    else if(header == CSV_HEADER_NONE)
    {
        strcpy(file_header_str, "None");
    }

    return file_header_str;
}

char* range_to_string(int start, int end, char* range_str)
{
    if (start >= 0 && end >= 0){
        sprintf(range_str, "%d-%d", start, end);
    }
    else if (start >= 0){
        sprintf(range_str, "%d-", start);
    }
    else if (end >= 0){
        sprintf(range_str, "-%d", end);
    }
    else{
        return NULL;
    }

    return range_str;
}