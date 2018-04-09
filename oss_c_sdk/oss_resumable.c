#include "aos_log.h"
#include "aos_define.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "aos_crc64.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_xml.h"
#include "oss_api.h"
#include "oss_resumable.h"

int32_t oss_get_thread_num(oss_resumable_clt_params_t *clt_params)
{
    if ((NULL == clt_params) || (clt_params->thread_num <= 0 || clt_params->thread_num > 1024)) {
        return 1;
    }
    return clt_params->thread_num;
}

void oss_get_upload_checkpoint_path(oss_resumable_clt_params_t *clt_params, const aos_string_t *filepath, 
                             aos_pool_t *pool, aos_string_t *checkpoint_path)
{
    if ((NULL == checkpoint_path) || (NULL == clt_params) || (!clt_params->enable_checkpoint)) {
        return;
    }

    if (aos_is_null_string(&clt_params->checkpoint_path)) {
        int len = filepath->len + strlen(".ucp") + 1;
        char *buffer = (char *)aos_pcalloc(pool, len);
        apr_snprintf(buffer, len, "%.*s.ucp", filepath->len, filepath->data);
        aos_str_set(checkpoint_path , buffer);
        return;
    }

    checkpoint_path->data = clt_params->checkpoint_path.data;
    checkpoint_path->len = clt_params->checkpoint_path.len;
}

void oss_get_download_checkpoint_path(oss_resumable_clt_params_t *clt_params, const aos_string_t *filepath, 
                             aos_pool_t *pool, aos_string_t *checkpoint_path)
{
    if ((NULL == checkpoint_path) || (NULL == clt_params) || (!clt_params->enable_checkpoint)) {
        return;
    }

    if (aos_is_null_string(&clt_params->checkpoint_path)) {
        int len = filepath->len + strlen(".dcp") + 1;
        char *buffer = (char *)aos_pcalloc(pool, len);
        apr_snprintf(buffer, len, "%.*s.dcp", filepath->len, filepath->data);
        aos_str_set(checkpoint_path , buffer);
        return;
    }

    checkpoint_path->data = clt_params->checkpoint_path.data;
    checkpoint_path->len = clt_params->checkpoint_path.len;
}

int oss_get_file_info(const aos_string_t *filepath, aos_pool_t *pool, apr_finfo_t *finfo) 
{
    apr_status_t s;
    char buf[256];
    apr_file_t *thefile;

    s = apr_file_open(&thefile, filepath->data, APR_READ, APR_UREAD | APR_GREAD, pool);
    if (s != APR_SUCCESS) {
        aos_error_log("apr_file_open failure, code:%d %s.", s, apr_strerror(s, buf, sizeof(buf)));
        return s;
    }

    s = apr_file_info_get(finfo, APR_FINFO_SIZE | APR_FINFO_MTIME, thefile);
    if (s != APR_SUCCESS) {
        apr_file_close(thefile);
        aos_error_log("apr_file_info_get failure, code:%d %s.", s, apr_strerror(s, buf, sizeof(buf)));
        return s;
    }
    apr_file_close(thefile);

    return AOSE_OK;
}

int oss_does_file_exist(const aos_string_t *filepath, aos_pool_t *pool) 
{
    apr_status_t s;
    apr_file_t *thefile;

    s = apr_file_open(&thefile, filepath->data, APR_READ, APR_UREAD | APR_GREAD, pool);
    if (s != APR_SUCCESS) {
        return AOS_FALSE;
    }

    apr_file_close(thefile);
    return AOS_TRUE;
}

int oss_open_checkpoint_file(aos_pool_t *pool,  aos_string_t *checkpoint_path, oss_checkpoint_t *checkpoint) 
{
    apr_status_t s;
    apr_file_t *thefile;
    char buf[256];
    s = apr_file_open(&thefile, checkpoint_path->data, APR_CREATE | APR_WRITE, APR_UREAD | APR_UWRITE | APR_GREAD, pool);
    if (s == APR_SUCCESS) {
        checkpoint->thefile = thefile;
    } else {
        aos_error_log("apr_file_open failure, code:%d %s.", s, apr_strerror(s, buf, sizeof(buf)));
    }
    return s;
}

int oss_get_part_num(int64_t file_size, int64_t part_size)
{
    int64_t num = 0;
    int64_t left = 0;
    left = (file_size % part_size == 0) ? 0 : 1;
    num = file_size / part_size + left;
    return (int)num;
}

void oss_build_parts(int64_t file_size, int64_t part_size, oss_checkpoint_part_t *parts)
{
    int i = 0;
    for (; i * part_size < file_size; i++) {
        parts[i].index = i;
        parts[i].offset = i * part_size;
        parts[i].size = aos_min(part_size, (file_size - i * part_size));
        parts[i].completed = AOS_FALSE;
    }
}

void oss_build_thread_params(oss_thread_params_t *thd_params, int part_num, 
                             aos_pool_t *parent_pool, oss_request_options_t *options, 
                             aos_string_t *bucket, aos_string_t *object, aos_string_t *filepath,
                             aos_string_t *upload_id, oss_checkpoint_part_t *parts,
                             oss_part_task_result_t *result) 
{
    int i = 0;
    aos_pool_t *subpool = NULL;
    oss_config_t *config = NULL;
    aos_http_controller_t *ctl;
    for (; i < part_num; i++) {
        aos_pool_create(&subpool, parent_pool); 
        config = oss_config_create(subpool);
        memcpy(config, options->config, sizeof(oss_config_t));
        ctl = aos_http_controller_create(subpool, 0);
        thd_params[i].options.config = config;
        thd_params[i].options.ctl = ctl;
        thd_params[i].options.pool = subpool;
        thd_params[i].bucket = bucket;
        thd_params[i].object = object;
        thd_params[i].filepath = filepath;
        thd_params[i].upload_id = upload_id;
        thd_params[i].part = parts + i;
        thd_params[i].result = result + i;
        thd_params[i].result->part = thd_params[i].part;
    }
}

void oss_destroy_thread_pool(oss_thread_params_t *thd_params, int part_num) 
{
    int i = 0;
    for (; i < part_num; i++) {
        aos_pool_destroy(thd_params[i].options.pool);
    }
}

void oss_set_task_tracker(oss_thread_params_t *thd_params, int part_num, 
                          apr_uint32_t *launched, apr_uint32_t *failed, apr_uint32_t *completed,
                          apr_queue_t *failed_parts, apr_queue_t *completed_parts) 
{
    int i = 0;
    for (; i < part_num; i++) {
        thd_params[i].launched = launched;
        thd_params[i].failed = failed;
        thd_params[i].completed = completed;
        thd_params[i].failed_parts = failed_parts;
        thd_params[i].completed_parts = completed_parts;
    }
}

int oss_verify_checkpoint_md5(aos_pool_t *pool, const oss_checkpoint_t *checkpoint)
{
    return AOS_TRUE;
}

void oss_build_upload_checkpoint(aos_pool_t *pool, oss_checkpoint_t *checkpoint, aos_string_t *file_path, 
                                 apr_finfo_t *finfo, aos_string_t *upload_id, int64_t part_size) 
{
    int i = 0;

    checkpoint->cp_type = OSS_CP_UPLOAD;
    aos_str_set(&checkpoint->file_path, aos_pstrdup(pool, file_path));
    checkpoint->file_size = finfo->size;
    checkpoint->file_last_modified = finfo->mtime;
    aos_str_set(&checkpoint->upload_id, aos_pstrdup(pool, upload_id));

    checkpoint->part_size = part_size;
    for (; i * part_size < finfo->size; i++) {
        checkpoint->parts[i].index = i;
        checkpoint->parts[i].offset = i * part_size;
        checkpoint->parts[i].size = aos_min(part_size, (finfo->size - i * part_size));
        checkpoint->parts[i].completed = AOS_FALSE;
        aos_str_set(&checkpoint->parts[i].etag , "");
    }
    checkpoint->part_num = i;
}

void oss_build_download_checkpoint(aos_pool_t *pool, oss_checkpoint_t *checkpoint, aos_string_t *file_path, 
        const char *object_name, int64_t object_size, const char *object_last_modified, 
        const char *object_etag, int64_t part_size) 
{
    int i = 0;

    checkpoint->cp_type = OSS_CP_DOWNLOAD;
    checkpoint->thefile = NULL;
    aos_str_set(&checkpoint->file_path, aos_pstrdup(pool, file_path));
    aos_str_set(&checkpoint->object_name, object_name);
    checkpoint->object_size = object_size;
    aos_str_set(&checkpoint->object_last_modified, object_last_modified);
    aos_str_set(&checkpoint->object_etag, object_etag);

    checkpoint->part_size = part_size;
    for (; i * part_size < object_size; i++) {
        checkpoint->parts[i].index = i;
        checkpoint->parts[i].offset = i * part_size;
        checkpoint->parts[i].size = aos_min(part_size, (object_size - i * part_size));
        checkpoint->parts[i].completed = AOS_FALSE;
        aos_str_set(&checkpoint->parts[i].etag , "");
    }
    checkpoint->part_num = i;
}


int oss_dump_checkpoint(aos_pool_t *pool, const oss_checkpoint_t *checkpoint) 
{
    char *xml_body = NULL;
    apr_status_t s;
    char buf[256];
    apr_size_t len;
    
    // to xml
    xml_body = oss_build_checkpoint_xml(pool, checkpoint);
    if (NULL == xml_body) {
        return AOSE_OUT_MEMORY;
    }

    // truncate to empty
    s = apr_file_trunc(checkpoint->thefile, 0);
    if (s != APR_SUCCESS) {
        aos_error_log("apr_file_write failure, code:%d %s.", s, apr_strerror(s, buf, sizeof(buf)));
        return AOSE_FILE_TRUNC_ERROR;
    }
   
    // write to file
    len = strlen(xml_body);
    s = apr_file_write(checkpoint->thefile, xml_body, &len);
    if (s != APR_SUCCESS) {
        aos_error_log("apr_file_write failure, code:%d %s.", s, apr_strerror(s, buf, sizeof(buf)));
        return AOSE_FILE_WRITE_ERROR;
    }

    // flush file
    s = apr_file_flush(checkpoint->thefile);
    if (s != APR_SUCCESS) {
        aos_error_log("apr_file_flush failure, code:%d %s.", s, apr_strerror(s, buf, sizeof(buf)));
        return AOSE_FILE_FLUSH_ERROR;
    }

    return AOSE_OK;
}

int oss_load_checkpoint(aos_pool_t *pool, const aos_string_t *filepath, oss_checkpoint_t *checkpoint) 
{
    apr_status_t s;
    char buf[256];
    apr_size_t len;
    apr_finfo_t finfo;
    char *xml_body = NULL;
    apr_file_t *thefile;

    // open file
    s = apr_file_open(&thefile, filepath->data, APR_READ, APR_UREAD | APR_GREAD, pool);
    if (s != APR_SUCCESS) {
        aos_error_log("apr_file_open failure, code:%d %s.", s, apr_strerror(s, buf, sizeof(buf)));
        return AOSE_OPEN_FILE_ERROR;
    }

    // get file stat
    s = apr_file_info_get(&finfo, APR_FINFO_SIZE, thefile);
    if (s != APR_SUCCESS) {
        aos_error_log("apr_file_info_get failure, code:%d %s.", s, apr_strerror(s, buf, sizeof(buf)));
        apr_file_close(thefile);
        return AOSE_FILE_INFO_ERROR;
    }

    xml_body = (char *)aos_palloc(pool, (apr_size_t)(finfo.size + 1));

    // read
    s = apr_file_read_full(thefile, xml_body, (apr_size_t)finfo.size, &len);
    if (s != APR_SUCCESS) {
        aos_error_log("apr_file_read_full failure, code:%d %s.", s, apr_strerror(s, buf, sizeof(buf)));
        apr_file_close(thefile);
        return AOSE_FILE_READ_ERROR;
    }
    apr_file_close(thefile);
    xml_body[len] = '\0';

    // parse
    return oss_checkpoint_parse_from_body(pool, xml_body, checkpoint);
}

int oss_is_upload_checkpoint_valid(aos_pool_t *pool, oss_checkpoint_t *checkpoint, apr_finfo_t *finfo)
{
    if (oss_verify_checkpoint_md5(pool, checkpoint) && 
            (checkpoint->cp_type == OSS_CP_UPLOAD) && 
            (checkpoint->file_size == finfo->size) && 
            (checkpoint->file_last_modified == finfo->mtime)) {
        return AOS_TRUE;
    }
    return AOS_FALSE;
}

int oss_is_download_checkpoint_valid(aos_pool_t *pool, 
        oss_checkpoint_t *checkpoint, const char *object_name, 
        int64_t object_size, const char *object_last_modified, 
        const char *object_etag)
{
    if (oss_verify_checkpoint_md5(pool, checkpoint) && 
            (checkpoint->cp_type == OSS_CP_DOWNLOAD) && 
            (checkpoint->object_size == object_size) && 
            !strcmp(checkpoint->object_last_modified.data, object_last_modified) &&
            !strcasecmp(checkpoint->object_etag.data, object_etag)) {
        return AOS_TRUE;
    }
    return AOS_FALSE;
}



void oss_update_checkpoint(aos_pool_t *pool, oss_checkpoint_t *checkpoint, 
        int32_t part_index, aos_string_t *etag, uint64_t crc64) 
{
    char *p = NULL;
    checkpoint->parts[part_index].completed = AOS_TRUE;
    p = apr_pstrdup(pool, etag->data);
    aos_str_set(&checkpoint->parts[part_index].etag, p);
    checkpoint->parts[part_index].crc64 = crc64;
}

void oss_get_checkpoint_todo_parts(oss_checkpoint_t *checkpoint, int *part_num, oss_checkpoint_part_t *parts)
{
    int i = 0;
    int idx = 0;
    for (; i < checkpoint->part_num; i++) {
        if (!checkpoint->parts[i].completed) {
            parts[idx].index = checkpoint->parts[i].index;
            parts[idx].offset = checkpoint->parts[i].offset;
            parts[idx].size = checkpoint->parts[i].size;
            parts[idx].completed = checkpoint->parts[i].completed;
            parts[idx].crc64 = checkpoint->parts[i].crc64;
            idx++;
        }
    }
    *part_num = idx;
}

void * APR_THREAD_FUNC upload_part(apr_thread_t *thd, void *data) 
{
    aos_status_t *s = NULL;
    oss_thread_params_t *params = NULL;
    oss_upload_file_t *upload_file = NULL;
    aos_table_t *resp_headers = NULL;
    int part_num;
    char *etag;
    
    params = (oss_thread_params_t *)data;
    if (apr_atomic_read32(params->failed) > 0) {
        apr_atomic_inc32(params->launched);
        return NULL;
    }

    part_num = params->part->index + 1;
    upload_file = oss_create_upload_file(params->options.pool);
    aos_str_set(&upload_file->filename, params->filepath->data);
    upload_file->file_pos = params->part->offset;
    upload_file->file_last = params->part->offset + params->part->size;

    s = oss_upload_part_from_file(&params->options, params->bucket, params->object, params->upload_id,
        part_num, upload_file, &resp_headers);
    if (!aos_status_is_ok(s)) {
        apr_atomic_inc32(params->failed);
        params->result->s = s;
        apr_queue_push(params->failed_parts, params->result);
        return s;
    }

    etag = apr_pstrdup(params->options.pool, (char*)apr_table_get(resp_headers, "ETag"));
    aos_str_set(&params->result->etag, etag);
    apr_atomic_inc32(params->completed);
    apr_queue_push(params->completed_parts, params->result);
    return NULL;
}

aos_status_t *oss_resumable_upload_file_without_cp(oss_request_options_t *options,
                                                   aos_string_t *bucket, 
                                                   aos_string_t *object, 
                                                   aos_string_t *filepath,                           
                                                   aos_table_t *headers,
                                                   aos_table_t *params,
                                                   int32_t thread_num,
                                                   int64_t part_size,
                                                   apr_finfo_t *finfo,
                                                   oss_progress_callback progress_callback,
                                                   aos_table_t **resp_headers,
                                                   aos_list_t *resp_body) 
{
    aos_pool_t *subpool = NULL;
    aos_pool_t *parent_pool = NULL;
    aos_status_t *s = NULL;
    aos_status_t *ret = NULL;
    aos_list_t completed_part_list;
    oss_complete_part_content_t *complete_content = NULL;
    aos_string_t upload_id;
    oss_checkpoint_part_t *parts;
    oss_part_task_result_t *results;
    oss_part_task_result_t *task_res;
    oss_thread_params_t *thd_params;
    aos_table_t *cb_headers = NULL;
    apr_thread_pool_t *thdp;
    apr_uint32_t launched = 0;
    apr_uint32_t failed = 0;
    apr_uint32_t completed = 0;
    apr_uint32_t total_num = 0;
    apr_queue_t *failed_parts;
    apr_queue_t *completed_parts;
    int64_t consume_bytes = 0;
    void *task_result;
    char *part_num_str;
    char *etag;
    int part_num = 0;
    int i = 0;
    int rv;

    // prepare
    parent_pool = options->pool;
    ret = aos_status_create(parent_pool);
    part_num = oss_get_part_num(finfo->size, part_size);
    parts = (oss_checkpoint_part_t *)aos_palloc(parent_pool, sizeof(oss_checkpoint_part_t) * part_num);
    oss_build_parts(finfo->size, part_size, parts);
    results = (oss_part_task_result_t *)aos_palloc(parent_pool, sizeof(oss_part_task_result_t) * part_num);
    thd_params = (oss_thread_params_t *)aos_palloc(parent_pool, sizeof(oss_thread_params_t) * part_num);
    oss_build_thread_params(thd_params, part_num, parent_pool, options, bucket, object, filepath, &upload_id, parts, results);
    
    // init upload
    aos_pool_create(&subpool, parent_pool);
    options->pool = subpool;
    s = oss_init_multipart_upload(options, bucket, object, &upload_id, headers, resp_headers);
    if (!aos_status_is_ok(s)) {
        s = aos_status_dup(parent_pool, s);
        aos_pool_destroy(subpool);
        options->pool = parent_pool;
        return s;
    }
    aos_str_set(&upload_id, apr_pstrdup(parent_pool, upload_id.data));
    options->pool = parent_pool;
    aos_pool_destroy(subpool);

    // upload parts    
    rv = apr_thread_pool_create(&thdp, 0, thread_num, parent_pool);
    if (APR_SUCCESS != rv) {
        aos_status_set(ret, rv, AOS_CREATE_THREAD_POOL_ERROR_CODE, NULL); 
        return ret;
    }

    rv = apr_queue_create(&failed_parts, part_num, parent_pool);
    if (APR_SUCCESS != rv) {
        aos_status_set(ret, rv, AOS_CREATE_QUEUE_ERROR_CODE, NULL); 
        return ret;
    }

    rv = apr_queue_create(&completed_parts, part_num, parent_pool);
    if (APR_SUCCESS != rv) {
        aos_status_set(ret, rv, AOS_CREATE_QUEUE_ERROR_CODE, NULL); 
        return ret;
    }

    // launch
    oss_set_task_tracker(thd_params, part_num, &launched, &failed, &completed, failed_parts, completed_parts);
    for (i = 0; i < part_num; i++) {
        apr_thread_pool_push(thdp, upload_part, thd_params + i, 0, NULL);
    }

    // wait until all tasks exit
    total_num = apr_atomic_read32(&launched) + apr_atomic_read32(&failed) + apr_atomic_read32(&completed);
    for ( ; total_num < (apr_uint32_t)part_num; ) {
        rv = apr_queue_trypop(completed_parts, &task_result);
        if (rv == APR_EINTR || rv == APR_EAGAIN) {
            apr_sleep(1000);
        } else if(rv == APR_EOF) {
            break;
        } else if(rv == APR_SUCCESS) {
            task_res = (oss_part_task_result_t*)task_result;
            if (NULL != progress_callback) {
                consume_bytes += task_res->part->size;
                progress_callback(consume_bytes, finfo->size);
            }
        }
        total_num = apr_atomic_read32(&launched) + apr_atomic_read32(&failed) + apr_atomic_read32(&completed);
    }

    // deal with left successful parts
    while(APR_SUCCESS == apr_queue_trypop(completed_parts, &task_result)) {
        task_res = (oss_part_task_result_t*)task_result;
        if (NULL != progress_callback) {
            consume_bytes += task_res->part->size;
            progress_callback(consume_bytes, finfo->size);
        }
    }

    // failed
    if (apr_atomic_read32(&failed) > 0) {
        apr_queue_pop(failed_parts, &task_result);
        task_res = (oss_part_task_result_t*)task_result;
        s = aos_status_dup(parent_pool, task_res->s);
        oss_destroy_thread_pool(thd_params, part_num);
        return s;
    }

    // successful
    aos_pool_create(&subpool, parent_pool);
    aos_list_init(&completed_part_list);
    for (i = 0; i < part_num; i++) {
        complete_content = oss_create_complete_part_content(subpool);
        part_num_str = apr_psprintf(subpool, "%d", thd_params[i].part->index + 1);
        aos_str_set(&complete_content->part_number, part_num_str);
        etag = apr_pstrdup(subpool, thd_params[i].result->etag.data);
        aos_str_set(&complete_content->etag, etag);
        aos_list_add_tail(&complete_content->node, &completed_part_list);
    }
    oss_destroy_thread_pool(thd_params, part_num);

    // complete upload
    options->pool = subpool;
    if (NULL != headers && NULL != apr_table_get(headers, OSS_CALLBACK)) {
        cb_headers = aos_table_make(subpool, 2);
        apr_table_set(cb_headers, OSS_CALLBACK, apr_table_get(headers, OSS_CALLBACK));
        if (NULL != apr_table_get(headers, OSS_CALLBACK_VAR)) {
            apr_table_set(cb_headers, OSS_CALLBACK_VAR, apr_table_get(headers, OSS_CALLBACK_VAR));
        }
    }
    s = oss_do_complete_multipart_upload(options, bucket, object, &upload_id, 
        &completed_part_list, cb_headers, NULL, resp_headers, resp_body);
    s = aos_status_dup(parent_pool, s);
    aos_pool_destroy(subpool);
    options->pool = parent_pool;

    return s;
}

aos_status_t *oss_resumable_upload_file_with_cp(oss_request_options_t *options,
                                                aos_string_t *bucket, 
                                                aos_string_t *object, 
                                                aos_string_t *filepath,                           
                                                aos_table_t *headers,
                                                aos_table_t *params,
                                                int32_t thread_num,
                                                int64_t part_size,
                                                aos_string_t *checkpoint_path,
                                                apr_finfo_t *finfo,
                                                oss_progress_callback progress_callback,
                                                aos_table_t **resp_headers,
                                                aos_list_t *resp_body) 
{
    aos_pool_t *subpool = NULL;
    aos_pool_t *parent_pool = NULL;
    aos_status_t *s = NULL;
    aos_status_t *ret = NULL;
    aos_list_t completed_part_list;
    oss_complete_part_content_t *complete_content = NULL;
    aos_string_t upload_id;
    oss_checkpoint_part_t *parts;
    oss_part_task_result_t *results;
    oss_part_task_result_t *task_res;
    oss_thread_params_t *thd_params;
    aos_table_t *cb_headers = NULL;
    apr_thread_pool_t *thdp;
    apr_uint32_t launched = 0;
    apr_uint32_t failed = 0;
    apr_uint32_t completed = 0;
    apr_uint32_t total_num = 0;
    apr_queue_t *failed_parts;
    apr_queue_t *completed_parts;
    oss_checkpoint_t *checkpoint = NULL;
    int need_init_upload = AOS_TRUE;
    int has_left_result = AOS_FALSE;
    int64_t consume_bytes = 0;
    void *task_result;
    char *part_num_str;
    int part_num = 0;
    int i = 0;
    int rv;

    // checkpoint
    parent_pool = options->pool;
    ret = aos_status_create(parent_pool);
    checkpoint = oss_create_checkpoint_content(parent_pool);
    if(oss_does_file_exist(checkpoint_path, parent_pool)) {
        if (AOSE_OK == oss_load_checkpoint(parent_pool, checkpoint_path, checkpoint) && 
            oss_is_upload_checkpoint_valid(parent_pool, checkpoint, finfo)) {
                aos_str_set(&upload_id, checkpoint->upload_id.data);
                need_init_upload = AOS_FALSE;
        } else {
            apr_file_remove(checkpoint_path->data, parent_pool);
        }
    }

    if (need_init_upload) {
        // init upload 
        aos_pool_create(&subpool, parent_pool);
        options->pool = subpool;
        s = oss_init_multipart_upload(options, bucket, object, &upload_id, headers, resp_headers);
        if (!aos_status_is_ok(s)) {
            s = aos_status_dup(parent_pool, s);
            aos_pool_destroy(subpool);
            options->pool = parent_pool;
            return s;
        }
        aos_str_set(&upload_id, apr_pstrdup(parent_pool, upload_id.data));
        options->pool = parent_pool;
        aos_pool_destroy(subpool);

        // build checkpoint
        oss_build_upload_checkpoint(parent_pool, checkpoint, filepath, finfo, &upload_id, part_size);
    }

    rv = oss_open_checkpoint_file(parent_pool, checkpoint_path, checkpoint);
    if (rv != APR_SUCCESS) {
        aos_status_set(ret, rv, AOS_OPEN_FILE_ERROR_CODE, NULL);
        return ret;
    }

    // prepare
    ret = aos_status_create(parent_pool);
    parts = (oss_checkpoint_part_t *)aos_palloc(parent_pool, sizeof(oss_checkpoint_part_t) * (checkpoint->part_num));
    oss_get_checkpoint_todo_parts(checkpoint, &part_num, parts);
    results = (oss_part_task_result_t *)aos_palloc(parent_pool, sizeof(oss_part_task_result_t) * part_num);
    thd_params = (oss_thread_params_t *)aos_palloc(parent_pool, sizeof(oss_thread_params_t) * part_num);
    oss_build_thread_params(thd_params, part_num, parent_pool, options, bucket, object, filepath, &upload_id, parts, results);

    // upload parts    
    rv = apr_thread_pool_create(&thdp, 0, thread_num, parent_pool);
    if (APR_SUCCESS != rv) {
        aos_status_set(ret, rv, AOS_CREATE_THREAD_POOL_ERROR_CODE, NULL); 
        return ret;
    }

    rv = apr_queue_create(&failed_parts, part_num, parent_pool);
    if (APR_SUCCESS != rv) {
        aos_status_set(ret, rv, AOS_CREATE_QUEUE_ERROR_CODE, NULL); 
        return ret;
    }

    rv = apr_queue_create(&completed_parts, part_num, parent_pool);
    if (APR_SUCCESS != rv) {
        aos_status_set(ret, rv, AOS_CREATE_QUEUE_ERROR_CODE, NULL); 
        return ret;
    }

    // launch
    oss_set_task_tracker(thd_params, part_num, &launched, &failed, &completed, failed_parts, completed_parts);
    for (i = 0; i < part_num; i++) {
        apr_thread_pool_push(thdp, upload_part, thd_params + i, 0, NULL);
    }

    // wait until all tasks exit
    total_num = apr_atomic_read32(&launched) + apr_atomic_read32(&failed) + apr_atomic_read32(&completed);
    for ( ; total_num < (apr_uint32_t)part_num; ) {
        rv = apr_queue_trypop(completed_parts, &task_result);
        if (rv == APR_EINTR || rv == APR_EAGAIN) {
            apr_sleep(1000);
        } else if(rv == APR_EOF) {
            break;
        } else if(rv == APR_SUCCESS) {
            task_res = (oss_part_task_result_t*)task_result;
            oss_update_checkpoint(parent_pool, checkpoint, task_res->part->index, &task_res->etag, 0);
            rv = oss_dump_checkpoint(parent_pool, checkpoint);
            if (rv != AOSE_OK) {
                int idx = task_res->part->index;
                aos_status_set(ret, rv, AOS_WRITE_FILE_ERROR_CODE, NULL);
                apr_atomic_inc32(&failed);
                thd_params[idx].result->s = ret;
                apr_queue_push(failed_parts, thd_params[idx].result);
            }
            if (NULL != progress_callback) {
                consume_bytes += task_res->part->size;
                progress_callback(consume_bytes, finfo->size);
            }
        }
        total_num = apr_atomic_read32(&launched) + apr_atomic_read32(&failed) + apr_atomic_read32(&completed);
    }

    // deal with left successful parts
    while(APR_SUCCESS == apr_queue_trypop(completed_parts, &task_result)) {
        task_res = (oss_part_task_result_t*)task_result;
        oss_update_checkpoint(parent_pool, checkpoint, task_res->part->index, &task_res->etag, 0);
        consume_bytes += task_res->part->size;
        has_left_result = AOS_TRUE;
    }
    if (has_left_result) {
        rv = oss_dump_checkpoint(parent_pool, checkpoint);
        if (rv != AOSE_OK) {
            aos_status_set(ret, rv, AOS_WRITE_FILE_ERROR_CODE, NULL);
            return ret;
        }
        if (NULL != progress_callback) {
            progress_callback(consume_bytes, finfo->size);
        }
    }
    apr_file_close(checkpoint->thefile);

    // failed
    if (apr_atomic_read32(&failed) > 0) {
        apr_queue_pop(failed_parts, &task_result);
        task_res = (oss_part_task_result_t*)task_result;
        s = aos_status_dup(parent_pool, task_res->s);
        oss_destroy_thread_pool(thd_params, part_num);
        return s;
    }
    
    // successful
    aos_pool_create(&subpool, parent_pool);
    aos_list_init(&completed_part_list);
    for (i = 0; i < checkpoint->part_num; i++) {
        complete_content = oss_create_complete_part_content(subpool);
        part_num_str = apr_psprintf(subpool, "%d", checkpoint->parts[i].index + 1);
        aos_str_set(&complete_content->part_number, part_num_str);
        aos_str_set(&complete_content->etag, checkpoint->parts[i].etag.data);
        aos_list_add_tail(&complete_content->node, &completed_part_list);
    }
    oss_destroy_thread_pool(thd_params, part_num);

    // complete upload
    options->pool = subpool;
    if (NULL != headers && NULL != apr_table_get(headers, OSS_CALLBACK)) {
        cb_headers = aos_table_make(subpool, 2);
        apr_table_set(cb_headers, OSS_CALLBACK, apr_table_get(headers, OSS_CALLBACK));
        if (NULL != apr_table_get(headers, OSS_CALLBACK_VAR)) {
            apr_table_set(cb_headers, OSS_CALLBACK_VAR, apr_table_get(headers, OSS_CALLBACK_VAR));
        }
    }
    s = oss_do_complete_multipart_upload(options, bucket, object, &upload_id, 
        &completed_part_list, cb_headers, NULL, resp_headers, resp_body);
    s = aos_status_dup(parent_pool, s);
    aos_pool_destroy(subpool);
    options->pool = parent_pool;

    // remove chepoint file
    apr_file_remove(checkpoint_path->data, parent_pool);
    
    return s;
}

aos_status_t *oss_resumable_upload_file(oss_request_options_t *options,
                                        aos_string_t *bucket, 
                                        aos_string_t *object, 
                                        aos_string_t *filepath,                           
                                        aos_table_t *headers,
                                        aos_table_t *params,
                                        oss_resumable_clt_params_t *clt_params, 
                                        oss_progress_callback progress_callback,
                                        aos_table_t **resp_headers,
                                        aos_list_t *resp_body) 
{
    int32_t thread_num = 0;
    int64_t part_size = 0;
    aos_string_t checkpoint_path;
    aos_pool_t *sub_pool;
    apr_finfo_t finfo;
    aos_status_t *s;
    int res;

    thread_num = oss_get_thread_num(clt_params);

    aos_pool_create(&sub_pool, options->pool);
    res = oss_get_file_info(filepath, sub_pool, &finfo);
    if (res != AOSE_OK) {
        aos_error_log("Open read file fail, filename:%s\n", filepath->data);
        s = aos_status_create(options->pool);
        aos_file_error_status_set(s, res);
        aos_pool_destroy(sub_pool);
        return s;
    }
    part_size = clt_params->part_size;
    oss_get_part_size(finfo.size, &part_size);

    if (NULL != clt_params && clt_params->enable_checkpoint) {
        oss_get_upload_checkpoint_path(clt_params, filepath, sub_pool, &checkpoint_path);
        s = oss_resumable_upload_file_with_cp(options, bucket, object, filepath, headers, params, thread_num, 
            part_size, &checkpoint_path, &finfo, progress_callback, resp_headers, resp_body);
    } else {
        s = oss_resumable_upload_file_without_cp(options, bucket, object, filepath, headers, params, thread_num, 
            part_size, &finfo, progress_callback, resp_headers, resp_body);
    }

    aos_pool_destroy(sub_pool);
    return s;
}


static void download_part(oss_request_options_t *options,
                          const aos_string_t *bucket, 
                          const aos_string_t *object, 
                          oss_checkpoint_part_t *part, 
                          const aos_string_t *filepath, 
                          oss_part_task_result_t *result)
{
    aos_status_t *s = NULL;
    aos_table_t *resp_headers = NULL;

    int rv;
    aos_http_request_t *req = NULL;
    aos_http_response_t *resp = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *query_params = NULL;
    aos_file_buf_t *fb = NULL;
    apr_off_t offset = 0;
   
    headers = aos_table_create_if_null(options, headers, 0);
    query_params = aos_table_create_if_null(options, query_params, 0);

    oss_headers_add_range(options->pool, headers, part->offset, part->size);

    fb = aos_create_file_buf(options->pool);
   
    if ((rv = aos_open_file_for_write_notrunc(options->pool, 
                    filepath->data, fb)) != AOSE_OK) {
        aos_error_log("Open write file fail, filename:%s\n", filepath->data);
        
        result->s = aos_status_create(options->pool);
        aos_file_error_status_set(result->s, rv);

        return;
    }
    offset = part->offset;
    apr_file_seek(fb->file, APR_SET, &offset);

    oss_init_object_request(options, bucket, object, HTTP_GET,
            &req, query_params, headers, NULL, 0, &resp);
    oss_init_read_response_body_to_fb(fb, filepath, resp);

    s = oss_process_request(options, req, resp);

    if (!aos_status_is_ok(s)) {
        result->s = s; 
    } else {
        // success
        const char *etag;

        oss_fill_read_response_header(resp, &resp_headers);
        
        etag = apr_table_get(resp_headers, "Etag");
        if (etag) {
            aos_str_set(&result->etag, apr_pstrdup(options->pool, etag));
        }

        result->crc64 = resp->crc64;
        result->s = s;
    }
    apr_file_close(fb->file);

    return;
}

void *APR_THREAD_FUNC download_part_thread(apr_thread_t *thd, void *data)
{
    apr_queue_t *task_queue = (apr_queue_t *)data;

    while (1) {
        apr_status_t status;
        oss_thread_params_t *params;

        status = apr_queue_trypop(task_queue, (void **)&params);
        if (status != APR_SUCCESS)
            break;

        if (apr_atomic_read32(params->failed) > 0) {
            // skip unstarted parts if parts failure happened
            apr_queue_push(params->task_result_queue, NULL);
        } else {
            download_part(&params->options, 
                    params->bucket, params->object,
                    params->part, params->filepath,
                    params->result);

            apr_queue_push(params->task_result_queue, params->result);
        }
    }
    return NULL;
}

aos_status_t *oss_resumable_download_file_internal(oss_request_options_t *options,
                                                   aos_string_t *bucket, 
                                                   aos_string_t *object, 
                                                   aos_string_t *filepath,                           
                                                   aos_table_t *headers,
                                                   aos_table_t *params,
                                                   int32_t thread_num,
                                                   int64_t part_size,
                                                   aos_string_t *checkpoint_path,
                                                   oss_progress_callback progress_callback,
                                                   aos_table_t **resp_headers)
{
    aos_status_t *s = NULL;
    int need_init_download = AOS_TRUE;
    oss_checkpoint_t *checkpoint = NULL;

    aos_table_t *head_resp_headers = NULL;
    aos_string_t tmp_filename;
    
    int i = 0;
    int part_num = 0;
    int64_t object_size = 0;
    const char *object_size_str = NULL;
    const char *object_last_modified = NULL;
    const char *object_etag = NULL;
    const char *crc64_str = NULL;
    oss_checkpoint_part_t *parts;
    oss_part_task_result_t *results;
    oss_part_task_result_t *task_res;
    oss_thread_params_t *thd_params;
    apr_uint32_t failed = 0;
    apr_queue_t *failed_parts;
    apr_queue_t *completed_parts;
    apr_queue_t *task_queue;
    apr_queue_t *task_result_queue;
    int64_t consume_bytes = 0;
    aos_file_buf_t *fb = NULL;
    apr_thread_t **thd_ids = NULL;
    int rv = AOSE_OK;
    
    oss_get_temporary_file_name(options->pool, filepath, &tmp_filename);

    // head object info
    s = oss_head_object(options, bucket, object, headers, &head_resp_headers);
    *resp_headers = head_resp_headers;
    if (!aos_status_is_ok(s))
        return s;

    object_last_modified = apr_table_get(head_resp_headers, "Last-Modified");
    object_etag = apr_table_get(head_resp_headers, "ETag");
    object_size_str = apr_table_get(head_resp_headers, OSS_CONTENT_LENGTH);
    crc64_str = apr_table_get(head_resp_headers, OSS_HASH_CRC64_ECMA);

    if (!object_last_modified || !object_etag || !object_size_str) {
        // Invalid http response header 
        s = aos_status_create(options->pool);
        aos_status_set(s, AOSE_INTERNAL_ERROR, AOS_SERVER_ERROR_CODE, "Unexpected response header");
        return s;
    }
    object_size = aos_atoi64(object_size_str);
    
    // ensure part_num will not exceed OSS_MAX_PART_NUM
    if (part_size * OSS_MAX_PART_NUM < object_size) {
        part_size = (object_size + OSS_MAX_PART_NUM - 1) / OSS_MAX_PART_NUM;

        aos_warn_log("Part number larger than max limit, "
                "part size Changed to:%" APR_INT64_T_FMT "\n",
                part_size);
    }

    need_init_download = AOS_TRUE;
    checkpoint = oss_create_checkpoint_content(options->pool);
    if (checkpoint_path) {
        do {
            apr_finfo_t tmp_finfo;

            if (!oss_does_file_exist(checkpoint_path, options->pool))
                break;

            if (AOSE_OK != oss_load_checkpoint(options->pool, checkpoint_path, checkpoint))
                break;
            if (!oss_is_download_checkpoint_valid(options->pool, checkpoint, object->data, 
                        object_size, object_last_modified, object_etag))
                break;

            if (!oss_does_file_exist(&tmp_filename, options->pool))
                break;
            
            if (apr_stat(&tmp_finfo, tmp_filename.data, APR_FINFO_SIZE, 
                        options->pool) != APR_SUCCESS || 
                    object_size != tmp_finfo.size)
                break;
            need_init_download = AOS_FALSE;
        } while (0);
    }

    if (need_init_download) {
        aos_debug_log("need init download\n");

        // build checkpoint
        oss_build_download_checkpoint(options->pool, checkpoint, filepath, object->data, 
                object_size, object_last_modified, object_etag, part_size);
    }

    if (checkpoint_path) {
        if ((rv = oss_open_checkpoint_file(options->pool, checkpoint_path, checkpoint)) != APR_SUCCESS) {
            s = aos_status_create(options->pool);
            aos_status_set(s, rv, AOS_OPEN_FILE_ERROR_CODE, NULL);
            return s;
        }
    }
    
    // Open and truncate the tmp file.
    fb = aos_create_file_buf(options->pool);
    if ((rv = aos_open_file_for_write_notrunc(options->pool, tmp_filename.data, fb)) != AOSE_OK) {
        if (checkpoint->thefile)
            apr_file_close(checkpoint->thefile);

        aos_error_log("Open write file fail, filename:%s\n", tmp_filename.data);
        aos_file_error_status_set(s, rv);
        return s;
    }
    apr_file_trunc(fb->file, object_size);
    apr_file_close(fb->file);

    parts = (oss_checkpoint_part_t *)aos_palloc(options->pool, sizeof(*parts) * checkpoint->part_num);
    oss_get_checkpoint_todo_parts(checkpoint, &part_num, parts);
    results = (oss_part_task_result_t *)aos_palloc(options->pool, sizeof(*results) * part_num);
    thd_params = (oss_thread_params_t *)aos_palloc(options->pool, sizeof(*thd_params) * part_num);
    oss_build_thread_params(thd_params, part_num, options->pool, options, bucket, object, &tmp_filename, NULL, parts, results);
    thd_ids = (apr_thread_t **)aos_palloc(options->pool, sizeof(*thd_ids) * thread_num);

    aos_debug_log("object_size: %" APR_INT64_T_FMT ", total parts: %d, parts to download: %d\n", 
            object_size, checkpoint->part_num, part_num);

    if ((rv = apr_queue_create(&failed_parts, part_num, options->pool)) != APR_SUCCESS || 
            (rv = apr_queue_create(&completed_parts, part_num, options->pool)) != APR_SUCCESS ||
            (rv = apr_queue_create(&task_queue, part_num, options->pool)) != APR_SUCCESS ||
            (rv = apr_queue_create(&task_result_queue, part_num, options->pool)) != APR_SUCCESS) {
        if (checkpoint->thefile)
            apr_file_close(checkpoint->thefile);

        s = aos_status_create(options->pool);
        aos_status_set(s, rv, AOS_CREATE_QUEUE_ERROR_CODE, NULL); 
        return s;
    }
    // launch
    for (i = 0; i < part_num; i++) {
        thd_params[i].failed = &failed;
        thd_params[i].task_result_queue = task_result_queue;

        apr_queue_push(task_queue, &thd_params[i]);
    }

    // download parts    
    for (i = 0; i < thread_num; i++) {
        apr_thread_create(&thd_ids[i], NULL, download_part_thread, (void *)task_queue, options->pool);
    }

    // wait until all tasks exit
    for (i = 0 ; i < part_num; i++) {
        rv = apr_queue_pop(task_result_queue, (void **)&task_res);
        if (task_res && aos_status_is_ok(task_res->s) &&
                !strcasecmp(object_etag, task_res->etag.data)) {
            // completed part
            oss_update_checkpoint(options->pool, checkpoint, task_res->part->index, 
                    &task_res->etag, task_res->crc64);
            if (checkpoint->thefile) {
                if ((rv = oss_dump_checkpoint(options->pool, checkpoint)) != AOSE_OK) {
                    aos_warn_log("failed to persist checkpoint file %s: %d\n", 
                            checkpoint_path->data, rv);
                }
            }
            apr_queue_push(completed_parts, task_res);

            if (progress_callback) {
                consume_bytes += task_res->part->size;
                progress_callback(consume_bytes, object_size);
            }
        } else if (task_res) {
            // failed parts
            apr_atomic_inc32(&failed);
            apr_queue_push(failed_parts, task_res);
        } else {
            // skipped parts
        }
    }
    if (checkpoint->thefile) {
        apr_file_close(checkpoint->thefile);
        checkpoint->thefile = NULL;
    }

    aos_debug_log("completed: %u, failed: %u, skipped: %u\n", 
            apr_queue_size(completed_parts), apr_queue_size(failed_parts), 
            part_num - apr_queue_size(completed_parts) - apr_queue_size(failed_parts));

    if (apr_atomic_read32(&failed) > 0) {
        // any parts failure
        apr_queue_pop(failed_parts, (void **)&task_res);
        s = aos_status_dup(options->pool, task_res->s);
    } else {
        // complete download for all parts
        rv = AOSE_OK;

        if (is_enable_crc(options) && crc64_str) {
            uint64_t iter_crc64 = 0;
            for (i = 0; i < checkpoint->part_num; i++) {
                iter_crc64 = aos_crc64_combine(iter_crc64, checkpoint->parts[i].crc64,
                        checkpoint->parts[i].size);
            }
            if ((rv = oss_check_crc_consistent(iter_crc64, head_resp_headers, s)) != AOSE_OK) {
                if (checkpoint_path) {
                    // checkpoint file should be removed here, otherwise retry downloads will 
                    // always be skipped and failed here in crc64 check
                    if (apr_file_remove(checkpoint_path->data, options->pool) != APR_SUCCESS) {
                        aos_warn_log("Failed to remove checkpoint file %s\n", 
                                checkpoint_path->data);
                    }
                }
                apr_file_remove(tmp_filename.data, options->pool);
            }
        }

        if (rv == AOSE_OK) {
            if (apr_file_rename(tmp_filename.data, filepath->data, options->pool) != APR_SUCCESS) {
                s = aos_status_create(options->pool);
                aos_status_set(s, rv, AOS_RENAME_FILE_ERROR_CODE, NULL);
            } else {
                if (checkpoint_path) {
                    apr_file_remove(checkpoint_path->data, options->pool);
                }
            }
        }
    }

    for (i = 0; i < thread_num; i++) {
        apr_status_t retval;
        apr_thread_join(&retval, thd_ids[i]);
    }
    oss_destroy_thread_pool(thd_params, part_num);

    return s;
}

aos_status_t *oss_resumable_download_file(oss_request_options_t *options,
                                        aos_string_t *bucket, 
                                        aos_string_t *object, 
                                        aos_string_t *filepath,                           
                                        aos_table_t *headers,
                                        aos_table_t *params,
                                        oss_resumable_clt_params_t *clt_params, 
                                        oss_progress_callback progress_callback,
                                        aos_table_t **resp_headers) 
{
    int32_t thread_num = 0;
    int64_t part_size = 0;
    aos_string_t checkpoint_path;
    aos_pool_t *sub_pool;
    aos_status_t *s;

    thread_num = oss_get_thread_num(clt_params);
    aos_pool_create(&sub_pool, options->pool);
    
    part_size = clt_params->part_size;
    
    if (NULL != clt_params && clt_params->enable_checkpoint) {
        oss_get_download_checkpoint_path(clt_params, filepath, sub_pool, &checkpoint_path);
        s = oss_resumable_download_file_internal(options, bucket, object, filepath, headers, params, thread_num, 
            part_size, &checkpoint_path, progress_callback, resp_headers);
    } else {
        s = oss_resumable_download_file_internal(options, bucket, object, filepath, headers, params, thread_num, 
            part_size, NULL, progress_callback, resp_headers);
    }

    aos_pool_destroy(sub_pool);
    return s;
}
