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

static int get_truncated_from_xml(aos_pool_t *p, mxml_node_t *xml_node, const char *truncated_xml_path)
{
    char *is_truncated;
    int truncated = 0;
    is_truncated = get_xmlnode_value(p, xml_node, truncated_xml_path);
    if (is_truncated) {
        truncated = strcasecmp(is_truncated, "false") == 0 ? 0 : 1;
    }
    return truncated;
}

static char* new_xml_buff(mxml_node_t *doc)
{
    int buff_size;
    int bytes;
    int retry;
    char *xml_buff;

    bytes = 0;
    retry = 3;
    xml_buff = NULL;
    buff_size = 4096;

    do {
        buff_size = buff_size * 10;
        xml_buff = (char*)realloc(xml_buff, buff_size);
        bytes = mxmlSaveString(doc, xml_buff, buff_size, MXML_NO_CALLBACK);
    } while(bytes < 0 && retry-- > 0);

    if (bytes < 0) {
        free(xml_buff);
        return NULL;
    }
    return xml_buff;
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
