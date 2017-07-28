#ifndef LIBOSS_RESUMABLE_H
#define LIBOSS_RESUMABLE_H

#include "aos_define.h"
#include "aos_string.h"
#include "aos_status.h"
#include "apr_atomic.h"
#include "apr_queue.h"
#include "apr_thread_pool.h"
#include "oss_define.h"

AOS_CPP_START

#define OSS_CP_UPLOAD   1
#define OSS_CP_DOWNLOAD 2

typedef struct {
    int32_t index;  // the index of part, start from 0
    int64_t offset; // the offset point of part
    int64_t size;   // the size of part
    int completed;  // AOS_TRUE completed, AOS_FALSE uncompleted
    aos_string_t etag; // the etag of part, for upload
    uint64_t crc64;
} oss_checkpoint_part_t;

typedef struct {
    aos_string_t md5;      // the md5 of checkout content
    int cp_type;           // 1 upload, 2 download
    apr_file_t *thefile;   // the handle of checkpoint file

    aos_string_t file_path;        // local file path 
    int64_t    file_size;          // local file size, for upload
    apr_time_t file_last_modified; // local file last modified time, for upload
    aos_string_t file_md5;         // the md5 of the local file content, for upload, reserved

    aos_string_t object_name;          // object name
    int64_t object_size;               // object size, for download
    aos_string_t object_last_modified; // object last modified time, for download
    aos_string_t object_etag;          // object etag, for download

    aos_string_t upload_id;  // upload id

    int  part_num;                 // the total number of parts
    int64_t part_size;             // the part size, byte
    oss_checkpoint_part_t *parts;  // the parts of local or object, from 0
} oss_checkpoint_t;

typedef struct {
    oss_checkpoint_part_t *part;
    aos_status_t *s;
    aos_string_t etag; 
    uint64_t crc64;
} oss_part_task_result_t;

typedef struct {
    oss_request_options_t options;
    aos_string_t *bucket;
    aos_string_t *object; 
    aos_string_t *upload_id;
    aos_string_t *filepath;
    oss_checkpoint_part_t *part;
    oss_part_task_result_t *result;

    apr_uint32_t *launched;        // the number of launched part tasks, use atomic
    apr_uint32_t *failed;          // the number of failed part tasks, use atomic
    apr_uint32_t *completed;       // the number of completed part tasks, use atomic
    apr_queue_t  *failed_parts;    // the queue of failed parts tasks, thread safe
    apr_queue_t  *completed_parts; // the queue of completed parts tasks, thread safe
    apr_queue_t  *task_result_queue;
} oss_thread_params_t;

int32_t oss_get_thread_num(oss_resumable_clt_params_t *clt_params);

void oss_get_upload_checkpoint_path(oss_resumable_clt_params_t *clt_params, const aos_string_t *filepath, 
                             aos_pool_t *pool, aos_string_t *checkpoint_path);

void oss_get_download_checkpoint_path(oss_resumable_clt_params_t *clt_params, const aos_string_t *filepath, 
                             aos_pool_t *pool, aos_string_t *checkpoint_path);

int oss_get_file_info(const aos_string_t *filepath, aos_pool_t *pool, apr_finfo_t *finfo);

int oss_does_file_exist(const aos_string_t *filepath, aos_pool_t *pool); 

int oss_open_checkpoint_file(aos_pool_t *pool,  aos_string_t *checkpoint_path, oss_checkpoint_t *checkpoint);

int oss_open_checkpoint_file(aos_pool_t *pool,  aos_string_t *checkpoint_path, oss_checkpoint_t *checkpoint); 

int oss_get_part_num(int64_t file_size, int64_t part_size);

void oss_build_parts(int64_t file_size, int64_t part_size, oss_checkpoint_part_t *parts);

void oss_build_thread_params(oss_thread_params_t *thr_params, int part_num, 
                             aos_pool_t *parent_pool, oss_request_options_t *options, 
                             aos_string_t *bucket, aos_string_t *object, aos_string_t *filepath,
                             aos_string_t *upload_id, oss_checkpoint_part_t *parts,
                             oss_part_task_result_t *result);

void oss_destroy_thread_pool(oss_thread_params_t *thr_params, int part_num);

void oss_set_task_tracker(oss_thread_params_t *thr_params, int part_num, 
                          apr_uint32_t *launched, apr_uint32_t *failed, apr_uint32_t *completed,
                          apr_queue_t *failed_parts, apr_queue_t *completed_parts);

int oss_verify_checkpoint_md5(aos_pool_t *pool, const oss_checkpoint_t *checkpoint);

void oss_build_upload_checkpoint(aos_pool_t *pool, oss_checkpoint_t *checkpoint, aos_string_t *file_path, 
                                 apr_finfo_t *finfo, aos_string_t *upload_id, int64_t part_size);

void oss_build_download_checkpoint(aos_pool_t *pool, oss_checkpoint_t *checkpoint, aos_string_t *file_path, 
        const char *object_name, int64_t object_size, const char *object_last_modified, 
        const char *object_etag, int64_t part_size);

int oss_dump_checkpoint(aos_pool_t *pool, const oss_checkpoint_t *checkpoint);

int oss_load_checkpoint(aos_pool_t *pool, const aos_string_t *filepath, oss_checkpoint_t *checkpoint);

int oss_is_upload_checkpoint_valid(aos_pool_t *pool, oss_checkpoint_t *checkpoint, apr_finfo_t *finfo);

void oss_update_checkpoint(aos_pool_t *pool, oss_checkpoint_t *checkpoint, int32_t part_index, 
        aos_string_t *etag, uint64_t crc64);

void oss_get_checkpoint_todo_parts(oss_checkpoint_t *checkpoint, int *part_num, oss_checkpoint_part_t *parts);

void *APR_THREAD_FUNC upload_part(apr_thread_t *thd, void *data);

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
                                                   aos_list_t *resp_body);

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
                                                aos_list_t *resp_body);


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
                                                   aos_table_t **resp_headers);

AOS_CPP_END

#endif
