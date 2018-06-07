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
}

void oss_lifecycle_rule_expire_parse(aos_pool_t *p, mxml_node_t * xml_node,
    oss_lifecycle_rule_content_t *content)
{
    char* days;
    char *date;
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

char *build_create_select_object_metadata_xml(aos_pool_t *p, const oss_select_metadata_option *option) {
    char *xml_buff;
    char *create_select_object_metadata_xml;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node;
    mxml_node_t *overwrite_node;
    mxml_node_t *input_serialization_node;
    mxml_node_t *compression_type_node;
    mxml_node_t *csv_node;
    mxml_node_t *record_delimiter_node;
    mxml_node_t *field_delimiter_node;
    mxml_node_t *quote_char_node;
    int encoded_len;

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "CsvMetaRequest");

    overwrite_node = mxmlNewElement(root_node, "OverwriteIfExists");
    mxmlNewText(overwrite_node, 0, option->overwrite == 0 ? "false" : "true");

    input_serialization_node = mxmlNewElement(root_node, "InputSerialization");
    compression_type_node = mxmlNewElement(input_serialization_node, "CompressionType");

    mxmlNewText(compression_type_node, 0, option->input_serialization.compression_info == NONE ? "NONE" : "GZIP");
    csv_node = mxmlNewElement(input_serialization_node, "CSV");
    record_delimiter_node = mxmlNewElement(csv_node, "RecordDelimiter");

    // encode and add record_delimiter
    char encoded[128];
    encoded_len = aos_base64_encode((const unsigned char*)option->input_serialization.csv_format.record_delimiter.data,
                                    option->input_serialization.csv_format.record_delimiter.len, encoded);
    encoded[encoded_len] = '\0';
    mxmlNewText(record_delimiter_node, 0, encoded);

    // encode and add field_delimiter
    encoded_len = aos_base64_encode((const unsigned char*)&(option->input_serialization.csv_format.field_delimiter),
                                    1, encoded);
    encoded[encoded_len] = '\0';
    field_delimiter_node = mxmlNewElement(csv_node, "FieldDelimiter");
    mxmlNewText(field_delimiter_node, 0, encoded);

    // encode and add field_quote
    encoded_len = aos_base64_encode((const unsigned char*)&(option->input_serialization.csv_format.field_quote),
                                    1, encoded);
    encoded[encoded_len] = '\0';
    quote_char_node = mxmlNewElement(csv_node, "QuoteCharacter");
    mxmlNewText(quote_char_node, 0, encoded);

    xml_buff = new_xml_buff(doc);
    if (xml_buff == NULL) {
        return NULL;
    }
    aos_str_set(&xml_doc, xml_buff);
    create_select_object_metadata_xml = aos_pstrdup(p, &xml_doc);

    free(xml_buff);
    mxmlDelete(doc);

    return create_select_object_metadata_xml;
}

void build_create_select_object_metadata_body(aos_pool_t *p, const oss_select_metadata_option *option, aos_list_t *body) {
    char *create_select_object_metadata_xml;
    aos_buf_t *b;

    create_select_object_metadata_xml = build_create_select_object_metadata_xml(p, option);
    aos_list_init(body);
    b = aos_buf_pack(p, create_select_object_metadata_xml, strlen(create_select_object_metadata_xml));
    aos_list_add_tail(&b->node, body);
}

char *build_select_object_metadata_xml(aos_pool_t *p, const oss_select_option *option) {
    char *xml_buff;
    char *select_object_xml;
    aos_string_t xml_doc;
    mxml_node_t *doc;
    mxml_node_t *root_node;
    mxml_node_t *expression_node;
    mxml_node_t *input_serialization_node;
    mxml_node_t *compression_type_node;
    mxml_node_t *csv_node;
    mxml_node_t *range_node;
    mxml_node_t *record_delimiter_node;
    mxml_node_t *field_delimiter_node;
    mxml_node_t *quote_char_node;
    mxml_node_t *comment_char_node;

    mxml_node_t *output_serialization_node;
    mxml_node_t *output_csv_node;
    mxml_node_t *output_record_delimiter_node;
    mxml_node_t *output_field_delimiter_node;
    mxml_node_t *output_quote_char_node;
    mxml_node_t *output_keep_all_columns_node;
    mxml_node_t *output_raw_data_node;

    int encoded_len;
    //TODO, make sure buffer is enough
    char buff[1024 * 1024];

    doc = mxmlNewXML("1.0");
    root_node = mxmlNewElement(doc, "SelectRequest");

    //expression
    expression_node = mxmlNewElement(root_node, "Expression");
    encoded_len = aos_base64_encode((const unsigned char*)option->expression.data, option->expression.len, buff);
    buff[encoded_len] = '\0';
    mxmlNewText(expression_node, 0, buff);

    input_serialization_node = mxmlNewElement(root_node, "InputSerialization");
    compression_type_node = mxmlNewElement(input_serialization_node, "CompressionType");
    mxmlNewText(compression_type_node, 0, option->input_serialization.compression_info == NONE ? "NONE" : "GZIP");

    csv_node = mxmlNewElement(input_serialization_node, "CSV");
    record_delimiter_node = mxmlNewElement(csv_node, "RecordDelimiter");

    // encode and add record_delimiter
    encoded_len = aos_base64_encode((const unsigned char*)option->input_serialization.csv_format.record_delimiter.data,
                                    option->input_serialization.csv_format.record_delimiter.len, buff);
    buff[encoded_len] = '\0';
    mxmlNewText(record_delimiter_node, 0, buff);
    // encode and add field_delimiter
    encoded_len = aos_base64_encode((const unsigned char*)&(option->input_serialization.csv_format.field_delimiter),
                                    1, buff);
    buff[encoded_len] = '\0';
    field_delimiter_node = mxmlNewElement(csv_node, "FieldDelimiter");
    mxmlNewText(field_delimiter_node, 0, buff);

    // encode and add field_quote
    encoded_len = aos_base64_encode((const unsigned char*)&(option->input_serialization.csv_format.field_quote),
                                    1, buff);
    buff[encoded_len] = '\0';
    quote_char_node = mxmlNewElement(csv_node, "QuoteCharacter");
    mxmlNewText(quote_char_node, 0, buff);

    // encode and add comment
    encoded_len = aos_base64_encode((const unsigned char*)&(option->input_serialization.csv_format.comment),
                                    1, buff);
    buff[encoded_len] = '\0';
    comment_char_node = mxmlNewElement(csv_node, "Comments");
    mxmlNewText(comment_char_node, 0, buff);
    // add range
    char* range_str = NULL;
    if (option->range_option == LINE) {
        range_str = range_to_string("line-range=", option->range[0], option->range[1], buff);
    } else if (option->range_option == SPLIT) {
        range_str = range_to_string("split-range=", option->range[0], option->range[1], buff);
    }

    if (range_str != NULL) {
        range_node = mxmlNewElement(csv_node, "Range");
        mxmlNewText(range_node, 0, range_str);
    }

    output_serialization_node = mxmlNewElement(root_node, "OutputSerialization");
    //output raw data
    output_raw_data_node = mxmlNewElement(output_serialization_node, "OutputRawData");
    mxmlNewText(output_raw_data_node, 0, "true");

    output_csv_node = mxmlNewElement(output_serialization_node, "CSV");
    output_record_delimiter_node = mxmlNewElement(output_csv_node, "RecordDelimiter");

    // encode and add record_delimiter
    encoded_len = aos_base64_encode((const unsigned char*)option->output_serialization.csv_format.record_delimiter.data,
                                    option->output_serialization.csv_format.record_delimiter.len, buff);
    buff[encoded_len] = '\0';
    mxmlNewText(output_record_delimiter_node, 0, buff);
    // encode and add field_delimiter
    encoded_len = aos_base64_encode((const unsigned char*)&(option->output_serialization.csv_format.field_delimiter),
                                    1, buff);
    buff[encoded_len] = '\0';
    output_field_delimiter_node = mxmlNewElement(output_csv_node, "FieldDelimiter");
    mxmlNewText(output_field_delimiter_node, 0, buff);

    // encode and add field_quote
    encoded_len = aos_base64_encode((const unsigned char*)&(option->output_serialization.csv_format.field_quote),
                                    1, buff);
    buff[encoded_len] = '\0';
    output_quote_char_node = mxmlNewElement(output_csv_node, "QuoteCharacter");
    mxmlNewText(output_quote_char_node, 0, buff);

    output_keep_all_columns_node = mxmlNewElement(output_csv_node, "KeepAllColumns");
    mxmlNewText(output_keep_all_columns_node, 0, option->output_serialization.keep_all_columns == 0 ? "false" : "true");

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

void build_select_object_metadata_body(aos_pool_t *p, const oss_select_option *option, aos_list_t *body) {
    char *select_object_xml;
    aos_buf_t *b;

    select_object_xml = build_select_object_metadata_xml(p, option);
    aos_list_init(body);
    b = aos_buf_pack(p, select_object_xml, strlen(select_object_xml));
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
