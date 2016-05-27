#include "CuTest.h"
#include "aos_log.h"
#include "aos_http_io.h"
#include "oss_config.h"

extern CuSuite *test_xml();
extern CuSuite *test_util();
extern CuSuite *test_file();
extern CuSuite *test_transport();
extern CuSuite *test_oss_bucket();
extern CuSuite *test_oss_object();
extern CuSuite *test_oss_multipart();
extern CuSuite *test_oss_util();
extern CuSuite *test_oss_xml();
extern CuSuite *test_aos();

static const struct testlist {
    const char *testname;
    CuSuite *(*func)();
} tests[] = {
    {"test_oss_bucket", test_oss_bucket},
    {"test_oss_object", test_oss_object},
    {"test_oss_multipart", test_oss_multipart},
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

    TEST_OSS_ENDPOINT = TEST_OSS_ENDPOINT != NULL ? 
                        TEST_OSS_ENDPOINT : getenv("OSS_TEST_ENDPOINT");
    TEST_ACCESS_KEY_ID = TEST_ACCESS_KEY_ID != NULL ? 
                         TEST_ACCESS_KEY_ID : getenv("OSS_TEST_ACCESS_KEY_ID");
    TEST_ACCESS_KEY_SECRET = TEST_ACCESS_KEY_SECRET != NULL ?
                             TEST_ACCESS_KEY_SECRET : getenv("OSS_TEST_ACCESS_KEY_SECRET");
    TEST_BUCKET_NAME = TEST_BUCKET_NAME != NULL ?
                       TEST_BUCKET_NAME : getenv("OSS_TEST_BUCKET");


    if (aos_http_io_initialize(NULL, 0) != AOSE_OK) {
        exit(1);
    }

    aos_log_set_level(AOS_LOG_OFF);
    exit_code = run_all_tests(argc, argv);

    //aos_http_io_deinitialize last
    aos_http_io_deinitialize();

    return exit_code;
}
