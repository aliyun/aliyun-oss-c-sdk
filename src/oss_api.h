#ifndef LIBOSS_API_H
#define LIBOSS_API_H

/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
**/

#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_define.h"
#include "oss_util.h"

OSS_CPP_START

/**
  * @brief  oss create bucket
  * @param[in]  oss_acl  the oss bucket acl
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_create_bucket(const oss_request_options_t *options, 
                                const aos_string_t *bucket, 
                                oss_acl_e oss_acl, 
                                aos_table_t **resp_headers);

/**
  * @brief  oss delete bucket
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_delete_bucket(const oss_request_options_t *options, 
                                const aos_string_t *bucket, 
                                aos_table_t **resp_headers);

/**
  * @brief  oss put bucket acl
  * @param[out]  oss_acl  the oss bucket acl to put
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_put_bucket_acl(const oss_request_options_t *options, 
                                 const aos_string_t *bucket, 
                                 oss_acl_e oss_acl,
                                 aos_table_t **resp_headers);

/**
  * @brief  oss get bucket acl
  * @param[out]  oss_acl  the oss bucket acl
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_get_bucket_acl(const oss_request_options_t *options, 
                                 const aos_string_t *bucket, 
                                 aos_string_t *oss_canned_acl, 
                                 aos_table_t **resp_headers);

/**
  * @brief  oss put bucket lifycycle
  * @param[in]  lifecycle_rule_list  the lifecycle to put
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_put_bucket_lifecycle(const oss_request_options_t *options,
                                       const aos_string_t *bucket, 
                                       aos_list_t *lifecycle_rule_list, 
                                       aos_table_t **resp_headers);

/**
  * @brief  oss get bucket lifecycle
  * @param[out] lifecycle rule list
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_get_bucket_lifecycle(const oss_request_options_t *options,
                                       const aos_string_t *bucket, 
                                       aos_list_t *lifecycle_rule_list, 
                                       aos_table_t **resp_headers);

/**
  * @brief  oss delete bucket lifecycle
  * @return AOSE_OK success, other failure
  **/
aos_status_t *oss_delete_bucket_lifecycle(const oss_request_options_t *options,
                                          const aos_string_t *bucket, 
                                          aos_table_t **resp_headers);

/**
  * @brief  list object for specific bucket 
  * @param[in]  params  parameters for list object including prefix, marker, delimiter, max_ret
  * @param[out] params  output parameters including truncated, next_marker, obje list
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_list_object(const oss_request_options_t *options, 
                              const aos_string_t *bucket, 
                              oss_list_object_params_t *params, 
                              aos_table_t **resp_headers);

/**
  * @brief  oss put object from buffer
  * @param[in]  buffer  the buffer containing object content
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_put_object_from_buffer(const oss_request_options_t *options, 
                                         const aos_string_t *bucket, 
                                         const aos_string_t *object, 
                                         aos_list_t *buffer, 
                                         aos_table_t *headers, 
                                         aos_table_t **resp_headers);

/**
  * @brief  oss put object from file
  * @param[in]  filename  the filename containing object content  
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_put_object_from_file(const oss_request_options_t *options,
                                       const aos_string_t *bucket, 
                                       const aos_string_t *object, 
                                       const aos_string_t *filename,
                                       aos_table_t *headers, 
                                       aos_table_t **resp_headers);

/**
  * @brief  oss get object content to buffer
  * @param[out]  buffer  the buffer containing download object content
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_get_object_to_buffer(const oss_request_options_t *options, 
                                       const aos_string_t *bucket, 
                                       const aos_string_t *object,
                                       aos_table_t *headers, 
                                       aos_list_t *buffer, 
                                       aos_table_t **resp_headers);

/**
  * @brief  oss get object content to file
  * @param[in] filename  the file containing download object content
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_get_object_to_file(const oss_request_options_t *options,
                                     const aos_string_t *bucket, 
                                     const aos_string_t *object,
                                     aos_table_t *headers, 
                                     aos_string_t *filename, 
                                     aos_table_t **resp_headers);

/**
  * @brief  oss head object
  * @param[out]  resp_headers  the response headers containing object meta
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_head_object(const oss_request_options_t *options, 
                              const aos_string_t *bucket, 
                              const aos_string_t *object,
                              aos_table_t *headers, 
                              aos_table_t **resp_headers);

/**
  * @brief  oss delete object
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_delete_object(const oss_request_options_t *options, 
                                const aos_string_t *bucket, 
                                const aos_string_t *object, 
                                aos_table_t **resp_headers);

/**
  * @brief  oss delete objects
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_delete_objects(const oss_request_options_t *options,
                                 const aos_string_t *bucket, 
                                 aos_list_t *object_list, int is_quiet,
                                 aos_table_t **resp_headers, 
                                 aos_list_t *deleted_object_list);

/**
  * @brief  oss delete objects by prefix
  * @param[in]  prefix  prefix of delete objects
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_delete_objects_by_prefix(oss_request_options_t *options,
                                           const aos_string_t *bucket, 
                                           const aos_string_t *prefix);

/**
  * @brief  oss copy object from source object
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_copy_object(const oss_request_options_t *options, 
                              const aos_string_t *source_bucket, 
                              const aos_string_t *source_object, 
                              const aos_string_t *dest_bucket, 
                              const aos_string_t *dest_object, 
                              aos_table_t *headers, 
                              aos_table_t **resp_headers);

/**
  * @brief  oss append object from buffer
  * @param[in]  position  the start postition append after object
  * @param[in]  buffer  the buffer containing appending content  
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_append_object_from_buffer(const oss_request_options_t *options,
                                            const aos_string_t *bucket, 
                                            const aos_string_t *object, 
                                            int64_t position,
                                            aos_list_t *buffer, 
                                            aos_table_t *headers, 
                                            aos_table_t **resp_headers);

/**
  * @brief  oss append object from file
  * @param[in]  position  the start position append after object
  * @param[in]  append_file  the file containing appending content 
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_append_object_from_file(const oss_request_options_t *options,
                                          const aos_string_t *bucket, 
                                          const aos_string_t *object, 
                                          int64_t position,
                                          const aos_string_t *append_file, 
                                          aos_table_t *headers, 
                                          aos_table_t **resp_headers);

/**
  * @brief  gen signed url for oss object api
  * @param[in]  options  the request options including config item and http controllter
  * @param[in]  expires  the expire time in Unix timestamp format
  * @return the signed url 
**/
char *oss_gen_signed_url(const oss_request_options_t *options, 
                         const aos_string_t *bucket,
                         const aos_string_t *object, 
                         int64_t expires, 
                         aos_http_request_t *req);

/**
  * @brief  oss put object from buffer using signed url
  * @param[in]  signed_url  the signed url for object
  * @param[in]  buffer  the buffer containing object content
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_put_object_from_buffer_by_url(const oss_request_options_t *options,
                                                const aos_string_t *signed_url, 
                                                aos_list_t *buffer, 
                                                aos_table_t *headers,
                                                aos_table_t **resp_headers);

/**
  * @brief  oss put object from file using signed_url
  * @param[in]  signed_url  the signed url for object
  * @param[in]  filename  the file containing object content  
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_put_object_from_file_by_url(const oss_request_options_t *options,
                                              const aos_string_t *signed_url, 
                                              aos_string_t *filename, 
                                              aos_table_t *headers,
                                              aos_table_t **resp_headers);

/**
  * @brief  oss get object to buffer using signed url
  * @param[in]  signed_url  the signed url for object
  * @param[in]  buffer  the buffer containing download object content 
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_get_object_to_buffer_by_url(const oss_request_options_t *options,
                                              const aos_string_t *signed_url, 
                                              aos_table_t *headers, 
                                              aos_list_t *buffer, 
                                              aos_table_t **resp_headers);

/**
  * @brief  oss get object to file using signed url
  * @param[in]  signed_url  the signed url for object
  * @param[in]  filename  the file containing download object content 
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_get_object_to_file_by_url(const oss_request_options_t *options,
                                            const aos_string_t *signed_url, 
                                            aos_table_t *headers, 
                                            aos_string_t *filename,
                                            aos_table_t **resp_headers);

/**
  * @brief  oss head object using signed url
  * @param[in]  signed_url  the signed url for object
  * @param[out]  resp_headers  the response headers containing object meta
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_head_object_by_url(const oss_request_options_t *options,
                                     const aos_string_t *signed_url, 
                                     aos_table_t *headers, 
                                     aos_table_t **resp_headers);

/**
  * @brief  oss init multipart upload
  * @param[out] upload_id  the returned upload id 
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_init_multipart_upload(const oss_request_options_t *options, 
                                        const aos_string_t *bucket, 
                                        const aos_string_t *object, 
                                        aos_table_t *headers, 
                                        aos_string_t *upload_id, 
                                        aos_table_t **resp_headers);

/**
  * @brief  oss upload part from buffer
  * @param[in]  upload_id  the upload id
  * @param[in]  part_num  the upload part number
  * @param[in]  buffer  the buffer containing upload part content 
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_upload_part_from_buffer(const oss_request_options_t *options, 
                                          const aos_string_t *bucket, 
                                          const aos_string_t *object, 
                                          const aos_string_t *upload_id, 
                                          int part_num, 
                                          aos_list_t *buffer, 
                                          aos_table_t **resp_headers);

/**
  * @brief  oss upload part from file
  * @param[in]  upload_id  the upload id
  * @param[in]  part_num  the upload part number
  * @param[in]  upload_file  the file params for range upload
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_upload_part_from_file(const oss_request_options_t *options,
                                        const aos_string_t *bucket, 
                                        const aos_string_t *object,
                                        const aos_string_t *upload_id, 
                                        int part_num, 
                                        oss_upload_file_t *upload_file,
                                        aos_table_t **resp_headers);

/**
  * @brief  oss abort multipart upload
  * @param[in]  upload_id  the upload id
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_abort_multipart_upload(const oss_request_options_t *options, 
                                         const aos_string_t *bucket, 
                                         const aos_string_t *object, 
                                         aos_string_t *upload_id, 
                                         aos_table_t **resp_headers);

/**
  * @brief  oss complete multipart upload
  * @param[in]  upload_id  the upload id
  * @param[in]  part_list  the uploaded part list to complete
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_complete_multipart_upload(const oss_request_options_t *options, 
                                            const aos_string_t *bucket, 
                                            const aos_string_t *object, 
                                            const aos_string_t *upload_id, 
                                            aos_list_t *part_list, 
                                            aos_table_t **resp_headers);

/**
  * @brief  oss list upload part with specific upload_id for object
  * @param[in]  upload_id  the upload id
  * @param[in]  params  the list upload part parameters incluing part_number_marker, max_ret
  * @param[out]  params  the output params including next_part_number_marker, part_list, truncated
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_list_upload_part(const oss_request_options_t *options, 
                                   const aos_string_t *bucket, 
                                   const aos_string_t *object, 
                                   const aos_string_t *upload_id, 
                                   oss_list_upload_part_params_t *params, 
                                   aos_table_t **resp_headers);

/**
  * @brief  oss list multipart upload for bucket
  * @param[in]  params  the list multipart upload parameters
  * @param[out]  params  the output params including next_key_marker, next_upload_id_markert, upload_list etc
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_list_multipart_upload(const oss_request_options_t *options, 
                                        const aos_string_t *bucket, 
                                        oss_list_multipart_upload_params_t *params, 
                                        aos_table_t **resp_headers);

/**
  * @brief  oss copy large object using upload part copy
  * @param[in]  params  the upload part copy parameters
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_upload_part_copy(const oss_request_options_t *options,
                                   oss_upload_part_copy_params_t *params, 
                                   aos_table_t *headers, 
                                   aos_table_t **resp_headers);

/**
  * @brief  oss upload file using multipart upload
  * @param[in]  part_size  the part size for multipart upload
  * @return AOSE_OK success, other failure
**/
aos_status_t *oss_upload_file(oss_request_options_t *options,
                              const aos_string_t *bucket, 
                              const aos_string_t *object, 
                              aos_string_t *upload_id,
                              aos_string_t *filename, 
                              int64_t part_size);

OSS_CPP_END

#endif
