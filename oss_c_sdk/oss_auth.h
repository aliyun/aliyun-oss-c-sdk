#ifndef LIB_OSS_AUTH_H
#define LIB_OSS_AUTH_H

#include "aos_util.h"
#include "aos_string.h"
#include "aos_http_io.h"
#include "oss_define.h"

OSS_CPP_START

/**
  * @brief  sign oss request
**/
int oss_sign_request(aos_http_request_t *req, const oss_config_t *config);

/**
  * @brief  get oss signed url
**/
int oss_get_signed_url(const oss_request_options_t *options, aos_http_request_t *req,
        const aos_string_t *expires, aos_string_t *auth_url);

/**
  * @brief  get oss rtmp signed url
**/
int oss_get_rtmp_signed_url(const oss_request_options_t *options, aos_http_request_t *req,
    const aos_string_t *expires, const aos_string_t *play_list_name, aos_table_t *params,
    aos_string_t *signed_url);

OSS_CPP_END

#endif
