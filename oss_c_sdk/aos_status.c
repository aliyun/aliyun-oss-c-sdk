#include "aos_log.h"
#include "aos_util.h"
#include "aos_status.h"

const char AOS_XML_PARSE_ERROR_CODE[] = "ParseXmlError";
const char AOS_OPEN_FILE_ERROR_CODE[] = "OpenFileFail";
const char AOS_WRITE_FILE_ERROR_CODE[] = "WriteFileFail";
const char AOS_RENAME_FILE_ERROR_CODE[] = "RenameFileFail";
const char AOS_HTTP_IO_ERROR_CODE[] = "HttpIoError";
const char AOS_UNKNOWN_ERROR_CODE[] = "UnknownError";
const char AOS_CLIENT_ERROR_CODE[] = "ClientError";
const char AOS_SERVER_ERROR_CODE[] = "ServerError";
const char AOS_UTF8_ENCODE_ERROR_CODE[] = "Utf8EncodeFail";
const char AOS_URL_ENCODE_ERROR_CODE[] = "UrlEncodeFail";
const char AOS_INCONSISTENT_ERROR_CODE[] = "InconsistentError";
const char AOS_CREATE_QUEUE_ERROR_CODE[] = "CreateQueueFail";
const char AOS_CREATE_THREAD_POOL_ERROR_CODE[] = "CreateThreadPoolFail";
const char AOS_EMPTY_STRING_ERROR[] = "EmptyStringError";
const char AOS_SELECT_OBJECT_ERROR[] = "SelectObjectError";
const char AOS_SELECT_OBJECT_CRC_ERROR[] = "SelectObjectCRCError";
const char AOS_CREATE_SELECT_OBJECT_META_ERROR[] = "CreateSelectObjectMetaError";

aos_status_t *aos_status_create(aos_pool_t *p)
{
    return (aos_status_t *)aos_pcalloc(p, sizeof(aos_status_t));
}

aos_status_t *aos_status_dup(aos_pool_t *p, aos_status_t *src)
{
    aos_status_t *dst = aos_status_create(p);
    dst->code = src->code;
    dst->error_code = apr_pstrdup(p, src->error_code);
    dst->error_msg = apr_pstrdup(p, src->error_msg);
    return dst;
}

int aos_should_retry(aos_status_t *s) 
{
    // HTTP Error
    if (s == NULL || s->code / 100 == 2) {
        return AOS_FALSE;
    }

    // OSS Error
    if (s->code / 100 == 5) {
        return AOS_TRUE;
    }

    // Curl Error
    if (s->code != AOSE_OK) {
        if (s->code == AOSE_CONNECTION_FAILED || s->code == AOSE_REQUEST_TIMEOUT || 
            s->code == AOSE_FAILED_CONNECT || s->code == AOSE_SERVICE_ERROR) {
            return AOS_TRUE;
        }
    }

    return AOS_FALSE;
}

aos_status_t *aos_status_parse_from_body(aos_pool_t *p, aos_list_t *bc, int code, aos_status_t *s)
{
    int res;
    mxml_node_t *root, *node;
    mxml_node_t *code_node, *message_node;
    char *node_content;

    if (s == NULL) {
        s = aos_status_create(p);
    }
    s->code = code;

    if (aos_http_is_ok(code)) {
        return s;
    }

    if (aos_list_empty(bc)) {
        s->error_code = (char *)AOS_UNKNOWN_ERROR_CODE;
        return s;
    }

    if ((res = aos_parse_xml_body(bc, &root)) != AOSE_OK) {
        s->error_code = (char *)AOS_UNKNOWN_ERROR_CODE;
        return s;
    }

    node = mxmlFindElement(root, root, "Error",NULL, NULL,MXML_DESCEND);
    if (NULL == node) {
        char *xml_content = aos_buf_list_content(p, bc);
        aos_error_log("Xml format invalid, root node name is not Error.\n");
        aos_error_log("Xml Content:%s\n", xml_content);
        s->error_code = (char *)AOS_UNKNOWN_ERROR_CODE;
        mxmlDelete(root);
        return s;
    }

    code_node = mxmlFindElement(node, root, "Code",NULL, NULL,MXML_DESCEND);
    if (NULL != code_node) {
        node_content = code_node->child->value.opaque;
        s->error_code = apr_pstrdup(p, (char *)node_content);
    }

    message_node = mxmlFindElement(node, root, "Message",NULL, NULL,MXML_DESCEND);
    if (NULL != message_node) {
        node_content = message_node->child->value.opaque;
        s->error_msg = apr_pstrdup(p, (char *)node_content);
    }

    mxmlDelete(root);

    return s;
}
