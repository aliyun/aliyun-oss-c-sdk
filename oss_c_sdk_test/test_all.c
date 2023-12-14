#include "CuTest.h"
#include "aos_log.h"
#include "aos_http_io.h"
#include "oss_config.h"
#include "apr_env.h"
#include "aos_string.h"

extern CuSuite *test_oss_bucket();
extern CuSuite *test_oss_object();
extern CuSuite *test_oss_multipart();
extern CuSuite *test_oss_live();
extern CuSuite *test_oss_image();
extern CuSuite *test_oss_progress();
extern CuSuite *test_oss_callback();
extern CuSuite *test_oss_crc();
extern CuSuite *test_aos();
extern CuSuite *test_oss_proxy();
extern CuSuite *test_oss_resumable();
extern CuSuite *test_oss_select_object();
extern CuSuite *test_oss_object_tagging();
extern CuSuite *test_oss_xml();
extern CuSuite *test_oss_https();
extern CuSuite* test_oss_sign();
extern void set_test_bucket_prefix(const char*prefix);
extern void clean_bucket_by_prefix(const char* prefix);

static const struct testlist {
    const char *testname;
    CuSuite *(*func)();
} tests[] = {
    {"test_oss_bucket", test_oss_bucket},
    {"test_oss_object", test_oss_object},
    {"test_oss_multipart", test_oss_multipart},
    {"test_oss_live", test_oss_live},
    {"test_oss_image", test_oss_image},
    {"test_oss_progress", test_oss_progress},
    {"test_oss_callback", test_oss_callback},
    {"test_oss_crc", test_oss_crc},
    {"test_oss_proxy", test_oss_proxy},
    {"test_oss_resumable", test_oss_resumable},
    {"test_aos", test_aos},
    {"test_oss_select_object", test_oss_select_object },
    {"test_oss_object_tagging", test_oss_object_tagging },
    {"test_oss_xml", test_oss_xml },
    {"test_oss_https", test_oss_https },
    {"test_oss_sign", test_oss_sign },
    {"LastTest", NULL}
};

static char *CFG_FILE_PATH = "";
static char BUCKET_PREIFX[64];

int has_cfg_info()
{
    return (TEST_OSS_ENDPOINT && TEST_ACCESS_KEY_ID && TEST_ACCESS_KEY_SECRET);
}

void load_cfg_from_env()
{
    char *str = NULL;
    apr_env_get(&str, "OSS_TEST_ENDPOINT", aos_global_pool);
    if (str) {
        TEST_OSS_ENDPOINT = str;
    }

    str = NULL;
    apr_env_get(&str, "OSS_TEST_ACCESS_KEY_ID", aos_global_pool);
    if (str) {
        TEST_ACCESS_KEY_ID = str;
    }

    str = NULL;
    apr_env_get(&str, "OSS_TEST_ACCESS_KEY_SECRET", aos_global_pool);
    if (str) {
        TEST_ACCESS_KEY_SECRET = str;
    }

    str = NULL;
    apr_env_get(&str, "OSS_TEST_CALLBACK_URL", aos_global_pool);
    if (str) {
        TEST_CALLBACK_URL = str;
    }
}

void load_cfg_from_file()
{
    apr_file_t *file;
    apr_status_t s;
    char buffer[256];
    char *ptr;

    if (has_cfg_info())
        return;

    s = apr_file_open(&file, CFG_FILE_PATH, APR_READ, APR_UREAD | APR_GREAD, aos_global_pool);

    if (s != APR_SUCCESS)
        return;

    while (apr_file_gets(buffer, 256, file) == APR_SUCCESS) {
        aos_string_t str;
        ptr = strchr(buffer, '=');
        if (!ptr) {
            continue;
        }

        if (!strncmp(buffer, "AccessKeyId", 11)) {
            aos_str_set(&str, ptr + 1);
            aos_trip_space_and_cntrl(&str);
            aos_unquote_str(&str);
            TEST_ACCESS_KEY_ID = aos_pstrdup(aos_global_pool, &str);
        }
        else if (!strncmp(buffer, "AccessKeySecret", 15)) {
            aos_str_set(&str, ptr + 1);
            aos_trip_space_and_cntrl(&str);
            aos_unquote_str(&str);
            TEST_ACCESS_KEY_SECRET = aos_pstrdup(aos_global_pool, &str);
        }
        else if (!strncmp(buffer, "Endpoint", 8)) {
            aos_str_set(&str, ptr + 1);
            aos_trip_space_and_cntrl(&str);
            aos_unquote_str(&str);
            TEST_OSS_ENDPOINT = aos_pstrdup(aos_global_pool, &str);
        }
        else if (!strncmp(buffer, "CallbackServer", 14)) {
            aos_str_set(&str, ptr + 1);
            aos_trip_space_and_cntrl(&str);
            aos_unquote_str(&str);
            TEST_CALLBACK_URL = aos_pstrdup(aos_global_pool, &str);
        }
    }
    apr_file_close(file);
}

int init_test_env()
{
    /*Get from env*/
    load_cfg_from_env();

    /*Get from file*/
    load_cfg_from_file();

    /*gen bucket prefix*/
    sprintf(BUCKET_PREIFX, "test-c-sdk-%"APR_TIME_T_FMT, apr_time_now()/1000);
    set_test_bucket_prefix(BUCKET_PREIFX);

    return has_cfg_info();
}

void deinit_test_env()
{
    clean_bucket_by_prefix(BUCKET_PREIFX);
}

int run_all_tests(int argc, char *argv[])
{
    int i;
    int exit_code;
    int list_provided = 0;
    CuSuite* suite = NULL;
    int j;
    int found;
    CuSuite *st = NULL;
    CuString *output = NULL;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) {
            continue;
        }
        if (!strcmp(argv[i], "-l")) {
            for (i = 0; tests[i].func != NULL; i++) {
                printf("%s\n", tests[i].testname);
            }
            exit(0);
        }
        if (!strcmp(argv[i], "-oss_cfg")) {
            aos_string_t str;
            aos_str_set(&str, argv[i + 1]);
            aos_strip_space(&str);
            CFG_FILE_PATH = aos_pstrdup(aos_global_pool, &str);
            i++;
            continue;
        }
        if (!strcmp(argv[i], "-d")) {
            aos_log_set_level(AOS_LOG_DEBUG);
            continue;
        }
        if (argv[i][0] == '-') {
            fprintf(stderr, "invalid option: `%s'\n", argv[i]);
            exit(1);
        }
        list_provided = 1;
    }

    if (!init_test_env()) {
        fprintf(stderr, "One of AK, SK or Endpoint is not configured.\n");
        exit(1);
    }

    suite = CuSuiteNew();

    if (!list_provided) {
        /* add everything */
        for (i = 0; tests[i].func != NULL; i++) {
            st = tests[i].func();
            CuSuiteAddSuite(suite, st);
            CuSuiteFree(st);
        }
    } else {
        /* add only the tests listed */
        for (i = 1; i < argc; i++) {
            found = 0;
            if (argv[i][0] == '-') {
                if (!strcmp(argv[i], "-oss_cfg")) {
                    i++;
                }
                continue;
            }
            for (j = 0; tests[j].func != NULL; j++) {
                if (!strcmp(argv[i], tests[j].testname)) {
                    CuSuiteAddSuite(suite, tests[j].func());
                    found = 1;
                }
            }
            if (!found) {
                fprintf(stderr, "invalid test name: `%s'\n", argv[i]);
                exit(1);
            }
        }
    }

    output = CuStringNew();
    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);

    exit_code = suite->failCount > 0 ? 1 : 0;

    CuSuiteFreeDeep(suite);
    CuStringFree(output);

    deinit_test_env();

    return exit_code;
}

int main(int argc, char *argv[])
{
    int exit_code = -1;

    if (aos_http_io_initialize(NULL, 0) != AOSE_OK) {
        exit(1);
    }

    aos_log_set_print(aos_log_print_default);
    aos_log_set_format(aos_log_format_default);
    aos_log_set_level(AOS_LOG_OFF);
    exit_code = run_all_tests(argc, argv);

    //aos_http_io_deinitialize last
    aos_http_io_deinitialize();

    return exit_code;
}
