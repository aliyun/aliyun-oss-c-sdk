#ifndef LIBOSS_UTIL_H
#define LIBOSS_UTIL_H

#include "aos_string.h"
#include "aos_transport.h"
#include "aos_status.h"
#include "oss_define.h"

OSS_CPP_START

#define init_sts_token_header() do { \
        if (options->config->sts_token.data != NULL) {\
            apr_table_set(headers, OSS_STS_SECURITY_TOKEN, options->config->sts_token.data);\
        }\
    } while(0)

/**
  * @brief  check hostname ends with specific oss domain suffix.
**/
int is_oss_domain(const aos_string_t *str);

/**
  * @brief  check hostname is ip.
**/
int is_valid_ip(const char *str);

/**
  * @brief  get oss acl str according oss_acl
  * @param[in]  oss_acl the oss bucket acl
  * @return oss acl str
**/
const char *get_oss_acl_str(oss_acl_e oss_acl);

/**
  * @brief  create oss config including host, port, access_key_id, access_key_secret, is_oss_domain
**/
oss_config_t *oss_config_create(aos_pool_t *p);

/**
  * @brief  create oss request options
  * @return oss request options
**/
oss_request_options_t *oss_request_options_create(aos_pool_t *p);

/**
  * @brief  init oss request
**/
void oss_init_request(const oss_request_options_t *options, http_method_e method,
        aos_http_request_t **req, aos_table_t *params, aos_table_t *headers, aos_http_response_t **resp);

/**
  * @brief  init oss bucket request
**/
void oss_init_bucket_request(const oss_request_options_t *options, const aos_string_t *bucket,
        http_method_e method, aos_http_request_t **req, aos_table_t *params, aos_table_t *headers,
        aos_http_response_t **resp);
 
/**
  * @brief  init oss object request
**/
void oss_init_object_request(const oss_request_options_t *options, const aos_string_t *bucket,
        const aos_string_t *object, http_method_e method, aos_http_request_t **req, 
        aos_table_t *params, aos_table_t *headers, aos_http_response_t **resp);

/**
  * @brief  init oss live channel request
**/
void oss_init_live_channel_request(const oss_request_options_t *options,
    const aos_string_t *bucket, const aos_string_t *live_channel,
    http_method_e method, aos_http_request_t **req, aos_table_t *params,
    aos_table_t *headers, aos_http_response_t **resp);

/**
  * @brief  init oss request with signed_url
**/
void oss_init_signed_url_request(const oss_request_options_t *options, const aos_string_t *signed_url,
        http_method_e method, aos_http_request_t **req,
        aos_table_t *params, aos_table_t *headers, aos_http_response_t **resp);

/**
  * @brief  oss send request
**/
aos_status_t *oss_send_request(aos_http_controller_t *ctl, aos_http_request_t *req,
        aos_http_response_t *resp);

/**
  * @brief process oss request including sign request, send request, get response
**/
aos_status_t *oss_process_request(const oss_request_options_t *options,
        aos_http_request_t *req, aos_http_response_t *resp);

/**
  * @brief process oss request with signed_url including send request, get response
**/
aos_status_t *oss_process_signed_request(const oss_request_options_t *options, 
        aos_http_request_t *req, aos_http_response_t *resp);

/**
  * @brief  get object uri using third-level domain if hostname is oss domain, otherwise second-level domain
**/
void oss_get_object_uri(const oss_request_options_t *options,
                        const aos_string_t *bucket,
                        const aos_string_t *object,
                        aos_http_request_t *req);

/**
  * @brief   bucket uri using third-level domain if hostname is oss domain, otherwise second-level domain
**/
void oss_get_bucket_uri(const oss_request_options_t *options, 
                        const aos_string_t *bucket,
                        aos_http_request_t *req);

/**
  * @brief  get rtmp uri using third-level domain if hostname is oss domain, otherwise second-level domain
**/
void oss_get_rtmp_uri(const oss_request_options_t *options,
                      const aos_string_t *bucket,
                      const aos_string_t *live_channel_id,
                      aos_http_request_t *req);

/**
  * @brief  write body content into oss request body from buffer
**/
void oss_write_request_body_from_buffer(aos_list_t *buffer, aos_http_request_t *req);

/**
  * @brief   write body content into oss request body from file
**/
int oss_write_request_body_from_file(aos_pool_t *p, const aos_string_t *filename, aos_http_request_t *req);

/**
  * @brief   write body content into oss request body from multipart upload file
**/
int oss_write_request_body_from_upload_file(aos_pool_t *p, oss_upload_file_t *upload_file, aos_http_request_t *req);

/**
  * @brief  read body content from oss response body to buffer
**/
void oss_init_read_response_body_to_buffer(aos_list_t *buffer, aos_http_response_t *resp);

/**
  * @brief  read body content from oss response body to file
**/
int oss_init_read_response_body_to_file(aos_pool_t *p, const aos_string_t *filename, aos_http_response_t *resp);

/**
  * @brief  create oss api result content
  * @return oss api result content
**/
void *oss_create_api_result_content(aos_pool_t *p, size_t size);
oss_list_object_content_t *oss_create_list_object_content(aos_pool_t *p);
oss_list_object_common_prefix_t *oss_create_list_object_common_prefix(aos_pool_t *p);
oss_list_part_content_t *oss_create_list_part_content(aos_pool_t *p);
oss_list_multipart_upload_content_t *oss_create_list_multipart_upload_content(aos_pool_t *p);
oss_complete_part_content_t *oss_create_complete_part_content(aos_pool_t *p);

/**
  * @brief  create oss api list parameters
  * @return oss api list parameters
**/
oss_list_object_params_t *oss_create_list_object_params(aos_pool_t *p);
oss_list_upload_part_params_t *oss_create_list_upload_part_params(aos_pool_t *p);
oss_list_multipart_upload_params_t *oss_create_list_multipart_upload_params(aos_pool_t *p);
oss_list_live_channel_params_t *oss_create_list_live_channel_params(aos_pool_t *p);

/**
  * @brief  create upload part copy params
  * @return upload part copy params struct for upload part copy
**/
oss_upload_part_copy_params_t *oss_create_upload_part_copy_params(aos_pool_t *p);

/**
  * @brief  create upload file struct for range multipart upload
  * @return upload file struct for range multipart upload
**/
oss_upload_file_t *oss_create_upload_file(aos_pool_t *p);

/**
  * @brief  get content-type for HTTP_POST request
  * @return content-type for HTTP_POST request
**/
void oss_set_multipart_content_type(aos_table_t *headers);

/**
  * @brief  create lifecycle rule content
  * @return lifecycle rule content
**/
oss_lifecycle_rule_content_t *oss_create_lifecycle_rule_content(aos_pool_t *p);

/**
  * @brief  create oss object content for delete objects
  * @return oss object content
**/
oss_object_key_t *oss_create_oss_object_key(aos_pool_t *p);

/**
  * @brief  create oss live channel publish url content for delete objects
  * @return oss live channel publish url content
**/
oss_live_channel_publish_url_t *oss_create_live_channel_publish_url(aos_pool_t *p);

/**
  * @brief  create oss live channel play url content for delete objects
  * @return oss live channel play url content
**/
oss_live_channel_play_url_t *oss_create_live_channel_play_url(aos_pool_t *p);

/**
  * @brief  create oss list live channel content for delete objects
  * @return oss list live channel content
**/
oss_live_channel_content_t *oss_create_list_live_channel_content(aos_pool_t *p);

/**
  * @brief  create oss live recored content for delete objects
  * @return oss live record content
**/
oss_live_record_content_t *oss_create_live_record_content(aos_pool_t *p);

/**
  * @brief  create live channel configuration content
  * @return live channel configuration content
**/
oss_live_channel_configuration_t *oss_create_live_channel_configuration_content(aos_pool_t *p);

/**
  * @brief  get part size for multipart upload
**/
void oss_get_part_size(int64_t filesize, int64_t *part_size);

/**
  * @brief  compare function for part sort
**/
int part_sort_cmp(const void *a, const void *b);

/**
  * @brief  set content type for object according to objectname
  * @return oss content type
**/
char *get_content_type(const char *name);
char *get_content_type_by_suffix(const char *suffix);

/**
  * @brief  set content type for object according to  filename
**/
void set_content_type(const char* filename, const char* key, aos_table_t *headers);

aos_table_t* aos_table_create_if_null(const oss_request_options_t *options, 
                                      aos_table_t *table, int table_size);

OSS_CPP_END

#endif
