#ifndef LIBOSS_XML_H
#define LIBOSS_XML_H

#include <mxml.h>
#include "aos_string.h"
#include "aos_transport.h"
#include "aos_status.h"
#include "oss_define.h"

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

/**
  * @bried  pares acl from xml body for get_bucket_acl
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

OSS_CPP_END

#endif
