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

#define aos_inconsistent_error_status_set(STATUS, RES) do {                     \
        aos_status_set(STATUS, RES, AOS_INCONSISTENT_ERROR_CODE, NULL); \
    } while(0)

extern const char OSS_CANNONICALIZED_HEADER_ACL[];
extern const char OSS_CANNONICALIZED_HEADER_SOURCE[];
extern const char OSS_CANNONICALIZED_HEADER_PREFIX[];
extern const char OSS_CANNONICALIZED_HEADER_DATE[];
extern const char OSS_CANNONICALIZED_HEADER_COPY_SOURCE[];
extern const char OSS_CONTENT_MD5[];
extern const char OSS_CONTENT_TYPE[];
extern const char OSS_CONTENT_LENGTH[];
extern const char OSS_DATE[];
extern const char OSS_AUTHORIZATION[];
extern const char OSS_ACCESSKEYID[];
extern const char OSS_EXPECT[];
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
extern const char OSS_SECURITY_TOKEN[];
extern const char OSS_STS_SECURITY_TOKEN[];
extern const char OSS_OBJECT_TYPE[];
extern const char OSS_NEXT_APPEND_POSITION[];
extern const char OSS_HASH_CRC64_ECMA[];
extern const char OSS_CALLBACK[];
extern const char OSS_CALLBACK_VAR[];
extern const char OSS_PROCESS[];
extern const char OSS_LIFECYCLE[];
extern const char OSS_DELETE[];
extern const char OSS_YES[];
extern const char OSS_OBJECT_TYPE_NORMAL[];
extern const char OSS_OBJECT_TYPE_APPENDABLE[];
extern const char OSS_LIVE_CHANNEL[];
extern const char OSS_LIVE_CHANNEL_STATUS[];
extern const char OSS_COMP[];
extern const char OSS_LIVE_CHANNEL_STAT[];
extern const char OSS_LIVE_CHANNEL_HISTORY[];
extern const char OSS_LIVE_CHANNEL_VOD[];
extern const char OSS_LIVE_CHANNEL_START_TIME[];
extern const char OSS_LIVE_CHANNEL_END_TIME[];
extern const char OSS_PLAY_LIST_NAME[];
extern const char LIVE_CHANNEL_STATUS_DISABLED[];
extern const char LIVE_CHANNEL_STATUS_ENABLED[];
extern const char LIVE_CHANNEL_STATUS_IDLE[];
extern const char LIVE_CHANNEL_STATUS_LIVE[];
extern const char LIVE_CHANNEL_DEFAULT_TYPE[];
extern const char LIVE_CHANNEL_DEFAULT_PLAYLIST[];
extern const int  LIVE_CHANNEL_DEFAULT_FRAG_DURATION;
extern const int  LIVE_CHANNEL_DEFAULT_FRAG_COUNT;
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
    aos_string_t endpoint;
    aos_string_t access_key_id;
    aos_string_t access_key_secret;
    aos_string_t sts_token;
    int is_cname;
    aos_string_t proxy_host;
    int proxy_port;
    aos_string_t proxy_user;
    aos_string_t proxy_passwd;
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

typedef struct {
    int64_t  part_size;  // bytes, default 1MB
    int32_t  thread_num;  // default 1
    int      enable_checkpoint; // default disable, false
    aos_string_t checkpoint_path;  // dafault ./filepath.ucp or ./filepath.dcp
} oss_resumable_clt_params_t;

typedef struct {
    aos_string_t type;
    int32_t frag_duration; 
    int32_t frag_count;
    aos_string_t play_list_name;
}oss_live_channel_target_t;

typedef struct {
    aos_string_t name;
    aos_string_t description;
    aos_string_t status;
    oss_live_channel_target_t target;
} oss_live_channel_configuration_t;

typedef struct {
    aos_list_t node;
    aos_string_t publish_url;
} oss_live_channel_publish_url_t;

typedef struct {
    aos_list_t node;
    aos_string_t play_url;
} oss_live_channel_play_url_t;

typedef struct {
    int32_t width;
    int32_t height;
    int32_t frame_rate;
    int32_t band_width;
    aos_string_t codec;
} oss_video_stat_t;

typedef struct {
    int32_t band_width;
    int32_t sample_rate;
    aos_string_t codec;
} oss_audio_stat_t;

typedef struct {
    aos_string_t pushflow_status;
    aos_string_t connected_time;
    aos_string_t remote_addr;
    oss_video_stat_t video_stat;
    oss_audio_stat_t audio_stat;
} oss_live_channel_stat_t;

typedef struct {
    aos_list_t node;
    aos_string_t name;
    aos_string_t description;
    aos_string_t status;
    aos_string_t last_modified;
    aos_list_t publish_url_list;
    aos_list_t play_url_list;
} oss_live_channel_content_t;

typedef struct {
    aos_string_t prefix;
    aos_string_t marker;
    int max_keys;
    int truncated;
    aos_string_t next_marker;
    aos_list_t live_channel_list;
} oss_list_live_channel_params_t;

typedef struct {
    aos_list_t node;
    aos_string_t start_time;
    aos_string_t end_time;
    aos_string_t remote_addr;
} oss_live_record_content_t;

#endif
