#ifndef OSS_SAMPLE_UTIL_H
#define OSS_SAMPLE_UTIL_H

#include "aos_http_io.h"
#include "aos_string.h"
#include "aos_transport.h"
#include "oss_define.h"

OSS_CPP_START

void make_rand_string(aos_pool_t *p, int len, aos_string_t *data);

aos_buf_t *make_random_buf(aos_pool_t *p, int len);

void make_random_body(aos_pool_t *p, int count, aos_list_t *bc);

void init_sample_config(oss_config_t *config, int is_cname);

void init_sample_request_options(oss_request_options_t *options, int is_cname);

int64_t get_file_size(const char *file_path);

void percentage(int64_t consumed_bytes, int64_t total_bytes);

OSS_CPP_END

#endif
