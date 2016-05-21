#ifndef LIB_OSS_AUTH_H
#define LIB_OSS_AUTH_H

#include "aos_util.h"
#include "aos_string.h"
#include "aos_http_io.h"
#include "oss_define.h"

OSS_CPP_START

/**
  * @brief  sign oss headers 
**/
void oss_sign_headers(aos_pool_t *p, 
                      const aos_string_t *signstr, 
                      const aos_string_t *access_key_id,
                      const aos_string_t *access_key_secret, 
                      aos_table_t *headers);

/**
  * @brief  get string to signature
**/
int oss_get_string_to_sign(aos_pool_t *p, 
                           http_method_e method, 
                           const aos_string_t *canon_res,
                           const aos_table_t *headers, 
                           const aos_table_t *params, 
                           aos_string_t *signstr);

/**
  * @brief  get signed oss request headers
**/
int oss_get_signed_headers(aos_pool_t *p, const aos_string_t *access_key_id, 
                           const aos_string_t *access_key_secret,
                           const aos_string_t* canon_res, aos_http_request_t *req);

/**
  * @brief  sign oss request
**/
int oss_sign_request(aos_http_request_t *req, const oss_config_t *config);

/**
  * @brief  generate oss request Signature
**/
int get_oss_request_signature(const oss_request_options_t *options, aos_http_request_t *req,
        const aos_string_t *expires, aos_string_t *signature);

/**
  * @brief  get oss signed url
**/
int oss_get_signed_url(const oss_request_options_t *options, aos_http_request_t *req,
        const aos_string_t *expires, aos_string_t *auth_url);

/**
  * @brief  get rtmp string to signature
**/
int oss_get_rtmp_string_to_sign(aos_pool_t *p, const aos_string_t *expires,
    const aos_string_t *canon_res, const aos_table_t *params,
    aos_string_t *signstr);

/**
  * @brief  generate oss rtmp request signature
**/
int get_oss_rtmp_request_signature(const oss_request_options_t *options, aos_http_request_t *req,
    const aos_string_t *expires, aos_string_t *signature);

/**
  * @brief  get oss rtmp signed url
**/
int oss_get_rtmp_signed_url(const oss_request_options_t *options, aos_http_request_t *req,
    const aos_string_t *expires, const aos_string_t *play_list_name, aos_table_t *params,
    aos_string_t *signed_url);

OSS_CPP_END

#endif
