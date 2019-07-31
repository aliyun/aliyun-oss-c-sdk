#include "aos_string.h"
#include "aos_list.h"
#include "aos_buf.h"
#include "aos_util.h"
#include "aos_log.h"
#include "aos_status.h"
#include "oss_util.h"
#include "oss_auth.h"
#include "oss_xml.h"
#include "oss_define.h"

static int get_truncated_from_xml(aos_pool_t *p, mxml_node_t *xml_node, const char *truncated_xml_path);

int get_truncated_from_xml(aos_pool_t *p, mxml_node_t *xml_node, const char *truncated_xml_path)
{
    char *is_truncated;
    int truncated = 0;
    is_truncated = get_xmlnode_value(p, xml_node, truncated_xml_path);
    if (is_truncated) {
        truncated = strcasecmp(is_truncated, "false") == 0 ? 0 : 1;
    }
    return truncated;
}

static char* new_xml_buff(mxml_node_t *doc);

char* new_xml_buff(mxml_node_t *doc)
{
    int	bytes;				
    char buffer[8192];
    char *s;

    bytes = mxmlSaveString(doc, buffer, sizeof(buffer), MXML_NO_CALLBACK);
    if (bytes <= 0) {
        return (NULL);
    }

    if (bytes < (int)(sizeof(buffer) - 1)) {
        return (strdup(buffer));
    }

    if ((s = malloc(bytes + 1)) == NULL) {
        return (NULL);
    }
    mxmlSaveString(doc, s, bytes + 1, MXML_NO_CALLBACK);

    return (s);
}

int get_xmldoc(aos_list_t *bc, mxml_node_t **root)
{
    int res;

    if (aos_list_empty(bc)) {
        return AOSE_XML_PARSE_ERROR;
    }

    if ((res = aos_parse_xml_body(bc, root)) != AOSE_OK) {
        return AOSE_XML_PARSE_ERROR;
    }

    return AOSE_OK;
}

char *get_xmlnode_value(aos_pool_t *p, mxml_node_t *xml_node, const char *xml_path)
{
    char *value = NULL;
    mxml_node_t *node;
    char *node_content;

    node = mxmlFindElement(xml_node, xml_node, xml_path, NULL, NULL, MXML_DESCEND);
    if (NULL != node && node->child != NULL) {
        node_content = node->child->value.opaque;
        value = apr_pstrdup(p, (char *)node_content);
    }

    return value;
}

int oss_acl_parse_from_body(aos_pool_t *p, aos_list_t *bc, aos_string_t *oss_acl)
{
    int res;
    mxml_node_t *doc = NULL;
    const char xml_path[] = "Grant";
    char *acl;

    res = get_xmldoc(bc, &doc);
    if (res == AOSE_OK) {
        acl = get_xmlnode_value(p, doc, xml_path);
        if (acl) {
            aos_str_set(oss_acl, acl);
        }
        mxmlDelete(doc);
    }

    return res;
}

int oss_location_parse_from_body(aos_pool_t *p, aos_list_t *bc, aos_string_t *oss_location)
{
    int res;
    mxml_node_t *doc = NULL;
    const char xml_path[] = "LocationConstraint";
    char *location;

    res = get_xmldoc(bc, &doc);
    if (res == AOSE_OK) {
        location = get_xmlnode_value(p, doc, xml_path);
        if (location) {
            aos_str_set(oss_location, location);
        }
        mxmlDelete(doc);
    }

    return res;
}

int oss_storage_capacity_parse_from_body(aos_pool_t *p, aos_list_t *bc, long *storage_capacity)
{
    int res;
    mxml_node_t *doc = NULL;
    const char xml_path[] = "StorageCapacity";
    char *capacity_str;

    res = get_xmldoc(bc, &doc);
    if (res == AOSE_OK) {
        capacity_str = get_xmlnode_value(p, doc, xml_path);
        if (capacity_str) {
            *storage_capacity = atol(capacity_str);
        }
        mxmlDelete(doc);
    }

    return res;
}

int oss_logging_parse_from_body(aos_pool_t *p, aos_list_t *bc, oss_logging_config_content_t *rule_content)
{
    const char xml_logging_status_path[] = "BucketLoggingStatus";
    const char xml_logging_state_path[] = "LoggingEnabled";
    const char xml_target_bucket_path[] = "TargetBucket";
    const char xml_log_prefix_path[] = "TargetPrefix";
    mxml_node_t *doc = NULL;
    mxml_node_t *logging_node;
    mxml_node_t *enabled_node;
    int res;

    res = get_xmldoc(bc, &doc);
    if (res == AOSE_OK) {
        logging_node = mxmlFindElement(doc, doc, xml_logging_status_path, NULL, NULL, MXML_DESCEND);
        if (logging_node) {
            enabled_node = mxmlFindElement(logging_node, doc, xml_logging_state_path, NULL, NULL, MXML_DESCEND);
            if (enabled_node) {
                char *prefix = NULL;
                char *target_bucket = NULL;
                rule_content->logging_enabled = 1;

                target_bucket = get_xmlnode_value(p, enabled_node, xml_target_bucket_path);
                if (target_bucket) {
                    aos_str_set(&rule_content->target_bucket, target_bucket);
                }

                prefix =  get_xmlnode_value(p, enabled_node, xml_log_prefix_path);
                if (prefix) {
                    aos_str_set(&rule_content->prefix, prefix);
                }
            }
        }
    }

    mxmlDelete(doc);
    return res;
}


void oss_list_objects_owner_parse(aos_pool_t *p, mxml_node_t *xml_node, oss_list_object_content_t *content)
{
    mxml_node_t *node;
    char *node_content;
    char *owner_id;
    char *owner_display_name;

    node = mxmlFindElement(xml_node, xml_node, "ID",NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        owner_id = apr_pstrdup(p, node_content);
        aos_str_set(&content->owner_id, owner_id);
    }

    node = mxmlFindElement(xml_node, xml_node, "DisplayName", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        owner_display_name = apr_pstrdup(p, node_content);
        aos_str_set(&content->owner_display_name, owner_display_name);
    }
}

void oss_list_objects_content_parse(aos_pool_t *p, mxml_node_t *xml_node, oss_list_object_content_t *content)
{
    char *key;
    char *last_modified;
    char *etag;
    char *size;
    char *node_content;
    mxml_node_t *node;

    node = mxmlFindElement(xml_node, xml_node, "Key", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        key = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->key, key);
    }

    node = mxmlFindElement(xml_node, xml_node, "LastModified", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        last_modified = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->last_modified, last_modified);
    }

    node = mxmlFindElement(xml_node, xml_node, "ETag", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        etag = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->etag, etag);
    }

    node = mxmlFindElement(xml_node, xml_node, "Size", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        size = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->size, size);
    }

    node = mxmlFindElement(xml_node, xml_node, "Owner", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        oss_list_objects_owner_parse(p, node, content);
    }
}

void oss_list_objects_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
    aos_list_t *object_list)
{
    mxml_node_t *content_node;
    oss_list_object_content_t *content;

    content_node = mxmlFindElement(root, root, xml_path, NULL, NULL, MXML_DESCEND);
    for ( ; content_node != NULL; ) {
        content = oss_create_list_object_content(p);
        oss_list_objects_content_parse(p, content_node, content);
        aos_list_add_tail(&content->node, object_list);
        content_node = mxmlFindElement(content_node, root, xml_path, NULL, NULL, MXML_DESCEND);
    }
}

void oss_list_objects_prefix_parse(aos_pool_t *p, mxml_node_t *xml_node, oss_list_object_common_prefix_t *common_prefix)
{
    char *prefix;
    mxml_node_t *node;
    char *node_content;
    
    node = mxmlFindElement(xml_node, xml_node, "Prefix", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        prefix = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&common_prefix->prefix, prefix);
    }
}

void oss_list_objects_common_prefix_parse(aos_pool_t *p, mxml_node_t *xml_node, const char *xml_path,
            aos_list_t *common_prefix_list)
{
    mxml_node_t *node;
    oss_list_object_common_prefix_t *common_prefix;

    node = mxmlFindElement(xml_node, xml_node, xml_path, NULL, NULL, MXML_DESCEND);
    for ( ; node != NULL; ) {
        common_prefix = oss_create_list_object_common_prefix(p);
        oss_list_objects_prefix_parse(p, node, common_prefix);
        aos_list_add_tail(&common_prefix->node, common_prefix_list);
        node = mxmlFindElement(node, xml_node, xml_path, NULL, NULL, MXML_DESCEND);
    }
}

int oss_list_objects_parse_from_body(aos_pool_t *p, aos_list_t *bc,
    aos_list_t *object_list, aos_list_t *common_prefix_list, aos_string_t *marker, int *truncated)
{
    int res;
    mxml_node_t *root;
    const char next_marker_xml_path[] = "NextMarker";
    const char truncated_xml_path[] = "IsTruncated";
    const char buckets_xml_path[] = "Contents";
    const char common_prefix_xml_path[] = "CommonPrefixes";
    char* next_marker;

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        next_marker = get_xmlnode_value(p, root, next_marker_xml_path);
        if (next_marker) {
            aos_str_set(marker, next_marker);
        }

        *truncated = get_truncated_from_xml(p, root, truncated_xml_path);
        
        oss_list_objects_contents_parse(p, root, buckets_xml_path, object_list);
        oss_list_objects_common_prefix_parse(p, root, common_prefix_xml_path, common_prefix_list);

        mxmlDelete(root);
    }
    
    return res;
}

void oss_list_buckets_content_parse(aos_pool_t *p, mxml_node_t *xml_node, aos_list_t *node_list)
{
    char *value, *xml_value;
    mxml_node_t *node; 
    oss_list_bucket_content_t *content;
    content = oss_create_list_bucket_content(p);
    if (content == NULL) {
        aos_error_log("malloc memory for list bucket failed\n");
        return;
    }

    node = mxmlFindElement(xml_node, xml_node, "Name", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        xml_value = node->child->value.opaque;
        value = apr_pstrdup(p, (char *)xml_value);
        aos_str_set(&content->name, value);
    }

    node = mxmlFindElement(xml_node, xml_node, "CreationDate", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        xml_value = node->child->value.opaque;
        value = apr_pstrdup(p, (char *)xml_value);
        aos_str_set(&content->create_date, value);
    }

    node = mxmlFindElement(xml_node, xml_node, "ExtranetEndpoint", NULL, NULL, MXML_DESCEND);
    if (NULL != node && NULL != node->child) {
        xml_value = node->child->value.opaque;
        value = apr_pstrdup(p, (char *)xml_value);
        aos_str_set(&content->extranet_endpoint, value);
    }

    node = mxmlFindElement(xml_node, xml_node, "IntranetEndpoint", NULL, NULL, MXML_DESCEND);
    if (NULL != node && NULL != node->child) {
        xml_value = node->child->value.opaque;
        value = apr_pstrdup(p, (char *)xml_value);
        aos_str_set(&content->intranet_endpoint, value);
    }

    node = mxmlFindElement(xml_node, xml_node, "Location", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        xml_value = node->child->value.opaque;
        value = apr_pstrdup(p, (char *)xml_value);
        aos_str_set(&content->location, value);
    }

    node = mxmlFindElement(xml_node, xml_node, "StorageClass", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        xml_value = node->child->value.opaque;
        value = apr_pstrdup(p, (char *)xml_value);
        aos_str_set(&content->storage_class, value);
    }

    aos_list_add_tail(&content->node, node_list);
}

void oss_list_node_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
     aos_list_t *node_list, NODE_PARSE_FUN parse_funptr)
{
    mxml_node_t *content_node;
    content_node = mxmlFindElement(root, root, xml_path, NULL, NULL, MXML_DESCEND);
    for ( ; content_node != NULL; ) {
        parse_funptr(p, content_node, node_list);
        content_node = mxmlFindElement(content_node, root, xml_path, NULL, NULL, MXML_DESCEND);
    }
}

void oss_list_buckets_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
     aos_list_t *buckets_list)
{
    oss_list_node_contents_parse(p, root, xml_path, buckets_list, oss_list_buckets_content_parse);
}

int oss_list_buckets_parse_from_body(aos_pool_t *p, aos_list_t *bc,
    oss_list_buckets_params_t *params)
{
    int res = AOSE_OK;
    mxml_node_t *root;
    const char next_marker_xml_path[] = "NextMarker";
    const char truncated_xml_path[] = "IsTruncated";
    const char owner_id_xml_path[] = "ID";
    const char owner_name_xml_path[] = "DisplayName";
    const char buckets_xml_path[] = "Bucket";
    char *next_marker, *owner_id, *owner_name;

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        next_marker = get_xmlnode_value(p, root, next_marker_xml_path);
        if (next_marker) {
            aos_str_set(&params->next_marker, next_marker);
        }

        params->truncated = get_truncated_from_xml(p, root, truncated_xml_path);

        owner_id = get_xmlnode_value(p, root, owner_id_xml_path);
        if (owner_id) {
            aos_str_set(&params->owner_id, owner_id);
        }

        owner_name = get_xmlnode_value(p, root, owner_name_xml_path);
        if (owner_name) {
            aos_str_set(&params->owner_name, owner_name);
        }
        
        oss_list_buckets_contents_parse(p, root, buckets_xml_path, &params->bucket_list);
        mxmlDelete(root);
    }
 
    return res;
}

int oss_get_bucket_info_parse_from_body(aos_pool_t *p, aos_list_t *bc,
    oss_bucket_info_t *bucket_info)
{
    int res = AOSE_OK;
    mxml_node_t *root;
    char *value;

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        value = get_xmlnode_value(p, root, "CreationDate");
        if (NULL != value) {
            aos_str_set(&bucket_info->created_date, value);
        }

        value = get_xmlnode_value(p, root, "ExtranetEndpoint");
        if (NULL != value) {
            aos_str_set(&bucket_info->extranet_endpoint, value);
        }

        value = get_xmlnode_value(p, root, "IntranetEndpoint");
        if (NULL != value) {
            aos_str_set(&bucket_info->intranet_endpoint, value);
        }

        value = get_xmlnode_value(p, root, "Location");
        if (NULL != value) {
            aos_str_set(&bucket_info->location, value);
        }
        
        value = get_xmlnode_value(p, root, "DisplayName");
        if (NULL != value) {
            aos_str_set(&bucket_info->owner_name, value);
        }

        value = get_xmlnode_value(p, root, "ID");
        if (NULL != value) {
            aos_str_set(&bucket_info->owner_id, value);
        }
 
        value = get_xmlnode_value(p, root, "Grant");
        if (NULL != value) {
            aos_str_set(&bucket_info->acl, value);
        }

        mxmlDelete(root);
    }
 
    return res;
}

int oss_get_bucket_stat_parse_from_body(aos_pool_t *p, aos_list_t *bc,
    oss_bucket_stat_t *bucket_stat)
{
    int res = AOSE_OK;
    mxml_node_t *root;
    char *value;

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        value = get_xmlnode_value(p, root, "Storage");
        if (NULL != value) {
            bucket_stat->storage_in_bytes = aos_atoui64(value);
        }

        value = get_xmlnode_value(p, root, "ObjectCount");
        if (NULL != value) {
            bucket_stat->object_count = aos_atoui64(value);
        }

        value = get_xmlnode_value(p, root, "MultipartUploadCount");
        if (NULL != value) {
            bucket_stat->multipart_upload_count = aos_atoui64(value);
        }

        mxmlDelete(root);
    }
 
    return res;
}

int oss_get_bucket_website_parse_from_body(aos_pool_t *p, aos_list_t *bc,
    oss_website_config_t *website_config)
{
    int res = AOSE_OK;
    mxml_node_t *root;
    char *value;

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        value = get_xmlnode_value(p, root, "Suffix");
        if (NULL != value) {
            aos_str_set(&website_config->suffix_str, value);
        }

        value = get_xmlnode_value(p, root, "Key");
        if (NULL != value) {
            aos_str_set(&website_config->key_str, value);
        }

        mxmlDelete(root);
    }
 
    return res;
}

void parse_referer_str(aos_pool_t *p, mxml_node_t *xml_node, aos_list_t *referer_config_ptr)
{
    char *value, *node_content;
    oss_referer_config_t *referer_config = (oss_referer_config_t *)referer_config_ptr;
    node_content = xml_node->child->value.opaque;
    value = apr_pstrdup(p, (char *)node_content);
    if (NULL != value) {
        oss_create_and_add_refer(p, referer_config, value);
    }
}

int oss_get_bucket_referer_config_parse_from_body(aos_pool_t *p, aos_list_t *bc,
    oss_referer_config_t *referer_config)
{
    int res = AOSE_OK;
    mxml_node_t *root;
    char *value;

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        value = get_xmlnode_value(p, root, "AllowEmptyReferer");
        if (NULL != value) {
            if (!strncmp(value, "true", 4)) {
                referer_config->allow_empty_referer = 1;
            } else {
                referer_config->allow_empty_referer = 0;
            }
        }

        oss_list_node_contents_parse(p, root, "Referer", (aos_list_t *)referer_config, 
                                     parse_referer_str);
        mxmlDelete(root);
    }
 
    return res;
}

void parse_sub_ctors_rule(aos_pool_t *p, mxml_node_t *xml_node, aos_list_t *sub_rule_list)
{
    char *value, *node_content;
    node_content = xml_node->child->value.opaque;
    value = apr_pstrdup(p, (char *)node_content);
    if (NULL != value) {
        oss_create_sub_cors_rule(p, sub_rule_list, value);
    }
}

void oss_cors_rule_content_parse(aos_pool_t *p, mxml_node_t *xml_node, aos_list_t *node_list)
{
    char *xml_value;
    mxml_node_t *node; 
    oss_cors_rule_t *content;
    content = oss_create_cors_rule(p);
    if (content == NULL) {
        aos_error_log("malloc memory for list bucket failed\n");
        return;
    }

    node = mxmlFindElement(xml_node, xml_node, "MaxAgeSeconds", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        xml_value = node->child->value.opaque;
        content->max_age_seconds = atoi(xml_value);
    }

    oss_list_node_contents_parse(p, xml_node, "AllowedOrigin", &content->allowed_origin_list, 
                                     parse_sub_ctors_rule);
    
    oss_list_node_contents_parse(p, xml_node, "AllowedMethod", &content->allowed_method_list, 
                                     parse_sub_ctors_rule);

    oss_list_node_contents_parse(p, xml_node, "AllowedHeader", &content->allowed_head_list, 
                                     parse_sub_ctors_rule);

    oss_list_node_contents_parse(p, xml_node, "ExposeHeader", &content->expose_head_list, 
                                     parse_sub_ctors_rule);

    aos_list_add_tail(&content->node, node_list);
}

int oss_get_bucket_cors_parse_from_body(aos_pool_t *p, aos_list_t *bc,
    aos_list_t *rule_list)
{
    int res = AOSE_OK;
    mxml_node_t *root;

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        oss_list_node_contents_parse(p, root, "CORSRule", rule_list,
                                     oss_cors_rule_content_parse);
        mxmlDelete(root);
    }
 
    return res;
}

int oss_upload_id_parse_from_body(aos_pool_t *p, aos_list_t *bc, aos_string_t *upload_id)
{
    int res;
    mxml_node_t *root;
    const char xml_path[] = "UploadId";
    char *id;

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        id = get_xmlnode_value(p, root, xml_path);
        if (id) {
            aos_str_set(upload_id, id);
        }
        mxmlDelete(root);
    }

    return res;
}

void oss_list_parts_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path, 
    aos_list_t *part_list)
{
    mxml_node_t *content_node;
    oss_list_part_content_t *content;

    content_node = mxmlFindElement(root, root, xml_path, NULL, NULL, MXML_DESCEND);
    for ( ; content_node != NULL; ) {
        content = oss_create_list_part_content(p);
        oss_list_parts_content_parse(p, content_node, content);
        aos_list_add_tail(&content->node, part_list);
        content_node = mxmlFindElement(content_node, root, xml_path, NULL, NULL, MXML_DESCEND);
    }
}

void oss_list_parts_content_parse(aos_pool_t *p, mxml_node_t *xml_node, oss_list_part_content_t *content)
{
    char *part_number;
    char *last_modified;
    char *etag;
    char *size;
    char *node_content;
    mxml_node_t *node;

    node = mxmlFindElement(xml_node, xml_node, "PartNumber", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        part_number = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->part_number, part_number);
    }

    node = mxmlFindElement(xml_node, xml_node, "LastModified", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        last_modified = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->last_modified, last_modified);
    }

    node = mxmlFindElement(xml_node, xml_node, "ETag", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        etag = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->etag, etag);
    }

    node = mxmlFindElement(xml_node, xml_node, "Size", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        size = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->size, size);
    }
}

int oss_list_parts_parse_from_body(aos_pool_t *p, aos_list_t *bc,
    aos_list_t *part_list, aos_string_t *partnumber_marker, int *truncated)
{
    int res;
    mxml_node_t *root;
    const char next_partnumber_marker_xml_path[] = "NextPartNumberMarker";
    const char truncated_xml_path[] = "IsTruncated";
    const char parts_xml_path[] = "Part";
    char *next_partnumber_marker;

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        next_partnumber_marker = get_xmlnode_value(p, root,
                next_partnumber_marker_xml_path);
        if (next_partnumber_marker) {
            aos_str_set(partnumber_marker, next_partnumber_marker);
        }

        *truncated = get_truncated_from_xml(p, root, truncated_xml_path);

        oss_list_parts_contents_parse(p, root, parts_xml_path, part_list);

        mxmlDelete(root);
    }

    return res;
}

void oss_list_multipart_uploads_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
    aos_list_t *upload_list)
{
    mxml_node_t *content_node;
    oss_list_multipart_upload_content_t *content;

    content_node = mxmlFindElement(root, root, xml_path, NULL, NULL, MXML_DESCEND);
    for ( ; content_node != NULL; ) {
        content = oss_create_list_multipart_upload_content(p);
        oss_list_multipart_uploads_content_parse(p, content_node, content);
        aos_list_add_tail(&content->node, upload_list);
        content_node = mxmlFindElement(content_node, root, xml_path, NULL, NULL, MXML_DESCEND);
    }
}

void oss_list_multipart_uploads_content_parse(aos_pool_t *p, mxml_node_t *xml_node, 
    oss_list_multipart_upload_content_t *content)
{
    char *key;
    char *upload_id;
    char *initiated;
    char *node_content;
    mxml_node_t *node;

    node = mxmlFindElement(xml_node, xml_node, "Key",NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        key = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->key, key);
    }

    node = mxmlFindElement(xml_node, xml_node, "UploadId",NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        upload_id = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->upload_id, upload_id);
    }

    node = mxmlFindElement(xml_node, xml_node, "Initiated",NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        initiated = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->initiated, initiated);
    }
}

int oss_list_multipart_uploads_parse_from_body(aos_pool_t *p, aos_list_t *bc,
    aos_list_t *upload_list, aos_string_t *key_marker,
    aos_string_t *upload_id_marker, int *truncated)
{
    int res;
    mxml_node_t *root;
    const char next_key_marker_xml_path[] = "NextKeyMarker";
    const char next_upload_id_marker_xml_path[] = "NextUploadIdMarker";
    const char truncated_xml_path[] = "IsTruncated";
    const char uploads_xml_path[] = "Upload";
    char *next_key_marker;
    char *next_upload_id_marker;

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        next_key_marker = get_xmlnode_value(p, root, next_key_marker_xml_path);
        if (next_key_marker) {
            aos_str_set(key_marker, next_key_marker);
        }

        next_upload_id_marker = get_xmlnode_value(p, root, next_upload_id_marker_xml_path);
        if (next_upload_id_marker) {
            aos_str_set(upload_id_marker, next_upload_id_marker);
        }

        *truncated = get_truncated_from_xml(p, root, truncated_xml_path);

        oss_list_multipart_uploads_contents_parse(p, root, uploads_xml_path, upload_list);

        mxmlDelete(root);
    }

    return res;
}

char *build_complete_multipart_upload_xml(aos_pool_t *p, aos_list_t *bc)
{
    char *xml_buff;
    char *complete_part_xml;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node;
    oss_complete_part_content_t *content;

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "CompleteMultipartUpload");
    aos_list_for_each_entry(oss_complete_part_content_t, content, bc, node) {
        mxml_node_t *part_node = mxmlNewElement(root_node, "Part");
        mxml_node_t *part_number_node = mxmlNewElement(part_node, "PartNumber");
        mxml_node_t *etag_node = mxmlNewElement(part_node, "ETag");
        mxmlNewText(part_number_node, 0, content->part_number.data);
        mxmlNewText(etag_node, 0, content->etag.data);
    }
    
    xml_buff = new_xml_buff(doc);
    if (xml_buff == NULL) {
        return NULL;
    }
    aos_str_set(&xml_doc, xml_buff);
    complete_part_xml = aos_pstrdup(p, &xml_doc);

    free(xml_buff);
    mxmlDelete(doc);

    return complete_part_xml;
}

void build_complete_multipart_upload_body(aos_pool_t *p, aos_list_t *part_list, aos_list_t *body)
{
    char *complete_multipart_upload_xml;
    aos_buf_t *b;
    
    complete_multipart_upload_xml = build_complete_multipart_upload_xml(p, part_list);
    aos_list_init(body);
    b = aos_buf_pack(p, complete_multipart_upload_xml, strlen(complete_multipart_upload_xml));
    aos_list_add_tail(&b->node, body);
}

char *build_bucket_logging_xml(aos_pool_t *p, oss_logging_config_content_t *content)
{
    char *logging_xml;
    char *xml_buff;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node, *log_node;

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "BucketLoggingStatus");
    log_node = mxmlNewElement(root_node, "LoggingEnabled");
    if (!aos_string_is_empty(&content->target_bucket)) {
        mxml_node_t *target_bucket_node = mxmlNewElement(log_node, "TargetBucket");
        mxmlNewText(target_bucket_node, 0, content->target_bucket.data);
    }

    if (!aos_string_is_empty(&content->prefix)) {
        mxml_node_t *prefix_node = mxmlNewElement(log_node, "TargetPrefix");
        mxmlNewText(prefix_node, 0, content->prefix.data);
    }
    
    xml_buff = new_xml_buff(doc);
    if (xml_buff == NULL) {
        return NULL;
    }
    aos_str_set(&xml_doc, xml_buff);
    logging_xml = aos_pstrdup(p, &xml_doc);
    
    free(xml_buff);
    mxmlDelete(doc);

    return logging_xml;
}

void build_bucket_logging_body(aos_pool_t *p, oss_logging_config_content_t *content, aos_list_t *body)
{
    char *logging_xml;
    aos_buf_t *b;
    logging_xml = build_bucket_logging_xml(p, content);
    aos_list_init(body);
    b = aos_buf_pack(p, logging_xml, strlen(logging_xml));
    aos_list_add_tail(&b->node, body);
}

char *build_lifecycle_xml(aos_pool_t *p, aos_list_t *lifecycle_rule_list)
{
    char *lifecycle_xml;
    char *xml_buff;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node;
    oss_lifecycle_rule_content_t *content;

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "LifecycleConfiguration");
    aos_list_for_each_entry(oss_lifecycle_rule_content_t, content, lifecycle_rule_list, node) {
        oss_tag_content_t *tag = NULL;
        mxml_node_t *rule_node = mxmlNewElement(root_node, "Rule");
        mxml_node_t *id_node = mxmlNewElement(rule_node, "ID");
        mxml_node_t *prefix_node = mxmlNewElement(rule_node, "Prefix");
        mxml_node_t *status_node = mxmlNewElement(rule_node, "Status");
        mxml_node_t *expire_node = mxmlNewElement(rule_node, "Expiration");
        mxmlNewText(id_node, 0, content->id.data);
        mxmlNewText(prefix_node, 0, content->prefix.data);
        mxmlNewText(status_node, 0, content->status.data);
        if (content->days != INT_MAX) {
            char value_str[64];
            mxml_node_t *days_node = mxmlNewElement(expire_node, "Days");
            apr_snprintf(value_str, sizeof(value_str), "%d", content->days);
            mxmlNewText(days_node, 0, value_str);
        } else if (content->date.len != 0 && strcmp(content->date.data, "") != 0) {
            mxml_node_t *date_node = mxmlNewElement(expire_node, "Date");
            mxmlNewText(date_node, 0, content->date.data);
        } else if (content->created_before_date.len != 0 && strcmp(content->created_before_date.data, "") != 0) {
            mxml_node_t *cbd_node = mxmlNewElement(expire_node, "CreatedBeforeDate");
            mxmlNewText(cbd_node, 0, content->created_before_date.data);
        }

        if (content->abort_multipart_upload_dt.days != INT_MAX) {
            char value_str[64];
            mxml_node_t *abort_mulpart_node = mxmlNewElement(rule_node, "AbortMultipartUpload");
            mxml_node_t *abort_days_node = mxmlNewElement(abort_mulpart_node, "Days");
            apr_snprintf(value_str, sizeof(value_str), "%d", content->abort_multipart_upload_dt.days);
            mxmlNewText(abort_days_node, 0, value_str);
        } else if (!aos_string_is_empty(&content->abort_multipart_upload_dt.created_before_date)) {
            mxml_node_t *abort_mulpart_node = mxmlNewElement(rule_node, "AbortMultipartUpload");
            mxml_node_t *abort_date_node = mxmlNewElement(abort_mulpart_node, "CreatedBeforeDate");
            mxmlNewText(abort_date_node, 0, content->abort_multipart_upload_dt.created_before_date.data);
        }
        
        //tag
        aos_list_for_each_entry(oss_tag_content_t, tag, &content->tag_list, node) {
            mxml_node_t *tag_node = mxmlNewElement(rule_node, "Tag");
            mxml_node_t *key_node = mxmlNewElement(tag_node, "Key");
            mxml_node_t *value_node = mxmlNewElement(tag_node, "Value");
            mxmlNewText(key_node, 0, tag->key.data);
            mxmlNewText(value_node, 0, tag->value.data);
        }
    }
    
    xml_buff = new_xml_buff(doc);
    if (xml_buff == NULL) {
        return NULL;
    }
    aos_str_set(&xml_doc, xml_buff);
    lifecycle_xml = aos_pstrdup(p, &xml_doc);
    
    free(xml_buff);
    mxmlDelete(doc);

    return lifecycle_xml;
}

void build_lifecycle_body(aos_pool_t *p, aos_list_t *lifecycle_rule_list, aos_list_t *body)
{
    char *lifecycle_xml;
    aos_buf_t *b;
    lifecycle_xml = build_lifecycle_xml(p, lifecycle_rule_list);
    aos_list_init(body);
    b = aos_buf_pack(p, lifecycle_xml, strlen(lifecycle_xml));
    aos_list_add_tail(&b->node, body);
}

char *build_referer_config_xml(aos_pool_t *p, oss_referer_config_t *referer_config)
{
    char *referer_config_xml;
    char *xml_buff;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node;
    mxml_node_t *sub_node;
    oss_referer_t *referer;

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "RefererConfiguration");
    sub_node = mxmlNewElement(root_node, "AllowEmptyReferer");
    mxmlNewText(sub_node, 0, referer_config->allow_empty_referer ? "true" : "false");
    sub_node = mxmlNewElement(root_node, "RefererList");

    aos_list_for_each_entry(oss_referer_t, referer, &referer_config->referer_list, node) {
        mxml_node_t *referer_node = mxmlNewElement(sub_node, "Referer");
        mxmlNewText(referer_node, 0, referer->referer.data);
    }
    
    xml_buff = new_xml_buff(doc);
    if (xml_buff == NULL) {
        return NULL;
    }
    aos_str_set(&xml_doc, xml_buff);
    referer_config_xml = aos_pstrdup(p, &xml_doc);
    
    free(xml_buff);
    mxmlDelete(doc);

    return referer_config_xml;
}

void build_referer_config_body(aos_pool_t *p, oss_referer_config_t *referer_config, aos_list_t *body)
{
    char *referer_config_xml;
    aos_buf_t *b;
    referer_config_xml = build_referer_config_xml(p, referer_config);
    aos_list_init(body);
    b = aos_buf_pack(p, referer_config_xml, strlen(referer_config_xml));
    aos_list_add_tail(&b->node, body);
}

char *build_cors_rule_xml(aos_pool_t *p, aos_list_t *rule_list)
{
    char *cors_rule_xml;
    char *xml_buff;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node;
    mxml_node_t *sub_node;
    oss_cors_rule_t *cors_rule;

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "CORSConfiguration");
    aos_list_for_each_entry(oss_cors_rule_t, cors_rule, rule_list, node) {
        oss_sub_cors_rule_t *sub_cors_rule;
        sub_node = mxmlNewElement(root_node, "CORSRule");

        aos_list_for_each_entry(oss_sub_cors_rule_t, sub_cors_rule, &cors_rule->allowed_origin_list, node) {
            mxml_node_t *list_node = mxmlNewElement(sub_node, "AllowedOrigin");
            mxmlNewText(list_node, 0, sub_cors_rule->rule.data);
        }

        aos_list_for_each_entry(oss_sub_cors_rule_t, sub_cors_rule, &cors_rule->allowed_method_list, node) {
            mxml_node_t *list_node = mxmlNewElement(sub_node, "AllowedMethod");
            mxmlNewText(list_node, 0, sub_cors_rule->rule.data);
        }

        aos_list_for_each_entry(oss_sub_cors_rule_t, sub_cors_rule, &cors_rule->allowed_head_list, node) {
            mxml_node_t *list_node = mxmlNewElement(sub_node, "AllowedHeader");
            mxmlNewText(list_node, 0, sub_cors_rule->rule.data);
        }

        aos_list_for_each_entry(oss_sub_cors_rule_t, sub_cors_rule, &cors_rule->expose_head_list, node) {
            mxml_node_t *list_node = mxmlNewElement(sub_node, "ExposeHeader");
            mxmlNewText(list_node, 0, sub_cors_rule->rule.data);
        }

        if (cors_rule->max_age_seconds != INT_MAX) {
            char value_str[64];
            mxml_node_t *list_node = mxmlNewElement(sub_node, "MaxAgeSeconds");
            apr_snprintf(value_str, sizeof(value_str), "%d", cors_rule->max_age_seconds);
            mxmlNewText(list_node, 0, value_str);
        }
    }

    xml_buff = new_xml_buff(doc);
    if (xml_buff == NULL) {
        return NULL;
    }
    aos_str_set(&xml_doc, xml_buff);
    cors_rule_xml = aos_pstrdup(p, &xml_doc);
    
    free(xml_buff);
    mxmlDelete(doc);

    return cors_rule_xml;
}

void build_cors_rule_body(aos_pool_t *p, aos_list_t *rule_list, aos_list_t *body)
{
    char *cors_rule_xml;
    aos_buf_t *b;
    cors_rule_xml = build_cors_rule_xml(p, rule_list);
    aos_list_init(body);
    b = aos_buf_pack(p, cors_rule_xml, strlen(cors_rule_xml));
    aos_list_add_tail(&b->node, body);
}

char *build_website_config_xml(aos_pool_t *p, oss_website_config_t *website_config)
{
    char *website_config_xml;
    char *xml_buff;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node;
    mxml_node_t *sub_node, *suffix_node, *key_node;

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "WebsiteConfiguration");
    sub_node = mxmlNewElement(root_node, "IndexDocument");
    suffix_node = mxmlNewElement(sub_node, "Suffix");
    mxmlNewText(suffix_node, 0, website_config->suffix_str.data);
    if (!aos_string_is_empty(&website_config->key_str)) {
        sub_node = mxmlNewElement(root_node, "ErrorDocument");
        key_node = mxmlNewElement(sub_node, "Key");
        mxmlNewText(key_node, 0, website_config->key_str.data);
    }

    xml_buff = new_xml_buff(doc);
    if (xml_buff == NULL) {
        return NULL;
    }
    aos_str_set(&xml_doc, xml_buff);
    website_config_xml = aos_pstrdup(p, &xml_doc);
    
    free(xml_buff);
    mxmlDelete(doc);

    return website_config_xml;
}

void build_website_config_body(aos_pool_t *p, oss_website_config_t *website_config, aos_list_t *body)
{
    char *website_config_xml;
    aos_buf_t *b;
    website_config_xml = build_website_config_xml(p, website_config);
    aos_list_init(body);
    b = aos_buf_pack(p, website_config_xml, strlen(website_config_xml));
    aos_list_add_tail(&b->node, body);
}

char *build_bucket_storage_class_xml(aos_pool_t *p, oss_storage_class_type_e storage_class)
{
    char *bucket_storage_class_xml;
    char *xml_buff;
    const char *storage_class_str;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node;
    mxml_node_t *storage_node;

    storage_class_str = get_oss_storage_class_str(storage_class);
    if (!storage_class_str) {
        return NULL;
    }

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "CreateBucketConfiguration");
    storage_node = mxmlNewElement(root_node, "StorageClass");
    mxmlNewText(storage_node, 0, storage_class_str);
    
    xml_buff = new_xml_buff(doc);
    if (xml_buff == NULL) {
        return NULL;
    }
    aos_str_set(&xml_doc, xml_buff);
    bucket_storage_class_xml = aos_pstrdup(p, &xml_doc);
    
    free(xml_buff);
    mxmlDelete(doc);

    return bucket_storage_class_xml;
}

void build_bucket_storage_class(aos_pool_t *p, oss_storage_class_type_e storage_class, aos_list_t *body)
{
    char *bucket_storage_class_xml;
    aos_buf_t *b;
    bucket_storage_class_xml = build_bucket_storage_class_xml(p, storage_class);
    if (bucket_storage_class_xml)
    {
        aos_list_init(body);
        b = aos_buf_pack(p, bucket_storage_class_xml, strlen(bucket_storage_class_xml));
        aos_list_add_tail(&b->node, body);
    }
}

char *build_bucket_storage_capacity_xml(aos_pool_t *p, long storage_capacity)
{
    char *bucket_storage_capacity_xml;
    char *xml_buff;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node;
    mxml_node_t *storage_node;
    char value_str[64];

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "BucketUserQos");
    apr_snprintf(value_str, sizeof(value_str), "%ld", storage_capacity);
    storage_node = mxmlNewElement(root_node, "StorageCapacity");
    mxmlNewText(storage_node, 0, value_str);

    xml_buff = new_xml_buff(doc);
    if (xml_buff == NULL) {
        return NULL;
    }
    aos_str_set(&xml_doc, xml_buff);
    bucket_storage_capacity_xml = aos_pstrdup(p, &xml_doc);
    
    free(xml_buff);
    mxmlDelete(doc);

    return bucket_storage_capacity_xml;
}

void build_bucket_storage_capacity_body(aos_pool_t *p, long storage_capacity, aos_list_t *body)
{
    char *bucket_storage_capacity_xml;
    aos_buf_t *b;
    bucket_storage_capacity_xml = build_bucket_storage_capacity_xml(p, storage_capacity);
    if (bucket_storage_capacity_xml)
    {
        aos_list_init(body);
        b = aos_buf_pack(p, bucket_storage_capacity_xml, strlen(bucket_storage_capacity_xml));
        aos_list_add_tail(&b->node, body);
    }
}

int oss_lifecycle_rules_parse_from_body(aos_pool_t *p, aos_list_t *bc, aos_list_t *lifecycle_rule_list)
{
    int res;
    mxml_node_t *root = NULL;
    const char rule_xml_path[] = "Rule";

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        oss_lifecycle_rule_contents_parse(p, root, rule_xml_path, lifecycle_rule_list);
        mxmlDelete(root);
    }

    return res;
}

void oss_lifecycle_rule_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
    aos_list_t *lifecycle_rule_list)
{
    mxml_node_t *node;
    oss_lifecycle_rule_content_t *content;

    node = mxmlFindElement(root, root, xml_path, NULL, NULL, MXML_DESCEND);
    for ( ; node != NULL; ) {
        content = oss_create_lifecycle_rule_content(p);
        oss_lifecycle_rule_content_parse(p, node, content);
        aos_list_add_tail(&content->node, lifecycle_rule_list);
        node = mxmlFindElement(node, root, xml_path, NULL, NULL, MXML_DESCEND);
    }
}

void oss_lifecycle_rule_content_parse(aos_pool_t *p, mxml_node_t * xml_node,
    oss_lifecycle_rule_content_t *content)
{
    char *id;
    char *prefix;
    char *status;
    char *node_content;
    mxml_node_t *node;

    node = mxmlFindElement(xml_node, xml_node, "ID",NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        id = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->id, id);
    }

    node = mxmlFindElement(xml_node, xml_node, "Prefix",NULL, NULL,MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        prefix = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->prefix, prefix);
    }

    node = mxmlFindElement(xml_node, xml_node, "Status",NULL, NULL,MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        status = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->status, status);
    }

    node = mxmlFindElement(xml_node, xml_node, "Expiration",NULL, NULL,MXML_DESCEND);
    if (NULL != node) {
        oss_lifecycle_rule_expire_parse(p, node, content);
    }

    node = mxmlFindElement(xml_node, xml_node, "AbortMultipartUpload",NULL, NULL,MXML_DESCEND);
    if (NULL != node) {
        oss_lifecycle_rule_date_parse(p, node, &content->abort_multipart_upload_dt);
    }

    node = mxmlFindElement(xml_node, xml_node, "Tag", NULL, NULL, MXML_DESCEND);
    for (; node != NULL; ) {
        oss_tag_content_t *tag = oss_create_tag_content(p);
        oss_lifecycle_rule_tag_parse(p, node, tag);
        aos_list_add_tail(&tag->node, &content->tag_list);
        node = mxmlFindElement(node, xml_node, "Tag", NULL, NULL, MXML_DESCEND);
    }
}

void oss_lifecycle_rule_date_parse(aos_pool_t *p, mxml_node_t * xml_node,
    oss_lifecycle_rule_date_t *rule_date)
{
    char* days;
    char *created_before_date;
    mxml_node_t *node;
    char *node_content;

    node = mxmlFindElement(xml_node, xml_node, "Days", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        days = apr_pstrdup(p, (char *)node_content);
        rule_date->days = atoi(days);
    }

    node = mxmlFindElement(xml_node, xml_node, "CreatedBeforeDate", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        created_before_date = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&rule_date->created_before_date, created_before_date);
    } 
}


void oss_lifecycle_rule_expire_parse(aos_pool_t *p, mxml_node_t * xml_node,
    oss_lifecycle_rule_content_t *content)
{
    char* days;
    char *date;
    char *created_before_date;
    mxml_node_t *node;
    char *node_content;

    node = mxmlFindElement(xml_node, xml_node, "Days", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        days = apr_pstrdup(p, (char *)node_content);
        content->days = atoi(days);
    }

    node = mxmlFindElement(xml_node, xml_node, "Date", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        date = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->date, date);
    }

    node = mxmlFindElement(xml_node, xml_node, "CreatedBeforeDate", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        created_before_date = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->created_before_date, created_before_date);
    } 
}

void oss_lifecycle_rule_tag_parse(aos_pool_t *p, mxml_node_t * xml_node,
    oss_tag_content_t *tag)
{
    mxml_node_t *node;
    char *node_content;

    node = mxmlFindElement(xml_node, xml_node, "Key", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        char *key;
        node_content = node->child->value.opaque;
        key = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&tag->key, key);
    }

    node = mxmlFindElement(xml_node, xml_node, "Value", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        char *value;
        node_content = node->child->value.opaque;
        value = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&tag->value, value);
    }
}

void oss_delete_objects_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
    aos_list_t *object_list)
{
    mxml_node_t *node;

    node = mxmlFindElement(root, root, xml_path, NULL, NULL, MXML_DESCEND);
    for ( ; node != NULL; ) {
        oss_object_key_t *content = oss_create_oss_object_key(p);
        oss_object_key_parse(p, node, content);
        aos_list_add_tail(&content->node, object_list);
        node = mxmlFindElement(node, root, xml_path, NULL, NULL, MXML_DESCEND);
    }
}

void oss_object_key_parse(aos_pool_t *p, mxml_node_t * xml_node,
    oss_object_key_t *content)
{   
    char *key;
    char *encoded_key;
    char *node_content;
    mxml_node_t *node;
    
    node = mxmlFindElement(xml_node, xml_node, "Key",NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        encoded_key = (char*)node_content;
        key = (char *) aos_palloc(p, strlen(encoded_key));
        aos_url_decode(encoded_key, key);
        aos_str_set(&content->key, key);
    }
}

int oss_delete_objects_parse_from_body(aos_pool_t *p, aos_list_t *bc, aos_list_t *object_list)
{
    int res;
    mxml_node_t *root = NULL;
    const char deleted_xml_path[] = "Deleted";

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        oss_delete_objects_contents_parse(p, root, deleted_xml_path, object_list);
        mxmlDelete(root);
    }

    return res;
}

void oss_publish_url_parse(aos_pool_t *p, mxml_node_t *node, oss_live_channel_publish_url_t *content)
{   
    char *url;
    char *node_content;
    
    if (NULL != node) {
        node_content = node->child->value.opaque;
        url = apr_pstrdup(p, node_content);
        aos_str_set(&content->publish_url, url);
    }
}

void oss_play_url_parse(aos_pool_t *p, mxml_node_t *node, oss_live_channel_play_url_t *content)
{   
    char *url;
    char *node_content;
    
    if (NULL != node) {
        node_content = node->child->value.opaque;
        url = apr_pstrdup(p, node_content);
        aos_str_set(&content->play_url, url);
    }
}

void oss_publish_urls_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
    aos_list_t *publish_xml_list)
{
    mxml_node_t *node;

    node = mxmlFindElement(root, root, xml_path, NULL, NULL, MXML_DESCEND);
    for ( ; node != NULL; ) {
        oss_live_channel_publish_url_t *content = oss_create_live_channel_publish_url(p);
        oss_publish_url_parse(p, node, content);
        aos_list_add_tail(&content->node, publish_xml_list);
        node = mxmlFindElement(node, root, xml_path, NULL, NULL, MXML_DESCEND);
    }
}

void oss_play_urls_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
    aos_list_t *play_xml_list)
{
    mxml_node_t *node;

    node = mxmlFindElement(root, root, xml_path, NULL, NULL, MXML_DESCEND);
    for ( ; node != NULL; ) {
        oss_live_channel_play_url_t *content = oss_create_live_channel_play_url(p);
        oss_play_url_parse(p, node, content);
        aos_list_add_tail(&content->node, play_xml_list);
        node = mxmlFindElement(node, root, xml_path, NULL, NULL, MXML_DESCEND);
    }
}

void oss_create_live_channel_content_parse(aos_pool_t *p, mxml_node_t *root,
    const char *publish_xml_path, aos_list_t *publish_url_list, 
    const char *play_xml_path, aos_list_t *play_url_list)
{
    mxml_node_t *node;
    const char url_xml_path[] = "Url";

    node = mxmlFindElement(root, root, publish_xml_path, NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        oss_publish_urls_contents_parse(p, node, url_xml_path, publish_url_list);
    }

    node = mxmlFindElement(root, root, play_xml_path, NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        oss_play_urls_contents_parse(p, node, url_xml_path, play_url_list);
    }
}    

int oss_create_live_channel_parse_from_body(aos_pool_t *p, aos_list_t *bc,
    aos_list_t *publish_url_list, aos_list_t *play_url_list)
{
    int res;
    mxml_node_t *root = NULL;
    const char publish_urls_xml_path[] = "PublishUrls";
    const char play_urls_xml_path[] = "PlayUrls";

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        oss_create_live_channel_content_parse(p, root, publish_urls_xml_path, publish_url_list,
            play_urls_xml_path, play_url_list);
        mxmlDelete(root);
    }

    return res;
}

char *build_create_live_channel_xml(aos_pool_t *p, oss_live_channel_configuration_t *config)
{
    char *xml_buff;
    char *complete_part_xml;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node;
    char value_str[64];
    mxml_node_t *description_node;
    mxml_node_t *status_node;
    mxml_node_t *target_node;
    mxml_node_t *type_node;
    mxml_node_t *frag_duration_node;
    mxml_node_t *frag_count_node;
    mxml_node_t *play_list_node;

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "LiveChannelConfiguration");

    description_node = mxmlNewElement(root_node, "Description");
    mxmlNewText(description_node, 0, config->description.data);

    status_node = mxmlNewElement(root_node, "Status");
    mxmlNewText(status_node, 0, config->status.data);

    // target
    target_node = mxmlNewElement(root_node, "Target");
    type_node = mxmlNewElement(target_node, "Type");
    mxmlNewText(type_node, 0, config->target.type.data);

    apr_snprintf(value_str, sizeof(value_str), "%d", config->target.frag_duration);
    frag_duration_node = mxmlNewElement(target_node, "FragDuration");
    mxmlNewText(frag_duration_node, 0, value_str);

    apr_snprintf(value_str, sizeof(value_str), "%d", config->target.frag_count);
    frag_count_node = mxmlNewElement(target_node, "FragCount");
    mxmlNewText(frag_count_node, 0, value_str);

    play_list_node = mxmlNewElement(target_node, "PlaylistName");
    mxmlNewText(play_list_node, 0, config->target.play_list_name.data);

    // dump
	xml_buff = new_xml_buff(doc);
	if (xml_buff == NULL) {
		return NULL;
	}
	aos_str_set(&xml_doc, xml_buff);
	complete_part_xml = aos_pstrdup(p, &xml_doc);

	free(xml_buff);
	mxmlDelete(doc);

    return complete_part_xml;
}

void build_create_live_channel_body(aos_pool_t *p, oss_live_channel_configuration_t *config, aos_list_t *body)
{
    char *live_channel_xml;
    aos_buf_t *b;

    live_channel_xml = build_create_live_channel_xml(p, config);
    aos_list_init(body);
    b = aos_buf_pack(p, live_channel_xml, strlen(live_channel_xml));
    aos_list_add_tail(&b->node, body);
}


void oss_live_channel_info_target_content_parse(aos_pool_t *p, mxml_node_t *xml_node, oss_live_channel_target_t *target)
{
    char *type;
    char *frag_duration;
    char *frag_count;
    char *play_list;
    char *node_content;
    mxml_node_t *node;

    node = mxmlFindElement(xml_node, xml_node, "Type", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        type = apr_pstrdup(p, node_content);
        aos_str_set(&target->type, type);
    }

    node = mxmlFindElement(xml_node, xml_node, "FragDuration", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        frag_duration = apr_pstrdup(p, node_content);
        target->frag_duration = atoi(frag_duration);
    }

    node = mxmlFindElement(xml_node, xml_node, "FragCount", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        frag_count = apr_pstrdup(p, node_content);
        target->frag_count = atoi(frag_count);
    }

    node = mxmlFindElement(xml_node, xml_node, "PlaylistName",NULL, NULL,MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        play_list = apr_pstrdup(p, node_content);
        aos_str_set(&target->play_list_name, play_list);
    }
}

void oss_live_channel_info_content_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
    oss_live_channel_configuration_t *info)
{
    mxml_node_t *cofig_node;
    mxml_node_t *target_node;

    cofig_node = mxmlFindElement(root, root, xml_path, NULL, NULL, MXML_DESCEND);
    if (NULL != cofig_node) {
        char *description;
        char *status;
        char *node_content;
        mxml_node_t *node;

        node = mxmlFindElement(cofig_node, cofig_node, "Description", NULL, NULL, MXML_DESCEND);
        if (NULL != node) {
            node_content = node->child->value.opaque;
            description = apr_pstrdup(p, node_content);
            aos_str_set(&info->description, description);
        }

        node = mxmlFindElement(cofig_node, cofig_node, "Status", NULL, NULL, MXML_DESCEND);
        if (NULL != node) {
            node_content = node->child->value.opaque;
            status = apr_pstrdup(p, node_content);
            aos_str_set(&info->status, status);
        }

        target_node = mxmlFindElement(cofig_node, cofig_node, "Target", NULL, NULL, MXML_DESCEND);
        if (NULL != target_node) {
            oss_live_channel_info_target_content_parse(p, target_node, &info->target);
        }
    }
}

int oss_live_channel_info_parse_from_body(aos_pool_t *p, aos_list_t *bc, oss_live_channel_configuration_t *info)
{
    int res;
    mxml_node_t *root;
    const char xml_path[] = "LiveChannelConfiguration";

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        oss_live_channel_info_content_parse(p, root, xml_path, info);
        mxmlDelete(root);
    }

    return res;
}

void oss_live_channel_stat_video_content_parse(aos_pool_t *p, mxml_node_t *xml_node, oss_video_stat_t *video_stat)
{
    char *width;
    char *height;
    char *frame_rate;
    char *band_width;
    char *codec;
    char *node_content;
    mxml_node_t *node;

    node = mxmlFindElement(xml_node, xml_node, "Width", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        width = apr_pstrdup(p, node_content);
        video_stat->width = atoi(width);
    }

    node = mxmlFindElement(xml_node, xml_node, "Height", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        height = apr_pstrdup(p, node_content);
        video_stat->height = atoi(height);
    }

    node = mxmlFindElement(xml_node, xml_node, "FrameRate", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        frame_rate = apr_pstrdup(p, node_content);
        video_stat->frame_rate = atoi(frame_rate);
    }

    node = mxmlFindElement(xml_node, xml_node, "Bandwidth", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        band_width = apr_pstrdup(p, node_content);
        video_stat->band_width = atoi(band_width);
    }

    node = mxmlFindElement(xml_node, xml_node, "Codec", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        codec = apr_pstrdup(p, node_content);
        aos_str_set(&video_stat->codec, codec);
    }
}

void oss_live_channel_stat_audio_content_parse(aos_pool_t *p, mxml_node_t *xml_node, oss_audio_stat_t *audio_stat)
{
    char *band_width;
    char *sample_rate;
    char *codec;
    char *node_content;
    mxml_node_t *node;

    node = mxmlFindElement(xml_node, xml_node, "Bandwidth", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        band_width = apr_pstrdup(p, node_content);
        audio_stat->band_width = atoi(band_width);
    }

    node = mxmlFindElement(xml_node, xml_node, "SampleRate", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        sample_rate = apr_pstrdup(p, node_content);
        audio_stat->sample_rate = atoi(sample_rate);
    }

    node = mxmlFindElement(xml_node, xml_node, "Codec", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        codec = apr_pstrdup(p, node_content);
        aos_str_set(&audio_stat->codec, codec);
    }
}

void oss_live_channel_stat_content_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path, oss_live_channel_stat_t *stat)
{
    mxml_node_t *stat_node;

    stat_node = mxmlFindElement(root, root, xml_path, NULL, NULL, MXML_DESCEND);
    if (NULL != stat_node) {
        char *status;
        char *connected_time;
        char *remote_addr;
        char *node_content;
        mxml_node_t *node;

        node = mxmlFindElement(stat_node, stat_node, "Status", NULL, NULL, MXML_DESCEND);
        if (NULL != node) {
            node_content = node->child->value.opaque;
            status = apr_pstrdup(p, (char *)node_content);
            aos_str_set(&stat->pushflow_status, status);
        }

        node = mxmlFindElement(stat_node, stat_node, "ConnectedTime", NULL, NULL, MXML_DESCEND);
        if (NULL != node) {
            node_content = node->child->value.opaque;
            connected_time = apr_pstrdup(p, (char *)node_content);
            aos_str_set(&stat->connected_time, connected_time);
        }

        node = mxmlFindElement(stat_node, stat_node, "RemoteAddr", NULL, NULL, MXML_DESCEND);
        if (NULL != node) {
            node_content = node->child->value.opaque;
            remote_addr = apr_pstrdup(p, (char *)node_content);
            aos_str_set(&stat->remote_addr, remote_addr);
        }

        node = mxmlFindElement(stat_node, stat_node, "Video", NULL, NULL, MXML_DESCEND);
        if (NULL != node) {
            oss_live_channel_stat_video_content_parse(p, node, &stat->video_stat);
        }

        node = mxmlFindElement(stat_node, stat_node, "Audio", NULL, NULL, MXML_DESCEND);
        if (NULL != node) {
            oss_live_channel_stat_audio_content_parse(p, node, &stat->audio_stat);
        }
    }
}

int oss_live_channel_stat_parse_from_body(aos_pool_t *p, aos_list_t *bc, oss_live_channel_stat_t *stat)
{
    int res;
    mxml_node_t *root;
    const char xml_path[] = "LiveChannelStat";

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        oss_live_channel_stat_content_parse(p, root, xml_path, stat);
        mxmlDelete(root);
    }

    return res;
}

void oss_list_live_channel_content_parse(aos_pool_t *p, mxml_node_t *xml_node, oss_live_channel_content_t *content)
{
    char *name;
    char *description;
    char *status;
    char *last_modified;
    char *node_content;
    mxml_node_t *node;

    node = mxmlFindElement(xml_node, xml_node, "Name", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        name = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->name, name);
    }

    node = mxmlFindElement(xml_node, xml_node, "Description", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        if (NULL != node->child) {
            node_content = node->child->value.opaque;
            description = apr_pstrdup(p, (char *)node_content);
            aos_str_set(&content->description, description);
        } else {
            aos_str_set(&content->description, "");
        }
    }

    node = mxmlFindElement(xml_node, xml_node, "Status", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        status = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->status, status);
    }

    node = mxmlFindElement(xml_node, xml_node, "LastModified", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        last_modified = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->last_modified, last_modified);
    }

    node = mxmlFindElement(xml_node, xml_node, "PublishUrls", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        oss_publish_urls_contents_parse(p, node, "Url", &content->publish_url_list);
    }

    node = mxmlFindElement(xml_node, xml_node, "PlayUrls", NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        oss_play_urls_contents_parse(p, node, "Url", &content->play_url_list);
    }
}

void oss_list_live_channel_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
    aos_list_t *live_channel_list)
{
    mxml_node_t *content_node;
    oss_live_channel_content_t *content;

    content_node = mxmlFindElement(root, root, xml_path, NULL, NULL, MXML_DESCEND);
    for ( ; content_node != NULL; ) {
        content = oss_create_list_live_channel_content(p);
        oss_list_live_channel_content_parse(p, content_node, content);
        aos_list_add_tail(&content->node, live_channel_list);
        content_node = mxmlFindElement(content_node, root, xml_path, NULL, NULL, MXML_DESCEND);
    }
}

int oss_list_live_channel_parse_from_body(aos_pool_t *p, aos_list_t *bc,
    aos_list_t *live_channel_list, aos_string_t *next_marker, int *truncated)
{
    int res;
    mxml_node_t *root;
    const char next_marker_xml_path[] = "NextMarker";
    const char truncated_xml_path[] = "IsTruncated";
    const char live_channel_xml_path[] = "LiveChannel";
    char *next_partnumber_marker;

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        next_partnumber_marker = get_xmlnode_value(p, root, next_marker_xml_path);
        if (next_partnumber_marker) {
            aos_str_set(next_marker, next_partnumber_marker);
        }

        *truncated = get_truncated_from_xml(p, root, truncated_xml_path);

        oss_list_live_channel_contents_parse(p, root, live_channel_xml_path, live_channel_list);

        mxmlDelete(root);
    }

    return res;
}

void oss_live_channel_history_content_parse(aos_pool_t *p, mxml_node_t * xml_node,
    oss_live_record_content_t *content)
{
    char *start_time;
    char *end_time;
    char *remote_addr;
    char *node_content;
    mxml_node_t *node;

    node = mxmlFindElement(xml_node, xml_node, "StartTime",NULL, NULL, MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        start_time = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->start_time, start_time);
    }

    node = mxmlFindElement(xml_node, xml_node, "EndTime",NULL, NULL,MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        end_time = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->end_time, end_time);
    }

    node = mxmlFindElement(xml_node, xml_node, "RemoteAddr",NULL, NULL,MXML_DESCEND);
    if (NULL != node) {
        node_content = node->child->value.opaque;
        remote_addr = apr_pstrdup(p, (char *)node_content);
        aos_str_set(&content->remote_addr, remote_addr);
    }
}

void oss_live_channel_history_contents_parse(aos_pool_t *p, mxml_node_t *root, const char *xml_path,
    aos_list_t *live_record_list)
{
    mxml_node_t *node;
    oss_live_record_content_t *content;

    node = mxmlFindElement(root, root, xml_path, NULL, NULL, MXML_DESCEND);
    for ( ; node != NULL; ) {
        content = oss_create_live_record_content(p);
        oss_live_channel_history_content_parse(p, node, content);
        aos_list_add_tail(&content->node, live_record_list);
        node = mxmlFindElement(node, root, xml_path, NULL, NULL, MXML_DESCEND);
    }
}

int oss_live_channel_history_parse_from_body(aos_pool_t *p, aos_list_t *bc, aos_list_t *live_record_list)
{
    int res;
    mxml_node_t *root = NULL;
    const char rule_xml_path[] = "LiveRecord";

    res = get_xmldoc(bc, &root);
    if (res == AOSE_OK) {
        oss_live_channel_history_contents_parse(p, root, rule_xml_path, live_record_list);
        mxmlDelete(root);
    }

    return res;
}

char *build_objects_xml(aos_pool_t *p, aos_list_t *object_list, const char *quiet)
{
    char *object_xml;
    char *xml_buff;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node;
    oss_object_key_t *content;
    mxml_node_t *quiet_node;

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "Delete");
    quiet_node = mxmlNewElement(root_node, "Quiet");
    mxmlNewText(quiet_node, 0, quiet);
    aos_list_for_each_entry(oss_object_key_t, content, object_list, node) {
        mxml_node_t *object_node = mxmlNewElement(root_node, "Object");
        mxml_node_t *key_node = mxmlNewElement(object_node, "Key");
        mxmlNewText(key_node, 0, content->key.data);
    }

    xml_buff = new_xml_buff(doc);
    if (xml_buff == NULL) {
        return NULL;
    }
    aos_str_set(&xml_doc, xml_buff);
    object_xml = aos_pstrdup(p, &xml_doc);

    free(xml_buff);
    mxmlDelete(doc);

    return object_xml;
}

void build_delete_objects_body(aos_pool_t *p, aos_list_t *object_list, int is_quiet, aos_list_t *body)
{
    char *objects_xml;
    aos_buf_t *b;
    char *quiet;
    quiet = is_quiet > 0 ? "true": "false";
    objects_xml = build_objects_xml(p, object_list, quiet);
    aos_list_init(body);
    b = aos_buf_pack(p, objects_xml, strlen(objects_xml));
    aos_list_add_tail(&b->node, body);
}

mxml_node_t	*set_xmlnode_value_str(mxml_node_t *parent, const char *name, const aos_string_t *value)
{
    mxml_node_t *node;
    char buff[AOS_MAX_XML_NODE_VALUE_LEN];
    node = mxmlNewElement(parent, name);
    apr_snprintf(buff, AOS_MAX_XML_NODE_VALUE_LEN, "%.*s", value->len, value->data);
    return mxmlNewText(node, 0, buff);
}

mxml_node_t	*set_xmlnode_value_int(mxml_node_t *parent, const char *name, int value)
{
    mxml_node_t *node;
    char buff[AOS_MAX_INT64_STRING_LEN];
    node = mxmlNewElement(parent, name);
    apr_snprintf(buff, AOS_MAX_INT64_STRING_LEN, "%d", value);
    return mxmlNewText(node, 0, buff);
}

mxml_node_t	*set_xmlnode_value_int64(mxml_node_t *parent, const char *name, int64_t value)
{
    mxml_node_t *node;
    char buff[AOS_MAX_INT64_STRING_LEN];
    node = mxmlNewElement(parent, name);
    apr_snprintf(buff, AOS_MAX_INT64_STRING_LEN, "%" APR_INT64_T_FMT, value);
    return mxmlNewText(node, 0, buff);
}

mxml_node_t	*set_xmlnode_value_uint64(mxml_node_t *parent, const char *name, uint64_t value)
{
    mxml_node_t *node;
    char buff[AOS_MAX_UINT64_STRING_LEN];
    node = mxmlNewElement(parent, name);
    apr_snprintf(buff, AOS_MAX_UINT64_STRING_LEN, "%" APR_UINT64_T_FMT, value);
    return mxmlNewText(node, 0, buff);
}

int get_xmlnode_value_str(aos_pool_t *p, mxml_node_t *xml_node, const char *xml_path, aos_string_t *value)
{
    char *node_content;
    node_content = get_xmlnode_value(p, xml_node, xml_path);
    if (NULL == node_content) {
        return AOS_FALSE;
    }
    aos_str_set(value, node_content);
    return AOS_TRUE;
}

int get_xmlnode_value_int(aos_pool_t *p, mxml_node_t *xml_node, const char *xml_path, int *value)
{
    char *node_content;
    node_content = get_xmlnode_value(p, xml_node, xml_path);
    if (NULL == node_content) {
        return AOS_FALSE;
    }
    *value = atoi(node_content);
    return AOS_TRUE;
}

int get_xmlnode_value_int64(aos_pool_t *p, mxml_node_t *xml_node, const char *xml_path, int64_t *value)
{
    char *node_content;
    node_content = get_xmlnode_value(p, xml_node, xml_path);
    if (NULL == node_content) {
        return AOS_FALSE;
    }
    *value = aos_atoi64(node_content);
    return AOS_TRUE;
}

int get_xmlnode_value_uint64(aos_pool_t *p, mxml_node_t *xml_node, const char *xml_path, uint64_t *value)
{
    char *node_content;
    node_content = get_xmlnode_value(p, xml_node, xml_path);
    if (NULL == node_content) {
        return AOS_FALSE;
    }
    *value = aos_atoui64(node_content);
    return AOS_TRUE;
}

char *oss_build_checkpoint_xml(aos_pool_t *p, const oss_checkpoint_t *checkpoint)
{
    char *checkpoint_xml;
    char *xml_buff;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node;
    mxml_node_t *local_node;
    mxml_node_t *object_node;
    mxml_node_t *cpparts_node;
    mxml_node_t *parts_node;
    int i = 0;

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "Checkpoint");

    // MD5
    set_xmlnode_value_str(root_node, "MD5", &checkpoint->md5);

    // Type
    set_xmlnode_value_int(root_node, "Type", checkpoint->cp_type);

    // LocalFile
    local_node = mxmlNewElement(root_node, "LocalFile");
    // LocalFile.Path
    set_xmlnode_value_str(local_node, "Path", &checkpoint->file_path);
    // LocalFile.Size
    set_xmlnode_value_int64(local_node, "Size", checkpoint->file_size);
    // LocalFile.LastModified
    set_xmlnode_value_int64(local_node, "LastModified", checkpoint->file_last_modified);
    // LocalFile.MD5
    set_xmlnode_value_str(local_node, "MD5", &checkpoint->file_md5);

    // Object
    object_node = mxmlNewElement(root_node, "Object");
    // Object.Key
    set_xmlnode_value_str(object_node, "Key", &checkpoint->object_name);
    // Object.Size
    set_xmlnode_value_int64(object_node, "Size", checkpoint->object_size);
    // Object.LastModified
    set_xmlnode_value_str(object_node, "LastModified", &checkpoint->object_last_modified);
    // Object.ETag
    set_xmlnode_value_str(object_node, "ETag", &checkpoint->object_etag);

    // UploadId
    set_xmlnode_value_str(root_node, "UploadId", &checkpoint->upload_id);

    // CpParts
    cpparts_node = mxmlNewElement(root_node, "CPParts");
    // CpParts.Number
    set_xmlnode_value_int(cpparts_node, "Number", checkpoint->part_num);
    // CpParts.Size
    set_xmlnode_value_int64(cpparts_node, "Size", checkpoint->part_size);
    // CpParts.Parts
    parts_node = mxmlNewElement(cpparts_node, "Parts");
    for (i = 0; i < checkpoint->part_num; i++) {
        mxml_node_t *part_node = mxmlNewElement(parts_node, "Part");
        set_xmlnode_value_int(part_node, "Index", checkpoint->parts[i].index);
        set_xmlnode_value_int64(part_node, "Offset", checkpoint->parts[i].offset);
        set_xmlnode_value_int64(part_node, "Size", checkpoint->parts[i].size);
        set_xmlnode_value_int(part_node, "Completed", checkpoint->parts[i].completed);
        set_xmlnode_value_str(part_node, "ETag", &checkpoint->parts[i].etag);
        set_xmlnode_value_uint64(part_node, "Crc64", checkpoint->parts[i].crc64);
    }

    // dump
    xml_buff = new_xml_buff(doc);
    if (xml_buff == NULL) {
        return NULL;
    }
    aos_str_set(&xml_doc, xml_buff);
    checkpoint_xml = aos_pstrdup(p, &xml_doc);

    free(xml_buff);
    mxmlDelete(doc);

    return checkpoint_xml;
}

int oss_checkpoint_parse_from_body(aos_pool_t *p, const char *xml_body, oss_checkpoint_t *checkpoint)
{
    mxml_node_t *root;
    mxml_node_t *local_node;
    mxml_node_t *object_node;
    mxml_node_t *cpparts_node;
    mxml_node_t *parts_node;
    mxml_node_t *node;
    int index = 0;

    root = mxmlLoadString(NULL, xml_body, MXML_OPAQUE_CALLBACK);
    if (NULL == root) {
        return AOSE_XML_PARSE_ERROR; 
    }

    // MD5
    get_xmlnode_value_str(p, root, "MD5", &checkpoint->md5);

    // Type
    get_xmlnode_value_int(p, root, "Type", &checkpoint->cp_type);

    // LocalFile
    local_node = mxmlFindElement(root, root, "LocalFile", NULL, NULL, MXML_DESCEND);
    // LocalFile.Path
    get_xmlnode_value_str(p, local_node, "Path", &checkpoint->file_path);
    // LocalFile.Size
    get_xmlnode_value_int64(p, local_node, "Size", &checkpoint->file_size);
    // LocalFile.LastModified
    get_xmlnode_value_int64(p, local_node, "LastModified", &checkpoint->file_last_modified);
    // LocalFile.MD5
    get_xmlnode_value_str(p, local_node, "MD5", &checkpoint->file_md5);

    // Object
    object_node = mxmlFindElement(root, root, "Object", NULL, NULL, MXML_DESCEND);
    // Object.Key
    get_xmlnode_value_str(p, object_node, "Key", &checkpoint->object_name);
    // Object.Size
    get_xmlnode_value_int64(p, object_node, "Size", &checkpoint->object_size);
    // Object.LastModified
    get_xmlnode_value_str(p, object_node, "LastModified", &checkpoint->object_last_modified);
    // Object.ETag
    get_xmlnode_value_str(p, object_node, "ETag", &checkpoint->object_etag);

    // UploadId
    get_xmlnode_value_str(p, root, "UploadId", &checkpoint->upload_id);

    // CpParts
    cpparts_node = mxmlFindElement(root, root, "CPParts", NULL, NULL, MXML_DESCEND);
    // CpParts.Number
    get_xmlnode_value_int(p, cpparts_node, "Number", &checkpoint->part_num);
    // CpParts.Size
    get_xmlnode_value_int64(p, cpparts_node, "Size", &checkpoint->part_size);
    // CpParts.Parts
    parts_node = mxmlFindElement(cpparts_node, cpparts_node, "Parts", NULL, NULL, MXML_DESCEND);
    node = mxmlFindElement(parts_node, parts_node, "Part", NULL, NULL, MXML_DESCEND);
    for ( ; node != NULL; ) {
        get_xmlnode_value_int(p, node, "Index", &index);
        checkpoint->parts[index].index = index;
        get_xmlnode_value_int64(p, node, "Offset", &checkpoint->parts[index].offset);
        get_xmlnode_value_int64(p, node, "Size", &checkpoint->parts[index].size);
        get_xmlnode_value_int(p, node, "Completed", &checkpoint->parts[index].completed);
        get_xmlnode_value_str(p, node, "ETag", &checkpoint->parts[index].etag);
        get_xmlnode_value_uint64(p, node, "Crc64", &checkpoint->parts[index].crc64);
        node = mxmlFindElement(node, parts_node, "Part", NULL, NULL, MXML_DESCEND);
    }

    mxmlDelete(root);

    return AOSE_OK;
}

static char *oss_build_select_object_xml(aos_pool_t *p, const aos_string_t *expression, 
    const oss_select_object_params_t *params)
{
    char *select_object_xml;
    char *xml_buff;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node;
    mxml_node_t *input_node;
    mxml_node_t *output_node;
    mxml_node_t *option_node;
    int b64_len;
    char b64_buf[2048];
    aos_string_t value;
    int max_b64_expression_len = 0;
    int32_t has_compression = 0;
    int32_t has_csv = 0;

    if (!expression || !params) {
        return NULL;
    }

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "SelectRequest");

    // Expression
    max_b64_expression_len = (expression->len + 1) * 4 / 3;
    if (max_b64_expression_len > sizeof(b64_buf)) {
        char *tmp = (char *)malloc(max_b64_expression_len);
        if (!tmp) {
            return NULL;
        }
        b64_len = aos_base64_encode((unsigned char*)(expression->data), expression->len, tmp);
        value.data = tmp;
        value.len = b64_len;
        set_xmlnode_value_str(root_node, "Expression", &value);
        free(tmp);
    } else {
        b64_len = aos_base64_encode((unsigned char*)(expression->data), expression->len, b64_buf);
        value.data = b64_buf;
        value.len  = b64_len;
        set_xmlnode_value_str(root_node, "Expression", &value);
    }

    // InputSerialization
    has_compression = !aos_string_is_empty(&params->input_param.compression_type);
    has_csv = !aos_string_is_empty(&params->input_param.file_header_info) ||
        !aos_string_is_empty(&params->input_param.record_delimiter) ||
        !aos_string_is_empty(&params->input_param.field_delimiter)  ||
        !aos_string_is_empty(&params->input_param.quote_character)  ||
        !aos_string_is_empty(&params->input_param.comment_character)||
        !aos_string_is_empty(&params->input_param.range);

    if (has_compression || has_csv) {
        
        input_node = mxmlNewElement(root_node, "InputSerialization");
        if (has_compression) {
            set_xmlnode_value_str(input_node, "CompressionType", &params->input_param.compression_type);
        }
        
        if (has_csv) {
            mxml_node_t *csv_node = mxmlNewElement(input_node, "CSV");
            if (!aos_string_is_empty(&params->input_param.file_header_info)) {
                set_xmlnode_value_str(csv_node, "FileHeaderInfo", &params->input_param.file_header_info);
            }
            if (!aos_string_is_empty(&params->input_param.record_delimiter)) {
                value = params->input_param.record_delimiter;
                b64_len = aos_base64_encode((unsigned char*)value.data, value.len, b64_buf);
                value.data = b64_buf;
                value.len = b64_len;
                set_xmlnode_value_str(csv_node, "RecordDelimiter", &value);
            }
            if (!aos_string_is_empty(&params->input_param.field_delimiter)) {
                value = params->input_param.field_delimiter;
                b64_len = aos_base64_encode((unsigned char*)value.data, value.len, b64_buf);
                value.data = b64_buf;
                value.len = b64_len;
                set_xmlnode_value_str(csv_node, "FieldDelimiter", &value);
            }
            if (!aos_string_is_empty(&params->input_param.quote_character)) {
                value = params->input_param.quote_character;
                b64_len = aos_base64_encode((unsigned char*)value.data, value.len, b64_buf);
                value.data = b64_buf;
                value.len = b64_len;
                set_xmlnode_value_str(csv_node, "QuoteCharacter", &value);
            }
            if (!aos_string_is_empty(&params->input_param.comment_character)) {
                value = params->input_param.comment_character;
                b64_len = aos_base64_encode((unsigned char*)value.data, value.len, b64_buf);
                value.data = b64_buf;
                value.len = b64_len;
                set_xmlnode_value_str(csv_node, "CommentCharacter", &value);
            }
            if (!aos_string_is_empty(&params->input_param.range)) {
                set_xmlnode_value_str(csv_node, "Range", &params->input_param.range);
            }
        }
    }

    // OutputSerialization
    has_csv = !aos_string_is_empty(&params->output_param.record_delimiter) ||
        !aos_string_is_empty(&params->output_param.field_delimiter);
    output_node = mxmlNewElement(root_node, "OutputSerialization");

    if (has_csv) {
        mxml_node_t *csv_node = mxmlNewElement(output_node, "CSV");
        if (!aos_string_is_empty(&params->output_param.record_delimiter)) {
            value = params->output_param.record_delimiter;
            b64_len = aos_base64_encode((unsigned char*)value.data, value.len, b64_buf);
            value.data = b64_buf;
            value.len = b64_len;
            set_xmlnode_value_str(csv_node, "RecordDelimiter", &value);
        }
        if (!aos_string_is_empty(&params->output_param.field_delimiter)) {
            value = params->output_param.field_delimiter;
            b64_len = aos_base64_encode((unsigned char*)value.data, value.len, b64_buf);
            value.data = b64_buf;
            value.len = b64_len;
            set_xmlnode_value_str(csv_node, "FieldDelimiter", &value);
        }
    }

    if (params->output_param.keep_all_columns != OSS_INVALID_VALUE) {
        if (params->output_param.keep_all_columns) {
            aos_str_set(&value, "true");
        }
        else {
            aos_str_set(&value, "false");
        }
        set_xmlnode_value_str(output_node, "KeepAllColumns", &value);
    }

    if (params->output_param.output_rawdata != OSS_INVALID_VALUE) {
        if (params->output_param.output_rawdata) {
            aos_str_set(&value, "true");
        }
        else {
            aos_str_set(&value, "false");
        }
        set_xmlnode_value_str(output_node, "OutputRawData", &value);
    }

    if (params->output_param.enable_payload_crc != OSS_INVALID_VALUE) {
        if (params->output_param.enable_payload_crc) {
            aos_str_set(&value, "true");
        }
        else {
            aos_str_set(&value, "false");
        }
        set_xmlnode_value_str(output_node, "EnablePayloadCrc", &value);
    }

    aos_str_set(&value, "false");
    if (params->output_param.output_header) {
        aos_str_set(&value, "true");
    }
    set_xmlnode_value_str(output_node, "OutputHeader", &value);

    //Options
    option_node = mxmlNewElement(root_node, "Options");
    aos_str_set(&value, "false");
    if (params->option_param.skip_partial_data_record) {
        aos_str_set(&value, "true");
    }
    set_xmlnode_value_str(option_node, "SkipPartialDataRecord", &value);

    // dump
    xml_buff = new_xml_buff(doc);
    if (xml_buff == NULL) {
        return NULL;
    }
    aos_str_set(&xml_doc, xml_buff);
    select_object_xml = aos_pstrdup(p, &xml_doc);

    free(xml_buff);
    mxmlDelete(doc);

    return select_object_xml;
}

void oss_build_select_object_body(aos_pool_t *p,
    const aos_string_t *expression,
    const oss_select_object_params_t *params,
    aos_list_t *body)
{
    char *select_object_xml;
    aos_buf_t *b;
    select_object_xml = oss_build_select_object_xml(p, expression, params);
    aos_list_init(body);
    b = aos_buf_pack(p, select_object_xml, strlen(select_object_xml));
    aos_list_add_tail(&b->node, body);
}


static char *oss_build_create_select_object_meta_xml(aos_pool_t *p, const oss_select_object_meta_params_t *params)
{
    char *meta_xml;
    char *xml_buff;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node;
    mxml_node_t *input_node;
    int b64_len;
    char b64_buf[1024];
    aos_string_t value;
    int32_t has_csv = 0;

    if (!params) {
        return NULL;
    }

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "CsvMetaRequest");

    // InputSerialization
    input_node = mxmlNewElement(root_node, "InputSerialization");
    aos_str_set(&value, "None");
    set_xmlnode_value_str(input_node, "CompressionType", &value);
    has_csv = !aos_string_is_empty(&params->record_delimiter) ||
        !aos_string_is_empty(&params->field_delimiter) ||
        !aos_string_is_empty(&params->quote_character);
    
    if (has_csv) {
        mxml_node_t *csv_node = mxmlNewElement(input_node, "CSV");
        if (!aos_string_is_empty(&params->record_delimiter)) {
            value = params->record_delimiter;
            b64_len = aos_base64_encode((unsigned char*)value.data, value.len, b64_buf);
            value.data = b64_buf;
            value.len = b64_len;
            set_xmlnode_value_str(csv_node, "RecordDelimiter", &value);
        }

        if (!aos_string_is_empty(&params->field_delimiter)) {
            value = params->field_delimiter;
            b64_len = aos_base64_encode((unsigned char*)value.data, value.len, b64_buf);
            value.data = b64_buf;
            value.len = b64_len;
            set_xmlnode_value_str(csv_node, "FieldDelimiter", &value);
        }

        if (!aos_string_is_empty(&params->quote_character)) {
            value = params->quote_character;
            b64_len = aos_base64_encode((unsigned char*)value.data, value.len, b64_buf);
            value.data = b64_buf;
            value.len = b64_len;
            set_xmlnode_value_str(csv_node, "QuoteCharacter", &value);
        }
    }

    if (params->over_write_if_existing != OSS_INVALID_VALUE) {
        if (params->over_write_if_existing) {
            aos_str_set(&value, "true");
        }
        else {
            aos_str_set(&value, "false");
        }
        set_xmlnode_value_str(root_node, "OverwriteIfExists", &value);
    }
 
    // dump
    xml_buff = new_xml_buff(doc);
    if (xml_buff == NULL) {
        return NULL;
    }
    aos_str_set(&xml_doc, xml_buff);
    meta_xml = aos_pstrdup(p, &xml_doc);

    free(xml_buff);
    mxmlDelete(doc);

    return meta_xml;
}


void oss_build_create_select_object_meta_body(aos_pool_t *p,
    const oss_select_object_meta_params_t *params,
    aos_list_t *body)
{
    char *meta_xml;
    aos_buf_t *b;
    meta_xml = oss_build_create_select_object_meta_xml(p, params);
    aos_list_init(body);
    b = aos_buf_pack(p, meta_xml, strlen(meta_xml));
    aos_list_add_tail(&b->node, body);
}

char *build_object_tagging_xml(aos_pool_t *p, aos_list_t *tag_list)
{
    char *tagging_xml;
    char *xml_buff;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node, *tags_node;
    oss_tag_content_t *content;

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "Tagging");
    tags_node = mxmlNewElement(root_node, "TagSet");
 
    aos_list_for_each_entry(oss_tag_content_t, content, tag_list, node) {
        mxml_node_t *tag_node = mxmlNewElement(tags_node, "Tag");
        mxml_node_t *key_node = mxmlNewElement(tag_node, "Key");
        mxml_node_t *value_node = mxmlNewElement(tag_node, "Value");
        mxmlNewText(key_node, 0, content->key.data);
        mxmlNewText(value_node, 0, content->value.data);
    }

    xml_buff = new_xml_buff(doc);
    if (xml_buff == NULL) {
        return NULL;
    }
    aos_str_set(&xml_doc, xml_buff);
    tagging_xml = aos_pstrdup(p, &xml_doc);

    free(xml_buff);
    mxmlDelete(doc);

    return tagging_xml;
}

void build_object_tagging_body(aos_pool_t *p, aos_list_t *tag_list, aos_list_t *body)
{
    char *tagging_xml;
    aos_buf_t *b;
    tagging_xml = build_object_tagging_xml(p, tag_list);
    aos_list_init(body);
    b = aos_buf_pack(p, tagging_xml, strlen(tagging_xml));
    aos_list_add_tail(&b->node, body);
}

int oss_get_tagging_parse_from_body(aos_pool_t *p, aos_list_t *bc, aos_list_t *tag_list)
{
    mxml_node_t *doc = NULL;
    mxml_node_t *tagging_node;
    mxml_node_t *tagset_node;
    int res;

    res = get_xmldoc(bc, &doc);
    if (res == AOSE_OK) {
        tagging_node = mxmlFindElement(doc, doc, "Tagging", NULL, NULL, MXML_DESCEND);
        if (tagging_node) {
            tagset_node = mxmlFindElement(tagging_node, doc, "TagSet", NULL, NULL, MXML_DESCEND);
            if (tagset_node) {
                mxml_node_t * node = mxmlFindElement(tagset_node, doc, "Tag", NULL, NULL, MXML_DESCEND);
                for (; node != NULL; ) {
                    oss_tag_content_t *content = oss_create_tag_content(p);
                    get_xmlnode_value_str(p, node, "Key", &content->key);
                    get_xmlnode_value_str(p, node, "Value", &content->value);
                    aos_list_add_tail(&content->node, tag_list);
                    node = mxmlFindElement(node, tagset_node, "Tag", NULL, NULL, MXML_DESCEND);
                }
            }
        }
    }

    mxmlDelete(doc);
    return res;
}