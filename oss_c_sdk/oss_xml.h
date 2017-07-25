#ifndef LIBOSS_XML_H
#define LIBOSS_XML_H

#include <mxml.h>
#include "aos_string.h"
#include "aos_transport.h"
#include "aos_status.h"
#include "oss_define.h"
#include "oss_resumable.h"

OSS_CPP_START

/**
  * @brief  functions for xml body parse 
**/
int get_xmldoc(aos_list_t *bc, mxml_node_t **root);
char *get_xmlnode_value(aos_pool_t *p, mxml_node_t * root, const char *xml_path);

/**
  * @brief  build xml body for complete_multipart_upload
**/
char *build_complete_multipart_upload_xml(aos_pool_t *p, aos_list_t *bc);

/**
  * @brief  build body for complete multipart upload
**/
void build_complete_multipart_upload_body(aos_pool_t *p, aos_list_t *part_list, aos_list_t *body);

/**
  * @brief  build xml body for put lifecycle
**/
char *build_lifecycle_xml(aos_pool_t *p, aos_list_t *lifecycle_rule_list);

/**
  * @brief  build body for put lifecycle
**/
void build_lifecycle_body(aos_pool_t *p, aos_list_t *lifecycle_rule_list, aos_list_t *body);

/**
  * @brief  build xml body for delete objects
**/
char *build_objects_xml(aos_pool_t *p, aos_list_t *object_list, const char *quiet);

/**
  * @brief  build body for delete objects
**/
void build_delete_objects_body(aos_pool_t *p, aos_list_t *object_list, int is_quiet, 
            aos_list_t *body);

mxml_node_t	*set_xmlnode_value_str(mxml_node_t *parent, const char *name, const aos_string_t *value);
mxml_node_t	*set_xmlnode_value_int(mxml_node_t *parent, const char *name, int value);
mxml_node_t	*set_xmlnode_value_int64(mxml_node_t *parent, const char *name, int64_t value);
mxml_node_t	*set_xmlnode_value_uint64(mxml_node_t *parent, const char *name, uint64_t value);

int get_xmlnode_value_str(aos_pool_t *p, mxml_node_t *xml_node, const char *xml_path, aos_string_t *value);
int get_xmlnode_value_int(aos_pool_t *p, mxml_node_t *xml_node, const char *xml_path, int *value);
int get_xmlnode_value_int64(aos_pool_t *p, mxml_node_t *xml_node, const char *xml_path, int64_t *value);
int get_xmlnode_value_uint64(aos_pool_t *p, mxml_node_t *xml_node, const char *xml_path, uint64_t *value);

/**
  * @brief  build xml for checkpoint
**/
char *oss_build_checkpoint_xml(aos_pool_t *p, const oss_checkpoint_t *checkpoint);

/**
  * @bried  parse checkpoint from xml
**/
int oss_checkpoint_parse_from_body(aos_pool_t *p, const char *xml_body, oss_checkpoint_t *checkpoint);

/**
  * @bried  parse acl from xml body for get_bucket_acl
**/
int oss_acl_parse_from_body(aos_pool_t *p, aos_list_t *bc, aos_string_t *oss_acl);

/**
  * @brief parse upload_id from xml body for init multipart upload
**/
int oss_upload_id_parse_from_body(aos_pool_t *p, aos_list_t *bc, aos_string_t *upload_id);

/**
  * @brief parse objects from xml body for list objects
**/
void oss_list_objects_owner_parse(aos_pool_t *p, mxml_node_t *xml_node, oss_list_object_content_t *content);
void oss_list_objects_content_parse(aos_pool_t *p, mxml_node_t *xml_node, oss_list_object_content_t *content);
void oss_list_objects_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
            aos_list_t *object_list);
void oss_list_objects_prefix_parse(aos_pool_t *p, mxml_node_t *root,     
            oss_list_object_common_prefix_t *common_prefix);
void oss_list_objects_common_prefix_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
            aos_list_t *common_prefix_list);
int oss_list_objects_parse_from_body(aos_pool_t *p, aos_list_t *bc, aos_list_t *object_list,
            aos_list_t *common_prefix_list, aos_string_t *marker, int *truncated);

/**
  * @brief parse parts from xml body for list upload part
**/
void oss_list_parts_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
            aos_list_t *part_list);
void oss_list_parts_content_parse(aos_pool_t *p, mxml_node_t *xml_node, oss_list_part_content_t *content);
int oss_list_parts_parse_from_body(aos_pool_t *p, aos_list_t *bc, aos_list_t *part_list, 
            aos_string_t *part_number_marker, int *truncated);

/**
  * @brief  parse uploads from xml body for list multipart upload
**/
void oss_list_multipart_uploads_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
            aos_list_t *upload_list);
void oss_list_multipart_uploads_content_parse(aos_pool_t *p, mxml_node_t *xml_node,
            oss_list_multipart_upload_content_t *content);
int oss_list_multipart_uploads_parse_from_body(aos_pool_t *p, aos_list_t *bc,
            aos_list_t *upload_list, aos_string_t *key_marker,
            aos_string_t *upload_id_marker, int *truncated);

/**
  * @brief parse lifecycle rules from xml body
**/
void oss_lifecycle_rule_expire_parse(aos_pool_t *p, mxml_node_t *xml_node, 
    oss_lifecycle_rule_content_t *content);
void oss_lifecycle_rule_content_parse(aos_pool_t *p, mxml_node_t *xml_node, 
    oss_lifecycle_rule_content_t *content);
void oss_lifecycle_rule_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
    aos_list_t *lifecycle_rule_list);
int oss_lifecycle_rules_parse_from_body(aos_pool_t *p, aos_list_t *bc, aos_list_t *lifecycle_rule_list);

/**
  * @brief parse delete objects contents from xml body
**/
void oss_delete_objects_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
    aos_list_t *object_list);
void oss_object_key_parse(aos_pool_t *p, mxml_node_t * xml_node, oss_object_key_t *content);
int oss_delete_objects_parse_from_body(aos_pool_t *p, aos_list_t *bc, aos_list_t *object_list);

/**
  * @brief  build body for create live channel
**/
char *build_create_live_channel_xml(aos_pool_t *p, oss_live_channel_configuration_t *config);
void build_create_live_channel_body(aos_pool_t *p, oss_live_channel_configuration_t *config, aos_list_t *body);

/**
  * @brief parse create live channel contents from xml body
**/
void oss_publish_url_parse(aos_pool_t *p, mxml_node_t *node, oss_live_channel_publish_url_t *content);
void oss_play_url_parse(aos_pool_t *p, mxml_node_t *node, oss_live_channel_play_url_t *content);
void oss_publish_urls_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
	aos_list_t *publish_xml_list);
void oss_play_urls_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
	aos_list_t *play_xml_list);
void oss_create_live_channel_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *publish_xml_path, 
	aos_list_t *publish_url_list, const char *play_xml_path, aos_list_t *play_url_list);
int oss_create_live_channel_parse_from_body(aos_pool_t *p, aos_list_t *bc, aos_list_t *publish_url_list, 
	aos_list_t *play_url_list);

/**
  * @brief parse live channel info content from xml body
**/
void oss_live_channel_info_target_content_parse(aos_pool_t *p, mxml_node_t *xml_node, oss_live_channel_target_t *target);
void oss_live_channel_info_content_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
    oss_live_channel_configuration_t *info);
int oss_live_channel_info_parse_from_body(aos_pool_t *p, aos_list_t *bc, oss_live_channel_configuration_t *info);

/**
  * @brief parse live channel stat content from xml body
**/
void oss_live_channel_stat_video_content_parse(aos_pool_t *p, mxml_node_t *xml_node, oss_video_stat_t *video_stat);
void oss_live_channel_stat_audio_content_parse(aos_pool_t *p, mxml_node_t *xml_node, oss_audio_stat_t *audio_stat);
void oss_live_channel_stat_content_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path, oss_live_channel_stat_t *stat);
int oss_live_channel_stat_parse_from_body(aos_pool_t *p, aos_list_t *bc, oss_live_channel_stat_t *stat);

/**
  * @brief parse live channel from xml body for list list channel
**/
void oss_list_live_channel_content_parse(aos_pool_t *p, mxml_node_t *xml_node, oss_live_channel_content_t *content);
void oss_list_live_channel_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
    aos_list_t *live_channel_list);
int oss_list_live_channel_parse_from_body(aos_pool_t *p, aos_list_t *bc,
    aos_list_t *live_channel_list, aos_string_t *next_marker, int *truncated);

/**
  * @brief parse live channel history content from xml body
**/
void oss_live_channel_history_content_parse(aos_pool_t *p, mxml_node_t * xml_node, oss_live_record_content_t *content);
void oss_live_channel_history_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
    aos_list_t *live_record_list);
int oss_live_channel_history_parse_from_body(aos_pool_t *p, aos_list_t *bc, aos_list_t *live_record_list);

OSS_CPP_END

#endif
