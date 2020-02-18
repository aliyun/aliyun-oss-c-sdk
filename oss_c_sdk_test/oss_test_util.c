#include <sys/stat.h>
#include "oss_config.h"
#include "oss_api.h"
#include "oss_test_util.h"
#include "cjson.h"

char bucket_prefix[64];

void make_rand_string(aos_pool_t *p, int len, aos_string_t *data)
{
    char *str = NULL;
    int i = 0;
    str = (char *)aos_palloc(p, len + 1);
    for ( ; i < len; i++) {
        str[i] = 'a' + rand() % 32;
    }
    str[len] = '\0';
    aos_str_set(data, str);
}

aos_buf_t *make_random_buf(aos_pool_t *p, int len)
{
    int bytes;
    aos_buf_t *b;
    aos_string_t str;

    make_rand_string(p, 16, &str);
    b = aos_create_buf(p, len);

    while (b->last < b->end) {
        bytes = b->end - b->last;
        bytes = aos_min(bytes, 16);
        memcpy(b->last, str.data, bytes);
        b->last += bytes;
    }

    return b;
}

void make_random_body(aos_pool_t *p, int count, aos_list_t *bc)
{
    int i = 0;
    int len;
    aos_buf_t *b;

    srand((int)time(0));
    for (; i < count; ++i) {
        len = 1 + (int)(4096.0*rand() / (RAND_MAX+1.0));
        b = make_random_buf(p, len);
        aos_list_add_tail(&b->node, bc);
    }
}

int make_random_file(aos_pool_t *p, const char *filename, int len)
{
    apr_file_t *file;
    aos_string_t str;
    apr_size_t nbytes;
    int ret;

    if ((ret = apr_file_open(&file, filename, APR_CREATE | APR_WRITE | APR_TRUNCATE,
        APR_UREAD | APR_UWRITE | APR_GREAD, p)) != APR_SUCCESS) {
            return ret;
    }

    make_rand_string(p, len, &str);
    nbytes = len;

    ret = apr_file_write(file, str.data, &nbytes);
    apr_file_close(file);

    return ret;
}

int fill_test_file(aos_pool_t *p, const char *filename, const char *content) 
{
    apr_file_t *file;
    apr_size_t nbytes;
    int ret;

    if ((ret = apr_file_open(&file, filename, APR_CREATE | APR_WRITE | APR_TRUNCATE,
        APR_UREAD | APR_UWRITE | APR_GREAD, p)) != APR_SUCCESS) {
            return ret;
    }

    nbytes = strlen(content);

    ret = apr_file_write(file, content, &nbytes);
    apr_file_close(file);

    return ret;
}

void init_test_config(oss_config_t *config, int is_cname)
{
    aos_str_set(&config->endpoint, TEST_OSS_ENDPOINT);
    aos_str_set(&config->access_key_id, TEST_ACCESS_KEY_ID);
    aos_str_set(&config->access_key_secret, TEST_ACCESS_KEY_SECRET);
    config->is_cname = is_cname;
}

void init_test_request_options(oss_request_options_t *options, int is_cname)
{
    options->config = oss_config_create(options->pool);
    init_test_config(options->config, is_cname);
    options->ctl = aos_http_controller_create(options->pool, 0);
}

aos_status_t * create_test_bucket(const oss_request_options_t *options,
                                  const char *bucket_name, 
                                  oss_acl_e oss_acl)
{
    aos_string_t bucket;
    aos_table_t *resp_headers;
    aos_status_t * s;

    aos_str_set(&bucket, bucket_name);

    s = oss_create_bucket(options, &bucket, oss_acl, &resp_headers);
    return s;
}

aos_status_t * create_test_bucket_with_storage_class(const oss_request_options_t *options,
                                  const char *bucket_name, 
                                  oss_acl_e oss_acl,
                                  oss_storage_class_type_e storage_class)
{
    aos_string_t bucket;
    aos_table_t *resp_headers;
    aos_status_t * s;

    aos_str_set(&bucket, bucket_name);

    s = oss_create_bucket_with_storage_class(options, &bucket, oss_acl, storage_class, &resp_headers);
    return s;
}

aos_status_t *create_test_object(const oss_request_options_t *options, 
                                 const char *bucket_name, 
                                 const char *object_name, 
                                 const char *data, 
                                 aos_table_t *headers)
{
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *resp_headers;
    aos_list_t buffer;
    aos_buf_t *content;
    aos_status_t * s;

    test_object_base();
    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, data, strlen(data));
    aos_list_add_tail(&content->node, &buffer);

    s = oss_put_object_from_buffer(options, &bucket, &object, 
                                   &buffer, headers, &resp_headers);
    return s;
}

aos_status_t *create_test_object_from_file(const oss_request_options_t *options, 
                                          const char *bucket_name,
                                          const char *object_name, 
                                          const char *filename, 
                                          aos_table_t *headers)
{
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t file;
    aos_table_t *resp_headers;
    aos_status_t * s;

    test_object_base();
    aos_str_set(&file, filename);

    s = oss_put_object_from_file(options, &bucket, &object, &file, 
                                 headers, &resp_headers);
    return s;
}

aos_status_t *delete_test_object(const oss_request_options_t *options, 
                                 const char *bucket_name, 
                                 const char *object_name)
{
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *resp_headers;
    aos_status_t * s;

    test_object_base();
    s = oss_delete_object(options, &bucket, &object, &resp_headers);
    return s;
}

aos_status_t *delete_test_object_by_prefix(const oss_request_options_t *options,
                                const char *bucket_name,
                                const char *object_name_prefix)
{
    aos_string_t bucket;
    aos_status_t * s = NULL;
    oss_list_object_params_t *params = NULL;
    oss_list_object_content_t *content = NULL;
    char *nextMarker = "";

    aos_str_set(&bucket, bucket_name);

    params = oss_create_list_object_params(options->pool);
    params->max_ret = 100;
    aos_str_set(&params->prefix, object_name_prefix);
    aos_str_set(&params->marker, nextMarker);

    do {
        s = oss_list_object(options, &bucket, params, NULL);
        if (!aos_status_is_ok(s))
        {
            return s;
        }

        aos_list_for_each_entry(oss_list_object_content_t, content, &params->object_list, node) {
            char object_name[128];
            sprintf(object_name, "%.*s", content->key.len, content->key.data);
            s = delete_test_object(options, bucket_name, object_name);
            if (!aos_status_is_ok(s))
            {
                return s;
            }
        }

        nextMarker = apr_psprintf(options->pool, "%.*s", params->next_marker.len, params->next_marker.data);
        aos_str_set(&params->marker, nextMarker);
        aos_list_init(&params->object_list);
        aos_list_init(&params->common_prefix_list);
    } while (params->truncated == AOS_TRUE);

    if (s == NULL) {
        s = aos_status_create(options->pool);
        aos_status_set(s, AOSE_OK, NULL, NULL);
    }

    return s;
}

void clean_bucket(const oss_request_options_t *options, const char* bucket_name)
{
    aos_table_t *resp_headers = NULL;
    oss_list_multipart_upload_params_t *list_upload_params = NULL;
    oss_list_live_channel_params_t *live_channel_params = NULL;
    aos_string_t bucket;

    // delete all objects
    delete_test_object_by_prefix(options, bucket_name, "");

    // upload
    aos_str_set(&bucket, bucket_name);
    list_upload_params = oss_create_list_multipart_upload_params(options->pool);
    do {
        aos_status_t *s;
        oss_list_multipart_upload_content_t *upload_content;
        aos_list_init(&list_upload_params->upload_list);

        s = oss_list_multipart_upload(options, &bucket, list_upload_params, &resp_headers);

        if (!aos_status_is_ok(s)) {
            break;
        }

        aos_list_for_each_entry(oss_list_multipart_upload_content_t, upload_content, &list_upload_params->upload_list, node) {
            abort_test_multipart_upload(options, bucket_name, upload_content->key.data, &upload_content->upload_id);
        }

        if (!list_upload_params->truncated) {
            break;
        }

        aos_str_set(&list_upload_params->key_marker, list_upload_params->next_key_marker.data);
        aos_str_set(&list_upload_params->upload_id_marker, list_upload_params->next_upload_id_marker.data);

    } while (1);

    // live channel
    live_channel_params = oss_create_list_live_channel_params(options->pool);
    do {
        aos_status_t *s;
        oss_live_channel_content_t *live_chan;
        aos_list_init(&live_channel_params->live_channel_list);

        s = oss_list_live_channel(options, &bucket, live_channel_params, NULL);

        if (!aos_status_is_ok(s)) {
            break;
        }

        aos_list_for_each_entry(oss_live_channel_content_t, live_chan, &live_channel_params->live_channel_list, node) {
            printf("live_chan->name.data:%s\n", live_chan->name.data);
            oss_delete_live_channel(options, &bucket, &live_chan->name, NULL);
        }

        if (!live_channel_params->truncated) {
            break;
        }

        aos_str_set(&live_channel_params->marker, live_channel_params->next_marker.data);

    } while (1);

    /* delete test bucket */
    oss_delete_bucket(options, &bucket, &resp_headers);

    //printf("clean bucket %s done\n", bucket_name);
}

void clean_bucket_by_prefix(const char* prefix)
{
    //printf("clean_bucket_by_prefix:%s\n", prefix);

    aos_pool_t *p = NULL;
    oss_request_options_t *options = NULL;
    int is_cname = 0;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    oss_list_buckets_params_t *params = NULL;
    oss_list_bucket_content_t *content = NULL;

    /* list all buckets */
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_cname);
    params = oss_create_list_buckets_params(p);
    params->max_keys = 100;
    aos_str_set(&params->prefix, prefix);
    s = oss_list_bucket(options, params, &resp_headers);

    if (!aos_status_is_ok(s)) {
        return;
    }

    aos_list_for_each_entry(oss_list_bucket_content_t, content, &params->bucket_list, node) {
        clean_bucket(options, content->name.data);
    }

    aos_pool_destroy(p);
}

aos_status_t *init_test_multipart_upload(const oss_request_options_t *options, 
                                         const char *bucket_name, 
                                         const char *object_name, 
                                         aos_string_t *upload_id)
{
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_status_t *s;

    test_object_base();
    headers = aos_table_make(options->pool, 5);

    s = oss_init_multipart_upload(options, &bucket, &object, 
                                  upload_id, headers, &resp_headers);

    return s;
}

aos_status_t *abort_test_multipart_upload(const oss_request_options_t *options, 
                                          const char *bucket_name, 
                                          const char *object_name, 
                                          aos_string_t *upload_id)
{
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *resp_headers;
    aos_status_t *s;

    test_object_base();
    s = oss_abort_multipart_upload(options, &bucket, &object, upload_id, 
                                   &resp_headers);

    return s;
}

aos_status_t *create_test_live_channel(const oss_request_options_t *options,
    const char *bucket_name, const char *live_channel)
{
    aos_list_t publish_url_list;
    aos_list_t play_url_list;
    oss_live_channel_configuration_t *config = NULL;
    aos_string_t bucket;
    aos_string_t channel_id;

    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&channel_id, live_channel);
    aos_list_init(&publish_url_list);
    aos_list_init(&play_url_list);

    config = oss_create_live_channel_configuration_content(options->pool);
    aos_str_set(&config->name, live_channel);
    aos_str_set(&config->description, "live channel description");

    return  oss_create_live_channel(options, &bucket, config, &publish_url_list,
        &play_url_list, NULL);
}

aos_status_t *delete_test_live_channel(const oss_request_options_t *options,
    const char *bucket_name, const char *live_channel)
{
    aos_string_t bucket;
    aos_string_t channel_id;

    aos_str_set(&bucket, bucket_name);
    aos_str_set(&channel_id, live_channel);

    return oss_delete_live_channel(options, &bucket, &channel_id, NULL);
}

aos_status_t *get_image_info(const oss_request_options_t *options,
    const char *bucket_name, const char *object_name, image_info_t *info)
{
    aos_status_t *s = NULL;
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_list_t buffer;

    aos_list_init(&buffer);
    aos_str_set(&bucket, bucket_name);
    aos_str_set(&object, object_name);

    /* test get object to buffer */
    params = aos_table_make(options->pool, 1);
    apr_table_set(params, OSS_PROCESS, "image/info");
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, params, &buffer, &resp_headers);

    if (aos_status_is_ok(s)) {
        aos_buf_t *content = NULL;
        char *buf = NULL;
        int64_t len = 0;
        int64_t size = 0;
        int64_t pos = 0;
        cJSON * root = NULL;
        cJSON * item = NULL;

        /* get buffer len */
        len = aos_buf_list_len(&buffer);
        buf = (char *)aos_pcalloc(options->pool, (apr_size_t)(len + 1));
        buf[len] = '\0';
        
        /* copy buffer content to memory */
        aos_list_for_each_entry(aos_buf_t, content, &buffer, node) {
            size = aos_buf_size(content);
            memcpy(buf + pos, content->pos, (size_t)size);
            pos += size;
        }
        
        /* parse image info from json */
        root = cJSON_Parse(buf);
        
        item = cJSON_GetObjectItem(root, "ImageHeight");
        item = cJSON_GetObjectItem(item, "value");
        info->height = atol(item->valuestring);
        
        item = cJSON_GetObjectItem(root, "ImageWidth");
        item = cJSON_GetObjectItem(item, "value");
        info->width = atol(item->valuestring);
        
        item = cJSON_GetObjectItem(root, "FileSize");
        item = cJSON_GetObjectItem(item, "value");
        info->size = atol(item->valuestring);
        
        item = cJSON_GetObjectItem(root, "Format");
        item = cJSON_GetObjectItem(item, "value");
        apr_snprintf(info->format, 64, "%s", item->valuestring);
        
        cJSON_Delete(root);
    }
    return s;
}


char* gen_test_signed_url(const oss_request_options_t *options, 
                          const char *bucket_name,
                          const char *object_name, 
                          int64_t expires, 
                          aos_http_request_t *req)
{
    aos_string_t bucket;
    aos_string_t object;
    char *signed_url = NULL;

    aos_str_set(&bucket, bucket_name);
    aos_str_set(&object, object_name);
    signed_url = oss_gen_signed_url(options, &bucket, &object, expires, req);
    return signed_url;
}

unsigned long get_file_size(const char *file_path)
{
    unsigned long filesize = -1; 
    struct stat statbuff;

    if(stat(file_path, &statbuff) < 0){
        return filesize;
    } else {
        filesize = statbuff.st_size;
    }

    return filesize;
}

char *decrypt(const char *encrypted_str, aos_pool_t *pool)
{
    char *res_str = NULL;
    int i = 0;

    if (encrypted_str == NULL) {
        return NULL;
    }

    res_str =  (char *)aos_palloc(pool, strlen(encrypted_str) + 1);

    while (*encrypted_str != '\0') {
        res_str[i] = 0x6a ^ *encrypted_str;
        encrypted_str++;
        i++;
    }
    res_str[i] = '\0';

    return res_str;
}

void progress_callback(int64_t consumed_bytes, int64_t total_bytes) 
{
    assert(total_bytes >= consumed_bytes);  
}

void percentage(int64_t consumed_bytes, int64_t total_bytes) 
{
    assert(total_bytes >= consumed_bytes);
}

char * get_text_file_data(aos_pool_t *pool, const char *filepath)
{
    apr_status_t s;
    apr_file_t *thefile;
    apr_finfo_t finfo;
    apr_size_t nread = 0;
    apr_off_t offset = 0;
    char *buf = NULL;

    s = apr_file_open(&thefile, filepath, APR_READ, APR_UREAD | APR_GREAD, pool);
    if (s != APR_SUCCESS) {
        return NULL;
    }

    s = apr_file_info_get(&finfo, APR_FINFO_SIZE | APR_FINFO_MTIME, thefile);
    if (s != APR_SUCCESS) {
        apr_file_close(thefile);
        return NULL;
    }

    nread = (apr_size_t)finfo.size;
    buf = aos_pcalloc(pool, nread + 1);
    apr_file_seek(thefile, APR_SET, &offset);
    apr_file_read(thefile, buf, &nread);
    buf[nread] = '\0';
    apr_file_close(thefile);
    return buf;
}

char *get_test_file_path()
{
    static int flag = 0;
    static char path[1024];
#if defined(WIN32)
    char ch = '\\';
#else
    char ch = '/';
#endif
    if (!flag) {
        char *filepath = __FILE__;
        char * pos = strrchr(filepath, ch);
        if (pos) {
            int len = (int)(pos - filepath + 1);
            sprintf(path, "%.*s", len, filepath);
        } else {
            sprintf(path, "oss_c_sdk_test%c", ch);
        }
        flag = 1;
    }
    return path;
}

char *get_test_bucket_name(aos_pool_t *p, const char* suffix)
{
    return apr_psprintf(p, "%s-%s-bucket", bucket_prefix, suffix);
}

void set_test_bucket_prefix(const char* prefix)
{
    sprintf(bucket_prefix, "%s", prefix);
}

