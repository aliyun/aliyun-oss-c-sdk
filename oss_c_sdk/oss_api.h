#ifndef LIBOSS_API_H
#define LIBOSS_API_H

#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_define.h"
#include "oss_util.h"

OSS_CPP_START

/*
 * @brief  create oss bucket
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[in]   oss_acl       the oss bucket acl
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_create_bucket(const oss_request_options_t *options,
                                const aos_string_t *bucket,
                                oss_acl_e oss_acl,
                                aos_table_t **resp_headers);

/*
 * @brief  create oss bucket with storage class
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[in]   oss_acl       the oss bucket acl
 * @param[in]   storage_class the oss bucket storage_class
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_create_bucket_with_storage_class(const oss_request_options_t *options, 
                                const aos_string_t *bucket, 
                                oss_acl_e oss_acl, 
                                oss_storage_class_type_e storage_class, 
                                aos_table_t **resp_headers);

/*
 * @brief  delete oss bucket
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_delete_bucket(const oss_request_options_t *options, 
                                const aos_string_t *bucket, 
                                aos_table_t **resp_headers);

/*
 * @brief  put oss bucket acl
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[in]   oss_acl       the oss bucket acl
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_bucket_acl(const oss_request_options_t *options, 
                                 const aos_string_t *bucket, 
                                 oss_acl_e oss_acl,
                                 aos_table_t **resp_headers);

/*
 * @brief  get oss bucket acl
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[out]  oss_acl       the oss bucket acl
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_bucket_acl(const oss_request_options_t *options,
                                 const aos_string_t *bucket,
                                 aos_string_t *oss_acl,
                                 aos_table_t **resp_headers);

/*
 * @brief  head oss bucket 
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_head_bucket(const oss_request_options_t *options, 
                              const aos_string_t *bucket, 
                              aos_table_t **resp_headers);

/*
 * @brief  get oss bucket location
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[out]  oss_location  the oss bucket location
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_bucket_location(const oss_request_options_t *options, 
                                      const aos_string_t *bucket, 
                                      aos_string_t *oss_location, 
                                      aos_table_t **resp_headers);

/*
 * @brief  get oss bucket info
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[out]  bucket_info   the oss bucket info
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_bucket_info(const oss_request_options_t *options, 
                                  const aos_string_t *bucket, 
                                  oss_bucket_info_t *bucket_info, 
                                  aos_table_t **resp_headers);

/*
 * @brief  get oss bucket stat
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[out]  bucket_stat   the oss bucket stat
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_bucket_stat(const oss_request_options_t *options, 
                                  const aos_string_t *bucket, 
                                  oss_bucket_stat_t *bucket_stat, 
                                  aos_table_t **resp_headers);

/*
 * @brief  get oss bucket cors
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[out]  rule_list     the oss bucket cors rule list
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_bucket_cors(const oss_request_options_t *options, 
                                  const aos_string_t *bucket, 
                                  aos_list_t *rule_list, 
                                  aos_table_t **resp_headers);

/*
 * @brief  delete oss bucket cors
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_delete_bucket_cors(const oss_request_options_t *options, 
                                     const aos_string_t *bucket, 
                                     aos_table_t **resp_headers);

/*
 * @brief  get oss bucket referer
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[out]  referer_config the oss bucket referer config
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_bucket_referer(const oss_request_options_t *options, 
                                     const aos_string_t *bucket, 
                                     oss_referer_config_t *referer_config, 
                                     aos_table_t **resp_headers);
/*
 * @brief  put oss bucket storage capacity
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[in]   storage_capacity  the oss bucket storage capacity, unit gigabyte
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_bucket_storage_capacity(const oss_request_options_t *options, 
                                              const aos_string_t *bucket, 
                                              long storage_capacity, 
                                              aos_table_t **resp_headers);

/*
 * @brief  get oss bucket storage capacity
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[out]  storage_capacity  the oss bucket storage capacity
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_bucket_storage_capacity(const oss_request_options_t *options, 
                                              const aos_string_t *bucket, 
                                              long *storage_capacity, 
                                              aos_table_t **resp_headers);

/*
 * @brief  put oss bucket lifecycle
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   lifecycle_rule_list the oss bucket lifecycle list
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_bucket_lifecycle(const oss_request_options_t *options,
                                       const aos_string_t *bucket, 
                                       aos_list_t *lifecycle_rule_list, 
                                       aos_table_t **resp_headers);

/*
 * @brief  put oss bucket referer 
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   referer_config      the oss bucket referer_config 
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_bucket_referer(const oss_request_options_t *options,
                                     const aos_string_t *bucket, 
                                     oss_referer_config_t *referer_config,
                                     aos_table_t **resp_headers);

/*
 * @brief  put oss bucket cors 
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   rule_list           the oss bucket ocor rule list
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_bucket_cors(const oss_request_options_t *options,
                                  const aos_string_t *bucket, 
                                  aos_list_t *rule_list,
                                  aos_table_t **resp_headers);

/*
 * @brief  put oss bucket website
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   website_config      the oss bucket website config 
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_bucket_website(const oss_request_options_t *options,
                                     const aos_string_t *bucket, 
                                     oss_website_config_t *website_config,
                                     aos_table_t **resp_headers);

/*
 * @brief  get oss bucket website
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[out]   website_config      the oss bucket website config 
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_bucket_website(const oss_request_options_t *options, 
                                     const aos_string_t *bucket, 
                                     oss_website_config_t *website_config, 
                                     aos_table_t **resp_headers);

/*
 * @brief  delete oss bucket website
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_delete_bucket_website(const oss_request_options_t *options, 
                                        const aos_string_t *bucket, 
                                        aos_table_t **resp_headers);

/*
 * @brief  put oss bucket logging
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   content             the oss bucket logging content rule
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_bucket_logging(const oss_request_options_t *options,
                                     const aos_string_t *bucket, 
                                     oss_logging_config_content_t *content, 
                                     aos_table_t **resp_headers);

/*
 * @brief  get oss bucket logging
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   content             the oss bucket logging content rule
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_bucket_logging(const oss_request_options_t *options, 
                                     const aos_string_t *bucket, 
                                     oss_logging_config_content_t *logging_content, 
                                     aos_table_t **resp_headers);

/*
 * @brief  delete oss bucket logging
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_delete_bucket_logging(const oss_request_options_t *options, 
                                        const aos_string_t *bucket, 
                                        aos_table_t **resp_headers);

/*
 * @brief  get oss bucket lifecycle
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[out]  lifecycle_rule_list the oss bucket lifecycle list
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_bucket_lifecycle(const oss_request_options_t *options,
                                       const aos_string_t *bucket, 
                                       aos_list_t *lifecycle_rule_list, 
                                       aos_table_t **resp_headers);

/*
 * @brief  delete oss bucket lifecycle
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_delete_bucket_lifecycle(const oss_request_options_t *options,
                                          const aos_string_t *bucket, 
                                          aos_table_t **resp_headers);

/*
 * @brief  list oss objects
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[in]   params        input params for list object request,
                              including prefix, marker, delimiter, max_ret
 * @param[out]  params        output params for list object response,
                              including truncated, next_marker, obje list
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_list_object(const oss_request_options_t *options, 
                              const aos_string_t *bucket, 
                              oss_list_object_params_t *params, 
                              aos_table_t **resp_headers);

/*
 * @brief  list oss buckets
 * @param[in]   options       the oss request options
 * @param[in]   params        input params for list bucket request,
                              including prefix, marker, max_keys
 * @param[out]  params        output params for list bucket response,
                              including truncated, next_marker, bucket list
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_list_bucket(const oss_request_options_t *options,
                              oss_list_buckets_params_t *params, 
                              aos_table_t **resp_headers);

/*
 * @brief  put oss object from buffer
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   buffer              the buffer containing object content
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_object_from_buffer(const oss_request_options_t *options, 
                                         const aos_string_t *bucket, 
                                         const aos_string_t *object, 
                                         aos_list_t *buffer, 
                                         aos_table_t *headers,
                                         aos_table_t **resp_headers);

/*
 * @brief  put oss object from file
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   filename            the filename to put
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_object_from_file(const oss_request_options_t *options,
                                       const aos_string_t *bucket, 
                                       const aos_string_t *object, 
                                       const aos_string_t *filename,
                                       aos_table_t *headers, 
                                       aos_table_t **resp_headers);

/*
 * @brief  put oss object from buffer
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   buffer              the buffer containing object content
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[in]   progress_callback   the progress callback function
 * @param[out]  resp_headers        oss server response headers
 * @param[out]  resp_body           oss server response body
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_do_put_object_from_buffer(const oss_request_options_t *options,
                                            const aos_string_t *bucket, 
                                            const aos_string_t *object, 
                                            aos_list_t *buffer,
                                            aos_table_t *headers, 
                                            aos_table_t *params,
                                            oss_progress_callback progress_callback,
                                            aos_table_t **resp_headers,
                                            aos_list_t *resp_body);

/*
 * @brief  put oss object from file
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   filename            the filename to put
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[in]   progress_callback   the progress callback function
 * @param[out]  resp_headers        oss server response headers
 * @param[out]  resp_body           oss server response body
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_do_put_object_from_file(const oss_request_options_t *options,
                                          const aos_string_t *bucket, 
                                          const aos_string_t *object, 
                                          const aos_string_t *filename,
                                          aos_table_t *headers, 
                                          aos_table_t *params,
                                          oss_progress_callback progress_callback,
                                          aos_table_t **resp_headers,
                                          aos_list_t *resp_body);

/*
 * @brief  get oss object to buffer
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[out]  buffer              the buffer containing object content
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_object_to_buffer(const oss_request_options_t *options, 
                                       const aos_string_t *bucket, 
                                       const aos_string_t *object,
                                       aos_table_t *headers, 
                                       aos_table_t *params,
                                       aos_list_t *buffer, 
                                       aos_table_t **resp_headers);

/*
 * @brief  restore oss object from archive bucket
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_restore_object(const oss_request_options_t *options, 
                                          const aos_string_t *bucket, 
                                          const aos_string_t *object,
                                          aos_table_t *headers, 
                                          aos_table_t **resp_headers);

/*
 * @brief  get oss object to buffer
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[in]   progress_callback   the progress callback function
 * @param[out]  buffer              the buffer containing object content
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_do_get_object_to_buffer(const oss_request_options_t *options, 
                                          const aos_string_t *bucket, 
                                          const aos_string_t *object,
                                          aos_table_t *headers, 
                                          aos_table_t *params,
                                          aos_list_t *buffer,
                                          oss_progress_callback progress_callback, 
                                          aos_table_t **resp_headers);

/*
 * @brief  get oss object to file
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[in]  filename             the filename storing object content
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_object_to_file(const oss_request_options_t *options,
                                     const aos_string_t *bucket, 
                                     const aos_string_t *object,
                                     aos_table_t *headers, 
                                     aos_table_t *params,
                                     aos_string_t *filename, 
                                     aos_table_t **resp_headers);

/*
 * @brief  get oss object to file
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[in]   filename            the filename storing object content
 * @param[in]   progress_callback   the progress callback function
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_do_get_object_to_file(const oss_request_options_t *options,
                                        const aos_string_t *bucket, 
                                        const aos_string_t *object,
                                        aos_table_t *headers, 
                                        aos_table_t *params,
                                        aos_string_t *filename, 
                                        oss_progress_callback progress_callback,
                                        aos_table_t **resp_headers);

/*
 * @brief  head oss object
 * @param[in]   options          the oss request options
 * @param[in]   bucket           the oss bucket name
 * @param[in]   object           the oss object name
 * @param[in]   headers          the headers for request
 * @param[out]  resp_headers     oss server response headers containing object meta
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_head_object(const oss_request_options_t *options, 
                              const aos_string_t *bucket, 
                              const aos_string_t *object,
                              aos_table_t *headers, 
                              aos_table_t **resp_headers);

/*
 * @brief get object meta
 * @param[in]   options          the oss request options
 * @param[in]   bucket           the oss bucket name
 * @param[in]   object           the oss object name
 * @param[out]  resp_headers     oss server response headers containing object meta
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_object_meta(const oss_request_options_t *options,
                                  const aos_string_t *bucket,
                                  const aos_string_t *object,
                                  aos_table_t **resp_headers);

/*
 * @brief put object acl
 * @param[in]   options          the oss request options
 * @param[in]   bucket           the oss bucket name
 * @param[in]   object           the oss object name
 * @param[in]   oss_acl          the oss object ACL
 * @param[out]  resp_headers     oss server response headers containing object meta
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_object_acl(const oss_request_options_t *options,
                                 const aos_string_t *bucket,
                                 const aos_string_t *object,
                                 oss_acl_e oss_acl,
                                 aos_table_t **resp_headers);

/*
 * @brief put object acl
 * @param[in]   options          the oss request options
 * @param[in]   bucket           the oss bucket name
 * @param[in]   object           the oss object name
 * @param[out]  oss_acl          the oss object ACL
 * @param[out]  resp_headers     oss server response headers containing object meta
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_object_acl(const oss_request_options_t *options,
                                 const aos_string_t *bucket,
                                 const aos_string_t *object,
                                 aos_string_t *oss_acl,
                                 aos_table_t **resp_headers);

/*
 * @brief  put symlink oss object
 * @param[in]   options          the oss request options
 * @param[in]   bucket           the oss bucket name
 * @param[in]   sym_object       the oss symlink object name
 * @param[in]   target_object    the oss target object game
 * @param[out]  resp_headers     oss server response headers containing object meta
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_symlink(const oss_request_options_t *options, 
                              const aos_string_t *bucket, 
                              const aos_string_t *sym_object,
                              const aos_string_t *target_object,
                              aos_table_t **resp_headers);

/*
 * @brief  put symlink oss object
 * @param[in]   options          the oss request options
 * @param[in]   bucket           the oss bucket name
 * @param[in]   sym_object       the oss symlink object name
 * @param[in]   target_object    the oss target object game
 * @param[in]   headers          the headers for request
 * @param[out]  resp_headers     oss server response headers containing object meta
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_do_put_symlink(const oss_request_options_t *options,
    const aos_string_t *bucket,
    const aos_string_t *sym_object,
    const aos_string_t *target_object,
    aos_table_t *headers,
    aos_table_t **resp_headers);

/*
 * @brief  get symlink oss object
 * @param[in]   options          the oss request options
 * @param[in]   bucket           the oss bucket name
 * @param[in]   sym_object       the oss symlink object name
 * @param[out]  resp_headers     oss server response headers containing object meta
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_symlink(const oss_request_options_t *options, 
                              const aos_string_t *bucket, 
                              const aos_string_t *sym_object,
                              aos_table_t **resp_headers);

/*
 * @brief  delete oss object
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_delete_object(const oss_request_options_t *options, 
                                const aos_string_t *bucket, 
                                const aos_string_t *object, 
                                aos_table_t **resp_headers);

/*
 * @brief  delete oss objects
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object_list         the oss object list name
 * @param[in]   is_quiet            is quiet or verbose
 * @param[out]  resp_headers        oss server response headers
 * @param[out]  deleted_object_list deleted object list
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_delete_objects(const oss_request_options_t *options,
                                 const aos_string_t *bucket, 
                                 aos_list_t *object_list, 
                                 int is_quiet,
                                 aos_table_t **resp_headers, 
                                 aos_list_t *deleted_object_list);

/*
 * @brief  delete oss objects by prefix
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   prefix              prefix of delete objects
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_delete_objects_by_prefix(oss_request_options_t *options,
                                           const aos_string_t *bucket, 
                                           const aos_string_t *prefix);

/*
 * @brief  copy oss objects
 * @param[in]   options             the oss request options
 * @param[in]   source_bucket       the oss source bucket name
 * @param[in]   object_list         the oss source object list name
 * @param[in]   dest_bucket         the oss dest bucket name
 * @param[in]   dest_list           the oss dest object list name
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_copy_object(const oss_request_options_t *options, 
                              const aos_string_t *source_bucket, 
                              const aos_string_t *source_object, 
                              const aos_string_t *dest_bucket, 
                              const aos_string_t *dest_object, 
                              aos_table_t *headers, 
                              aos_table_t **resp_headers);

/*
 * @brief  append oss object from buffer
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   position            the start position append
 * @param[in]   buffer              the buffer containing object content
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_append_object_from_buffer(const oss_request_options_t *options,
                                            const aos_string_t *bucket, 
                                            const aos_string_t *object, 
                                            int64_t position,
                                            aos_list_t *buffer, 
                                            aos_table_t *headers, 
                                            aos_table_t **resp_headers);

/*
 * @brief  append oss object from buffer
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   position            the start position append
 * @param[in]   init_crc             the initial crc value
 * @param[in]   buffer              the buffer containing object content
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[in]   progress_callback   the progress callback function
 * @param[out]  resp_headers        oss server response headers
 * @param[out]  resp_body           oss server response body
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_do_append_object_from_buffer(const oss_request_options_t *options,
                                               const aos_string_t *bucket, 
                                               const aos_string_t *object, 
                                               int64_t position,
                                               uint64_t init_crc,
                                               aos_list_t *buffer, 
                                               aos_table_t *headers, 
                                               aos_table_t *params,
                                               oss_progress_callback progress_callback,
                                               aos_table_t **resp_headers,
                                               aos_list_t *resp_body);

/*
 * @brief  append oss object from file
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   position            the start position append
 * @param[in]   append_file         the file containing appending content 
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_append_object_from_file(const oss_request_options_t *options,
                                          const aos_string_t *bucket, 
                                          const aos_string_t *object, 
                                          int64_t position,
                                          const aos_string_t *append_file, 
                                          aos_table_t *headers, 
                                          aos_table_t **resp_headers);

/*
 * @brief  append oss object from file
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   position            the start position append
 * @param[in]   init_crc             the initial crc value
 * @param[in]   append_file         the file containing appending content 
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[in]   progress_callback   the progress callback function
 * @param[out]  resp_headers        oss server response headers
 * @param[out]  resp_body           oss server response body
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_do_append_object_from_file(const oss_request_options_t *options,
                                             const aos_string_t *bucket, 
                                             const aos_string_t *object, 
                                             int64_t position,
                                             uint64_t init_crc,
                                             const aos_string_t *append_file, 
                                             aos_table_t *headers, 
                                             aos_table_t *params,
                                             oss_progress_callback progress_callback,
                                             aos_table_t **resp_headers,
                                             aos_list_t *resp_body);

/*
 * @brief  gen signed url for oss object api
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   expires             the end expire time for signed url
 * @param[in]   req                 the aos http request
 * @return  signed url, non-NULL success, NULL failure
 */
char *oss_gen_signed_url(const oss_request_options_t *options, 
                         const aos_string_t *bucket,
                         const aos_string_t *object, 
                         int64_t expires, 
                         aos_http_request_t *req);

/*
 * @brief  oss put object from buffer using signed url
 * @param[in]   options             the oss request options
 * @param[in]   signed_url          the signed url for put object
 * @param[in]   buffer              the buffer containing object content
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_object_from_buffer_by_url(const oss_request_options_t *options,
                                                const aos_string_t *signed_url, 
                                                aos_list_t *buffer, 
                                                aos_table_t *headers,
                                                aos_table_t **resp_headers);

/*
 * @brief  oss put object from file using signed url
 * @param[in]   options             the oss request options
 * @param[in]   signed_url          the signed url for put object
 * @param[in]   filename            the filename containing object content
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_object_from_file_by_url(const oss_request_options_t *options,
                                              const aos_string_t *signed_url, 
                                              aos_string_t *filename, 
                                              aos_table_t *headers,
                                              aos_table_t **resp_headers);

/*
 * @brief  oss get object to buffer using signed url
 * @param[in]   options             the oss request options
 * @param[in]   signed_url          the signed url for put object
 * @param[in]   buffer              the buffer containing object content
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_object_to_buffer_by_url(const oss_request_options_t *options,
                                              const aos_string_t *signed_url, 
                                              aos_table_t *headers,
                                              aos_table_t *params,
                                              aos_list_t *buffer,
                                              aos_table_t **resp_headers);

/*
 * @brief  oss get object to file using signed url
 * @param[in]   options             the oss request options
 * @param[in]   signed_url          the signed url for put object
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[in]   filename            the filename containing object content
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_object_to_file_by_url(const oss_request_options_t *options,
                                            const aos_string_t *signed_url,
                                            aos_table_t *headers, 
                                            aos_table_t *params,
                                            aos_string_t *filename,
                                            aos_table_t **resp_headers);

/*
 * @brief  oss head object using signed url
 * @param[in]   options             the oss request options
 * @param[in]   signed_url          the signed url for put object
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_head_object_by_url(const oss_request_options_t *options,
                                     const aos_string_t *signed_url, 
                                     aos_table_t *headers, 
                                     aos_table_t **resp_headers);

/*
 * @brief  oss get object meta using signed url
 * @param[in]   options             the oss request options
 * @param[in]   signed_url          the signed url for put object
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_object_meta_by_url(const oss_request_options_t *options,
                                     const aos_string_t *signed_url,
                                     aos_table_t **resp_headers);

/*
 * @brief  oss init multipart upload
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_init_multipart_upload(const oss_request_options_t *options, 
                                        const aos_string_t *bucket, 
                                        const aos_string_t *object, 
                                        aos_string_t *upload_id,
                                        aos_table_t *headers,
                                        aos_table_t **resp_headers);

/*
 * @brief  oss upload part from buffer
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[in]   part_num            the upload part number
 * @param[in]   buffer              the buffer containing upload part content
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_upload_part_from_buffer(const oss_request_options_t *options, 
                                          const aos_string_t *bucket, 
                                          const aos_string_t *object, 
                                          const aos_string_t *upload_id, 
                                          int part_num, 
                                          aos_list_t *buffer, 
                                          aos_table_t **resp_headers);

/*
 * @brief  oss upload part from buffer
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[in]   part_num            the upload part number
 * @param[in]   buffer              the buffer containing upload part content
 * @param[in]   progress_callback   the progress callback function
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[out]  resp_headers        oss server response headers
 * @param[out]  resp_body           oss server response body
 * @return  aos_status_t, code is 2xx success, other failure
 */
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
                                             aos_list_t *resp_body);

/*
 * @brief  oss upload part from file
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[in]   part_num            the upload part number
 * @param[in]   upload_file         the file containing upload part content
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_upload_part_from_file(const oss_request_options_t *options,
                                        const aos_string_t *bucket, 
                                        const aos_string_t *object,
                                        const aos_string_t *upload_id, 
                                        int part_num, 
                                        oss_upload_file_t *upload_file,
                                        aos_table_t **resp_headers);

/*
 * @brief  oss upload part from file
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[in]   part_num            the upload part number
 * @param[in]   upload_file         the file containing upload part content
 * @param[in]   progress_callback   the progress callback function
 * @param[in]   headers             the headers for request
 * @param[in]   params              the params for request
 * @param[out]  resp_headers        oss server response headers
 * @param[out]  resp_body           oss server response body
 * @return  aos_status_t, code is 2xx success, other failure
 */
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
                                           aos_list_t *resp_body);

/*
 * @brief  oss abort multipart upload
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_abort_multipart_upload(const oss_request_options_t *options, 
                                         const aos_string_t *bucket, 
                                         const aos_string_t *object, 
                                         aos_string_t *upload_id, 
                                         aos_table_t **resp_headers);


/*
 * @brief  oss complete multipart upload
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[in]   part_list           the uploaded part list to complete
 * @param[in]   headers             the headers for request          
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_complete_multipart_upload(const oss_request_options_t *options, 
                                            const aos_string_t *bucket, 
                                            const aos_string_t *object, 
                                            const aos_string_t *upload_id, 
                                            aos_list_t *part_list, 
                                            aos_table_t *headers,
                                            aos_table_t **resp_headers);

/*
 * @brief  oss complete multipart upload
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[in]   part_list           the uploaded part list to complete
 * @param[in]   headers             the headers for request    
 * @param[in]   params              the params for request
 * @param[out]  resp_headers        oss server response headers
 * @param[out]  resp_body           oss server response body
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_do_complete_multipart_upload(const oss_request_options_t *options, 
                                               const aos_string_t *bucket, 
                                               const aos_string_t *object, 
                                               const aos_string_t *upload_id, 
                                               aos_list_t *part_list, 
                                               aos_table_t *headers,
                                               aos_table_t *params,
                                               aos_table_t **resp_headers,
                                               aos_list_t *resp_body);

/*
 * @brief  oss list upload part with specific upload_id for object
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[in]   params              the input list upload part parameters,
                                    incluing part_number_marker, max_ret
 * @param[out]  params              the output params,
                                    including next_part_number_marker, part_list, truncated
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_list_upload_part(const oss_request_options_t *options, 
                                   const aos_string_t *bucket, 
                                   const aos_string_t *object, 
                                   const aos_string_t *upload_id, 
                                   oss_list_upload_part_params_t *params, 
                                   aos_table_t **resp_headers);

/*
 * @brief  oss list multipart upload for bucket
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   params              the input list multipart upload parameters
 * @param[out]  params              the output params including next_key_marker, next_upload_id_markert, upload_list etc
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_list_multipart_upload(const oss_request_options_t *options, 
                                        const aos_string_t *bucket, 
                                        oss_list_multipart_upload_params_t *params, 
                                        aos_table_t **resp_headers);

/*
 * @brief  oss copy large object using upload part copy
 * @param[in]   options             the oss request options
 * @param[in]   paramsthe           upload part copy parameters
 * @param[in]   headers             the headers for request
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_upload_part_copy(const oss_request_options_t *options,
                                   oss_upload_part_copy_params_t *params, 
                                   aos_table_t *headers, 
                                   aos_table_t **resp_headers);

/*
 * @brief  oss upload file using multipart upload
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   upload_id           the upload id to upload if has
 * @param[in]   filename            the filename containing object content
 * @param[in]   part_size           the part size for multipart upload
 * @param[in]   headers             the headers for request
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_upload_file(oss_request_options_t *options,
                              const aos_string_t *bucket, 
                              const aos_string_t *object, 
                              aos_string_t *upload_id,
                              aos_string_t *filename, 
                              int64_t part_size,
                              aos_table_t *headers);

/*
 * @brief  oss upload file with mulit-thread and resumable
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   filepath            the filename containing object content
 * @param[in]   headers             the headers for request    
 * @param[in]   params              the params for request
 * @param[in]   clt_params          the control params of upload
 * @param[in]   progress_callback   the progress callback function
 * @param[out]  resp_headers        oss server response headers
 * @param[out]  resp_body           oss server response body
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_resumable_upload_file(oss_request_options_t *options,
                                        aos_string_t *bucket, 
                                        aos_string_t *object, 
                                        aos_string_t *filepath,                           
                                        aos_table_t *headers,
                                        aos_table_t *params,
                                        oss_resumable_clt_params_t *clt_params, 
                                        oss_progress_callback progress_callback,
                                        aos_table_t **resp_headers,
                                        aos_list_t *resp_body);

/*
 * @brief  oss upload file with mulit-thread and resumable
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[in]   filepath            download object to the file
 * @param[in]   headers             the headers for request    
 * @param[in]   params              the params for request
 * @param[in]   clt_params          the control params of upload
 * @param[in]   progress_callback   the progress callback function
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_resumable_download_file(oss_request_options_t *options,
                                          aos_string_t *bucket, 
                                          aos_string_t *object, 
                                          aos_string_t *filepath,                           
                                          aos_table_t *headers,
                                          aos_table_t *params,
                                          oss_resumable_clt_params_t *clt_params, 
                                          oss_progress_callback progress_callback,
                                          aos_table_t **resp_headers);

/*
 * @brief  oss create live channel
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   config              the oss live channel configuration
 * @param[in]   publish_url_list    the publish url list
 * @param[in]   play_url_list       the play url list
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_create_live_channel(const oss_request_options_t *options,
                                      const aos_string_t *bucket,
                                      oss_live_channel_configuration_t *config,
                                      aos_list_t *publish_url_list,
                                      aos_list_t *play_url_list,
                                      aos_table_t **resp_headers);

/*
 * @brief  oss set live channel status
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   live_channel        the oss live channel name
 * @param[in]   live_channel_status the oss live channel status, enabled or disabled
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_live_channel_status(const oss_request_options_t *options,
                                          const aos_string_t *bucket,
                                          const aos_string_t *live_channel,
                                          const aos_string_t *live_channel_status,
                                          aos_table_t **resp_headers);

/*
 * @brief  oss get live channel information
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   live_channel        the oss live channel name
 * @param[out]  info                the oss live channel information
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_live_channel_info(const oss_request_options_t *options,
                                        const aos_string_t *bucket,
                                        const aos_string_t *live_channel,
                                        oss_live_channel_configuration_t *info,
                                        aos_table_t **resp_headers);

/*
 * @brief  oss get live channel stat
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   live_channel        the oss live channel name
 * @param[out]  stat                the oss live channel stat
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_live_channel_stat(const oss_request_options_t *options,
                                        const aos_string_t *bucket,
                                        const aos_string_t *live_channel,
                                        oss_live_channel_stat_t *stat,
                                        aos_table_t **resp_headers);

/*
 * @brief  delete oss live channel
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   live_channel        the oss live channel name
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_delete_live_channel(const oss_request_options_t *options,
                                      const aos_string_t *bucket,
                                      const aos_string_t *live_channel,
                                      aos_table_t **resp_headers);

/*
 * @brief  list oss live channels
 * @param[in]   options       the oss request options
 * @param[in]   bucket        the oss bucket name
 * @param[in]   params        input params for list live channel request,
                              including prefix, marker, max_key
 * @param[out]  params        output params for list object response,
                              including truncated, next_marker, live channel list
 * @param[out]  resp_headers  oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_list_live_channel(const oss_request_options_t *options,
                                    const aos_string_t *bucket,
                                    oss_list_live_channel_params_t *params,
                                    aos_table_t **resp_headers);

/*
 * @brief  oss get live record history of live channel
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   live_channel        the oss live channel name
 * @param[out]  live_record_list    the oss live records of live channel
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_live_channel_history(const oss_request_options_t *options,
                                           const aos_string_t *bucket,
                                           const aos_string_t *live_channel,
                                           aos_list_t *live_record_list,
                                           aos_table_t **resp_headers);

/*
 * @brief  generate vod play list for a period of time
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   live_channel        the oss live channel name
 * @param[in]   play_list_name      the oss live channel play list name
 * @param[in]   start_time          the start epoch time of play list, such as 1459922368
 * @param[in]   end_time            the end epoch time of play list, such as 1459922563
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_gen_vod_play_list(const oss_request_options_t *options,
                                     const aos_string_t *bucket,
                                     const aos_string_t *live_channel,
                                     const aos_string_t *play_list_name,
                                     const int64_t start_time,
                                     const int64_t end_time,
                                     aos_table_t **resp_headers);

/*
 * @brief  gen signed url for put rtmp stream
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   live_channel        the oss live channel name
 * @param[in]   play_list_name      the oss live channel play list name
 * @param[in]   expires             the end expire time for signed url
 * @return  signed url, non-NULL success, NULL failure
 */
char *oss_gen_rtmp_signed_url(const oss_request_options_t *options,
                              const aos_string_t *bucket,
                              const aos_string_t *live_channel,
                              const aos_string_t *play_list_name,
                              const int64_t expires);

/*
* @brief  select oss object to buffer
* @param[in]   options             the oss request options
* @param[in]   bucket              the oss bucket name
* @param[in]   object              the oss object name
* @param[in]   expression          sql select expression
* @param[in]   select_params       select object parameters
* @param[out]  buffer              the buffer containing object content
* @param[out]  resp_headers        oss server response headers
* @return  aos_status_t, code is 2xx success, other failure
*/
aos_status_t *oss_select_object_to_buffer(const oss_request_options_t *options,
                            const aos_string_t *bucket,
                            const aos_string_t *object,
                            const aos_string_t *expression,
                            oss_select_object_params_t *select_params,
                            aos_list_t *buffer,
                            aos_table_t **resp_headers);

/*
* @brief  select oss object to buffer
* @param[in]   options             the oss request options
* @param[in]   bucket              the oss bucket name
* @param[in]   object              the oss object name
* @param[in]   expression          sql select expression
* @param[in]   select_params       select object parameters
* @param[in]   headers             the headers for request
* @param[in]   params              the params for request
* @param[out]  buffer              the buffer containing object content
* @param[in]   progress_callback   the progress callback function
* @param[out]  resp_headers        oss server response headers
* @return  aos_status_t, code is 2xx success, other failure
*/
aos_status_t *oss_do_select_object_to_buffer(const oss_request_options_t *options,
                            const aos_string_t *bucket,
                            const aos_string_t *object,
                            const aos_string_t *expression,
                            oss_select_object_params_t *select_params,
                            aos_table_t *headers,
                            aos_table_t *params,
                            aos_list_t *buffer,
                            oss_progress_callback progress_callback,
                            aos_table_t **resp_headers);

/*
* @brief  select oss object to file
* @param[in]   options             the oss request options
* @param[in]   bucket              the oss bucket name
* @param[in]   object              the oss object name
* @param[in]   expression          sql select expression
* @param[in]   select_params       select object parameters
* @param[in]   filename            the filename storing object content
* @param[out]  resp_headers        oss server response headers
* @return  aos_status_t, code is 2xx success, other failure
*/
aos_status_t *oss_select_object_to_file(const oss_request_options_t *options,
                            const aos_string_t *bucket,
                            const aos_string_t *object,
                            const aos_string_t *expression,
                            oss_select_object_params_t *select_params,
                            aos_string_t *filename,
                            aos_table_t **resp_headers);

/*
* @brief  select oss object to file
* @param[in]   options             the oss request options
* @param[in]   bucket              the oss bucket name
* @param[in]   object              the oss object name
* @param[in]   expression          sql select expression
* @param[in]   select_params       select object parameters
* @param[in]   headers             the headers for request
* @param[in]   params              the params for request
* @param[in]   filename            the filename storing object content
* @param[in]   progress_callback   the progress callback function
* @param[out]  resp_headers        oss server response headers
* @return  aos_status_t, code is 2xx success, other failure
*/
aos_status_t *oss_do_select_object_to_file(const oss_request_options_t *options,
                            const aos_string_t *bucket,
                            const aos_string_t *object,
                            const aos_string_t *expression,
                            oss_select_object_params_t *select_params,
                            aos_table_t *headers,
                            aos_table_t *params,
                            aos_string_t *filename,
                            oss_progress_callback progress_callback,
                            aos_table_t **resp_headers);

/*
* @brief  create select oss object meta
* @param[in]   options             the oss request options
* @param[in]   bucket              the oss bucket name
* @param[in]   object              the oss object name
* @param[in]   meta_params         create select object meta parameters
* @param[out]  resp_headers        oss server response headers
* @return  aos_status_t, code is 2xx success, other failure
*/
aos_status_t *oss_create_select_object_meta(const oss_request_options_t *options,
    const aos_string_t *bucket,
    const aos_string_t *object,
    oss_select_object_meta_params_t *meta_params,
    aos_table_t **resp_headers);

/*
 * @brief  put oss object tagging
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[out]  tag_list            the oss object tag list
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_put_object_tagging(const oss_request_options_t *options,
    const aos_string_t *bucket,
    const aos_string_t *object,
    aos_list_t *tag_list,
    aos_table_t **resp_headers);

/*
 * @brief  get oss object tagging
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[out]  tag_list            the oss object tag list
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_get_object_tagging(const oss_request_options_t *options,
    const aos_string_t *bucket,
    const aos_string_t *object,
    aos_list_t *tag_list,
    aos_table_t **resp_headers);

/*
 * @brief  delete oss object tagging
 * @param[in]   options             the oss request options
 * @param[in]   bucket              the oss bucket name
 * @param[in]   object              the oss object name
 * @param[out]  resp_headers        oss server response headers
 * @return  aos_status_t, code is 2xx success, other failure
 */
aos_status_t *oss_delete_object_tagging(const oss_request_options_t *options,
    const aos_string_t *bucket,
    const aos_string_t *object,
    aos_table_t **resp_headers);


OSS_CPP_END

#endif
