#ifndef LIBOSS_DEFINE_H
#define LIBOSS_DEFINE_H

#include "aos_string.h"
#include "aos_list.h"
#include "aos_transport.h"

#ifdef __cplusplus
# define OSS_CPP_START extern "C" {
# define OSS_CPP_END }
#else
# define OSS_CPP_START
# define OSS_CPP_END
#endif

#define aos_xml_error_status_set(STATUS, RES) do {                   \
        aos_status_set(STATUS, RES, AOS_XML_PARSE_ERROR_CODE, NULL); \
    } while(0)

#define aos_file_error_status_set(STATUS, RES) do {                   \
        aos_status_set(STATUS, RES, AOS_OPEN_FILE_ERROR_CODE, NULL); \
    } while(0)

extern const char OSS_CANNONICALIZED_HEADER_ACL[];
extern const char OSS_CANNONICALIZED_HEADER_SOURCE[];
extern const char OSS_CANNONICALIZED_HEADER_PREFIX[];
extern const char OSS_CANNONICALIZED_HEADER_DATE[];
extern const char OSS_CANNONICALIZED_HEADER_COPY_SOURCE[];
extern const char OSS_CONTENT_MD5[];
extern const char OSS_CONTENT_TYPE[];
extern const char OSS_DATE[];
extern const char OSS_AUTHORIZATION[];
extern const char OSS_ACCESSKEYID[];
extern const char OSS_EXPIRES[];
extern const char OSS_SIGNATURE[];
extern const char OSS_ACL[];
extern const char OSS_PREFIX[];
extern const char OSS_DELIMITER[];
extern const char OSS_MARKER[];
extern const char OSS_MAX_KEYS[];
extern const char OSS_UPLOADS[];
extern const char OSS_UPLOAD_ID[];
extern const char OSS_MAX_PARTS[];
extern const char OSS_KEY_MARKER[];
extern const char OSS_UPLOAD_ID_MARKER[];
extern const char OSS_MAX_UPLOADS[];
extern const char OSS_PARTNUMBER[];
extern const char OSS_PART_NUMBER_MARKER[];
extern const char OSS_APPEND[];
extern const char OSS_POSITION[];
extern const char OSS_MULTIPART_CONTENT_TYPE[];
extern const char OSS_COPY_SOURCE[];
extern const char OSS_COPY_SOURCE_RANGE[];
extern const char OSS_STS_SECURITY_TOKEN[];
extern const char OSS_LIFECYCLE[];
extern const char OSS_DELETE[];
extern const int OSS_MAX_PART_NUM;
extern const int OSS_PER_RET_NUM;
extern const int MAX_SUFFIX_LEN;

typedef struct oss_lib_curl_initializer_s oss_lib_curl_initializer_t;

/**
 * oss_acl is an ACL that can be specified when an object is created or
 * updated.  Each canned ACL has a predefined value when expanded to a full
 * set of OSS ACL Grants.
 * Private canned ACL gives the owner FULL_CONTROL and no other permissions
 *     are issued
 * Public Read canned ACL gives the owner FULL_CONTROL and all users Read
 *     permission 
 * Public Read Write canned ACL gives the owner FULL_CONTROL and all users
 *     Read and Write permission
 **/
typedef enum {
    OSS_ACL_PRIVATE                  = 0,   /*< private */
    OSS_ACL_PUBLIC_READ              = 1,   /*< public read */
    OSS_ACL_PUBLIC_READ_WRITE        = 2    /*< public read write */
} oss_acl_e;

typedef struct {
    aos_string_t host;  /*< oss hostname */
    int port;           /*< oss port, default 80 */
    aos_string_t id;    /*< oss access_key_id */
    aos_string_t key;   /*< oss access_key_secret */
    int is_oss_domain;  /*< oss hostname is oss domain or not, check by function is_oss_domain */
    aos_string_t sts_token;
} oss_config_t;

typedef struct {
    oss_config_t *config;
    aos_http_controller_t *ctl; /*< aos http controller, more see aos_transport.h */
    aos_pool_t *pool;
} oss_request_options_t;

typedef struct {
    aos_list_t node;
    aos_string_t key;
    aos_string_t last_modified;
    aos_string_t etag;
    aos_string_t size;
    aos_string_t owner_id;
    aos_string_t owner_display_name;
} oss_list_object_content_t;

typedef struct {
    aos_list_t node;
    aos_string_t prefix;
} oss_list_object_common_prefix_t;

typedef struct {
    aos_list_t node;
    aos_string_t key;
    aos_string_t upload_id;
    aos_string_t initiated;
} oss_list_multipart_upload_content_t;

typedef struct {
    aos_list_t node;
    aos_string_t part_number;
    aos_string_t size;
    aos_string_t etag;
    aos_string_t last_modified;
} oss_list_part_content_t;

typedef struct {
    aos_list_t node;
    aos_string_t part_number;
    aos_string_t etag;
} oss_complete_part_content_t;

typedef struct {
    int part_num;
    char *etag;
} oss_upload_part_t;

typedef struct {
    aos_string_t prefix;
    aos_string_t marker;
    aos_string_t delimiter;
    int max_ret;
    int truncated;
    aos_string_t next_marker;
    aos_list_t object_list;
    aos_list_t common_prefix_list;
} oss_list_object_params_t;

typedef struct {
    aos_string_t part_number_marker;
    int max_ret;
    int truncated;
    aos_string_t next_part_number_marker;
    aos_list_t part_list;
} oss_list_upload_part_params_t;

typedef struct {
    aos_string_t prefix;
    aos_string_t key_marker;
    aos_string_t upload_id_marker;
    aos_string_t delimiter;
    int max_ret;
    int truncated;
    aos_string_t next_key_marker;
    aos_string_t next_upload_id_marker;
    aos_list_t upload_list;
} oss_list_multipart_upload_params_t;

typedef struct {
    aos_string_t source_bucket;
    aos_string_t source_object;
    aos_string_t dest_bucket;
    aos_string_t dest_object;
    aos_string_t upload_id;
    int part_num;
    int64_t range_start;
    int64_t range_end;
} oss_upload_part_copy_params_t;

typedef struct {
    aos_string_t filename;  /**< file range read filename */
    int64_t file_pos;   /**< file range read start position */
    int64_t file_last;  /**< file range read last position */
} oss_upload_file_t;

typedef struct {
    aos_list_t node;
    aos_string_t id;
    aos_string_t prefix;
    aos_string_t status;
    int days;
    aos_string_t date;
} oss_lifecycle_rule_content_t;

typedef struct {
    aos_list_t node;
    aos_string_t key;
} oss_object_key_t;

typedef struct {
    char *suffix;
    char *type;
} oss_content_type_t;

#endif
