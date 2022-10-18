#include "oss_auth.h"
#include "aos_log.h"
#include "oss_util.h"

static const char *g_s_oss_sub_resource_list[] = {
    "acl",
    "location",
    "bucketInfo",
    "stat",
    "referer",
    "cors",
    "website",
    "restore",
    "logging",
    "symlink",
    "qos",
    "uploadId",
    "uploads",
    "partNumber",
    "response-content-type",
    "response-content-language",
    "response-expires",
    "response-cache-control",
    "response-content-disposition",
    "response-content-encoding",
    "append",
    "position",
    "lifecycle",
    "delete",
    "live",
    "status",
    "comp",
    "vod",
    "startTime",
    "endTime",
    "x-oss-process",
    "security-token",
    "objectMeta",
    "tagging",
    "x-oss-sign-origin-only",
    NULL,
};

static int is_oss_sub_resource(const char *str);
static int is_oss_canonicalized_header(const char *str);
static int oss_get_canonicalized_headers(aos_pool_t *p, 
        const aos_table_t *headers, aos_buf_t *signbuf);
static int oss_get_canonicalized_resource(aos_pool_t *p, 
        const aos_table_t *params, aos_buf_t *signbuf);
static int oss_get_canonicalized_params(aos_pool_t *p,
    const aos_table_t *params, aos_buf_t *signbuf);
static int oss_sign_request_v4(aos_http_request_t *req, const oss_config_t *config);

static int is_oss_sub_resource(const char *str)
{
    int i = 0;
    for ( ; g_s_oss_sub_resource_list[i]; i++) {
        if (apr_strnatcmp(g_s_oss_sub_resource_list[i], str) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_oss_canonicalized_header(const char *str)
{
    size_t len = strlen(OSS_CANNONICALIZED_HEADER_PREFIX);
    return strncasecmp(str, OSS_CANNONICALIZED_HEADER_PREFIX, len) == 0;
}

static int oss_get_canonicalized_headers(aos_pool_t *p, 
                                         const aos_table_t *headers, 
                                         aos_buf_t *signbuf)
{
    int pos;
    int meta_count = 0;
    int i;
    int len;
    const aos_array_header_t *tarr;
    const aos_table_entry_t *telts;
    char **meta_headers;
    const char *value;
    aos_string_t tmp_str;
    char *tmpbuf = (char*)malloc(AOS_MAX_HEADER_LEN + 1);
    if (NULL == tmpbuf) {
        aos_error_log("malloc %d memory failed.", AOS_MAX_HEADER_LEN + 1);
        return AOSE_OVER_MEMORY;
    }

    if (apr_is_empty_table(headers)) {
        free(tmpbuf);
        return AOSE_OK;
    }

    // sort user meta header
    tarr = aos_table_elts(headers);
    telts = (aos_table_entry_t*)tarr->elts;
    meta_headers = aos_pcalloc(p, tarr->nelts * sizeof(char*));
    for (pos = 0; pos < tarr->nelts; ++pos) {
        if (is_oss_canonicalized_header(telts[pos].key)) {
            aos_string_t key = aos_string(telts[pos].key);
            aos_string_tolower(&key);
            meta_headers[meta_count++] = key.data;
        }
    }
    if (meta_count == 0) {
        free(tmpbuf);
        return AOSE_OK;
    }
    aos_gnome_sort((const char **)meta_headers, meta_count);

    // sign string
    for (i = 0; i < meta_count; ++i) {
        value = apr_table_get(headers, meta_headers[i]);
        aos_str_set(&tmp_str, value);
        aos_strip_space(&tmp_str);
        len = apr_snprintf(tmpbuf, AOS_MAX_HEADER_LEN + 1, "%s:%.*s", 
                           meta_headers[i], tmp_str.len, tmp_str.data);
        if (len > AOS_MAX_HEADER_LEN) {
            free(tmpbuf);
            aos_error_log("user meta header too many, %d > %d.", 
                          len, AOS_MAX_HEADER_LEN);
            return AOSE_INVALID_ARGUMENT;
        }
        tmp_str.data = tmpbuf;
        tmp_str.len = len;
        aos_buf_append_string(p, signbuf, tmpbuf, len);
        aos_buf_append_string(p, signbuf, "\n", sizeof("\n")-1);
    }

    free(tmpbuf);
    return AOSE_OK;
}

static int oss_get_canonicalized_resource(aos_pool_t *p, 
                                          const aos_table_t *params, 
                                          aos_buf_t *signbuf)
{
    int pos;
    int subres_count = 0;
    int i;
    int len;
    char sep;
    const char *value;
    char tmpbuf[AOS_MAX_QUERY_ARG_LEN+1];
    char **subres_headers;
    const aos_array_header_t *tarr;
    const aos_table_entry_t *telts;

    if (apr_is_empty_table(params)) {
        return AOSE_OK;
    }

    // sort sub resource param
    tarr = aos_table_elts(params);
    telts = (aos_table_entry_t*)tarr->elts;
    subres_headers = aos_pcalloc(p, tarr->nelts * sizeof(char*));
    for (pos = 0; pos < tarr->nelts; ++pos) {
        if (is_oss_sub_resource(telts[pos].key)) {
            subres_headers[subres_count++] = telts[pos].key;
        }
    }
    if (subres_count == 0) {
        return AOSE_OK;
    }
    aos_gnome_sort((const char **)subres_headers, subres_count);

    // sign string
    sep = '?';
    for (i = 0; i < subres_count; ++i) {
        value = apr_table_get(params, subres_headers[i]);
        if (value != NULL && *value != '\0') {
            len = apr_snprintf(tmpbuf, sizeof(tmpbuf), "%c%s=%s", 
                    sep, subres_headers[i], value);
        } else {
            len = apr_snprintf(tmpbuf, sizeof(tmpbuf), "%c%s", 
                    sep, subres_headers[i]);
        }
        if (len >= AOS_MAX_QUERY_ARG_LEN) {
            aos_error_log("http query params too long, %s.", tmpbuf);
            return AOSE_INVALID_ARGUMENT;
        }
        aos_buf_append_string(p, signbuf, tmpbuf, len);
        sep = '&';
    }

    return AOSE_OK;
}    

int oss_get_string_to_sign(aos_pool_t *p, 
                           http_method_e method, 
                           const aos_string_t *canon_res,
                           const aos_table_t *headers, 
                           const aos_table_t *params, 
                           aos_string_t *signstr)
{
    int res;
    aos_buf_t *signbuf;
    const char *value;
    aos_str_null(signstr);

    signbuf = aos_create_buf(p, 1024);

#define signbuf_append_from_headers(KEY) do {                            \
        if ((value = apr_table_get(headers, KEY)) != NULL) {            \
            aos_buf_append_string(p, signbuf, value, strlen(value));    \
        }                                                               \
        aos_buf_append_string(p, signbuf, "\n", sizeof("\n")-1);        \
    } while (0)

#define signbuf_append(VALUE, LEN) do {                                 \
        aos_buf_append_string(p, signbuf, VALUE, LEN);                  \
        aos_buf_append_string(p, signbuf, "\n", sizeof("\n")-1);        \
    } while (0)
    
    value = aos_http_method_to_string(method);
    signbuf_append(value, strlen(value));    

    signbuf_append_from_headers(OSS_CONTENT_MD5);
    signbuf_append_from_headers(OSS_CONTENT_TYPE);

    // date
    if ((value = apr_table_get(headers, OSS_CANNONICALIZED_HEADER_DATE)) == NULL) {
        value = apr_table_get(headers, OSS_DATE);
    }
    if (NULL == value || *value == '\0') {
        aos_error_log("http header date is empty.");
        return AOSE_INVALID_ARGUMENT;
    }
    signbuf_append(value, strlen(value));

    // user meta headers
    if ((res = oss_get_canonicalized_headers(p, headers, signbuf)) != AOSE_OK) {
        return res;
    }

    // canonicalized resource
    aos_buf_append_string(p, signbuf, canon_res->data, canon_res->len);
    
    if (params != NULL && (res = oss_get_canonicalized_resource(p, params, signbuf)) != AOSE_OK) {
        return res;
    }

    // result
    signstr->data = (char *)signbuf->pos;
    signstr->len = aos_buf_size(signbuf);

    return AOSE_OK;
}

void oss_sign_headers(aos_pool_t *p, 
                      const aos_string_t *signstr, 
                      const aos_string_t *access_key_id,
                      const aos_string_t *access_key_secret, 
                      aos_table_t *headers)
{
    int b64Len;
    char *value;
    unsigned char hmac[20];
    char b64[((20 + 1) * 4) / 3];

    HMAC_SHA1(hmac, (unsigned char *)access_key_secret->data, access_key_secret->len,
              (unsigned char *)signstr->data, signstr->len);

    // Now base-64 encode the results
    b64Len = aos_base64_encode(hmac, 20, b64);
    value = apr_psprintf(p, "OSS %.*s:%.*s", access_key_id->len, access_key_id->data, b64Len, b64);
    apr_table_addn(headers, OSS_AUTHORIZATION, value);

    return;
}

int oss_get_signed_headers(aos_pool_t *p, 
                           const aos_string_t *access_key_id, 
                           const aos_string_t *access_key_secret,
                           const aos_string_t* canon_res, 
                           aos_http_request_t *req)
{
    int res;
    aos_string_t signstr;

    res = oss_get_string_to_sign(p, req->method, canon_res, 
                                 req->headers, req->query_params, &signstr);
    
    if (res != AOSE_OK) {
        return res;
    }
    
    aos_debug_log("signstr:%.*s.", signstr.len, signstr.data);

    oss_sign_headers(p, &signstr, access_key_id, access_key_secret, req->headers);

    return AOSE_OK;
}

int oss_sign_request(aos_http_request_t *req, 
                     const oss_config_t *config)
{
    aos_string_t canon_res;
    char canon_buf[AOS_MAX_URI_LEN];
    char datestr[AOS_MAX_GMT_TIME_LEN];
    const char *value;
    int res = AOSE_OK;
    int len = 0;
    
    if (config->signature_version == 4) {
        return oss_sign_request_v4(req, config);
    }

    canon_res.data = canon_buf;
    if (req->resource != NULL) {
        len = strlen(req->resource);
        if (len >= AOS_MAX_URI_LEN - 1) {
            aos_error_log("http resource too long, %s.", req->resource);
            return AOSE_INVALID_ARGUMENT;
        }
        canon_res.len = apr_snprintf(canon_buf, sizeof(canon_buf), "/%s", req->resource);
    } else {
        canon_res.len = apr_snprintf(canon_buf, sizeof(canon_buf), "/");
    }

    if ((value = apr_table_get(req->headers, OSS_CANNONICALIZED_HEADER_DATE)) == NULL) {
        aos_get_gmt_str_time(datestr);
        apr_table_set(req->headers, OSS_DATE, datestr);
    }

    res = oss_get_signed_headers(req->pool, &config->access_key_id, 
                                 &config->access_key_secret, &canon_res, req);
    return res;
}

int get_oss_request_signature(const oss_request_options_t *options, 
                              aos_http_request_t *req,
                              const aos_string_t *expires, 
                              aos_string_t *signature)
{
    aos_string_t canon_res;
    char canon_buf[AOS_MAX_URI_LEN];
    const char *value;
    aos_string_t signstr;
    int res = AOSE_OK;
    int b64Len;
    unsigned char hmac[20];
    char b64[((20 + 1) * 4) / 3];

    canon_res.data = canon_buf;
    canon_res.len = apr_snprintf(canon_buf, sizeof(canon_buf), "/%s", req->resource);

    apr_table_set(req->headers, OSS_DATE, expires->data);

    if ((res = oss_get_string_to_sign(options->pool, req->method, &canon_res, 
        req->headers, req->query_params, &signstr))!= AOSE_OK) {
        return res;
    }

    HMAC_SHA1(hmac, (unsigned char *)options->config->access_key_secret.data, 
              options->config->access_key_secret.len,
              (unsigned char *)signstr.data, signstr.len);

    b64Len = aos_base64_encode(hmac, 20, b64);
    value = apr_psprintf(options->pool, "%.*s", b64Len, b64);
    aos_str_set(signature, value);

    return res;
}

int oss_get_signed_url(const oss_request_options_t *options, 
                       aos_http_request_t *req,
                       const aos_string_t *expires, 
                       aos_string_t *signed_url)
{
    char *signed_url_str;
    aos_string_t querystr;
    char uristr[3*AOS_MAX_URI_LEN+1];
    int res = AOSE_OK;
    aos_string_t signature;
    const char *proto;

    if (options->config->sts_token.data != NULL) {
        apr_table_set(req->query_params, OSS_SECURITY_TOKEN, options->config->sts_token.data);
    }

    res = get_oss_request_signature(options, req, expires, &signature);
    if (res != AOSE_OK) {
        return res;
    }

    apr_table_set(req->query_params, OSS_ACCESSKEYID, options->config->access_key_id.data);
    apr_table_set(req->query_params, OSS_EXPIRES, expires->data);
    apr_table_set(req->query_params, OSS_SIGNATURE, signature.data);

    uristr[0] = '\0';
    aos_str_null(&querystr);
    res = aos_url_encode_ex(uristr, req->uri, AOS_MAX_URI_LEN, req->normalize_url);
    if (res != AOSE_OK) {
        return res;
    }

    res = aos_query_params_to_string(options->pool, req->query_params, &querystr);
    if (res != AOSE_OK) {
        return res;
    }

    proto = strlen(req->proto) != 0 ? req->proto : AOS_HTTP_PREFIX;
    signed_url_str = apr_psprintf(options->pool, "%s%s/%s%.*s",
                                  proto, req->host, uristr,
                                  querystr.len, querystr.data);
    aos_str_set(signed_url, signed_url_str);

    return res;
}

int oss_get_rtmp_signed_url(const oss_request_options_t *options,
                            aos_http_request_t *req,
                            const aos_string_t *expires,
                            const aos_string_t *play_list_name,
                            aos_table_t *params,
                            aos_string_t *signed_url)
{
    char *signed_url_str;
    aos_string_t querystr;
    char uristr[3*AOS_MAX_URI_LEN+1];
    int res = AOSE_OK;
    aos_string_t signature;
    int pos = 0;
    const aos_array_header_t *tarr;
    const aos_table_entry_t *telts;

    if (NULL != params) {
        tarr = aos_table_elts(params);
        telts = (aos_table_entry_t*)tarr->elts;
        for (pos = 0; pos < tarr->nelts; ++pos) {
            apr_table_set(req->query_params, telts[pos].key, telts[pos].val);
        }
    }
    apr_table_set(req->query_params, OSS_PLAY_LIST_NAME, play_list_name->data);

    res = get_oss_rtmp_request_signature(options, req, expires,&signature);
    if (res != AOSE_OK) {
        return res;
    }

    apr_table_set(req->query_params, OSS_ACCESSKEYID,
                  options->config->access_key_id.data);
    apr_table_set(req->query_params, OSS_EXPIRES, expires->data);
    apr_table_set(req->query_params, OSS_SIGNATURE, signature.data);

    uristr[0] = '\0';
    aos_str_null(&querystr);
    res = aos_url_encode(uristr, req->uri, AOS_MAX_URI_LEN);
    if (res != AOSE_OK) {
        return res;
    }

    res = aos_query_params_to_string(options->pool, req->query_params, &querystr);
    if (res != AOSE_OK) {
        return res;
    }

    signed_url_str = apr_psprintf(options->pool, "%s%s/%s%.*s",
                                  req->proto, req->host, uristr,
                                  querystr.len, querystr.data);
    aos_str_set(signed_url, signed_url_str);

    return res;
}

int get_oss_rtmp_request_signature(const oss_request_options_t *options,
                                   aos_http_request_t *req,
                                   const aos_string_t *expires,
                                   aos_string_t *signature)
{
    aos_string_t canon_res;
    char canon_buf[AOS_MAX_URI_LEN];
    const char *value;
    aos_string_t signstr;
    int res = AOSE_OK;
    int b64Len;
    unsigned char hmac[20];
    char b64[((20 + 1) * 4) / 3];

    canon_res.data = canon_buf;
    canon_res.len = apr_snprintf(canon_buf, sizeof(canon_buf), "/%s", req->resource);

    if ((res = oss_get_rtmp_string_to_sign(options->pool, expires, &canon_res,
        req->query_params, &signstr))!= AOSE_OK) {
        return res;
    }

    HMAC_SHA1(hmac, (unsigned char *)options->config->access_key_secret.data,
              options->config->access_key_secret.len,
              (unsigned char *)signstr.data, signstr.len);

    b64Len = aos_base64_encode(hmac, 20, b64);
    value = apr_psprintf(options->pool, "%.*s", b64Len, b64);
    aos_str_set(signature, value);

    return res;
}

int oss_get_rtmp_string_to_sign(aos_pool_t *p,
                                const aos_string_t *expires,
                                const aos_string_t *canon_res,
                                const aos_table_t *params,
                                aos_string_t *signstr)
{
    int res;
    aos_buf_t *signbuf;
    aos_str_null(signstr);

    signbuf = aos_create_buf(p, 1024);

    // expires
    aos_buf_append_string(p, signbuf, expires->data, expires->len);
    aos_buf_append_string(p, signbuf, "\n", sizeof("\n")-1);

    // canonicalized params
    if ((res = oss_get_canonicalized_params(p, params, signbuf)) != AOSE_OK) {
        return res;
    }

    // canonicalized resource
    aos_buf_append_string(p, signbuf, canon_res->data, canon_res->len);

    // result
    signstr->data = (char *)signbuf->pos;
    signstr->len = aos_buf_size(signbuf);

    return AOSE_OK;
}

static int oss_get_canonicalized_params(aos_pool_t *p,
                                        const aos_table_t *params,
                                        aos_buf_t *signbuf)
{
    int pos;
    int meta_count = 0;
    int i;
    int len;
    const aos_array_header_t *tarr;
    const aos_table_entry_t *telts;
    char **meta_headers;
    const char *value;
    aos_string_t tmp_str;
    char *tmpbuf = (char*)malloc(AOS_MAX_HEADER_LEN + 1);
    if (NULL == tmpbuf) {
        aos_error_log("malloc %d memory failed.", AOS_MAX_HEADER_LEN + 1);
        return AOSE_OVER_MEMORY;
    }

    if (apr_is_empty_table(params)) {
        free(tmpbuf);
        return AOSE_OK;
    }

    // sort user meta header
    tarr = aos_table_elts(params);
    telts = (aos_table_entry_t*)tarr->elts;
    meta_headers = aos_pcalloc(p, tarr->nelts * sizeof(char*));
    for (pos = 0; pos < tarr->nelts; ++pos) {
        aos_string_t key = aos_string(telts[pos].key);
        meta_headers[meta_count++] = key.data;
    }
    if (meta_count == 0) {
        free(tmpbuf);
        return AOSE_OK;
    }
    aos_gnome_sort((const char **)meta_headers, meta_count);

    // sign string
    for (i = 0; i < meta_count; ++i) {
        value = apr_table_get(params, meta_headers[i]);
        aos_str_set(&tmp_str, value);
        aos_strip_space(&tmp_str);
        len = apr_snprintf(tmpbuf, AOS_MAX_HEADER_LEN + 1, "%s:%.*s",
                           meta_headers[i], tmp_str.len, tmp_str.data);
        if (len > AOS_MAX_HEADER_LEN) {
            free(tmpbuf);
            aos_error_log("rtmp parameters too many, %d > %d.",
                          len, AOS_MAX_HEADER_LEN);
            return AOSE_INVALID_ARGUMENT;
        }
        tmp_str.data = tmpbuf;
        tmp_str.len = len;
        aos_buf_append_string(p, signbuf, tmpbuf, len);
        aos_buf_append_string(p, signbuf, "\n", sizeof("\n")-1);
    }

    free(tmpbuf);
    return AOSE_OK;
}

//v4
#define AOS_SHA256_HASH_LEN 32

static int cmp_table_key_v4(const void* v1, const void* v2)
{
    const apr_table_entry_t* s1 = (const apr_table_entry_t*)v1;
    const apr_table_entry_t* s2 = (const apr_table_entry_t*)v2;
    return strcmp(s1->key, s2->key);
}

static int is_oss_signed_header_v4(const char* str)
{
    if (strncasecmp(str, OSS_CANNONICALIZED_HEADER_PREFIX, strlen(OSS_CANNONICALIZED_HEADER_PREFIX)) == 0 ||
        strncasecmp(str, OSS_CONTENT_MD5, strlen(OSS_CONTENT_MD5)) == 0 ||
        strncasecmp(str, OSS_CONTENT_TYPE, strlen(OSS_CONTENT_TYPE)) == 0)
    {
        return 1;
    }
    return 0;
}

static int oss_build_canonical_request_v4(aos_pool_t* p, aos_http_request_t* req, aos_string_t* out)
{
    int pos;
    const char* value;
    aos_buf_t* signbuf;
    const aos_array_header_t* arr;
    const aos_table_entry_t* elts;
    aos_table_t *canon_querys;
    aos_table_t* canon_headers;

    signbuf = aos_create_buf(p, 1024);

    //http method + "\n"
    value = aos_http_method_to_string(req->method);
    aos_buf_append_string(p, signbuf, value, strlen(value));
    aos_buf_append_string(p, signbuf, "\n", 1);

    //Canonical URI + "\n"
    aos_buf_append_string(p, signbuf, "/", 1);
    if (req->resource != NULL) {
        char canon_buf[AOS_MAX_URI_LEN];
        canon_buf[0] = '\0';
        aos_url_encode_ex(canon_buf, req->resource, AOS_MAX_URI_LEN, 1);
        aos_buf_append_string(p, signbuf, canon_buf, strlen(canon_buf));
    }
    aos_buf_append_string(p, signbuf, "\n", 1);

    //Canonical Query String + "\n"
    arr = aos_table_elts(req->query_params);
    elts = (aos_table_entry_t*)arr->elts;
    canon_querys = aos_table_make(p, 0);
    for (pos = 0; pos < arr->nelts; ++pos) {
        char enc_key[AOS_MAX_QUERY_ARG_LEN];
        char enc_value[AOS_MAX_URI_LEN];
        aos_url_encode(enc_key, elts[pos].key, AOS_MAX_QUERY_ARG_LEN);
        aos_url_encode(enc_value, elts[pos].val, AOS_MAX_QUERY_ARG_LEN);
        apr_table_set(canon_querys, enc_key, enc_value);
    }
    arr = aos_table_elts(canon_querys);
    qsort(arr->elts, arr->nelts, arr->elt_size, cmp_table_key_v4);

    elts = (aos_table_entry_t*)arr->elts;
    for (pos = 0; pos < arr->nelts; ++pos) {
        if (pos != 0) {
            aos_buf_append_string(p, signbuf, "&", 1);
        }
        value = elts[pos].key;
        aos_buf_append_string(p, signbuf, value, strlen(value));

        value = elts[pos].val;
        if (value != NULL && *value != '\0') {
            aos_buf_append_string(p, signbuf, "=", 1);
            aos_buf_append_string(p, signbuf, value, strlen(value));
        }
    }
    aos_buf_append_string(p, signbuf, "\n", 1);

    //Canonical Headers + "\n"
    arr = aos_table_elts(req->headers);
    elts = (aos_table_entry_t*)arr->elts;
    canon_headers = aos_table_make(p, 0);
    for (pos = 0; pos < arr->nelts; ++pos) {
        if (is_oss_signed_header_v4(elts[pos].key)) {
            aos_string_t key;
            aos_str_set(&key, apr_pstrdup(p, elts[pos].key));
            aos_string_tolower(&key);
            aos_strip_space(&key);
            apr_table_addn(canon_headers, key.data, elts[pos].val);
        }
    }
    arr = aos_table_elts(canon_headers);
    qsort(arr->elts, arr->nelts, arr->elt_size, cmp_table_key_v4);

    elts = (aos_table_entry_t*)arr->elts;
    for (pos = 0; pos < arr->nelts; ++pos) {
        aos_string_t tmp_str;
        aos_str_set(&tmp_str, elts[pos].val);
        aos_strip_space(&tmp_str);
        aos_buf_append_string(p, signbuf, elts[pos].key, strlen(elts[pos].key));
        aos_buf_append_string(p, signbuf, ":", 1);
        aos_buf_append_string(p, signbuf, tmp_str.data, tmp_str.len);
        aos_buf_append_string(p, signbuf, "\n", 1);
    }
    aos_buf_append_string(p, signbuf, "\n", 1);

    //Additional Headers + "\n"
    aos_buf_append_string(p, signbuf, "\n", 1);

    //Hashed PayLoad
    value = apr_table_get(req->headers, OSS_CONTENT_SHA256);
    aos_buf_append_string(p, signbuf, value, strlen(value));

    // result
    out->data = (char*)signbuf->pos;
    out->len = aos_buf_size(signbuf);

    return AOSE_OK;
}

static int oss_build_string_to_sign_v4(aos_pool_t* p, const aos_string_t* datetime, const aos_string_t* date, const aos_string_t* region, const aos_string_t* product, const aos_string_t* canonical_request, aos_string_t* out)
{
    char hash[AOS_SHA256_HASH_LEN];
    char hex[AOS_SHA256_HASH_LEN * 2 + 1];
    aos_buf_t* signbuf;

    signbuf = aos_create_buf(p, 256);

    // OSS4-HMAC-SHA256 + \n +
    // dateime + \n +
    // data/region/product/aliyun_v4_request + \n +
    // toHex(sha256(canonical_request));
    aos_buf_append_string(p, signbuf, "OSS4-HMAC-SHA256", 16); aos_buf_append_string(p, signbuf, "\n", 1);
    aos_buf_append_string(p, signbuf, datetime->data, datetime->len); aos_buf_append_string(p, signbuf, "\n", 1);

    //scope
    aos_buf_append_string(p, signbuf, date->data, date->len); aos_buf_append_string(p, signbuf, "/", 1);
    aos_buf_append_string(p, signbuf, region->data, region->len); aos_buf_append_string(p, signbuf, "/", 1);
    aos_buf_append_string(p, signbuf, product->data, product->len); aos_buf_append_string(p, signbuf, "/", 1);
    aos_buf_append_string(p, signbuf, "aliyun_v4_request", 17); aos_buf_append_string(p, signbuf, "\n", 1);

    aos_SHA256(hash, canonical_request->data, canonical_request->len);
    aos_encode_hex(hex, hash, AOS_SHA256_HASH_LEN, NULL);
    aos_buf_append_string(p, signbuf, hex, AOS_SHA256_HASH_LEN * 2);

    // result
    out->data = (char*)signbuf->pos;
    out->len = aos_buf_size(signbuf);

    return AOSE_OK;
}

static int oss_build_signing_key_v4(aos_pool_t* p, const aos_string_t* access_key_secret, const aos_string_t* date, const aos_string_t* region, const aos_string_t* product, char signing_key[32])
{
    char* signing_secret;
    char signing_date[AOS_SHA256_HASH_LEN];
    char signing_region[AOS_SHA256_HASH_LEN];
    char signing_product[AOS_SHA256_HASH_LEN];
    signing_secret = apr_psprintf(p, "aliyun_v4%.*s", access_key_secret->len, access_key_secret->data);
    aos_HMAC_SHA256(signing_date, signing_secret, strlen(signing_secret), date->data, date->len);
    aos_HMAC_SHA256(signing_region, signing_date, AOS_SHA256_HASH_LEN, region->data, region->len);
    aos_HMAC_SHA256(signing_product, signing_region, AOS_SHA256_HASH_LEN, product->data, product->len);
    aos_HMAC_SHA256(signing_key, signing_product, AOS_SHA256_HASH_LEN, "aliyun_v4_request", 17);

    return AOSE_OK;
}

static int oss_build_signature_v4(aos_pool_t* p, const char signing_key[32], const aos_string_t* string_to_sign, aos_string_t* out)
{
    char signature[AOS_SHA256_HASH_LEN];
    aos_buf_t* signbuf;

    signbuf = aos_create_buf(p, AOS_SHA256_HASH_LEN * 2 + 1);
    aos_HMAC_SHA256(signature, signing_key, AOS_SHA256_HASH_LEN, string_to_sign->data, string_to_sign->len);
    aos_encode_hex((char*)signbuf->pos, signature, AOS_SHA256_HASH_LEN, NULL);

    out->data = (char*)signbuf->pos;
    out->len = AOS_SHA256_HASH_LEN * 2;

    return AOSE_OK;
}

static int oss_sign_request_v4(aos_http_request_t *req, const oss_config_t *config)
{
    aos_string_t datetime;
    aos_string_t date;
    aos_string_t region;
    aos_string_t product;
    aos_string_t canonical_request;
    aos_string_t string_to_sign;
    aos_string_t signature;
    char signing_key[AOS_SHA256_HASH_LEN];
    const char* value;
    int res = AOSE_OK;
    aos_string_t gmt_suffix;


    //default, ex payload, x-oss-date 
    apr_table_set(req->headers, OSS_CONTENT_SHA256, "UNSIGNED-PAYLOAD");

    if ((value = apr_table_get(req->headers, OSS_CANNONICALIZED_HEADER_DATE)) == NULL) {
        char datestr[AOS_MAX_GMT_TIME_LEN];
        aos_get_iso8601_str_time(datestr);
        apr_table_set(req->headers, OSS_CANNONICALIZED_HEADER_DATE, datestr);
    }

    //datetime & date
    value = apr_table_get(req->headers, OSS_CANNONICALIZED_HEADER_DATE);
    datetime.data = (char *)value;
    datetime.len = strlen(value);

    aos_str_set(&gmt_suffix, "GMT");
    if (aos_ends_with(&datetime, &gmt_suffix)) {
        aos_error_log("x-oss-date should be iso8601 format %s.", datetime.data);
        return AOSE_INVALID_ARGUMENT;
    }
    else {
        date.data = datetime.data;
        date.len = aos_min(8, datetime.len);
    }

    //region
    if (!aos_string_is_empty(&config->cloudbox_id)) {
        region.data = config->cloudbox_id.data;
        region.len = config->cloudbox_id.len;
    }
    else {
        region.data = config->region.data;
        region.len = config->region.len;
    }

    //product, oss or "oss-cloudbox"
    if (!aos_string_is_empty(&config->cloudbox_id)) {
        aos_str_set(&product, "oss-cloudbox");
    } else {
        aos_str_set(&product, "oss");
    }

    //canonical request
    if ((res = oss_build_canonical_request_v4(req->pool, req, &canonical_request)) != AOSE_OK) {
        return res;
    }
    //printf("\ncanonical_request:\n%s", canonical_request.data);

    //string to sign
    if ((res = oss_build_string_to_sign_v4(req->pool, &datetime, &date, &region, &product, &canonical_request, &string_to_sign)) != AOSE_OK) {
        return res;
    }
    //printf("\nstring_to_sign:\n%s", string_to_sign.data);

    //signing key
    if ((res = oss_build_signing_key_v4(req->pool, &config ->access_key_secret, &date, &region, &product, signing_key)) != AOSE_OK) {
        return res;
    }

    //signature
    if ((res = oss_build_signature_v4(req->pool, signing_key, &string_to_sign, &signature)) != AOSE_OK) {
        return res;
    }
    //printf("\nsignature:\n%s", signature.data);

    //sign header
    value = apr_psprintf(req->pool, "OSS4-HMAC-SHA256 Credential=%.*s/%.*s/%.*s/%.*s/aliyun_v4_request,Signature=%.*s",
        config->access_key_id.len, config->access_key_id.data,
        date.len, date.data,
        region.len, region.data,
        product.len, product.data,
        signature.len, signature.data);
    apr_table_addn(req->headers, OSS_AUTHORIZATION, value);

    //printf("\nAuthorization:\n%s", value);

    return res;
}
