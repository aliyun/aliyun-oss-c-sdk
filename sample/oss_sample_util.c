#include <sys/stat.h>
#include "oss_config.h"
#include "oss_api.h"
#include "oss_sample_util.h"

void make_rand_string(aos_pool_t *p, int len, aos_string_t *data)
{
    char *str = NULL;
    int i = 0;
    str = (char *)aos_palloc(p, len + 1);
    for ( ; i < len; i++) {
        str[i] = rand() % 128;
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

    for (; i < count; ++i) {
        len = random() % 4096;
        b = make_random_buf(p, len);
        aos_list_add_tail(&b->node, bc);
    }
}

void init_sample_config(oss_config_t *config, int is_oss_domain)
{
    aos_str_set(&config->host, OSS_HOST);
    aos_str_set(&config->id, ACCESS_KEY_ID);
    aos_str_set(&config->key, ACCESS_KEY_SECRET);
    config->is_oss_domain = is_oss_domain;
}

void init_sample_request_options(oss_request_options_t *options, int is_oss_domain)
{
    options->config = oss_config_create(options->pool);
    init_sample_config(options->config, is_oss_domain);
    options->ctl = aos_http_controller_create(options->pool, 0);
}

int64_t get_file_size(const char *file_path)
{
    int64_t filesize = -1;
    struct stat statbuff;

    if(stat(file_path, &statbuff) < 0){
        return filesize;
    } else {
        filesize = statbuff.st_size;
    }

    return filesize;
}
