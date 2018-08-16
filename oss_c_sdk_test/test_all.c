#include "CuTest.h"
#include "aos_log.h"
#include "aos_http_io.h"
#include "oss_config.h"
#include "apr_env.h"

extern CuSuite *test_xml();
extern CuSuite *test_util();
extern CuSuite *test_file();
extern CuSuite *test_transport();
extern CuSuite *test_oss_bucket();
extern CuSuite *test_oss_object();
extern CuSuite *test_oss_multipart();
extern CuSuite *test_oss_live();
extern CuSuite *test_oss_image();
extern CuSuite *test_oss_progress();
extern CuSuite *test_oss_callback();
extern CuSuite *test_oss_util();
extern CuSuite *test_oss_xml();
extern CuSuite *test_oss_crc();
extern CuSuite *test_aos();
extern CuSuite *test_oss_proxy();
extern CuSuite *test_oss_resumable();

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
    {"LastTest", NULL}
};

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
        if (argv[i][0] == '-') {
            fprintf(stderr, "invalid option: `%s'\n", argv[i]);
            exit(1);
        }
        list_provided = 1;
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

    return exit_code;
}

int main(int argc, char *argv[])
{
    int exit_code = -1;
    char *str;

    if (aos_http_io_initialize(NULL, 0) != AOSE_OK) {
        exit(1);
    }

    if (TEST_OSS_ENDPOINT == NULL) {
        str = NULL;
        apr_env_get(&str, "OSS_TEST_ENDPOINT", aos_global_pool);
        TEST_OSS_ENDPOINT = str;
    }

    if (TEST_ACCESS_KEY_ID == NULL) {
        str = NULL;
        apr_env_get(&str, "OSS_TEST_ACCESS_KEY_ID", aos_global_pool);
        TEST_ACCESS_KEY_ID = str;
    }

    if (TEST_ACCESS_KEY_SECRET == NULL) {
        str = NULL;
        apr_env_get(&str, "OSS_TEST_ACCESS_KEY_SECRET", aos_global_pool);
        TEST_ACCESS_KEY_SECRET = str;
    }

    if (TEST_BUCKET_NAME == NULL) {
        str = NULL;
        apr_env_get(&str, "OSS_TEST_BUCKET", aos_global_pool);
        TEST_BUCKET_NAME = str;
    }
    aos_log_set_level(AOS_LOG_OFF);
    exit_code = run_all_tests(argc, argv);

    //aos_http_io_deinitialize last
    aos_http_io_deinitialize();

    return exit_code;
}
