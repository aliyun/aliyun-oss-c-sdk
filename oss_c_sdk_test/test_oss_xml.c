#include "CuTest.h"
#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_xml.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_test_util.h"

static void test_oss_xml_setup(CuTest *tc)
{
}

static void test_oss_xml_cleanup(CuTest *tc)
{
}

static void test_get_xmldoc_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "invalid";
    mxml_node_t *root;
    int ret;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
 
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);
    
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_acl_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "invalid";
    char *no_node_xml_grant =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;
    aos_string_t ret_str;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_acl_parse_from_body(p, &buffer, &ret_str);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_str_null(&ret_str);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml_grant, strlen(no_node_xml_grant));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_acl_parse_from_body(p, &buffer, &ret_str);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, ret_str.data == NULL);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_location_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "invalid";
    char *no_node_xml_grant =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;
    aos_string_t ret_str;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_location_parse_from_body(p, &buffer, &ret_str);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_str_null(&ret_str);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml_grant, strlen(no_node_xml_grant));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_location_parse_from_body(p, &buffer, &ret_str);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, ret_str.data == NULL);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_storage_capacity_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "invalid";
    char *no_node_xml_grant =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;
    long ret_long = 2;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_storage_capacity_parse_from_body(p, &buffer, &ret_long);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml_grant, strlen(no_node_xml_grant));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_storage_capacity_parse_from_body(p, &buffer, &ret_long);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertIntEquals(tc, 2, ret_long);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_logging_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "invalid";
    char *no_node_xml_grant =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    char *no_node_xml_enable =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<BucketLoggingStatus></BucketLoggingStatus>";
    
    int ret;
    oss_logging_config_content_t logging_content;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_logging_parse_from_body(p, &buffer, &logging_content);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    logging_content.logging_enabled = 2;
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml_grant, strlen(no_node_xml_grant));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_logging_parse_from_body(p, &buffer, &logging_content);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertIntEquals(tc, 2, logging_content.logging_enabled);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml_enable, strlen(no_node_xml_enable));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_logging_parse_from_body(p, &buffer, &logging_content);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertIntEquals(tc, 2, logging_content.logging_enabled);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_list_objects_owner_parse_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;
    mxml_node_t *root;
    oss_list_object_content_t object_content;

    aos_pool_create(&p, NULL);

    aos_str_null(&object_content.owner_id);
    aos_str_null(&object_content.owner_display_name);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, root != NULL);

    oss_list_objects_owner_parse(p, root, &object_content);
    CuAssertTrue(tc, object_content.owner_id.data == NULL);
    CuAssertTrue(tc, object_content.owner_display_name.data == NULL);

    mxmlDelete(root);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_list_objects_content_parse_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;
    mxml_node_t *root;
    oss_list_object_content_t object_content;

    aos_pool_create(&p, NULL);

    aos_str_null(&object_content.owner_id);
    aos_str_null(&object_content.owner_display_name);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, root != NULL);

    oss_list_objects_content_parse(p, root, &object_content);

    mxmlDelete(root);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_list_objects_prefix_parse_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;
    mxml_node_t *root;
    oss_list_object_common_prefix_t common_prefix;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, root != NULL);

    oss_list_objects_prefix_parse(p, root, &common_prefix);

    mxmlDelete(root);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_list_objects_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *full_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<ListBucketResult>"
        "<Name>oss-example</Name>"
        "<Prefix>Prefix</Prefix>"
        "<Marker>Marker</Marker>"
        "<MaxKeys>100</MaxKeys>"
        "<Delimiter>/</Delimiter>"
        "<IsTruncated>false</IsTruncated>"
        "<NextMarker>nextMarker</NextMarker>"
        "<Contents>"
        "   <Key>fun/movie/001.avi</Key>"
        "   <LastModified>2012-02-24T08:43:07.000Z</LastModified>"
        "   <ETag>&quot;5B3C1A2E053D763E1B002CC607C&quot;</ETag>"
        "   <Type>Normal</Type>"
        "   <Size>344606</Size>"
        "   <StorageClass>Standard</StorageClass>"
        "   <Owner>"
        "       <ID>0022012****</ID>"
        "       <DisplayName>user-example</DisplayName>"
        "   </Owner>"
        "</Contents>"
        "<Contents>"
        "   <Key>fun/movie/007.avi</Key>"
        "   <LastModified>2012-02-24T08:43:27.000Z</LastModified>"
        "   <ETag>&quot;5B3C1A2E053D763E1B002CC607C&quot;</ETag>"
        "   <Type>Normal</Type>"
        "   <Size>344606</Size>"
        "   <StorageClass>Standard</StorageClass>"
        "   <Owner>"
        "       <ID>0022012****</ID>"
        "       <DisplayName>user-example</DisplayName>"
        "   </Owner>"
        "</Contents>"
        "</ListBucketResult>";
    int ret;
    aos_list_t object_list;
    aos_list_t common_prefix_list;
    aos_string_t marker;
    int truncated;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_list_objects_parse_from_body(p, &buffer, NULL, NULL, NULL, NULL);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_list_init(&object_list);
    aos_list_init(&common_prefix_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, full_node_xml, strlen(full_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_list_objects_parse_from_body(p, &buffer, &object_list, &common_prefix_list, &marker, &truncated);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_list_buckets_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<ListAllMyBucketsResult><Buckets><Bucket></Bucket></Buckets></ListAllMyBucketsResult>";
    char *full_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<ListAllMyBucketsResult>"
        "  <Owner>"
        "    <ID>512**</ID>"
        "    <DisplayName>51264</DisplayName>"
        "  </Owner>"
        "  <Buckets>"
        "    <Bucket>"
        "      <CreationDate>2015-12-17T18:12:43.000Z</CreationDate>"
        "      <ExtranetEndpoint></ExtranetEndpoint>"
        "      <IntranetEndpoint></IntranetEndpoint>"
        "      <Location>oss-cn-shanghai</Location>"
        "      <Name>app-base-oss</Name>"
        "      <StorageClass>Standard</StorageClass>"
        "    </Bucket>"
        "  </Buckets>"
        "</ListAllMyBucketsResult>";
    int ret;
    mxml_node_t *root;
    oss_list_buckets_params_t *params = NULL;
    aos_list_t node_list;

    aos_pool_create(&p, NULL);

    params = oss_create_list_buckets_params(p);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_list_buckets_parse_from_body(p, &buffer, params);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_list_buckets_parse_from_body(p, &buffer, params);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_list_init(&buffer);
    aos_list_init(&node_list);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, root != NULL);
    oss_list_buckets_content_parse(p, root, &node_list);
    oss_list_buckets_content_parse(NULL, root, &node_list);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, full_node_xml, strlen(full_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_list_buckets_parse_from_body(p, &buffer, params);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    mxmlDelete(root);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_get_bucket_info_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;
    oss_bucket_info_t bucket_info;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_bucket_info_parse_from_body(p, &buffer, &bucket_info);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);


    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_bucket_info_parse_from_body(p, &buffer, &bucket_info);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_get_bucket_stat_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;
    oss_bucket_stat_t bucket_stat;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_bucket_stat_parse_from_body(p, &buffer, &bucket_stat);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);


    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_bucket_stat_parse_from_body(p, &buffer, &bucket_stat);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_get_bucket_website_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;
    oss_website_config_t website_config;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_bucket_website_parse_from_body(p, &buffer, &website_config);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);


    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_bucket_website_parse_from_body(p, &buffer, &website_config);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_get_bucket_referer_config_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    char *full_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<RefererConfiguration>"
        "    <AllowEmptyReferer>false</AllowEmptyReferer>"
        "    <RefererList>"
        "        <Referer>http://abc.com</Referer>"
        "        <Referer>http://www.*.com</Referer>"
        "    </RefererList>"
        "</RefererConfiguration>";

    int ret;
    oss_referer_config_t referer_config;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    aos_list_init(&referer_config.referer_list);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_bucket_referer_config_parse_from_body(p, &buffer, &referer_config);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);


    aos_list_init(&buffer);
    aos_list_init(&referer_config.referer_list);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_bucket_referer_config_parse_from_body(p, &buffer, &referer_config);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_list_init(&buffer);
    aos_list_init(&referer_config.referer_list);
    content = aos_buf_pack(p, full_node_xml, strlen(full_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_bucket_referer_config_parse_from_body(p, &buffer, &referer_config);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_cors_rule_content_parse_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;
    mxml_node_t *root;
    aos_list_t node_list;

    aos_pool_create(&p, NULL);

    aos_list_init(&node_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, root != NULL);

    oss_cors_rule_content_parse(p, root, &node_list);

    oss_cors_rule_content_parse(NULL, root, &node_list);

    mxmlDelete(root);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_get_bucket_cors_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    char *full_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<CORSConfiguration>"
        "    <CORSRule>"
        "      <AllowedOrigin>abc</AllowedOrigin>"
        "      <AllowedMethod>GET</AllowedMethod>"
        "      <AllowedHeader>*</AllowedHeader>"
        "      <ExposeHeader>x-oss-test</ExposeHeader>"
        "      <MaxAgeSeconds>100</MaxAgeSeconds>"
        "    </CORSRule>"
        "</CORSConfiguration>";
    int ret;
    aos_list_t rule_list;

    aos_pool_create(&p, NULL);

    aos_list_init(&rule_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_bucket_cors_parse_from_body(p, &buffer, &rule_list);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_list_init(&rule_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_bucket_cors_parse_from_body(p, &buffer, &rule_list);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_list_init(&rule_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_bucket_cors_parse_from_body(NULL, &buffer, &rule_list);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_list_init(&rule_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, full_node_xml, strlen(full_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_bucket_cors_parse_from_body(p, &buffer, &rule_list);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_list_parts_content_parse_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;
    mxml_node_t *root;
    oss_list_part_content_t part_content;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, root != NULL);

    oss_list_parts_content_parse(p, root, &part_content);

    mxmlDelete(root);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_list_parts_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    int ret;
    aos_list_t rule_list;

    aos_pool_create(&p, NULL);

    aos_list_init(&rule_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_list_parts_parse_from_body(p, &buffer, NULL, NULL, NULL);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_list_multipart_uploads_content_parse_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;
    mxml_node_t *root;
    oss_list_multipart_upload_content_t upload_content;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, root != NULL);

    oss_list_multipart_uploads_content_parse(p, root, &upload_content);

    mxmlDelete(root);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_list_multipart_uploads_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;

    aos_list_t upload_list;
    aos_string_t key_marker;
    aos_string_t upload_id_marker;
    int truncated;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_list_multipart_uploads_parse_from_body(p, &buffer, NULL, NULL, NULL, NULL);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_list_init(&upload_list);
    aos_str_set(&key_marker, "");
    aos_str_set(&upload_id_marker, "");
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_list_multipart_uploads_parse_from_body(p, &buffer, &upload_list, &key_marker, &upload_id_marker, &truncated);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_upload_id_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;
    aos_string_t upload_id;

    aos_pool_create(&p, NULL);

    aos_str_set(&upload_id, "");
    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_upload_id_parse_from_body(p, &buffer, &upload_id);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_str_set(&upload_id, "");
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_upload_id_parse_from_body(p, &buffer, &upload_id);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_lifecycle_rule_content_parse_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;
    mxml_node_t *root;
    oss_lifecycle_rule_content_t rule_content;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, root != NULL);

    oss_lifecycle_rule_content_parse(p, root, &rule_content);

    mxmlDelete(root);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_live_channel_info_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    char *root_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<LiveChannelConfiguration></LiveChannelConfiguration>";
    char *sub_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<LiveChannelConfiguration><Target></Target></LiveChannelConfiguration>";

    int ret;
    oss_live_channel_configuration_t info;
    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_live_channel_info_parse_from_body(p, &buffer, &info);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_live_channel_info_parse_from_body(p, &buffer, &info);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, root_node_xml, strlen(root_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_live_channel_info_parse_from_body(p, &buffer, &info);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, sub_node_xml, strlen(sub_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_live_channel_info_parse_from_body(p, &buffer, &info);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}


//oss_list_live_channel_content_parse
//
static void test_oss_live_channel_stat_video_content_parse(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";

    char *full_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Video>"
        "<Width>1280</Width>"
        "<Height>536</Height>"
        "<FrameRate>24</FrameRate>"
        "<Bandwidth>0</Bandwidth>"
        "<Codec>H264</Codec>"
        "</Video>";

    int ret;
    mxml_node_t *root;
    oss_video_stat_t video_stat;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, root != NULL);
    oss_live_channel_stat_video_content_parse(p, root, &video_stat);
    mxmlDelete(root);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, full_node_xml, strlen(full_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, root != NULL);
    oss_live_channel_stat_video_content_parse(p, root, &video_stat);
    CuAssertIntEquals(tc, 0, video_stat.band_width);
    CuAssertIntEquals(tc, 1280, video_stat.width);
    CuAssertIntEquals(tc, 536, video_stat.height);
    CuAssertIntEquals(tc, 24, video_stat.frame_rate);
    CuAssertStrEquals(tc, "H264", video_stat.codec.data);

    mxmlDelete(root);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);

}

static void test_oss_live_channel_stat_audio_content_parse(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";

    char *full_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Audio>"
        "<Bandwidth>0</Bandwidth>"
        "<SampleRate>44100</SampleRate>"
        "<Codec>ADPCM</Codec>"
        "</Audio>";

    int ret;
    mxml_node_t *root;
    oss_audio_stat_t audio_stat;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, root != NULL);
    oss_live_channel_stat_audio_content_parse(p, root, &audio_stat);
    mxmlDelete(root);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, full_node_xml, strlen(full_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, root != NULL);
    oss_live_channel_stat_audio_content_parse(p, root, &audio_stat);
    CuAssertIntEquals(tc, 0, audio_stat.band_width);
    CuAssertIntEquals(tc, 44100, audio_stat.sample_rate);
    CuAssertStrEquals(tc, "ADPCM", audio_stat.codec.data);

    mxmlDelete(root);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);

}

static void test_oss_live_channel_stat_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    char *root_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<LiveChannelStat></LiveChannelStat>";
    char *full_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<LiveChannelStat>"
        "  <Status>Live</Status>"
        "  <ConnectedTime>2016-08-25T06:25:15.000Z</ConnectedTime>"
        "  <RemoteAddr>10.1.2.3:47745</RemoteAddr>"
        "  <Video>"
        "    <Width>1280</Width>"
        "    <Height>536</Height>"
        "    <FrameRate>24</FrameRate>"
        "    <Bandwidth>0</Bandwidth>"
        "    <Codec>H264</Codec>"
        "  </Video>"
        "  <Audio>"
        "    <Bandwidth>0</Bandwidth>"
        "    <SampleRate>44100</SampleRate>"
        "    <Codec>ADPCM</Codec>"
        "  </Audio>"
        "</LiveChannelStat>";

    int ret;
    oss_live_channel_stat_t stat;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_live_channel_stat_parse_from_body(p, &buffer, &stat);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_live_channel_stat_parse_from_body(p, &buffer, &stat);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, root_node_xml, strlen(root_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_live_channel_stat_parse_from_body(p, &buffer, &stat);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, full_node_xml, strlen(full_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_live_channel_stat_parse_from_body(p, &buffer, &stat);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    CuAssertStrEquals(tc, "2016-08-25T06:25:15.000Z", stat.connected_time.data);
    CuAssertStrEquals(tc, "10.1.2.3:47745", stat.remote_addr.data);

    CuAssertIntEquals(tc, 0, stat.video_stat.band_width);
    CuAssertIntEquals(tc, 1280, stat.video_stat.width);
    CuAssertIntEquals(tc, 536, stat.video_stat.height);
    CuAssertIntEquals(tc, 24, stat.video_stat.frame_rate);
    CuAssertStrEquals(tc, "H264", stat.video_stat.codec.data);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_list_live_channel_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    char *root_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<LiveChannel></LiveChannel>";
    char *full_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<ListLiveChannelResult>"
        "  <Prefix></Prefix>"
        "  <Marker></Marker>"
        "  <MaxKeys>1</MaxKeys>"
        "  <IsTruncated>true</IsTruncated>"
        "  <NextMarker>channel-0</NextMarker>"
        "  <LiveChannel>"
        "    <Name>channel-0</Name>"
        "    <Description></Description>"
        "    <Status>disabled</Status>"
        "    <LastModified>2016-07-30T01:54:21.000Z</LastModified>"
        "    <PublishUrls>"
        "      <Url>rtmp://test-bucket.oss-cn-hangzhou.aliyuncs.com/live/channel-0</Url>"
        "    </PublishUrls>"
        "    <PlayUrls>"
        "      <Url>http://test-bucket.oss-cn-hangzhou.aliyuncs.com/channel-0/playlist.m3u8</Url>"
        "    </PlayUrls>"
        "  </LiveChannel>"
        "</ListLiveChannelResult>";

    int ret;
    aos_list_t live_channel_list;
    aos_string_t next_marker;
    int truncated;

    aos_pool_create(&p, NULL);

    aos_list_init(&live_channel_list);
    aos_str_set(&next_marker, "");
    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_list_live_channel_parse_from_body(p, &buffer, &live_channel_list, &next_marker, &truncated);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_list_init(&live_channel_list);
    aos_str_set(&next_marker, "");
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_list_live_channel_parse_from_body(p, &buffer, &live_channel_list, &next_marker, &truncated);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_list_init(&live_channel_list);
    aos_str_set(&next_marker, "");
    aos_list_init(&buffer);
    content = aos_buf_pack(p, root_node_xml, strlen(root_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_list_live_channel_parse_from_body(p, &buffer, &live_channel_list, &next_marker, &truncated);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_list_init(&live_channel_list);
    aos_str_set(&next_marker, "");
    aos_list_init(&buffer);
    content = aos_buf_pack(p, full_node_xml, strlen(full_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_list_live_channel_parse_from_body(p, &buffer, &live_channel_list, &next_marker, &truncated);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_live_channel_history_content_parse(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";

    char *full_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<StartTime>2016-07-30T01:53:21.000Z</StartTime>"
        "<EndTime>2016-07-30T01:53:31.000Z</EndTime>"
        "<RemoteAddr>10.101.194.148:56861</RemoteAddr>";

    int ret;
    mxml_node_t *root;
    oss_live_record_content_t record_content;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, root != NULL);
    oss_live_channel_history_content_parse(p, root, &record_content);
    mxmlDelete(root);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, full_node_xml, strlen(full_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, root != NULL);
    oss_live_channel_history_content_parse(p, root, &record_content);
    CuAssertStrEquals(tc, "2016-07-30T01:53:21.000Z", record_content.start_time.data);
    CuAssertStrEquals(tc, "2016-07-30T01:53:31.000Z", record_content.end_time.data);
    CuAssertStrEquals(tc, "10.101.194.148:56861", record_content.remote_addr.data);

    mxmlDelete(root);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);

}

static void test_oss_live_channel_history_parse_from_body(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    char *full_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<LiveChannelHistory>"
        "<LiveRecord>"
        "<StartTime>2016-07-30T01:53:21.000Z</StartTime>"
        "<EndTime>2016-07-30T01:53:31.000Z</EndTime>"
        "<RemoteAddr>10.101.194.148:56861</RemoteAddr>"
        "</LiveRecord>"
        "<LiveRecord>"
        "<StartTime>2016-07-30T01:53:35.000Z</StartTime>"
        "<EndTime>2016-07-30T01:53:45.000Z</EndTime>"
        "<RemoteAddr>10.101.194.148:57126</RemoteAddr>"
        "</LiveRecord>"
        "<LiveRecord>"
        "<StartTime>2016-07-30T01:53:49.000Z</StartTime>"
        "<EndTime>2016-07-30T01:53:59.000Z</EndTime>"
        "<RemoteAddr>10.101.194.148:57577</RemoteAddr>"
        "</LiveRecord>"
        "<LiveRecord>"
        "<StartTime>2016-07-30T01:54:04.000Z</StartTime>"
        "<EndTime>2016-07-30T01:54:14.000Z</EndTime>"
        "<RemoteAddr>10.101.194.148:57632</RemoteAddr>"
        "</LiveRecord>"
        "</LiveChannelHistory>";
    aos_list_t live_record_list;
    int ret;

    aos_pool_create(&p, NULL);

    aos_list_init(&live_record_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_live_channel_history_parse_from_body(p, &buffer, &live_record_list);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_list_init(&live_record_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_live_channel_history_parse_from_body(p, &buffer, &live_record_list);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_list_init(&live_record_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, full_node_xml, strlen(full_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_live_channel_history_parse_from_body(p, &buffer, &live_record_list);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_create_live_channel_parse_from_body(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_create_live_channel_parse_from_body(p, &buffer, NULL, NULL);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_create_live_channel_parse_from_body(p, &buffer, NULL, NULL);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_get_tagging_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    char *root_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Tagging></Tagging>";

    int ret;
    aos_list_t tag_list;

    aos_pool_create(&p, NULL);

    aos_list_init(&tag_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_tagging_parse_from_body(p, &buffer, &tag_list);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_list_init(&tag_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_tagging_parse_from_body(p, &buffer, &tag_list);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_list_init(&tag_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, root_node_xml, strlen(root_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_get_tagging_parse_from_body(p, &buffer, &tag_list);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_get_xmlnode_value(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    mxml_node_t *root;
    int ret;

    aos_pool_create(&p, NULL);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    get_xmlnode_value_int(p, root, "Node", NULL);
    get_xmlnode_value_int64(p, root, "Node", NULL);

    mxmlDelete(root);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);

}

static void test_oss_lifecycle_rules_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";

    int ret;
    aos_list_t lifecycle_rule_list;

    aos_pool_create(&p, NULL);

    aos_list_init(&lifecycle_rule_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_lifecycle_rules_parse_from_body(p, &buffer, &lifecycle_rule_list);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_list_init(&lifecycle_rule_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_lifecycle_rules_parse_from_body(p, &buffer, &lifecycle_rule_list);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_lifecycle_rule_tag_parse_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    int ret;
    mxml_node_t *root;
    oss_tag_content_t tag;

    aos_pool_create(&p, NULL);

    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = get_xmldoc(&buffer, &root);
    CuAssertIntEquals(tc, AOSE_OK, ret);
    CuAssertTrue(tc, root != NULL);

    oss_lifecycle_rule_tag_parse(p, root, &tag);

    mxmlDelete(root);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_delete_objects_parse_from_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t buffer;
    aos_buf_t *content;
    char *invalid_xml = "Invalid";
    char *no_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    char *root_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<Invalid></Invalid>";
    char *full_node_xml =
        "<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n"
        "<DeleteResult>"
        "    <Deleted>"
        "    </Deleted>"
        "    <Deleted>"
        "       <Key>test.jpg</Key>"
        "    </Deleted>"
        "</DeleteResult>";
    int ret;
    aos_list_t object_list;

    aos_pool_create(&p, NULL);

    aos_list_init(&object_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, invalid_xml, strlen(invalid_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_delete_objects_parse_from_body(p, &buffer, &object_list);
    CuAssertIntEquals(tc, AOSE_XML_PARSE_ERROR, ret);

    aos_list_init(&object_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, no_node_xml, strlen(no_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_delete_objects_parse_from_body(p, &buffer, &object_list);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_list_init(&object_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, root_node_xml, strlen(root_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_delete_objects_parse_from_body(p, &buffer, &object_list);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_list_init(&object_list);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, full_node_xml, strlen(full_node_xml));
    aos_list_add_tail(&content->node, &buffer);
    ret = oss_delete_objects_parse_from_body(p, &buffer, &object_list);
    CuAssertIntEquals(tc, AOSE_OK, ret);

    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}

static void test_build_bucket_logging_body_negative(CuTest *tc)
{
    aos_pool_t *p;
    oss_logging_config_content_t content;
    aos_list_t body;

    aos_pool_create(&p, NULL);

    aos_list_init(&body);
    aos_str_set(&content.target_bucket, "");
    aos_str_set(&content.prefix, "");
    build_bucket_logging_body(p, &content, &body);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

static void test_build_bucket_storage_class_negative(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t body;

    aos_pool_create(&p, NULL);

    aos_list_init(&body);
    build_bucket_storage_class(p, OSS_STORAGE_CLASS_BUTT, &body);
    CuAssertTrue(tc, aos_list_empty(&body) == 1);

    aos_list_init(&body);
    build_bucket_storage_class(p, OSS_STORAGE_CLASS_STANDARD, &body);
    CuAssertTrue(tc, aos_list_empty(&body) == 0);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_publish_url_parse(CuTest *tc)
{
    aos_pool_t *p;
    aos_pool_create(&p, NULL);

    oss_publish_url_parse(p, NULL, NULL);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_play_url_parse(CuTest *tc)
{
    aos_pool_t *p;
    aos_pool_create(&p, NULL);

    oss_play_url_parse(p, NULL, NULL);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_build_select_object_body(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t body;
    aos_string_t expression;
    aos_pool_create(&p, NULL);

    oss_build_select_object_body(p, NULL, NULL, &body);

    oss_build_select_object_body(p, &expression, NULL, &body);

    aos_pool_destroy(p);

    printf("%s ok\n", __FUNCTION__);
}

static void test_oss_build_create_select_object_meta_body(CuTest *tc)
{
    aos_pool_t *p;
    aos_list_t body;
    aos_pool_create(&p, NULL);
    oss_build_create_select_object_meta_body(p, NULL, &body);
    aos_pool_destroy(p);
    printf("%s ok\n", __FUNCTION__);
}


//
//build_bucket_storage_capacity_body
//oss_build_create_select_object_meta_xml
//


CuSuite *test_oss_xml()
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_oss_xml_setup);
    SUITE_ADD_TEST(suite, test_get_xmldoc_negative);
    SUITE_ADD_TEST(suite, test_oss_acl_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_oss_location_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_oss_storage_capacity_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_oss_logging_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_oss_list_objects_owner_parse_negative);
    SUITE_ADD_TEST(suite, test_oss_list_objects_content_parse_negative);
    SUITE_ADD_TEST(suite, test_oss_list_objects_prefix_parse_negative);
    SUITE_ADD_TEST(suite, test_oss_list_objects_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_oss_list_buckets_parse_from_body_negative);

    SUITE_ADD_TEST(suite, test_oss_get_bucket_info_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_oss_get_bucket_stat_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_oss_get_bucket_website_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_oss_get_bucket_referer_config_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_oss_cors_rule_content_parse_negative);
    SUITE_ADD_TEST(suite, test_oss_get_bucket_cors_parse_from_body_negative);
    
    SUITE_ADD_TEST(suite, test_oss_list_parts_content_parse_negative);
    SUITE_ADD_TEST(suite, test_oss_list_parts_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_oss_list_multipart_uploads_content_parse_negative);
    SUITE_ADD_TEST(suite, test_oss_list_multipart_uploads_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_oss_upload_id_parse_from_body_negative);
    
    SUITE_ADD_TEST(suite, test_oss_lifecycle_rule_content_parse_negative);

    SUITE_ADD_TEST(suite, test_oss_live_channel_info_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_oss_live_channel_stat_video_content_parse);
    SUITE_ADD_TEST(suite, test_oss_live_channel_stat_audio_content_parse);
    SUITE_ADD_TEST(suite, test_oss_live_channel_stat_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_oss_list_live_channel_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_oss_live_channel_history_content_parse);
    SUITE_ADD_TEST(suite, test_oss_live_channel_history_parse_from_body);
    SUITE_ADD_TEST(suite, test_oss_create_live_channel_parse_from_body);
    
    SUITE_ADD_TEST(suite, test_oss_get_tagging_parse_from_body_negative);

    SUITE_ADD_TEST(suite, test_get_xmlnode_value);
    SUITE_ADD_TEST(suite, test_oss_lifecycle_rules_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_oss_lifecycle_rule_tag_parse_negative);
    SUITE_ADD_TEST(suite, test_oss_delete_objects_parse_from_body_negative);
    SUITE_ADD_TEST(suite, test_build_bucket_logging_body_negative);
    SUITE_ADD_TEST(suite, test_build_bucket_storage_class_negative);

    SUITE_ADD_TEST(suite, test_oss_publish_url_parse);
    SUITE_ADD_TEST(suite, test_oss_play_url_parse);
    SUITE_ADD_TEST(suite, test_oss_build_select_object_body);
    SUITE_ADD_TEST(suite, test_oss_build_create_select_object_meta_body);
    
    SUITE_ADD_TEST(suite, test_oss_xml_cleanup);

    return suite;
}

